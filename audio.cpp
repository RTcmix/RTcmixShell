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

#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QTimer>
#include <QVector>
#include <qmath.h>
#include <qendian.h>

#include <math.h>

#include "audio.h"
#include "mainwindow.h"
#include "record.h"
#define EMBEDDEDAUDIO
#include "RTcmix_API.h"
#include "preferences.h"
#include "utils.h"

// FIXME: Might want to realloc this if numchans changes
const int ringBufferNumSamps = 1024 * 32;

const int consecutiveFullScaleSamps = 2;
const int clippingTimerInterval = 50;


Audio::Audio()
    : portAudioInitialized(false)
    , rtcmixInitialized(false)
    , stream(NULL)
    , recordFile(NULL)
    , recordBuffer(NULL)
    , transferBuffer(NULL)
    , recordThreadController(NULL)
    , nowRecording(false)
    , detectClipping(true)
    , consecutiveSamps(NULL)
    , clippingCounts(NULL)
    , clippingTimer(NULL)
{
    // This syncs with the MainWindow-owned settings, even though it's a different object.
    audioPreferences = new Preferences();

    audioApiID = audioPreferences->audioApiID();
    inputDeviceID = audioPreferences->audioInputDeviceID();
    outputDeviceID = audioPreferences->audioOutputDeviceID();
    samplingRate = audioPreferences->audioSamplingRate();
#ifdef INDEPENDENT_INCHANS
    numInChannels = audioPreferences->audioNumInputChannels();
    numOutChannels = audioPreferences->audioNumOutputChannels();
#else
    numOutChannels = audioPreferences->audioNumOutputChannels();
#ifdef USE_INPUT_DEVICE
    numInChannels = numOutChannels;
#else
    numInChannels = 0;
#endif
#endif
    bufferSize = audioPreferences->audioBufferSize();
    busCount = audioPreferences->audioNumBuses();

    mainWindow = getMainWindow();

    int result = initializeAudio();
    if (result == 0) {
        initializeRTcmix();
    }
}

Audio::~Audio()
{
    if (portAudioInitialized) {
        PaError err = Pa_CloseStream(stream);
        if (err != paNoError) {
            const QString msg = QString(tr("Error closing audio device\n(Pa_CloseStream: %1)")).arg(Pa_GetErrorText(err));
            warnAlert(nullptr, msg);
        }
        err = Pa_Terminate();
        if (err != paNoError) {
            const QString msg = QString(tr("Error closing audio device\n(Pa_Terminate: %1)")).arg(Pa_GetErrorText(err));
            warnAlert(nullptr, msg);
        }
    }
    if (rtcmixInitialized)
        RTcmix_destroy();
    if (recordBuffer)
        free(recordBuffer);
    if (transferBuffer)
        free(transferBuffer);
    delete consecutiveSamps;
    delete clippingCounts;
    delete recordThreadController;
    delete clippingTimer;
}

int Audio::initializeAudio()
{
    // Initialize
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        const QString msg = QString(tr("Error initializing audio system\n(Pa_Initialize: %1)")).arg(Pa_GetErrorText(err));
        warnAlert(nullptr, msg);
        return -1;
    }
    portAudioInitialized = true;

#ifdef USE_INPUT_CHANNELS
    PaStreamParameters inputParameters;
    memset(&inputParameters, 0, sizeof(inputParameters));
    inputParameters.channelCount = numInChannels;
    inputParameters.device = inputDeviceID;
    inputParameters.sampleFormat = paFloat32;
#endif
    PaStreamParameters outputParameters;
    memset(&outputParameters, 0, sizeof(outputParameters));
    outputParameters.channelCount = numOutChannels;
    outputParameters.device = outputDeviceID;
    outputParameters.sampleFormat = paFloat32;
#ifdef USE_INPUT_CHANNELS
    err = Pa_IsFormatSupported(&inputParameters, &outputParameters, samplingRate);
    if (err == paFormatIsSupported) {
        err = Pa_OpenStream(&stream,
                            &inputParameters,
                            &outputParameters,
                            samplingRate,
                            bufferSize,
                            paClipOff | paDitherOff,  // clipping happens inside RTcmix
                            &paCallback,
                            this);
    }
