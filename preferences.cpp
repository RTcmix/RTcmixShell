#include <QtWidgets>
#include <QtDebug>

#include "preferences.h"
#include "utils.h"

// Preferences dialog code based on "tabdialog" example in Qt5.

#ifdef GENERALTAB
GeneralTab::GeneralTab(QWidget *parent) : QWidget(parent)
{

}
#endif

EditorTab::EditorTab(QWidget *parent) : QWidget(parent)
{

}

SyntaxHighlightingTab::SyntaxHighlightingTab(QWidget *parent) : QWidget(parent)
{

}

AudioTab::AudioTab(QWidget *parent) : QWidget(parent)
{

}

PreferencesDialog::PreferencesDialog(QWidget *parent)
    : QDialog(parent)
{
    tabWidget = new QTabWidget;
#ifdef GENERALTAB
    tabWidget->addTab(new GeneralTab(), tr("General"));
#endif
    tabWidget->addTab(new EditorTab(), tr("Editor"));
    tabWidget->addTab(new SyntaxHighlightingTab(), tr("Syntax Highlighting"));
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
    // need signals/slots to force update of these params in other modules

    // Editor tab
    prefs->setEditorFontName();
    prefs->setEditorFontSize();
    prefs->setEditorTabWidth();
    prefs->setEditorShowLineNumbers();
    prefs->setLogFontName();
    prefs->setLogFontSize();

    // Syntax Highlighting tab
    prefs->setEditorDoSyntaxHighlighting();
    // add support for changing colors

    // Audio tab
    // prefs->setAudioInputDeviceID();
    prefs->setAudioOutputDeviceID();
    prefs->setAudioSamplingRate();
    // prefs->setAudioNumInputChannels();
    prefs->setAudioNumOutputChannels();
    prefs->setAudioBlockSize();
    prefs->setAudioNumBuses();
    prefs->setAudioShowOverlappingScoresWarning();
#endif
}


//-------------------------------------------------------------------------------

Preferences::Preferences()
{
    settings = new QSettings();
    settings->sync();
    reportError();
}

void Preferences::savePreferences()
{
    settings->sync();
    reportError();
}

void Preferences::showPreferencesDialog()
{
    PreferencesDialog dlog;
    int result = dlog.exec();     // modal
    if (result == QDialog::Accepted)
        dlog.applyPreferences(this);
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

