/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QDebug>
#include <QVBoxLayout>
#include <qmath.h>
#include <qendian.h>

#include "audio.h"
#define EMBEDDEDAUDIO
#include "RTcmix_API.h"
#include "sndfile.h"
#include "utils.h"

const float DefaultSamplingRate = 44100.0;
const int DefaultNumInChannels = 0;
const int DefaultNumOutChannels = 2;
const int DefaultBlockSize = 512;
const int DefaultBusCount = 32;

// FIXME: Might want to realloc this if numchans changes
const int ringBufferNumSamps = 1024 * 32;

Audio::Audio()
    : portAudioInitialized(false)
    , rtcmixInitialized(false)
    , stream(NULL)
    , requestedInputDeviceID(0)
    , requestedOutputDeviceID(0)
    , requestedSamplingRate(DefaultSamplingRate)
    , requestedNumInChannels(DefaultNumInChannels)
    , requestedNumOutChannels(DefaultNumOutChannels)
    , requestedBlockSize(DefaultBlockSize)
    , busCount(DefaultBusCount)
    , recording(false)
{
    int result = initializeAudio();
    if (result == 0) {
        result = initializeRTcmix();
        if (result == 0)
            startAudio();
    }

    recordBuffer = (float *) calloc(ringBufferNumSamps, sizeof(float));
    PaUtil_InitializeRingBuffer(&recordRingBuffer, sizeof(float), ringBufferNumSamps, recordBuffer);
    transferBuffer = (float *) calloc(ringBufferNumSamps, sizeof(float));
}

Audio::~Audio()
{
    if (portAudioInitialized) {
        PaError err = Pa_CloseStream(stream);
        if (err != paNoError) {
            qWarning("Pa_CloseStream error: `%s'", Pa_GetErrorText(err));
        }
        err = Pa_Terminate();
        if (err != paNoError) {
            qWarning("Pa_Terminate error: `%s'", Pa_GetErrorText(err));
        }
    }
    if (rtcmixInitialized)
        RTcmix_destroy();
}

int Audio::initializeAudio()
{
    // Initialize
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        qWarning("Pa_Initialize error: `%s'", Pa_GetErrorText(err));    // FIXME: pop alert instead
        return -1;
    }
    portAudioInitialized = true;

#ifdef NOTYET // determine whether what we want is available; should be in loop?
    PaStreamParameters inputParameters, outputParameters;
    bzero(&inputParameters, sizeof(inputParameters));
    inputParameters.channelCount = requestedNumInChannels;
    inputParameters.device = requestedInputDeviceID;
    inputParameters.sampleFormat = paFloat32;
    bzero(&outputParameters, sizeof(outputParameters));
    outputParameters.channelCount = requestedNumOutChannels;
    outputParameters.device = requestedOutputDeviceID;
    outputParameters.sampleFormat = paFloat32;
    err = Pa_IsFormatSupported(inputParameters, outputParameters, requestedSamplingRate);
    if (err != paFormatIsSupported) {}
    // then you would open stream. See http://portaudio.com/docs/v19-doxydocs/querying_devices.html
#endif

    // TODO: pick requested values up from prefs, incl. device choice, using Pa_OpenStream instead of Pa_OpenDefaultStream
    err = Pa_OpenDefaultStream(&stream,
                               requestedNumInChannels,
                               requestedNumOutChannels,
                               paFloat32,
                               requestedSamplingRate,
                               requestedBlockSize,
                               &paCallback,
                               this);
    if (err != paNoError) {
        qWarning("Pa_OpenDefaultStream error: `%s'", Pa_GetErrorText(err));    // FIXME: pop alert instead
        return -1;
    }

    // FIXME: should be checking to see what the actual srate and blocksize are, etc.

    qDebug("Audio initialized");
    return 0;
}

int Audio::startAudio()
{
    if (portAudioInitialized && stream != NULL) {
        PaError err = Pa_StartStream(stream);
        if (err != paNoError) {
            qWarning("Pa_StartStream error: `%s'", Pa_GetErrorText(err));    // FIXME: pop alert instead
            return -1;
        }
        qDebug("Pa_StartStream returned noerr");
    }
    return 0;
}

int Audio::stopAudio()
{
    if (portAudioInitialized && stream != NULL) {
        PaError err = Pa_StopStream(stream);
        if (err != paNoError) {
            qWarning("Pa_StopStream error: `%s'", Pa_GetErrorText(err));    // FIXME: pop alert instead
            return -1;
        }
    }
    return 0;
}

