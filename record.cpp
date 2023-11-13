#include <atomic>
#include <QDebug>
#include "record.h"

std::atomic<bool> keepRecording;

RecordWorker::RecordWorker(int numOutChans, PaUtilRingBuffer *ringBuffer, SNDFILE *outFile, float *transferBuffer)
        : numOutChans(numOutChans)
        , ringBuffer(ringBuffer)
        , outFile(outFile)
        , transferBuffer(transferBuffer)
{
}

RecordWorker::~RecordWorker()
{
}

void RecordWorker::record()
{
//qDebug("entering RecordWorker::record()");
//int totsampswritten = 0;
    // NB: This will write a total number of frames that is evenly divisible by the audio block size
    while (keepRecording) {
        int sampsAvail = PaUtil_GetRingBufferReadAvailable(ringBuffer);
        if (sampsAvail) {
            int sampsRead = PaUtil_ReadRingBuffer(ringBuffer, transferBuffer, sampsAvail);
            if (sampsRead != sampsAvail)
                qDebug("RecordWorker::record(): ringbuf read request doesn't match samps delivered");
//qDebug("RecordWorker::record(): sampsRead=%d", sampsRead);
            sf_count_t sampsWritten = sf_write_float(outFile, transferBuffer, sampsRead);
            if (sampsWritten != sampsRead)
                qDebug().nospace() << "RecordWorker::record(): sf_write_float didn't write all the samps (" << sampsRead << " => " << sampsWritten;
//totsampswritten += sampsRead;
        }
        //sf_write_sync(outFile);  messes up playback. call less frequently, or not at all?
    }
    if (sf_close(outFile) != 0) {
        const QString msg = QString(tr("Error closing recorded sound file\n(RecordWorker::record: sf_close: %1)")).arg(sf_strerror(outFile));
        warnAlert(nullptr, msg);
    }
//qDebug("finished recording - frames written: %d", totsampswritten / 2);

    emit finished();
}

void RecordThreadController::start()
{
    worker->moveToThread(&workerThread);
    keepRecording = true;
    workerThread.start(/*QThread::LowPriority*/);
}

void RecordThreadController::stop()
{
    keepRecording = false;
}
