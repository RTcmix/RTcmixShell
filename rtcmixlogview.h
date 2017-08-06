#ifndef RTCMIXLOGVIEW_H
#define RTCMIXLOGVIEW_H

#include <QPlainTextEdit>
#include "pa_ringbuffer.h"

QT_BEGIN_NAMESPACE
class QString;
class QTimer;
class QWidget;
QT_END_NAMESPACE

class RTcmixLogView : public QPlainTextEdit
{
    Q_OBJECT

public:
    RTcmixLogView(QWidget *parent = 0);

    void startLog();
    void stopLog();
    void printLogSeparator(const QString &fileName);

public slots:
    void clearLog();

private slots:
    void checkLogOutput();

private:
    QTimer *logTimer;
    PaUtilRingBuffer logRingBuffer;
    char *ringBufferBlock;
};

#endif // RTCMIXLOGVIEW_H
