#ifndef RECORD_H
#define RECORD_H

#include <QObject>
#include <QThread>
#include "pa_ringbuffer.h"
#include "sndfile.h"
#include "utils.h"

class RecordWorker : public QObject
{
    Q_OBJECT

public:
    RecordWorker(int, PaUtilRingBuffer *, SNDFILE *, float *);
    ~RecordWorker();

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

class RecordThreadController : public QObject
{
    Q_OBJECT

    QThread workerThread;
    RecordWorker *worker;

public:
    RecordThreadController(int numChans, PaUtilRingBuffer *ringBuf, SNDFILE *file, float *transferBuffer) {
        worker = new RecordWorker(numChans, ringBuf, file, transferBuffer);
        CHECKED_CONNECT(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
        CHECKED_CONNECT(&workerThread, &QThread::started, worker, &RecordWorker::record);
    }
    ~RecordThreadController() {
        workerThread.quit();
        workerThread.wait();
    }
    void start();
    void stop();
};

#endif // RECORD_H
