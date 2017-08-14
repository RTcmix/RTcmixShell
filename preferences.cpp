#include <QList>
#include <QtWidgets>
#include <QtDebug>

#include "audio.h"
#include "highlighter.h"
#include "mainwindow.h"
#include "preferences.h"
#include "utils.h"

// Preferences dialog code based on "tabdialog" example in Qt5.

const int minNumBuses = 8;
const int maxNumBuses = 96;


// SelectColorButton adapted from jpo38 at https://stackoverflow.com/questions/18257281/qt-color-picker-widget.
SelectColorButton::SelectColorButton(QWidget *parent)
{
    Q_UNUSED(parent);
    CHECKED_CONNECT(this, &QPushButton::clicked, this, &SelectColorButton::changeColor);
}

void SelectColorButton::updateColor()
{
    setStyleSheet("background-color: " + color.name());
}

void SelectColorButton::changeColor()
{
    QColor newColor = QColorDialog::getColor(color, parentWidget());
    if (newColor != color)
        setColor(newColor);
}

void SelectColorButton::setColor(const QColor &color)
{
    this->color = color;
    updateColor();
    emit colorChanged(this->color);
}

const QColor &SelectColorButton::getColor()
{
    return color;
}


#ifdef GENERALTAB
GeneralTab::GeneralTab(QWidget *parent) : QWidget(parent)
{
}

void GeneralTab::initFromPreferences(Preferences *prefs)
{
    Q_UNUSED(prefs);
}

void GeneralTab::applyPreferences(Preferences *prefs)
{
    Q_UNUSED(prefs);
}

void GeneralTab::cancelPreferences(Preferences *prefs)
{

}

#endif


//-------------------------------------------------------------------------------

AudioTab::AudioTab(QWidget *parent) : QWidget(parent)
{
    initing = true;

/* QGridLayout:
    Output Device:   [popup menu: names from portaudio]
    Sampling Rate:   [popup menu: e.g., 22050, 44100, 48000, 88200, 96000]
    Output Channels: [QSpinBox: 1-16]
    Buffer Size:     [popup menu: e.g., 64, 128, 256, 512, 1024, 2048, 4096]
    Internal Buses:  [QSpinbox: 8-64]

    (outside grid layout)
    [x] Warn when choosing Allow Overlapping Scores

    Values constrained by conformValuesToSelectedDevice().
*/
    QLabel *outDeviceLabel = new QLabel(tr("Output Device:"));
    outDeviceMenu = new QComboBox();
    initDeviceMenus();
    CHECKED_CONNECT(outDeviceMenu, QOverload<int>::of(&QComboBox::activated), this, &AudioTab::conformValuesToSelectedDevice);

    QLabel *samplingRateLabel = new QLabel(tr("Sampling Rate:"));
    samplingRateMenu = new QComboBox();

    QLabel *outChannelsLabel = new QLabel(tr("Output Channels:"));
    outChannelsSpin = new QSpinBox();

    QLabel *bufferSizeLabel = new QLabel(tr("Buffer Size:"));
    bufferSizeMenu = new QComboBox();

    QLabel *numBusesLabel = new QLabel(tr("Internal Buses:"));
    numBusesSpin = new QSpinBox();
    numBusesSpin->setRange(minNumBuses, maxNumBuses);

    warnOverlappingScores = new QCheckBox(tr("Warn when choosing Allow Overlapping Scores"));

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
    mainLayout->addWidget(warnOverlappingScores, 5, 0, 1, 2);
    setLayout(mainLayout);
}

