
#ifndef AUDIO_H
#define AUDIO_H

#include <atomic>
#include <QObject>

QT_BEGIN_NAMESPACE
class QString;
class QTimer;
QT_END_NAMESPACE
class MainWindow;
class RecordThreadController;
class Preferences;

#include "portaudio.h"
#include "pa_ringbuffer.h"
#include "sndfile.h"

int availableInputDeviceIDs(QVector<int> &);
int availableOutputDeviceIDs(QVector<int> &);
#ifdef UNUSED
int defaultOutputDevice();
#endif
int deviceIDFromName(const QString &);
int deviceNameFromID(const int, QString &);
int maxInputChannelCount(const int);
int maxOutputChannelCount(const int);
int availableSamplingRates(const int, const int, QVector<int> &);
int availableBufferSizes(const int, QVector<int> &);

class Audio : public QObject
{
    Q_OBJECT

public:
    Audio();
    ~Audio();
    int reinitializeRTcmix(bool interactive=false);
    int startAudio();
    bool startRecording(const QString &);
    void stopRecording();

private:
    int initializeAudio();
    int initializeRTcmix(bool interactive=false);
    int stopAudio();

    // We use a static method wrapper for our callback to make portaudio work from C++,
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

    // sync these with prefs dlog
    int inputDeviceID;
    int outputDeviceID;
    float samplingRate;
    int numInChannels;
    int numOutChannels;
    int bufferSize;
    int busCount;

    MainWindow *mainWindow;
    PaUtilRingBuffer recordRingBuffer;
    SNDFILE *recordFile;
    float *recordBuffer;
    float *transferBuffer;
    RecordThreadController *recordThreadController;
    std::atomic<bool> nowRecording;
    std::atomic<bool> detectClipping;
    int *consecutiveSamps;
    std::atomic<int> *clippingCounts;
    QTimer *clippingTimer;

    Preferences *audioPreferences;

private slots:
    void checkClipping();

signals:
    void didClip(int clipCount);

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