#else
    err = Pa_IsFormatSupported(NULL, &outputParameters, samplingRate);
    if (err == paFormatIsSupported) {
        err = Pa_OpenStream(&stream,
                            NULL,
                            &outputParameters,
                            samplingRate,
                            bufferSize,
                            paClipOff | paDitherOff,  // clipping happens inside RTcmix
                            &paCallback,
                            this);
    }
#endif
    else {
        err = Pa_OpenDefaultStream(&stream,
                                   numInChannels,
                                   numOutChannels,
                                   paFloat32,
                                   samplingRate,
                                   bufferSize,
                                   &paCallback,
                                   this);
    }
    if (err != paNoError) {
        const QString msg = QString(tr("Error opening audio device\n(Pa_OpenStream: %1)")).arg(Pa_GetErrorText(err));
        warnAlert(nullptr, msg);
        return -1;
    }

    consecutiveSamps = new int [numOutChannels];
    clippingCounts = new std::atomic<int> [numOutChannels];
    for (int i = 0; i < numOutChannels; i++)
        consecutiveSamps[i] = clippingCounts[i] = 0;

    clippingTimer = new QTimer(this);
    CHECKED_CONNECT(clippingTimer, &QTimer::timeout, this, &Audio::checkClipping);
    CHECKED_CONNECT(this, &Audio::didClip, mainWindow, &MainWindow::showClipping);

    recordBuffer = (float *) calloc(ringBufferNumSamps, sizeof(float));
    PaUtil_InitializeRingBuffer(&recordRingBuffer, sizeof(float), ringBufferNumSamps, recordBuffer);
    transferBuffer = (float *) calloc(ringBufferNumSamps, sizeof(float));

    qDebug("Audio initialized (srate=%d, inchans=%d, outchans=%d, bufsize=%d)", int(samplingRate), numInChannels, numOutChannels, bufferSize);
    return 0;
}

int Audio::startAudio()
{
    if (portAudioInitialized && stream != NULL && Pa_IsStreamStopped(stream)) {
        PaError err = Pa_StartStream(stream);
        if (err != paNoError) {
            const QString msg = QString(tr("Error starting audio\n(Pa_StartStream: %1)")).arg(Pa_GetErrorText(err));
            warnAlert(nullptr, msg);
            return -1;
        }
//        qDebug("Pa_StartStream returned noerr");
    }

    if (detectClipping) {
        if (!clippingTimer->isActive())
            clippingTimer->start(clippingTimerInterval);
    }

    return 0;
}

int Audio::stopAudio()
{
    if (clippingTimer->isActive())
        clippingTimer->stop();

    if (portAudioInitialized && stream != NULL && Pa_IsStreamActive(stream)) {
        PaError err = Pa_StopStream(stream);
        if (err != paNoError) {
            const QString msg = QString(tr("Error stopping audio\n(Pa_StopStream: %1)")).arg(Pa_GetErrorText(err));
            warnAlert(nullptr, msg);
            return -1;
        }
    }
    return 0;
}

void Audio::checkClipping()
{
    int clipCount = 0;
    for (int c = 0; c < numOutChannels; c++) {
        clipCount += clippingCounts[c];
        clippingCounts[c] = 0;
    }
    if (clipCount)
        emit didClip(clipCount);
}

int Audio::memberCallback(
            const void *input,
            void *output,
            unsigned long frameCount,
            const PaStreamCallbackTimeInfo *timeInfo,
            PaStreamCallbackFlags statusFlags)
{
    (void) timeInfo;

#ifdef DEBUG_IN_CALLBACK
    callbackCount++;
    if ((callbackCount % 100) == 0)
        qDebug("time: %f", timeInfo->currentTime);
    if ((statusFlags & paOutputUnderflow)) {
        outputUnderflowCount++;
        if ((outputUnderflowCount % 20) == 0)
            qDebug("Five OUTPUT UNDERFLOWs");
    }
#else
    (void) statusFlags;
#endif

    int result = RTcmix_runAudio(const_cast<void *>(input), output, frameCount);
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
            qDebug("20 buffers of silence");
    }