int Audio::memberCallback(
            const void *input,
            void *output,
            unsigned long frameCount,
            const PaStreamCallbackTimeInfo *timeInfo,
            PaStreamCallbackFlags statusFlags)
{
    (void) input;
    (void) timeInfo;
    (void) statusFlags;

#ifdef DEBUG_IN_CALLBACK
    callbackCount++;
    if ((callbackCount % 100) == 0)
        qDebug("time: %f", timeInfo->currentTime);
    if ((statusFlags & paOutputUnderflow)) {
        outputUnderflowCount++;
        if ((outputUnderflowCount % 20) == 0)
            qDebug("Five OUTPUT UNDERFLOWs");
    }
#endif

    if (recording) {
        int failCount = 0;
        float *ptr = (float *) output;
        int inSampCount = int(frameCount * requestedNumOutChannels);
        while (inSampCount) {
            int sampsAvail = PaUtil_GetRingBufferWriteAvailable(&recordRingBuffer);
            int writeCount = qMin(inSampCount, sampsAvail);
            if (writeCount > 0) {
                int sampsWritten = PaUtil_WriteRingBuffer(&recordRingBuffer, ptr, writeCount);
                inSampCount -= sampsWritten;
//qDebug("audio callback: sampsAvail=%d, sampsWritten=%d", sampsAvail, sampsWritten);
                ptr += sampsWritten;
            }
            else
                failCount++;
            if (failCount > 0) {
qDebug("audio callback: ring buffer write not available (1 time)");
                failCount = 0;
//                break;
            }
        }
//qDebug("audio callback: loop done (inSampCount=%d)", inSampCount);
    }

    int result = RTcmix_runAudio(NULL /*input*/, output, frameCount);
    (void) result;
#ifdef DEBUG_IN_CALLBACK
    float *p = (float *)output;
    bool nonzero = false;
    for (unsigned long i = 0; i < frameCount; i++) {
        if (fabs(*p++) > 0.0) {
            nonzero = true;
            break;
        }
    }
    if ((callbackCount % 100) == 0) {
        if (nonzero)
            qDebug("has sound");
        else
            qDebug("20 blocks of silence");
    }
//    qDebug("RTcmix_runAudio called (result=%d, frameCount=%ld, output=%p)", result, frameCount, output);
#endif

    return paContinue;
}

int Audio::initializeRTcmix()
{
    // initialize RTcmix
    int status = RTcmix_init();
    if (status != 0) {
        qWarning("RTcmix_init returned error (%d)", status);
        return -1;
    }
    rtcmixInitialized = true;
    // need to destroy rtcmix in case we're here after a config change -- or reconfig it with RTcmix_resetAudio

    status = RTcmix_setAudioBufferFormat(AudioFormat_32BitFloat_Normalized, requestedNumOutChannels);
    if (status != 0) {
        qWarning("RTcmix_setAudioBufferFormat returned error (%d)", status);
        return -1;
    }

    // TODO: handle input as well as output
    int recording = 0;
    status = RTcmix_setparams(requestedSamplingRate, requestedNumOutChannels, requestedBlockSize, recording, DefaultBusCount);
    if (status != 0) {
        qWarning("RTcmix_setparams returned error (%d)", status);
        return -1;
    }

    qDebug("RTcmix initialized");
    return 0;
}

int Audio::reInitializeRTcmix()
{
    if (rtcmixInitialized) {
        stopAudio();
        RTcmix_destroy();
        Pa_Sleep(100);
    }
    initializeRTcmix();
    int result = startAudio();
    if (result != 0)
        return -1;
    return 0;
}

