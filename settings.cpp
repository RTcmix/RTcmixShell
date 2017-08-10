#include "settings.h"

Settings::Settings()
{
    settings = new QSettings();
}

void Settings::saveSettings()
{
    settings->sync();
}
