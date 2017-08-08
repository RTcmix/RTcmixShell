
#ifndef AUDIO_H
#define AUDIO_H

#include <QObject>
#include <QThread>

QT_BEGIN_NAMESPACE
class QMutex;
class QString;
QT_END_NAMESPACE

#include "portaudio.h"
#include "pa_ringbuffer.h"
#include "sndfile.h"

class RecordWorker : public QObject
{
    Q_OBJECT

public:
    RecordWorker(int, PaUtilRingBuffer *, SNDFILE *, QObject *parentIN = 0);
    ~RecordWorker();
    void start()
    {
        this->moveToThread(&thread);
        thread.start(/*QThread::LowPriority*/);
    }
    void stop();

    QObject *parent;
    QThread thread;

public slots:
    void record();

signals:
    void finished();

private:
    int numOutChans;
    PaUtilRingBuffer *ringBuffer;
    SNDFILE *outFile;
    float *transferBuffer;
};

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
    RecordWorker *recordWorker;
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
