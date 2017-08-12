#include <QList>
#include <QtWidgets>
#include <QtDebug>

#include "audio.h"
#include "preferences.h"
#include "utils.h"

// Preferences dialog code based on "tabdialog" example in Qt5.

const int minNumBuses = 8;
const int maxNumBuses = 96;



#ifdef GENERALTAB
GeneralTab::GeneralTab(QWidget *parent) : QWidget(parent)
{

}
#endif

EditorTab::EditorTab(QWidget *parent) : QWidget(parent)
{
/* Layout:
    Font:                    Size:
    Tab width: [QSpinBox: 1-8]
    Log Font Size:
    (font and size combo boxes as in current toolbar)
*/
}

SyntaxHighlightingTab::SyntaxHighlightingTab(QWidget *parent) : QWidget(parent)
{
/*  QHBoxLayout:
       [x] Enable Syntax Highlighting

    QGridLayout:
        Reserved words  [swatch]
        Numbers         [swatch]
        Strings         [swatch]
        Functions       [swatch]
        Unused commands [swatch]
        Comments        [swatch]
     (clicking swatches invokes QColorDialog
      for swatch, see answer by jpo38:
      https://stackoverflow.com/questions/18257281/qt-color-picker-widget )
*/
}

AudioTab::AudioTab(QWidget *parent) : QWidget(parent)
{
/* QGridLayout:
    Output Device:   [popup menu: names from portaudio]
    Sampling Rate:   [popup menu: e.g., 22050, 44100, 48000, 88200, 96000]
    Output Channels: [QSpinBox: 1-16]
    Buffer Size:     [popup menu: e.g., 64, 128, 256, 512, 1024, 2048, 4096, 8192]
    Internal Buses:  [QSpinbox: 8-64]

    (outside grid layout)
    [x] Warn when choosing Allow Overlapping Scores

    model on "audiodevices" example project, except that uses a .ui file
    popup menus are QComboBox without setEditable(true)
*/
    QLabel *outDeviceLabel = new QLabel(tr("Output Device:"));
    outDeviceMenu = new QComboBox();
    initDeviceMenus();
    CHECKED_CONNECT(outDeviceMenu, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AudioTab::conformValuesToSelectedDevice);

    QLabel *samplingRateLabel = new QLabel(tr("Sampling Rate:"));
    samplingRateMenu = new QComboBox();

    QLabel *outChannelsLabel = new QLabel(tr("Output Channels:"));
    outChannelsSpin = new QSpinBox();

    QLabel *bufferSizeLabel = new QLabel(tr("Buffer Size:"));
    bufferSizeMenu = new QComboBox();

    QLabel *numBusesLabel = new QLabel(tr("Internal Buses:"));
    numBusesSpin = new QSpinBox();
    numBusesSpin->setRange(minNumBuses, maxNumBuses);

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(outDeviceLabel, 0, 0);
    mainLayout->addWidget(outDeviceMenu, 0, 1);
    mainLayout->addWidget(samplingRateLabel, 1, 0);
    mainLayout->addWidget(samplingRateMenu, 1, 1);
    mainLayout->addWidget(outChannelsLabel, 2, 0);
    mainLayout->addWidget(outChannelsSpin, 2, 1);
    mainLayout->addWidget(bufferSizeLabel, 3, 0);
    mainLayout->addWidget(bufferSizeMenu, 3, 1);
    mainLayout->addWidget(numBusesLabel, 4, 0);
    mainLayout->addWidget(numBusesSpin, 4, 1);
    setLayout(mainLayout);
}

// Create the input and output device menus.
// For now, using only an output device.
void AudioTab::initDeviceMenus()
{
    outDeviceMenu->clear();
    int devCount = availableOutputDeviceIDs(outputDeviceList);
    for (int i = 0; i < devCount; i++) {
        QString name;
        int result = deviceNameFromID(outputDeviceList[i], name);
        if (result == 0)
           outDeviceMenu->addItem(name);
    }
}

// Set the available values of widgets to match the capabilities
// of the selected device.
void AudioTab::conformValuesToSelectedDevice(int outputDeviceMenuID)
{
    // Set the popup menu to show the menu item just chosen.
    outDeviceMenu->setCurrentIndex(outputDeviceMenuID);

    // Get current output device ID, and set ranges of other items,
    // and possibly selected items, to valid values.
    int deviceID = deviceIDFromName(outDeviceMenu->currentText());

    // Update number of channels
    int maxChans = maxOutputChannelCount(deviceID);
    int curVal = outChannelsSpin->value();
    outChannelsSpin->setRange(1, maxChans);
    outChannelsSpin->setValue(curVal);
    int numChans = outChannelsSpin->value();  // value might have been constrained by widget

    // Update sampling rate
    QVector<int> samplingRates;
    int count = availableSamplingRates(deviceID, numChans, samplingRates);
    if (count > 0) {
        QString curRate = samplingRateMenu->currentText();
        samplingRateMenu->clear();
        for (int i = 0; i < samplingRates.size(); i++) {
            QString s = QString::number(samplingRates.at(i));
            samplingRateMenu->addItem(s);
        }
        int index = samplingRateMenu->findText(curRate);
        if (index == -1)
            index = 0;
        samplingRateMenu->setCurrentIndex(index);
    }
    else
        qDebug("conform: no valid sampling rates for this device (%d chans)", numChans);

    // Update buffer sizes
    QVector<int> bufferSizes;
    count = availableBufferSizes(deviceID, bufferSizes);
    if (count > 0) {
        QString curSize = bufferSizeMenu->currentText();
        bufferSizeMenu->clear();
        for (int i = 0; i < bufferSizes.size(); i++) {
            QString s = QString::number(bufferSizes.at(i));
            bufferSizeMenu->addItem(s);
        }
        int index = bufferSizeMenu->findText(curSize);
        if (index == -1)
            index = 0;
        bufferSizeMenu->setCurrentIndex(index);
    }
    else
        qDebug("conform: no valid buffer sizes for this device");
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

void PreferencesDialog::initFromPreferences(Preferences *prefs)
{

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
    prefs->setAudioBufferSize();
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
    dlog.initFromPreferences(this);
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

