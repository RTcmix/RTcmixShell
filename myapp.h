#ifndef MYAPP_H
#define MYAPP_H

#include <QApplication>
#include <QFileOpenEvent>
#include <QtDebug>
#include "mainwindow.h"
#include "utils.h"

class MyApplication : public QApplication
{

public:
    MyApplication(int &argc, char **argv)
        : QApplication(argc, argv)
        , mainWindow(NULL)
    {
    }

    void setMainWindow(MainWindow *mw) { mainWindow = mw; }

    bool event(QEvent *event)
    {
        if (event->type() == QEvent::FileOpen) {
            QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
            //qDebug() << "Open file" << openEvent->file();
            if (mainWindow)
                mainWindow->loadFile(openEvent->file());
        }
        return QApplication::event(event);
    }

private:
    MainWindow *mainWindow;
};

#endif // MYAPP_H
