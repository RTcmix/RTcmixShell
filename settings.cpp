#include <QtDebug>
#include "settings.h"

Settings::Settings()
{
    settings = new QSettings();
    settings->sync();
    reportError();
}

void Settings::saveSettings()
{
    settings->sync();
    reportError();
}

void Settings::reportError()
{
    if (settings->status() == QSettings::AccessError) {
        qDebug("access error reading settings file");
    }
    else if (settings->status() == QSettings::FormatError) {
        qDebug("Your settings file is corrupted; using defaults.");
    }
}
