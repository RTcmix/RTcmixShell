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
    setFixedSize(72, 36);
}

void SelectColorButton::changeColor()
{
    QColor newColor = QColorDialog::getColor(color, parentWidget());
    if (newColor.isValid()) {
        setColor(newColor);
        update();   // force paintEvent
    }
}

void SelectColorButton::setColor(const QColor &color)
{
    this->color = color;
    emit colorChanged(this->color);
}

const QColor &SelectColorButton::getColor()
{
    return color;
}

void SelectColorButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);

    // Get rect of QPushButton, and inset it so that it just covers
    // the actual button rect that is sensitive to mouse clicks.
    // Not sure how to get this otherwise. If we don't do this,
    // clicks outside the covered-up button graphic will not do
    // anything, which is very confusing for the user.
    QRect rect = event->rect();
    QPainter painter(this);
    painter.setPen(Qt::darkGray);
    painter.setBrush(QBrush(color));    // comment out to debug rect
    rect.adjust(6, 3, -6, -6);          // these are okay for macOS 14 (Mojave)
    //qDebug() << "rect: " << rect;
    painter.drawRect(rect);
}


//-------------------------------------------------------------------------------

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
    Driver:          [popup menu: names of portaudio APIs] (e.g., CoreAudio, Jack, etc.)
    Input Device:    [popup menu: names from portaudio]
    Output Device:   [popup menu: names from portaudio]
    Sampling Rate:   [popup menu: e.g., 22050, 44100, 48000, 88200, 96000]
    // NB: until RTcmix can support in and out chan counts that differ, we'll
    // just have one field for channels
    Input Channels:  [QSpinBox: 1-16]
    Output Channels: [QSpinBox: 1-16]
    Buffer Size:     [popup menu: e.g., 64, 128, 256, 512, 1024, 2048, 4096]
    Internal Buses:  [QSpinbox: 8-64]

    (outside grid layout)
    [x] Warn when choosing Allow Overlapping Scores

    Values constrained by conformValuesToSelectedDevice().
*/

    // set up action widgets
#ifdef HOST_API
   audioApiMenu = new QComboBox();
#endif

#ifdef USE_INPUT_DEVICE
   inDeviceMenu = new QComboBox();
#endif
    outDeviceMenu = new QComboBox();
    initDeviceMenus();
#ifdef HOST_API
    CHECKED_CONNECT(audioApiMenu, QOverload<int>::of(&QComboBox::activated), this, &AudioTab::conformValuesToSelectedAudioAPI);
#endif
#ifdef USE_INPUT_DEVICE
    CHECKED_CONNECT(inDeviceMenu, QOverload<int>::of(&QComboBox::activated), this, &AudioTab::conformValuesToSelectedInputDevice);
#endif
    CHECKED_CONNECT(outDeviceMenu, QOverload<int>::of(&QComboBox::activated), this, &AudioTab::conformValuesToSelectedOutputDevice);

    samplingRateMenu = new QComboBox();

#ifdef INDEPENDENT_INCHANS
    inChannelsSpin = new QSpinBox();
#endif
    outChannelsSpin = new QSpinBox();

    bufferSizeMenu = new QComboBox();

    numBusesSpin = new QSpinBox();
    numBusesSpin->setRange(minNumBuses, maxNumBuses);

    warnOverlappingScores = new QCheckBox(tr("Warn when choosing Allow Overlapping Scores"));

    // set up layouts

    QGroupBox *audioGroupBox = new QGroupBox(tr("Audio"));
    QFormLayout *audioLayout = new QFormLayout;
#ifdef HOST_API
    audioLayout->addRow(tr("Driver:"), audioApiMenu);
#endif

#ifdef USE_INPUT_DEVICE
    audioLayout->addRow(tr("Input Device:"), inDeviceMenu);
#endif
    audioLayout->addRow(tr("Output Device:"), outDeviceMenu);
    audioLayout->addRow(tr("Sampling Rate:"), samplingRateMenu);
#ifdef INDEPENDENT_INCHANS
    audioLayout->addRow(tr("Input Channels:"), inChannelsSpin);
    audioLayout->addRow(tr("Output Channels:"), outChannelsSpin);
