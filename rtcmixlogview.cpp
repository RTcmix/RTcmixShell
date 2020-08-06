#include "rtcmixlogview.h"
#include "RTcmix_API.h"
#include "utils.h"

#include <QFileInfo>
#include <QTimer>

// Output messages arrive on an RTcmix thread in rtcmixPrintCallback(). We cannot
// simply print these to the window, because Qt GUI widget objects are not thread
// safe: you can call into them only from the main GUI thread. Instead, we use a
// ring buffer to move the incoming messages over to checkJobOutput(), invoked
// from a timer in the main thread. The buffer is a single block comprising many
// equal-sized strings. Usually, these won't be filled to capacity.

//#define RTCMIX_PRINT_DEBUG

void rtcmixPrintCallback(const char *printBuffer, void *inContext);

// NB: WAVETABLE4.sco blows past 4096 strings
// NB: MULTI_FM2.sco can have strings as long as 272 chars!
const int ringBufferNumStrings = 1024 * 16;      // must be power of 2
const int ringBufferStringCapacity = 512;        // ditto
const int logTimerInterval = 10;      // msec
const int logMaxLines = 1024 * 16;

RTcmixLogView::RTcmixLogView(QWidget *parent) : QPlainTextEdit(parent)
{
    setReadOnly(true);
    setMaximumBlockCount(logMaxLines);

    // set background to very light gray
    QPalette p = palette();
    p.setColor(QPalette::Active, QPalette::Base, QColor(240, 240, 240));
    p.setColor(QPalette::Inactive, QPalette::Base, QColor(240, 240, 240));
    //FIXME: I disabled the gray background, because it doesn't change
    // when you use dark mode in macOS, making the log unreadable.
    // They need a simple way to deal with dark mode; not sure it's there yet.
    //setPalette(p);

    // set up the ring buffer
    ringBufferBlock = (char *) calloc(ringBufferNumStrings, ringBufferStringCapacity);
    PaUtil_InitializeRingBuffer(&logRingBuffer, ringBufferStringCapacity, ringBufferNumStrings, ringBufferBlock);
    RTcmix_setPrintCallback(rtcmixPrintCallback, &logRingBuffer);

    logTimer = new QTimer(this);
    CHECKED_CONNECT(logTimer, &QTimer::timeout, this, &RTcmixLogView::checkLogOutput);

    viewport()->setAcceptDrops(false);
}

void rtcmixPrintCallback(const char *printBuffer, void *inContext)
{
    // This is complicated, because we don't know how large printBuffer is,
    // only that it comprises any number of C-strings laid end-to-end.
    // The end of the buffer is marked by at least two consecutive nulls.
    PaUtilRingBuffer *ringBuf = reinterpret_cast<PaUtilRingBuffer *>(inContext);

    // Skip initial null; return if there are two consecutive nulls.
    const char *p = printBuffer;
    if (p[0] == 0) {
        if (p[1] == 0) {
            qDebug("rtcmixPrintCallback: printBuffer begins with two nulls");
            return;
        }
        else
            p++;
    }

#ifdef RTCMIX_PRINT_DEBUG
    int numstr = 0;
    int maxlen = 0;
    int totlen = 0;
#endif
    while (PaUtil_GetRingBufferWriteAvailable(ringBuf)) {
        int len = int(strlen(p) + 1);                 // including terminal null
        if (len > ringBufferStringCapacity) {    // break it into pieces (not likely)
            qDebug("rtcmixPrintCallback: incoming string too long (%d)", len);
#ifdef NOCANDO
            p[ringBufferStringCapacity - 1] = 0;
#endif
            len = ringBufferStringCapacity;
        }
        int rbCount = PaUtil_WriteRingBuffer(ringBuf, p, 1);
        if (rbCount != 1)
            qDebug("rtcmixPrintCallback: PaUtil_WriteRingBuffer failure");
#ifdef RTCMIX_PRINT_DEBUG
        maxlen = qMax(maxlen, len);
        totlen += len;
        numstr++;
#endif
        p += len;   // skip to next C-string
        if (*p == 0)
            break;
    }
#ifdef RTCMIX_PRINT_DEBUG
    qDebug("rtcmixPrintCallback: %d strings written, total len: %d, max len: %d", numstr, totlen, maxlen);
#endif
}

void RTcmixLogView::startLog()
{
    // this just resets the read and write pointers; does not clear block
    PaUtil_FlushRingBuffer(&logRingBuffer);

    if (!logTimer->isActive())
        logTimer->start(logTimerInterval);
}

void RTcmixLogView::stopLog()
{
    logTimer->stop();
}

void RTcmixLogView::clearLog()
{
    clear();
}

void RTcmixLogView::printLogSeparator(const QString &fileName)
{
    QString shownName;
    if (fileName.isEmpty())
        shownName = "untitled.sco";
    else
        shownName = QFileInfo(fileName).fileName();
    appendPlainText(QString(
            tr("\n++++++++++ PLAYING SCORE: %1 ++++++++++\n")).arg(shownName));
    moveCursor(QTextCursor::End);
}

// this runs periodically while user plays a score, invoked by a timer
void RTcmixLogView::checkLogOutput()
{
    bool wroteSome = false;

    while (PaUtil_GetRingBufferReadAvailable(&logRingBuffer)) {
        char buf[ringBufferStringCapacity + 16];
        int rbCount = PaUtil_ReadRingBuffer(&logRingBuffer, buf, 1);
        if (rbCount != 1)
            qDebug("checkJobOutput: PaUtil_ReadRingBuffer failure");
        // It is possible for buf to be unterminated, in case incoming string
        // was longer than ringBufferStringCapacity and needed to be broken up.
        // Here we simply print the string across multiple lines.
        buf[ringBufferStringCapacity] = 0;  // append null for benefit of QString, but within buf block
        int len = int(strlen(buf));
        if (len) {
            if (buf[len-1] == '\n')   // chomp line ending, since appendPlainText adds one
                buf[len-1] = 0;
            appendPlainText(QString(buf));
            wroteSome = true;
        }
    }
    if (wroteSome)
        moveCursor(QTextCursor::End);
}