void AudioTab::initFromPreferences(Preferences *prefs)
{
    // output device
    int deviceID = prefs->audioOutputDeviceID();
    QString str;
    int index = 0;
    int result = deviceNameFromID(deviceID, str);
    if (result == 0) {
        int idx = outDeviceMenu->findText(str);
        if (idx != -1)
            index = idx;
    }
    outDeviceMenu->setCurrentIndex(index);
    deviceID = deviceIDFromName(outDeviceMenu->currentText()); // could've been overridden

    // output channels
    int numChans = prefs->audioNumOutputChannels();
    outChannelsSpin->setValue(numChans);
    outChannelsSpin->selectAll();

    // sampling rate
    QVector<int> samplingRates;
    int count = availableSamplingRates(deviceID, numChans, samplingRates);
    if (count > 0) {
        samplingRateMenu->clear();
        for (int i = 0; i < samplingRates.size(); i++) {
            QString s = QString::number(samplingRates.at(i));
            samplingRateMenu->addItem(s);
        }
    }
    else
        qDebug("init: no valid sampling rates for this device (%d chans)", numChans);
    str = QString::number(prefs->audioSamplingRate());
    index = samplingRateMenu->findText(str);
    samplingRateMenu->setCurrentIndex(index);

    // buffer size
    QVector<int> bufferSizes;
    count = availableBufferSizes(deviceID, bufferSizes);
    if (count > 0) {
        bufferSizeMenu->clear();
        for (int i = 0; i < bufferSizes.size(); i++) {
            QString s = QString::number(bufferSizes.at(i));
            bufferSizeMenu->addItem(s);
        }
    }
    else
        qDebug("init: no valid buffer sizes for this device");
    str = QString::number(prefs->audioBufferSize());
    index = bufferSizeMenu->findText(str);
    bufferSizeMenu->setCurrentIndex(index);

    // buses
    numBusesSpin->setValue(prefs->audioNumBuses());

    // overlapping scores warning alert
    warnOverlappingScores->setChecked(prefs->audioShowOverlappingScoresWarning());

    initing = false;
}