#else
//    audioLayout->addRow(tr("Channels (in and out):"), outChannelsSpin);
    audioLayout->addRow(tr("Channels:"), outChannelsSpin);
#endif
    audioLayout->addRow(tr("Buffer Size:"), bufferSizeMenu);
    audioLayout->addRow(tr("Internal Buses:"), numBusesSpin);
    audioLayout->setHorizontalSpacing(10);  // default appears to be 10 -- too tight
    audioGroupBox->setLayout(audioLayout);

    QGroupBox *scoreGroupBox = new QGroupBox(tr("Score"));
    QVBoxLayout *scoreLayout = new QVBoxLayout;
    scoreLayout->addWidget(warnOverlappingScores);
    scoreGroupBox->setLayout(scoreLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(audioGroupBox);
    mainLayout->addWidget(scoreGroupBox);
    setLayout(mainLayout);
}

void AudioTab::initFromPreferences(Preferences *prefs)
{
    QString str;

#ifdef HOST_API
    // audio API
    PaHostApiIndex apiID = prefs->audioApiID();
    int menuIndex = 0;
    int result = audioApiNameFromId(apiID, str);
    if (result == 0) {
        int idx = audioApiMenu->findText(str);
        if (idx != -1)
            menuIndex = idx;
    }
    audioApiMenu->setCurrentIndex(menuIndex);
    apiID = audioApiIdFromName(audioApiMenu->currentText()); // could've been overridden
    menuIndex = audioApiMenu->currentIndex();
    conformValuesToSelectedAudioAPI(menuIndex);   // so other widget ranges will be set for this device
#endif

#ifdef USE_INPUT_DEVICE
    // input device
    PaDeviceIndex deviceID = prefs->audioInputDeviceID();
    menuIndex = 0;
    result = deviceNameFromId(deviceID, str);
    if (result == 0) {
        int idx = inDeviceMenu->findText(str);
        if (idx != -1)
            menuIndex = idx;
    }
    inDeviceMenu->setCurrentIndex(menuIndex);
    deviceID = deviceIdFromName(inDeviceMenu->currentText()); // could've been overridden
    menuIndex = inDeviceMenu->currentIndex();
    conformValuesToSelectedInputDevice(menuIndex);   // so other widget ranges will be set for this device
#endif

    // output device
    PaDeviceIndex deviceID = prefs->audioOutputDeviceID();
    int menuIndex = 0;
    int result = deviceNameFromId(deviceID, str);
    if (result == 0) {
        int idx = outDeviceMenu->findText(str);
        if (idx != -1)
            menuIndex = idx;
    }
    outDeviceMenu->setCurrentIndex(menuIndex);
    deviceID = deviceIdFromName(outDeviceMenu->currentText()); // could've been overridden
    menuIndex = outDeviceMenu->currentIndex();
    conformValuesToSelectedOutputDevice(menuIndex);   // so other widget ranges will be set for this device

#ifdef INDEPENDENT_INCHANS
    // input channels
    int numChans = prefs->audioNumInputChannels();
    inChannelsSpin->setValue(numChans);
    inChannelsSpin->selectAll();
#endif

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
    else {
        const QString msg = QString(tr("No valid sampling rates for audio device %1\n(initFromPreferences)")).arg(deviceID);
        warnAlert(nullptr, msg);
    }
    str = QString::number(prefs->audioSamplingRate());
    menuIndex = samplingRateMenu->findText(str);
    samplingRateMenu->setCurrentIndex(menuIndex);

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
    else {
        const QString msg = QString(tr("No valid buffer sizes for audio device %1\n(initFromPreferences)")).arg(deviceID);
        warnAlert(nullptr, msg);
    }
    str = QString::number(prefs->audioBufferSize());
    menuIndex = bufferSizeMenu->findText(str);
    bufferSizeMenu->setCurrentIndex(menuIndex);

    // buses
    numBusesSpin->setValue(prefs->audioNumBuses());

    // overlapping scores warning alert
    warnOverlappingScores->setChecked(prefs->audioShowOverlappingScoresWarning());

    initing = false;
}

