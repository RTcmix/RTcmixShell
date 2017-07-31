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

// NB: I abandoned the Qt Multimedia approach after finding its design terrible
// for realtime audio. People use Rtaudio and PortAudio with Qt and have much
// better experiences.

#ifndef AUDIO_H
#define AUDIO_H

#include <math.h>

#include <QByteArray>
#include <QComboBox>
#include <QLabel>
#include <QMainWindow>
#include <QObject>
#include <QPushButton>
#include <QSlider>

#include "portaudio.h"

class Audio
{

public:
    Audio();
    ~Audio();
    int reInitializeRTcmix();

private:
    int initializeAudio();
    int initializeRTcmix();
    int startAudio();
    int stopAudio();

    // We use a static method wrapper for out callback to make portaudio work from C++,
    // as described here: https://app.assembla.com/wiki/show/portaudio/Tips_CPlusPlus .
    // For general problem, google: "function pointer C++ static" e.g.,
    // https://stackoverflow.com/questions/8302226/how-to-send-a-pointer-on-a-callback-function-which-is-encapsulated-in-a-class
    int memberCallback(const void *, void *, unsigned long, const PaStreamCallbackTimeInfo *, PaStreamCallbackFlags);
    static int paCallback(
            const void *input,
            void *output,
            unsigned long frameCount,
            const PaStreamCallbackTimeInfo *timeInfo,
            PaStreamCallbackFlags statusFlags,
            void *userData) // set to <this> when opening stream
    {
        Audio *thisclass = reinterpret_cast<Audio *>(userData);
        return thisclass->memberCallback(input, output, frameCount, timeInfo, statusFlags);
    }

private:
    bool portAudioInitialized;
    bool rtcmixInitialized;
    PaStream *stream;
    int callbackCount;
    int outputUnderflowCount;

    // sync these with settings dlog
    int requestedInputDeviceID;
    int requestedOutputDeviceID;
    float requestedSamplingRate;
    int requestedNumInChannels;
    int requestedNumOutChannels;
    int requestedBlockSize;
    int busCount;

#ifdef NOTYET   // should move to main window
    // Owned by layout
    QComboBox *m_deviceBox;
    QLabel *m_volumeLabel;
    QSlider *m_volumeSlider;
#endif

private slots:
    void deviceChanged(int);
    void volumeChanged(int);
};

#endif // AUDIO_H
