#include <QtWidgets>
#include <QtDebug>

#include "preferences.h"
#include "utils.h"

// Preferences dialog code based on "tabdialog" example in Qt5.

GeneralTab::GeneralTab(QWidget *parent) : QWidget(parent)
{

}

EditorTab::EditorTab(QWidget *parent) : QWidget(parent)
{

}

AudioTab::AudioTab(QWidget *parent) : QWidget(parent)
{

}

PreferencesDialog::PreferencesDialog(QWidget *parent)
    : QDialog(parent)
{
    tabWidget = new QTabWidget;
    tabWidget->addTab(new GeneralTab(), tr("General"));
    tabWidget->addTab(new EditorTab(), tr("Editor"));
    tabWidget->addTab(new AudioTab(), tr("Audio"));

    okCancelButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    CHECKED_CONNECT(okCancelButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    CHECKED_CONNECT(okCancelButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(okCancelButtonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("RTcmixShell Preferences"));
}

void PreferencesDialog::applyPreferences(Preferences *prefs)
{
#ifdef NOTYET
    prefs->setEditorFontName()
#endif
}


//-------------------------------------------------------------------------------

Preferences::Preferences()
{
    settings = new QSettings();
    settings->sync();
    reportError();
}

void Preferences::saveSettings()
{
    settings->sync();
    reportError();
}

void Preferences::showPreferencesDialog()
{
    PreferencesDialog dlog(this);
    int result = dlog.exec();     // modal
    if (result == QDialog::Accepted)
        dlog.applyPreferences(prefs);
}

void Preferences::reportError()
{
    if (settings->status() == QSettings::AccessError) {
        qDebug("access error reading settings file");
    }
    else if (settings->status() == QSettings::FormatError) {
        qDebug("Your settings file is corrupted; using defaults.");
    }
}