void AudioTab::applyPreferences(Preferences *prefs)
{
    bool changed = false;

#ifdef HOST_API
    int oldVal = prefs->audioApiID();
    int newVal = audioApiIdFromName(audioApiMenu->currentText());
    prefs->setAudioApiID(newVal);
    if (newVal != oldVal)
        changed = true;
#endif

#ifdef USE_INPUT_DEVICE
    oldVal = prefs->audioInputDeviceID();
    newVal = deviceIdFromName(inDeviceMenu->currentText());
    prefs->setAudioInputDeviceID(newVal);
    if (newVal != oldVal)
        changed = true;
#endif

    int oldVal = prefs->audioOutputDeviceID();
    int newVal = deviceIdFromName(outDeviceMenu->currentText());
    prefs->setAudioOutputDeviceID(newVal);
    if (newVal != oldVal)
        changed = true;

#ifdef INDEPENDENT_INCHANS
    oldVal = prefs->audioNumInputChannels();
    newVal = inChannelsSpin->value();
    prefs->setAudioNumInputChannels(newVal);
    if (newVal != oldVal)
        changed = true;
#endif

    oldVal = prefs->audioNumOutputChannels();
    newVal = outChannelsSpin->value();
    prefs->setAudioNumOutputChannels(newVal);
    if (newVal != oldVal)
        changed = true;
#ifdef USE_INPUT_DEVICE
#ifndef INDEPENDENT_INCHANS
    // set to num output chans
    oldVal = prefs->audioNumInputChannels();
    prefs->setAudioNumInputChannels(newVal);
    if (newVal != oldVal)
        changed = true;
#endif
#endif

    oldVal = prefs->audioSamplingRate();
    newVal = samplingRateMenu->currentText().toInt();
    prefs->setAudioSamplingRate(newVal);
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
void AudioTab::initDeviceMenus()
{
#ifdef HOST_API
    audioApiMenu->clear();
    PaHostApiIndex apiCount = availableAudioApiIDs(audioAPIList);
    for (PaHostApiIndex i = 0; i < apiCount; i++) {
        QString name;
        int result = audioApiNameFromId(audioAPIList[i], name);
        if (result == 0)
            audioApiMenu->addItem(name);
    }
#endif

#ifdef USE_INPUT_DEVICE
    inDeviceMenu->clear();
    PaDeviceIndex devCount = availableInputDeviceIDs(inputDeviceList);
    for (PaDeviceIndex i = 0; i < devCount; i++) {
        QString name;
        int result = deviceNameFromId(inputDeviceList[i], name);
        if (result == 0)
           inDeviceMenu->addItem(name);
    }
#endif

    outDeviceMenu->clear();
    PaDeviceIndex devCount = availableOutputDeviceIDs(outputDeviceList);
    for (PaDeviceIndex i = 0; i < devCount; i++) {
        QString name;
        int result = deviceNameFromId(outputDeviceList[i], name);
        if (result == 0)
           outDeviceMenu->addItem(name);
    }
}

// Set the available values of widgets to match the capabilities
// of the selected output device.
void AudioTab::conformValuesToSelectedOutputDevice(int outputDeviceMenuID)
{
    // Set the popup menu to show the menu item just chosen.
    outDeviceMenu->setCurrentIndex(outputDeviceMenuID);

    // Get current output device ID, and set ranges of other items,
    // and possibly selected items, to valid values.
    int deviceID = deviceIdFromName(outDeviceMenu->currentText());

    // Update number of channels
    int maxChans = maxOutputChannelCount(deviceID);
    int curVal = outChannelsSpin->value();
#ifndef INDEPENDENT_INCHANS
#ifdef USE_INPUT_DEVICE
    int inputDeviceID = deviceIdFromName(inDeviceMenu->currentText());
    int maxInChans = maxInputChannelCount(inputDeviceID);
    if (maxInChans < maxChans)
        maxChans = maxInChans;
#endif
#endif
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
    else {
        const QString msg = QString(tr("No valid sampling rates for audio device %1\n(conformValuesToSelectedOutputDevice)")).arg(deviceID);
        warnAlert(nullptr, msg);
    }

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
    else {
        const QString msg = QString(tr("No valid buffer sizes for audio device %1\n(conformValuesToSelectedOutputDevice)")).arg(deviceID);
        warnAlert(nullptr, msg);
    }
}

// Set the available values of widgets to match the capabilities
// of the selected input device.
void AudioTab::conformValuesToSelectedInputDevice(int inputDeviceMenuID)
{
    // Set the popup menu to show the menu item just chosen.
    inDeviceMenu->setCurrentIndex(inputDeviceMenuID);

    // Get current input device ID, and set ranges of other items,
    // and possibly selected items, to valid values.
    int deviceID = deviceIdFromName(inDeviceMenu->currentText());

#ifdef INDEPENDENT_INCHANS
    // Update number of channels
    int maxChans = maxInputChannelCount(deviceID);
    int curVal = inChannelsSpin->value();
    inChannelsSpin->setRange(1, maxChans);
    inChannelsSpin->setValue(curVal);
    int numChans = inChannelsSpin->value();  // value might have been constrained by widget
#else
    // force num channels to be min(numinchans, numoutchans)
    int maxChans = maxInputChannelCount(deviceID);
    int outputDeviceID = deviceIdFromName(outDeviceMenu->currentText());
    int maxOutChans = maxOutputChannelCount(outputDeviceID);
    int curVal = outChannelsSpin->value();
    outChannelsSpin->setRange(1, qMin(maxChans, maxOutChans));
    outChannelsSpin->setValue(curVal);
#endif

// **** FIXME: Not sure what should happen if srates disagree for input/output devs, buf sizes

#ifdef NOTYET // availableSamplingRates doesn't work for input devices now
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
    else {
        const QString msg = QString(tr("No valid sampling rates for audio device %1\n(conformValuesToSelectedInputDevice)")).arg(deviceID);
        warnAlert(nullptr, msg);
    }

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
    else {
        const QString msg = QString(tr("No valid buffer sizes for audio device %1\n(conformValuesToSelectedInputDevice)")).arg(deviceID);
        warnAlert(nullptr, msg);
    }
#endif
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
    Editor:
        [QFormLayout with three rows]
        Font Family: [QFontComboBox]
        Font Size:   [QComboBox]
        Tab width:   [QSpinBox: 1-8]
    Log:
        [QVBoxLayout with 2-row QFormLayout and the checkbox]
        Font Family: [QFontComboBox]
        Font Size:   [QComboBox]
        [x] Link Family
    (Link changes Log font family in sync with editor font.)
*/

    mainWindow = getMainWindow();

    // set up action widgets

    editorFontFamilyMenu = new QFontComboBox;
    //The filters are too slow. The writing system filter puts weird chars into the menu.
    //editorFontFamilyMenu->setFontFilters(QFontComboBox::ScalableFonts /* | QFontComboBox::MonospacedFonts */);
    //editorFontFamilyMenu->setWritingSystem(QFontDatabase::Latin);
    CHECKED_CONNECT(editorFontFamilyMenu, QOverload<const QString &>::of(&QComboBox::textActivated), mainWindow, &MainWindow::editorFontFamily);

    editorFontSizeMenu = new QComboBox;
    static const int fontSizes[] = { 9, 10, 11, 12, 13, 14, 15, 16, 18, 20, 22, 24, 28, 32, 40, 48, 64, 72, 96, 128, 192 };
    const int len = sizeof(fontSizes) / sizeof(fontSizes[0]);
    for (int i = 0; i < len; i++)
        editorFontSizeMenu->addItem(QString::number(fontSizes[i]));
    CHECKED_CONNECT(editorFontSizeMenu, QOverload<const QString &>::of(&QComboBox::textActivated), mainWindow, &MainWindow::editorFontSize);

    editorTabWidthSpin = new QSpinBox;
    editorTabWidthSpin->setRange(1, 8);
    CHECKED_CONNECT(editorTabWidthSpin, QOverload<int>::of(&QSpinBox::valueChanged), mainWindow, &MainWindow::editorTabWidth);

    logFontFamilyMenu = new QFontComboBox;
    CHECKED_CONNECT(logFontFamilyMenu, QOverload<const QString &>::of(&QComboBox::textActivated), mainWindow, &MainWindow::logFontFamily);

    logFontSizeMenu = new QComboBox;
    for (int i = 0; i < len; i++)
        logFontSizeMenu->addItem(QString::number(fontSizes[i]));
    CHECKED_CONNECT(logFontSizeMenu, QOverload<const QString &>::of(&QComboBox::textActivated), mainWindow, &MainWindow::logFontSize);

    logLinkFamily = new QCheckBox(tr("Always set log font family to editor font"));
    CHECKED_CONNECT(logLinkFamily, &QCheckBox::clicked, this, &EditorTab::logLinkFamilyClicked);

    // set up layouts

    QGroupBox *editorGroupBox = new QGroupBox(tr("Editor"));
    QFormLayout *editorLayout = new QFormLayout;
    editorLayout->addRow(tr("Font Family:"), editorFontFamilyMenu);
    editorLayout->addRow(tr("Font Size:"), editorFontSizeMenu);
    editorLayout->addRow(tr("Tab Width:"), editorTabWidthSpin);
    editorLayout->setHorizontalSpacing(8);
    editorGroupBox->setLayout(editorLayout);

    QGroupBox *logGroupBox = new QGroupBox(tr("Log"));
    QVBoxLayout *logTopLayout = new QVBoxLayout;
    QFormLayout *logFontLayout = new QFormLayout;
    logFontLayout->addRow(tr("Font Family:"), logFontFamilyMenu);
    logFontLayout->addRow(tr("Font Size:"), logFontSizeMenu);
    logFontLayout->setHorizontalSpacing(8);
    logTopLayout->addLayout(logFontLayout);
    logTopLayout->addWidget(logLinkFamily);
    logGroupBox->setLayout(logTopLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(editorGroupBox);
    mainLayout->addWidget(logGroupBox);
    setLayout(mainLayout);
}

void EditorTab::initFromPreferences(Preferences *prefs)
{
    prevEditorFontFamily = prefs->editorFontFamily();
    int index = editorFontFamilyMenu->findText(QFontInfo(prevEditorFontFamily).family());
    editorFontFamilyMenu->setCurrentIndex(index);

    prevEditorFontSize = prefs->editorFontSize();
    index = editorFontSizeMenu->findText(QString::number(prevEditorFontSize));
    editorFontSizeMenu->setCurrentIndex(index);

    prevEditorTabWidth = prefs->editorTabWidth();
    editorTabWidthSpin->setValue(prevEditorTabWidth);

    prevLogFontFamily = prefs->logFontFamily();
    index = logFontFamilyMenu->findText(QFontInfo(prevLogFontFamily).family());
    logFontFamilyMenu->setCurrentIndex(index);

    prevLogFontSize = prefs->logFontSize();
    index = logFontSizeMenu->findText(QString::number(prevLogFontSize));
    logFontSizeMenu->setCurrentIndex(index);

    prevLogLinkFamily = prefs->logLinkFamily();
    logLinkFamily->setChecked(prevLogLinkFamily);
    logLinkFamilyClicked(prevLogLinkFamily);
}

void EditorTab::applyPreferences(Preferences *prefs)
{
    // These changes have already been made in MainWindow, but
    // they need to go also into prefs.
    prefs->setEditorFontFamily(editorFontFamilyMenu->currentText());
    prefs->setEditorFontSize(editorFontSizeMenu->currentText().toInt());
    prefs->setEditorTabWidth(editorTabWidthSpin->value());
    prefs->setLogFontFamily(editorFontFamilyMenu->currentText());
    prefs->setLogFontSize(logFontSizeMenu->currentText().toInt());
    prefs->setLogLinkFamily(logLinkFamily->isChecked());
}

void EditorTab::cancelPreferences(Preferences *prefs)
{
    mainWindow->editorFontFamily(prevEditorFontFamily);
    prefs->setEditorFontFamily(prevEditorFontFamily);

    mainWindow->editorFontSize(QString::number(prevEditorFontSize));
    prefs->setEditorFontSize(prevEditorFontSize);

    mainWindow->editorTabWidth(prevEditorTabWidth);
    prefs->setEditorTabWidth(prevEditorTabWidth);

    mainWindow->logFontFamily(prevLogFontFamily);
    prefs->setLogFontFamily(prevLogFontFamily);

    mainWindow->logFontSize(QString::number(prevLogFontSize));
    prefs->setLogFontSize(prevLogFontSize);

    prefs->setLogLinkFamily(prevLogLinkFamily);
}

void EditorTab::linkedEditorFontFamilyChanged(const QString &family)
{
    int index = editorFontFamilyMenu->currentIndex();
    logFontFamilyMenu->setCurrentIndex(index);
    mainWindow->logFontFamily(family);
}

void EditorTab::logLinkFamilyClicked(bool isChecked)
{
    if (isChecked) {
        logFontFamilyMenu->setEnabled(false);
        CHECKED_CONNECT(editorFontFamilyMenu, QOverload<const QString &>::of(&QComboBox::textActivated), this, &EditorTab::linkedEditorFontFamilyChanged);
    }
    else {
        disconnect(editorFontFamilyMenu, QOverload<const QString &>::of(&QComboBox::textActivated), this, &EditorTab::linkedEditorFontFamilyChanged);
        logFontFamilyMenu->setEnabled(true);
    }
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
    CHECKED_CONNECT(xableHighlighting, &QCheckBox::clicked, this, &SyntaxHighlightingTab::setHighlighting);

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
    colorLayout->setHorizontalSpacing(10);
    colorGroupBox->setLayout(colorLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(behaviorGroupBox);
    mainLayout->addWidget(colorGroupBox);
    setLayout(mainLayout);

//    highlighter->dumpRules();
}

void SyntaxHighlightingTab::initFromPreferences(Preferences *prefs)
{
    prevDoSyntaxHighlighting = prefs->editorDoSyntaxHighlighting();
    xableHighlighting->setChecked(prevDoSyntaxHighlighting);

    prevCommentColor = prefs->editorCommentColor();
    commentButton->setColor(prevCommentColor);

    prevFunctionColor = prefs->editorFunctionColor();
    functionButton->setColor(prevFunctionColor);

    prevNumberColor = prefs->editorNumberColor();
    numberButton->setColor(prevNumberColor);

    prevReservedColor = prefs->editorReservedColor();
    reservedButton->setColor(prevReservedColor);

    prevStringColor = prefs->editorStringColor();
    stringButton->setColor(prevStringColor);

    prevUnusedColor = prefs->editorUnusedColor();
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

static int prevTabIndex = 0;

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

    tabWidget->setCurrentIndex(prevTabIndex);

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

    prevTabIndex = tabWidget->currentIndex();
}

void PreferencesDialog::cancelPreferences(Preferences *prefs)
{
#ifdef GENERALTAB
    generalTab->cancelPreferences(prefs);
#endif
    audioTab->cancelPreferences(prefs);
    editorTab->cancelPreferences(prefs);
    syntaxHighlightingTab->cancelPreferences(prefs);

    prevTabIndex = tabWidget->currentIndex();
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
        warnAlert(nullptr, tr("Access error reading preferences file"));
    }
    else if (settings->status() == QSettings::FormatError) {
        warnAlert(nullptr, tr("Preferences file appears to be corrupted. Using defaults."));
    }
}

void Preferences::dump()
{
    // If prefs seem to stick around on macOS, even after you've deleted the
    // relevant files, then you need to force macOS to dump its cache of
    // prefs. Quit the user's cfprefsd process in Activity Monitor.

    QStringList keys;

    qDebug() << "mainwindow" << "..............................................";
    settings->beginGroup("mainwindow");
    keys = settings->allKeys();
    foreach (const QString &key, keys) {
        qDebug() << "key:" << key << "val:" << settings->value(key);
    }
    settings->endGroup();

    qDebug() << "audio" << "...................................................";
    settings->beginGroup("audio");
    keys = settings->allKeys();
    foreach (const QString &key, keys) {
        qDebug() << "key:" << key << "val:" << (settings->value(key)).toString();
    }
    settings->endGroup();

    qDebug() << "editor" << "..................................................";
    settings->beginGroup("editor");
    keys = settings->allKeys();
    foreach (const QString &key, keys) {
        qDebug() << "key:" << key << "val:" << (settings->value(key)).toString();
    }
    settings->endGroup();

    qDebug() << "log" << ".....................................................";
    settings->beginGroup("log");
    keys = settings->allKeys();
    foreach (const QString &key, keys) {
        qDebug() << "key:" << key << "val:" << (settings->value(key)).toString();
    }
    settings->endGroup();
}