//    qDebug("RTcmix_runAudio called (result=%d, frameCount=%ld, output=%p)", result, frameCount, output);
#endif

    if (detectClipping) {
        // This (over-) simple clipping algorithm thinks a consecutive run of full-scale
        // samples of length >= consecutiveFullScaleSamps indicates clipping, even if
        // the signal oscillates between -1 and +1. This is per channel.
        float *sampPtr = (float *) output;
        for (unsigned long i = 0; i < frameCount; i++) {
            for (int c = 0; c < numOutChannels; c++) {
                float samp = fabs(*sampPtr++);
                if (samp > 0.999)
                    consecutiveSamps[c]++;
                else {
                    if (consecutiveSamps[c] > consecutiveFullScaleSamps)
                        clippingCounts[c]++;
                    consecutiveSamps[c] = 0;
                }
            }
        }
    }

    if (nowRecording) {
        int failCount = 0;
        float *ptr = (float *) output;
        int inSampCount = int(frameCount * numOutChannels);
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
//qDebug("audio callback: ring buffer write not available (1 time)");
                failCount = 0;
//                break;
            }
        }
//qDebug("audio callback: loop done (inSampCount=%d)", inSampCount);
    }

    return paContinue;
}

int Audio::initializeRTcmix(bool interactive)
{
    // initialize RTcmix
    int status = RTcmix_init();
    if (status != 0) {
        const QString msg = QString(tr("Error initializing RTcmix\n(RTcmix_init: %1)")).arg(status);
        warnAlert(nullptr, msg);
        return -1;
    }
    rtcmixInitialized = true;
    // need to destroy rtcmix in case we're here after a config change -- or reconfig it with RTcmix_resetAudio

    status = RTcmix_setAudioBufferFormat(AudioFormat_32BitFloat_Normalized, numOutChannels);
    if (status != 0) {
        const QString msg = QString(tr("Error configuring RTcmix audio buffer\n(RTcmix_setAudioBufferFormat: %1)")).arg(status);
        warnAlert(nullptr, msg);
        return -1;
    }

    // TODO: handle input as well as output
#ifdef USE_INPUT_DEVICE
    int takingInput = 1;
#else
    int takingInput = 0;
#endif
    status = RTcmix_setparams(samplingRate, numOutChannels, bufferSize, takingInput, busCount);
    if (status != 0) {
        const QString msg = QString(tr("Error configuring RTcmix audio parameters\n(RTcmix_setparams: %1)")).arg(status);
        warnAlert(nullptr, msg);
        return -1;
    }

	RTcmix_setInteractive(interactive);
	
    qDebug("RTcmix initialized");
    return 0;
}

int Audio::reinitializeRTcmix(bool interactive)
{
    if (rtcmixInitialized) {
        stopAudio();
        RTcmix_destroy();
        Pa_Sleep(100);
    }
    initializeRTcmix(interactive);
    return 0;
}

bool Audio::startRecording(const QString &fileName)
{
    QByteArray ba = fileName.toLatin1();
    char *fname = ba.data();

    SF_INFO sfinfo;
    memset(&sfinfo, 0, sizeof(sfinfo));
    sfinfo.samplerate = samplingRate;
    sfinfo.channels = numOutChannels;
    if (fileName.endsWith(".wav"))
        sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;        // TODO: option for SF_FORMAT_PCM_24
    else if (fileName.endsWith(".aif") || fileName.endsWith(".aiff"))
        sfinfo.format = SF_FORMAT_AIFF | SF_FORMAT_FLOAT;
    else {
        const QString msg = QString(tr("Invalid sound file name extension (must be \".wav\", \".aif\", or \".aiff\""));
        warnAlert(nullptr, msg);
        return false;
    }
    if (!sf_format_check(&sfinfo)) {
        const QString msg = QString(tr("Invalid sound file format requested (startRecording)"));
        warnAlert(nullptr, msg);
        return false;
    }

    recordFile = sf_open(fname, SFM_WRITE, &sfinfo);
    if (recordFile == NULL) {
        const QString msg = QString(tr("Error opening sound file for recording\n(startRecording: sf_open returned NULL (%2))")).arg(sf_strerror(recordFile));
        warnAlert(nullptr, msg);
        return false;
    }

    if (nowRecording) {
        sf_close(recordFile);
        const QString msg = QString(tr("Error: starting recording while recording already in progress"));
        warnAlert(nullptr, msg);
        return false;
    }

    delete recordThreadController;
    recordThreadController = new RecordThreadController(numOutChannels, &recordRingBuffer, recordFile, transferBuffer);
    PaUtil_FlushRingBuffer(&recordRingBuffer);
    nowRecording = true;
    recordThreadController->start();

    return true;
}

