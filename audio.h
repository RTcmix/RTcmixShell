
#ifndef AUDIO_H
#define AUDIO_H

#include <QObject>

QT_BEGIN_NAMESPACE
class QString;
QT_END_NAMESPACE
class RecordThreadController;
class Settings;

#include "portaudio.h"
#include "pa_ringbuffer.h"
#include "sndfile.h"


class Audio : public QObject
{
    Q_OBJECT

public:
    Audio();
    ~Audio();
    int reInitializeRTcmix();
    bool startRecording(const QString &);
    void stopRecording();

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

    bool portAudioInitialized;
    bool rtcmixInitialized;
    PaStream *stream;
    int callbackCount;
    int outputUnderflowCount;

    // sync these with settings dlog
    Settings *audioSettings;
    int inputDeviceID;
    int outputDeviceID;
    float samplingRate;
    int numInChannels;
    int numOutChannels;
    int blockSize;
    int busCount;

    PaUtilRingBuffer recordRingBuffer;
    SNDFILE *recordFile;
    float *recordBuffer;
    float *transferBuffer;
    RecordThreadController *recordThreadController;
    std::atomic<bool> nowRecording;

#ifdef NOTYET   // should move to main window
    // Owned by layout
    QComboBox *m_deviceBox;
    QLabel *m_volumeLabel;
    QSlider *m_volumeSlider;

private slots:
    void deviceChanged(int);
    void volumeChanged(int);
#endif
};

#endif // AUDIO_H
