
#include <QApplication>
#include <QMessageBox>
#include <QTime>
#include <QWidget>
#include "mainwindow.h"
#include "utils.h"

MainWindow *getMainWindow()
{
    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        QString name = widget->objectName();
        if (name == "MainWindow")
            return static_cast<MainWindow *>(widget);
    }
    return NULL;
}

void warnAlert(QWidget *parent, const QString &errorText)
{
    // NB: can pass NULL as parent, but then alert will appear over
    // topmost window, which might not have generated the error.
    // See answer by Linville at https://stackoverflow.com/questions/15503000/how-to-call-qmessagebox-static-api-outside-of-a-qwidget-sub-class.
    QMessageBox::warning(parent, QCoreApplication::applicationName(), errorText, QMessageBox::Ok, QMessageBox::NoButton);
}

// adapted from GraehamF answer: https://stackoverflow.com/questions/3752742/how-do-i-create-a-pause-wait-function-using-qt
void delay(int millisecondsToWait)
{
    QTime dieTime = QTime::currentTime().addMSecs(millisecondsToWait);
    while (QTime::currentTime() < dieTime) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    }
}