void Audio::stopRecording()
{
    if (!nowRecording)
        return;
    nowRecording = false;
    if (recordThreadController)
        recordThreadController->stop();
}


// --------------------------------------------------------------------------
// Device discovery and info

// Return number of audio host APIs, or -1 if error.
// Pass back QVector of current audio API IDs.
// QVector is owned by caller.
int availableAudioApiIDs(QVector<PaHostApiIndex> &idList)
{
    PaHostApiIndex numAPIs = Pa_GetHostApiCount();
    if (numAPIs < 0) {
        const QString msg = QString(QObject::tr("Error finding available audio host APIs\n(availableAudioAPIIDs: no audio API IDs discovered)"));
        warnAlert(nullptr, msg);
        return -1;
    }

    const PaHostApiInfo *apiInfo;
    PaHostApiIndex count = 0;
    for (PaHostApiIndex id = 0; id < numAPIs; id++) {
        apiInfo = Pa_GetHostApiInfo(id);
        if (apiInfo == NULL) {
            const QString msg = QString(QObject::tr("Error getting information for audio host API %1\n(availableAudioAPIIDs)")).arg(id);
            warnAlert(nullptr, msg);
            return -1;
        }
        idList.append(id);
        count++;
    }

    return count;
}

// Return number of input devices, or -1 if error.
// Pass back QVector of current input device IDs.
// QVector is owned by caller.
int availableInputDeviceIDs(QVector<PaDeviceIndex> &idList)
{
    PaDeviceIndex numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        const QString msg = QString(QObject::tr("Error finding available audio input devices\n(availableInputDeviceIDs: no device IDs discovered)"));
        warnAlert(nullptr, msg);
        return -1;
    }

    const PaDeviceInfo *deviceInfo;
    int count = 0;
    for (PaDeviceIndex id = 0; id < numDevices; id++) {
        deviceInfo = Pa_GetDeviceInfo(id);
        if (deviceInfo == NULL) {
            const QString msg = QString(QObject::tr("Error getting information for audio input device %1\n(availableInputDeviceIDs)")).arg(id);
            warnAlert(nullptr, msg);
            return -1;
        }
        if (deviceInfo->maxInputChannels > 0) {
            idList.append(id);
            count++;
        }
    }

    return count;
}

// Return number of output devices, or -1 if error.
// Pass back QVector of current output device IDs.
// QVector is owned by caller.
int availableOutputDeviceIDs(QVector<PaDeviceIndex> &idList)
{
    PaDeviceIndex numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        const QString msg = QString(QObject::tr("Error finding available audio output devices\n(availableOutputDeviceIDs: no device IDs discovered)"));
        warnAlert(nullptr, msg);
        return -1;
    }

    const PaDeviceInfo *deviceInfo;
    int count = 0;
    for (PaDeviceIndex id = 0; id < numDevices; id++) {
        deviceInfo = Pa_GetDeviceInfo(id);
        if (deviceInfo == NULL) {
            const QString msg = QString(QObject::tr("Error getting information for audio output device %1\n(availableOutputDeviceIDs)")).arg(id);
            warnAlert(nullptr, msg);
            return -1;
        }
        if (deviceInfo->maxOutputChannels > 0) {
            idList.append(id);
            count++;
        }
    }

    return count;
}

#ifdef UNUSED
// Return device ID of default output device, or -1 if error.
PaDeviceIndex defaultOutputDevice()
{
    PaDeviceIndex numDevices = Pa_GetDeviceCount();
    for (PaDeviceIndex id = 0; id < numDevices; id++) {
        if (id == Pa_GetDefaultOutputDevice())
            return id;
    }
    return -1;
}
#endif