void Audio::startRecording(const QString &fileName)
{
    QByteArray ba = fileName.toLatin1();
    char *fname = ba.data();

    SF_INFO sfinfo;
    bzero(&sfinfo, sizeof(sfinfo));
    sfinfo.samplerate = requestedSamplingRate;
    sfinfo.channels = requestedNumOutChannels;
    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT | SF_ENDIAN_LITTLE;
    if (!sf_format_check(&sfinfo)) {
        qDebug("startRecording: invalid sound file format requested");
        return;
    }

    SNDFILE *sf = sf_open(fname, SFM_WRITE, &sfinfo);
    if (sf == NULL) {
        qDebug("startRecording: sf_open returned NULL (%s)", sf_strerror(sf));
        return;
    }

    if (recording) {
        sf_close(sf);
        qDebug("startRecording called while already recording!");
        return;
    }
    PaUtil_FlushRingBuffer(&recordRingBuffer);
    recording = true;

//******* here's where we should start working in another thread

    // FIXME: for now, test this using the main thread for a definite duration.
    // This will block the GUI thread.
    int numFrames = 44100 * 6;
int totframes = numFrames;
int totsampswritten = 0;
    // NB: This will write a total number of frames that is evenly divisible by the audio block size
    while (numFrames > 0) {
        int sampsAvail = PaUtil_GetRingBufferReadAvailable(&recordRingBuffer);
//        int readCount = sampsAvail - (sampsAvail % requestedNumOutChannels); // align on frame boundary
        if (sampsAvail) {
            int sampsRead = PaUtil_ReadRingBuffer(&recordRingBuffer, transferBuffer, sampsAvail);
            if (sampsRead != sampsAvail)
                qDebug("record ringbuf read request doesn't match samps delivered");
            numFrames -= (sampsRead / requestedNumOutChannels);
//qDebug("startRecord: sampsRead=%d => numFrames remaining=%d", sampsRead, numFrames);
            sf_count_t sampsWritten = sf_write_float(sf, transferBuffer, sampsRead);
            if (sampsWritten != sampsRead)
                qDebug().nospace() << "startRecording: sf_write_float didn't write all the samps (" << sampsRead << " => " << sampsWritten;
totsampswritten += sampsRead;
        }
        //sf_write_sync(sf);  messes up playback. call less frequently, or not at all?
    }
    if (sf_close(sf) != 0)
        qDebug("startRecording: sf_close error: %s", sf_strerror(sf));
qDebug("finished recording - frames requested: %d, frames written: %d", totframes, totsampswritten / 2);
    stopRecording();
}

void Audio::stopRecording()
{
    if (!recording)
        return;
    recording = false;
}




// ---------------------------- NOTHING ENABLED BELOW ---------------------------

#ifdef NOTYET // should be owned by app and exchange device, etc. info with Audio class
void Audio::initializeWindow()
{
    QScopedPointer<QWidget> window(new QWidget);
    QScopedPointer<QVBoxLayout> layout(new QVBoxLayout);

    m_deviceBox = new QComboBox(this);
    const QAudioDeviceInfo &defaultDeviceInfo = QAudioDeviceInfo::defaultOutputDevice();
    m_deviceBox->addItem(defaultDeviceInfo.deviceName(), qVariantFromValue(defaultDeviceInfo));
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)) {
        if (deviceInfo != defaultDeviceInfo)
            m_deviceBox->addItem(deviceInfo.deviceName(), qVariantFromValue(deviceInfo));
    }
    CHECKED_CONNECT(m_deviceBox,SIGNAL(activated(int)),SLOT(deviceChanged(int)));
    layout->addWidget(m_deviceBox);

    QHBoxLayout *volumeBox = new QHBoxLayout;
    m_volumeLabel = new QLabel;
    m_volumeLabel->setText(tr(VOLUME_LABEL));
    m_volumeSlider = new QSlider(Qt::Horizontal);
    m_volumeSlider->setMinimum(0);
    m_volumeSlider->setMaximum(100);
    m_volumeSlider->setSingleStep(10);
    CHECKED_CONNECT(m_volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(volumeChanged(int)));
    volumeBox->addWidget(m_volumeLabel);
    volumeBox->addWidget(m_volumeSlider);
    layout->addLayout(volumeBox);

    window->setLayout(layout.data());
    layout.take(); // ownership transferred

    setCentralWidget(window.data());
    QWidget *const windowPtr = window.take(); // ownership transferred
    windowPtr->show();
}
#endif


// Qt audio volume stuff
// should be owned by main window and exchange info with Audio class
#define VOLUME_LABEL    "Volume:"

#ifdef NOTYET
    qreal initialVolume = QAudio::convertVolume(m_audioOutput->volume(),
                                                QAudio::LinearVolumeScale,
                                                QAudio::LogarithmicVolumeScale);
    m_volumeSlider->setValue(qRound(initialVolume * 100));

void Audio::deviceChanged(int index)
{
    Q_UNUSED(index);
    initializeAudio();
}

void Audio::volumeChanged(int value)
{
    if (m_audioOutput) {
        qreal linearVolume = QAudio::convertVolume(value / qreal(100),
                                                   QAudio::LogarithmicVolumeScale,
                                                   QAudio::LinearVolumeScale);
        m_audioOutput->setVolume(linearVolume);
    }
}
#endif