void AudioTab::applyPreferences(Preferences *prefs)
{
    bool changed = false;

    int oldVal = prefs->audioOutputDeviceID();
    int newVal = deviceIDFromName(outDeviceMenu->currentText());
    prefs->setAudioOutputDeviceID(newVal);
    if (newVal != oldVal)
        changed = true;

    oldVal = prefs->audioSamplingRate();
    newVal = samplingRateMenu->currentText().toInt();
    prefs->setAudioSamplingRate(newVal);
    if (newVal != oldVal)
        changed = true;

    oldVal = prefs->audioNumOutputChannels();
    newVal = outChannelsSpin->value();
    prefs->setAudioNumOutputChannels(newVal);
    if (newVal != oldVal)
        changed = true;

    oldVal = prefs->audioBufferSize();
    newVal = bufferSizeMenu->currentText().toInt();
    prefs->setAudioBufferSize(newVal);
    if (newVal != oldVal)
        changed = true;

    oldVal = prefs->audioNumBuses();
    newVal = numBusesSpin->value();
    prefs->setAudioNumBuses(newVal);
    if (newVal != oldVal)
        changed = true;

    prefs->setAudioShowOverlappingScoresWarning(warnOverlappingScores->isChecked());

    if (changed) {
        MainWindow *mw = getMainWindow();
        if (mw)
            mw->reinitializeAudio();
    }
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

void AudioTab::cancelPreferences(Preferences *prefs)
{
    // We don't apply audio prefs while in the dialog, so this method isn't used.
    Q_UNUSED(prefs);
}


//-------------------------------------------------------------------------------

EditorTab::EditorTab(QWidget *parent) : QWidget(parent)
{
/* Layout:
    Font:                    Size:
    Tab width: [QSpinBox: 1-8]
    Log Font Size:
    (font and size combo boxes as in current toolbar)
*/
}

void EditorTab::initFromPreferences(Preferences *prefs)
{
    Q_UNUSED(prefs);
}

void EditorTab::applyPreferences(Preferences *prefs)
{
    Q_UNUSED(prefs);
}

void EditorTab::cancelPreferences(Preferences *prefs)
{
    Q_UNUSED(prefs);
}


//-------------------------------------------------------------------------------

SyntaxHighlightingTab::SyntaxHighlightingTab(QWidget *parent)
    : QWidget(parent)
    , highlighter(NULL)
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
     (clicking swatches invokes QColorDialog)
*/
    // get a pointer to the syntax highlighter

    MainWindow *mw = getMainWindow();
    if (mw)
        highlighter = mw->getHighlighter();

    // set up action widgets

    xableHighlighting = new QCheckBox(tr("Enable Syntax Highlighting"));
    CHECKED_CONNECT(xableHighlighting, &QCheckBox::toggled, this, &SyntaxHighlightingTab::setHighlighting);

    commentButton = new SelectColorButton();
    CHECKED_CONNECT(commentButton, &SelectColorButton::colorChanged, this, &SyntaxHighlightingTab::setCommentColor);

    functionButton = new SelectColorButton();
    CHECKED_CONNECT(functionButton, &SelectColorButton::colorChanged, this, &SyntaxHighlightingTab::setFunctionColor);

    numberButton = new SelectColorButton();
    CHECKED_CONNECT(numberButton, &SelectColorButton::colorChanged, this, &SyntaxHighlightingTab::setNumberColor);

    reservedButton = new SelectColorButton();
    CHECKED_CONNECT(reservedButton, &SelectColorButton::colorChanged, this, &SyntaxHighlightingTab::setReservedColor);

    stringButton = new SelectColorButton();
    CHECKED_CONNECT(stringButton, &SelectColorButton::colorChanged, this, &SyntaxHighlightingTab::setStringColor);

    unusedButton = new SelectColorButton();
    CHECKED_CONNECT(unusedButton, &SelectColorButton::colorChanged, this, &SyntaxHighlightingTab::setUnusedColor);

    // set up layouts

    QGroupBox *behaviorGroupBox = new QGroupBox(tr("Behavior"));
    QVBoxLayout *behaviorLayout = new QVBoxLayout;
    behaviorLayout->addWidget(xableHighlighting);
    behaviorGroupBox->setLayout(behaviorLayout);

    QGroupBox *colorGroupBox = new QGroupBox(tr("Colors"));
    QFormLayout *colorLayout = new QFormLayout;
    colorLayout->addRow(tr("Comments:"), commentButton);
    colorLayout->addRow(tr("Functions:"), functionButton);
    colorLayout->addRow(tr("Numbers:"), numberButton);
    colorLayout->addRow(tr("Reserved words:"), reservedButton);
    colorLayout->addRow(tr("Strings:"), stringButton);
    colorLayout->addRow(tr("Unused commands:"), unusedButton);
    colorLayout->setHorizontalSpacing(20);  // default appears to be 10 -- too tight
    colorGroupBox->setLayout(colorLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(behaviorGroupBox);
    mainLayout->addWidget(colorGroupBox);
    setLayout(mainLayout);


    highlighter->dumpRules();
}

void SyntaxHighlightingTab::initFromPreferences(Preferences *prefs)
{
    prevDoSyntaxHighlighting = prefs->editorDoSyntaxHighlighting();
    xableHighlighting->setChecked(prevDoSyntaxHighlighting);

    prevCommentColor = prefs->editorCommentColor();
    prevFunctionColor = prefs->editorFunctionColor();
    prevNumberColor = prefs->editorNumberColor();
    prevReservedColor = prefs->editorReservedColor();
    prevStringColor = prefs->editorStringColor();
    prevUnusedColor = prefs->editorUnusedColor();

    commentButton->setColor(prevCommentColor);
    functionButton->setColor(prevFunctionColor);
    numberButton->setColor(prevNumberColor);
    reservedButton->setColor(prevReservedColor);
    stringButton->setColor(prevStringColor);
    unusedButton->setColor(prevUnusedColor);
}

void SyntaxHighlightingTab::applyPreferences(Preferences *prefs)
{
    // These changes have already been made in highlighter, but
    // they need to go also into prefs.
    prefs->setEditorDoSyntaxHighlighting(xableHighlighting->isChecked());
    prefs->setEditorCommentColor(commentButton->getColor());
    prefs->setEditorFunctionColor(functionButton->getColor());
    prefs->setEditorNumberColor(numberButton->getColor());
    prefs->setEditorReservedColor(reservedButton->getColor());
    prefs->setEditorStringColor(stringButton->getColor());
    prefs->setEditorUnusedColor(unusedButton->getColor());
}

void SyntaxHighlightingTab::cancelPreferences(Preferences *prefs)
{
    setHighlighting(prevDoSyntaxHighlighting);
    prefs->setEditorDoSyntaxHighlighting(prevDoSyntaxHighlighting);

    setCommentColor(prevCommentColor);
    prefs->setEditorCommentColor(prevCommentColor);

    setFunctionColor(prevFunctionColor);
    prefs->setEditorFunctionColor(prevFunctionColor);

    setNumberColor(prevNumberColor);
    prefs->setEditorNumberColor(prevNumberColor);

    setReservedColor(prevReservedColor);
    prefs->setEditorReservedColor(prevReservedColor);

    setStringColor(prevStringColor);
    prefs->setEditorStringColor(prevStringColor);

    setUnusedColor(prevUnusedColor);
    prefs->setEditorUnusedColor(prevUnusedColor);

    if (highlighter)
        highlighter->rehighlight();
}

void SyntaxHighlightingTab::setHighlighting(bool checked)
{
    if (highlighter) {
        highlighter->xableAllRules(checked);
        highlighter->rehighlight();
    }
}

void SyntaxHighlightingTab::setCommentColor(QColor color)
{
    if (highlighter)
        highlighter->setRuleColor(Highlighter::CommentRule, color);
}

void SyntaxHighlightingTab::setFunctionColor(QColor color)
{
    if (highlighter)
        highlighter->setRuleColor(Highlighter::FunctionRule, color);
}

void SyntaxHighlightingTab::setNumberColor(QColor color)
{
    if (highlighter)
        highlighter->setRuleColor(Highlighter::NumberRule, color);
}

void SyntaxHighlightingTab::setReservedColor(QColor color)
{
    if (highlighter)
        highlighter->setRuleColor(Highlighter::ReservedRule, color);
}

void SyntaxHighlightingTab::setStringColor(QColor color)
{
    if (highlighter)
        highlighter->setRuleColor(Highlighter::StringRule, color);
}

void SyntaxHighlightingTab::setUnusedColor(QColor color)
{
    if (highlighter)
        highlighter->setRuleColor(Highlighter::UnusedRule, color);
}


//-------------------------------------------------------------------------------

PreferencesDialog::PreferencesDialog(QWidget *parent)
    : QDialog(parent)
{
    tabWidget = new QTabWidget;
#ifdef GENERALTAB
    generalTab = new GeneralTab();
#endif
    audioTab = new AudioTab();
    editorTab = new EditorTab();
    syntaxHighlightingTab = new SyntaxHighlightingTab();

#ifdef GENERALTAB
    tabWidget->addTab(generalTab, tr("General"));
#endif
    tabWidget->addTab(audioTab, tr("Audio"));
    tabWidget->addTab(editorTab, tr("Editor"));
    tabWidget->addTab(syntaxHighlightingTab, tr("Syntax Highlighting"));

    okCancelButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    CHECKED_CONNECT(okCancelButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    CHECKED_CONNECT(okCancelButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(okCancelButtonBox);
    setLayout(mainLayout);

    this->layout()->setSizeConstraint(QLayout::SetFixedSize);

    setWindowTitle(tr("RTcmixShell Preferences"));
}

void PreferencesDialog::initFromPreferences(Preferences *prefs)
{
#ifdef GENERALTAB
    generalTab->initFromPreferences(prefs);
#endif
    audioTab->initFromPreferences(prefs);
    editorTab->initFromPreferences(prefs);
    syntaxHighlightingTab->initFromPreferences(prefs);
}

void PreferencesDialog::applyPreferences(Preferences *prefs)
{
#ifdef GENERALTAB
    generalTab->applyPreferences(prefs);
#endif
    audioTab->applyPreferences(prefs);
    editorTab->applyPreferences(prefs);
    syntaxHighlightingTab->applyPreferences(prefs);

#ifdef NOTYET
    prefs->setEditorFontName();
    prefs->setEditorFontSize();
    prefs->setEditorTabWidth();
    prefs->setEditorShowLineNumbers();
    prefs->setLogFontName();
    prefs->setLogFontSize();
#endif
}

void PreferencesDialog::cancelPreferences(Preferences *prefs)
{
#ifdef GENERALTAB
    generalTab->cancelPreferences(prefs);
#endif
    audioTab->cancelPreferences(prefs);
    editorTab->cancelPreferences(prefs);
    syntaxHighlightingTab->cancelPreferences(prefs);
}


//===============================================================================

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
    else
        dlog.cancelPreferences(this);
}

void Preferences::reportError()
{
    if (settings->status() == QSettings::AccessError) {
        qDebug("Access error reading settings file");
    }
    else if (settings->status() == QSettings::FormatError) {
        qDebug("Your settings file is corrupted; using defaults.");
    }
}

