
#include <QApplication>
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