PaHostApiIndex audioApiIdFromName(const QString &name)
{
    const PaHostApiInfo *apiInfo;
    PaHostApiIndex numAPIs = Pa_GetHostApiCount();
    for (PaHostApiIndex id = 0; id < numAPIs; id++) {
        apiInfo = Pa_GetHostApiInfo(id);
        if (apiInfo == NULL) {
            const QString msg = QString(QObject::tr("Error getting information for audio API %1\n(audioApiIdFromName)")).arg(id);
            warnAlert(nullptr, msg);
            return -1;
        }
        if (apiInfo->name == name)
            return id;
    }
    return -1;
}

// Pass back a string, owned by caller, with the name of the API
// with the given API ID. Return -1 if error, 0 if not.
int audioApiNameFromId(const PaHostApiIndex apiID, QString &name)
{
    const PaHostApiInfo *apiInfo = Pa_GetHostApiInfo(apiID);
    if (apiInfo == NULL) {
        const QString msg = QString(QObject::tr("Error getting information for audio API %1\n(audioApiNameFromId)")).arg(apiID);
        warnAlert(nullptr, msg);
        return -1;
    }
    name = apiInfo->name;
    return 0;
}

PaDeviceIndex deviceIdFromName(const QString &name)
{
    const PaDeviceInfo *deviceInfo;
    PaDeviceIndex numDevices = Pa_GetDeviceCount();
    for (PaDeviceIndex id = 0; id < numDevices; id++) {
        deviceInfo = Pa_GetDeviceInfo(id);
        if (deviceInfo == NULL) {
            const QString msg = QString(QObject::tr("Error getting information for audio device %1\n(deviceIdFromName)")).arg(id);
            warnAlert(nullptr, msg);
            return -1;
        }
        if (deviceInfo->name == name)
            return id;
    }
    return -1;
}

// Pass back a string, owned by caller, with the name of the device
// with the given device ID. Return -1 if error, 0 if not.
int deviceNameFromId(const PaDeviceIndex deviceID, QString &name)
{
    const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(deviceID);
    if (deviceInfo == NULL) {
        const QString msg = QString(QObject::tr("Error getting information for audio device %1\n(deviceNameFromId)")).arg(deviceID);
        warnAlert(nullptr, msg);
        return -1;
    }
    name = deviceInfo->name;
    return 0;
}

// Return the max input channel count for the given device.
int maxInputChannelCount(const PaDeviceIndex deviceID)
{
    const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(deviceID);
    return deviceInfo->maxInputChannels;
}

// Return the max output channel count for the given device.
int maxOutputChannelCount(const PaDeviceIndex deviceID)
{
    const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(deviceID);
    return deviceInfo->maxOutputChannels;
}

// Return number of available sampling rates, if any, that are valid for the
// given device ID and output channel count. Pass back QVector of valid
// sampling rates, which is owned by caller.
// NB: This gives us many rates as valid for the MOTU 1248 that probably aren't.
int availableSamplingRates(const PaDeviceIndex deviceID, const int numOutputChannels, QVector<int> &rates)
{
    PaStreamParameters outParams;
    memset(&outParams, 0, sizeof(outParams));
    outParams.channelCount = numOutputChannels;
    outParams.device = deviceID;
    outParams.sampleFormat = paFloat32;

    static int standardSamplingRates[] = { 44100, 48000, 88200, 96000, 176400, 192000, -1 };
    int count = 0;
    for (int i = 0; standardSamplingRates[i] > 0; i++) {
        PaError err = Pa_IsFormatSupported(NULL, &outParams, standardSamplingRates[i]);
        if (err == paFormatIsSupported) {
            rates.append(standardSamplingRates[i]);
            count++;
        }   // we don't report an err, because the point is to exclude invalid rates silently
    }
    return count;
}

// Return number of available buffer sizes, if any, that are valid for the
// given device ID. Pass back QVector of valid buffer sizes, which is owned by caller.
// FIXME: doesn't appear possible in PortAudio, outside of ASIO devices.
// The only way to get this is to try opening a stream and see if it fails.
int availableBufferSizes(const PaDeviceIndex deviceID, QVector<int> &sizes)
{
    Q_UNUSED(deviceID);

    static int standardBufferSizes[] = { 32, 64, 128, 256, 512, 1024, 2048, 4096, -1 };
    int count = 0;
    for (int i = 0; standardBufferSizes[i] > 0; i++) {
        sizes.append(standardBufferSizes[i]);
        count++;
    }
    return count;
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
