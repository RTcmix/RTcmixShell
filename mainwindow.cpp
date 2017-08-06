
#include <QtWidgets>
#include <QtDebug>

#include "audio.h"
#include "editor.h"
#include "highlighter.h"
#include "rtcmixlogview.h"
#include "mainwindow.h"
#include "RTcmix_API.h"
#include "utils.h"

#ifdef Q_OS_MAC
const QString rsrcPath = ":/images/mac";
#else
const QString rsrcPath = ":/images/win";
#endif

const int logFontSize = 12;
const int scoreFinishedTimerInterval = 100; // msec

void rtcmixFinishedCallback(long long frameCount, void *inContext);


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , firstFileDialog(true)
{
#ifdef Q_OS_OSX
    setUnifiedTitleAndToolBarOnMac(true);
#endif
    setWindowTitle(QCoreApplication::applicationName());

    audio = new Audio;

    createEditors();
    rtcmixLogView = new RTcmixLogView(this);
    createVerticalSplitter();

    createActions();
    createMenus();
    createToolbars();

    setDefaultFont();

    setAcceptDrops(true);

    RTcmix_setFinishedCallback(rtcmixFinishedCallback, this);
    scoreFinishedTimer = new QTimer(this);
    CHECKED_CONNECT(scoreFinishedTimer, SIGNAL(timeout()), this, SLOT(checkScoreFinished()));
    setScorePlayMode(); // defaults to Exclusive, because menu action initially unchecked

    curEditor->setFocus();
}

void MainWindow::createActions()
{
    createFileActions();
    createEditActions();
    createScoreActions();
    createTextActions();
}

// NB: editor must exist before this is called
void MainWindow::createFileActions()
{
    actionNewFile = new QAction(tr("&New"), this);
    actionNewFile->setShortcut(QKeySequence::New);
    actionNewFile->setStatusTip(tr("Create a new score"));
    CHECKED_CONNECT(actionNewFile, &QAction::triggered, this, &MainWindow::fileNew);

    actionOpenFile = new QAction(tr("&Open"), this);
    actionOpenFile->setShortcut(QKeySequence::Open);
    actionOpenFile->setStatusTip(tr("Open an existing score file"));
    CHECKED_CONNECT(actionOpenFile, &QAction::triggered, this, &MainWindow::fileOpen);

    actionSaveFile = new QAction(tr("&Save"), this);
    actionSaveFile->setShortcut(QKeySequence::Save);
    actionSaveFile->setStatusTip(tr("Save the edited score to disk"));
    actionSaveFile->setEnabled(curEditor->document()->isModified());
    CHECKED_CONNECT(actionSaveFile, &QAction::triggered, this, &MainWindow::fileSave);
    CHECKED_CONNECT(curEditor->document(), &QTextDocument::modificationChanged, actionSaveFile, &QAction::setEnabled);

    actionSaveFileAs = new QAction(tr("Save &As..."), this);
    actionSaveFileAs->setShortcut(QKeySequence::SaveAs);
    actionSaveFileAs->setStatusTip(tr("Save the edited score to disk"));
    CHECKED_CONNECT(actionSaveFileAs, &QAction::triggered, this, &MainWindow::fileSaveAs);

    actionQuit = new QAction(tr("&Quit"), this);
    actionQuit->setShortcut(Qt::CTRL + Qt::Key_Q);
    CHECKED_CONNECT(actionQuit, &QAction::triggered, this, &QWidget::close);
}

// NB: editor must exist before this is called
void MainWindow::createEditActions()
{
    actionUndo = new QAction(tr("&Undo"), this);
    actionUndo->setShortcut(QKeySequence::Undo);
    actionUndo->setStatusTip(tr("Undo the last operation"));
    actionUndo->setEnabled(curEditor->document()->isUndoAvailable());
    CHECKED_CONNECT(actionUndo, &QAction::triggered, curEditor, &QTextEdit::undo);
    CHECKED_CONNECT(curEditor->document(), &QTextDocument::undoAvailable, actionUndo, &QAction::setEnabled);

    actionRedo = new QAction(tr("&Redo"), this);
    actionRedo->setShortcut(QKeySequence::Redo);
    actionRedo->setStatusTip(tr("Redo the last operation"));
    actionRedo->setEnabled(curEditor->document()->isRedoAvailable());
    CHECKED_CONNECT(actionRedo, &QAction::triggered, curEditor, &QTextEdit::redo);
    CHECKED_CONNECT(curEditor->document(), &QTextDocument::redoAvailable, actionRedo, &QAction::setEnabled);

    actionCut = new QAction(tr("Cu&t"), this);
    actionCut->setShortcut(QKeySequence::Cut);
    actionCut->setStatusTip(tr("Cut the current selection to the clipboard"));
    actionCut->setEnabled(false);
    CHECKED_CONNECT(actionCut, &QAction::triggered, curEditor, &QTextEdit::cut);

    actionCopy = new QAction(tr("&Copy"), this);
    actionCopy->setShortcut(QKeySequence::Copy);
    actionCopy->setStatusTip(tr("Copy the current selection to the clipboard"));
    actionCopy->setEnabled(false);
    CHECKED_CONNECT(actionCopy, &QAction::triggered, curEditor, &QTextEdit::copy);

    actionPaste = new QAction(tr("&Paste"), this);
    actionPaste->setShortcut(QKeySequence::Paste);
    actionPaste->setStatusTip(tr("Paste the clipboard into the current selection"));
    CHECKED_CONNECT(actionPaste, &QAction::triggered, curEditor, &QTextEdit::paste);

    CHECKED_CONNECT(QApplication::clipboard(), &QClipboard::dataChanged, this, &MainWindow::clipboardDataChanged);
}

void MainWindow::createScoreActions()
{
    actionPlay = new QAction(tr("&Play"), this);
    actionPlay->setShortcut(Qt::CTRL + Qt::Key_P);
    actionPlay->setStatusTip(tr("Play the score"));
    CHECKED_CONNECT(actionPlay, &QAction::triggered, this, &MainWindow::playScore);

    actionStop = new QAction(tr("&Stop"), this);
    actionStop->setShortcut(Qt::CTRL + Qt::Key_Period);
    actionStop->setStatusTip(tr("Stop playing the score"));
    actionStop->setEnabled(false);
    CHECKED_CONNECT(actionStop, &QAction::triggered, this, &MainWindow::stopScore);

#ifdef NOTYET
    const QIcon recordIcon = QIcon::fromTheme("media-record", QIcon(rsrcPath + "/record.png"));
    actionRecord = new QAction(recordIcon, tr("&Record"), this);
    actionRecord->setShortcut(Qt::CTRL + Qt::Key_R);
    actionRecord->setStatusTip(tr("Record the sound that's playing to a sound file"));
    CHECKED_CONNECT(actionRecord, &QAction::triggered, this, &MainWindow::record);
#endif

    actionAllowOverlappingScores = new QAction(tr("Allow Overlapping Scores"), this);
    actionAllowOverlappingScores->setStatusTip(tr("Permit one score to be played while another one is playing"));
    actionAllowOverlappingScores->setCheckable(true);
    CHECKED_CONNECT(actionAllowOverlappingScores, &QAction::triggered, this, &MainWindow::setScorePlayMode);

    actionClearLog = new QAction(tr("&Clear"), this);
    actionClearLog->setShortcut(Qt::CTRL + Qt::Key_B);
    actionClearLog->setStatusTip(tr("Clear the score report area at the bottom of the window"));
    CHECKED_CONNECT(actionClearLog, &QAction::triggered, rtcmixLogView, &RTcmixLogView::clearLog);
}

void MainWindow::createTextActions()
{
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(actionNewFile);
    fileMenu->addAction(actionOpenFile);
    fileMenu->addSeparator();
    fileMenu->addAction(actionSaveFile);
    fileMenu->addAction(actionSaveFileAs);
    fileMenu->addSeparator();
    fileMenu->addAction(actionQuit);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(actionUndo);
    editMenu->addAction(actionRedo);
    editMenu->addSeparator();
    editMenu->addAction(actionCut);
    editMenu->addAction(actionCopy);
    editMenu->addAction(actionPaste);

    scoreMenu = menuBar()->addMenu(tr("&Score"));
    scoreMenu->addAction(actionPlay);
    scoreMenu->addAction(actionStop);
#ifdef NOTYET
    scoreMenu->addAction(actionRecord);
#endif
    scoreMenu->addSeparator();
    scoreMenu->addAction(actionAllowOverlappingScores);
    scoreMenu->addAction(actionClearLog);

    QMenu *helpMenu = menuBar()->addMenu(tr("Help"));
    helpMenu->addAction(tr("About"), this, &MainWindow::about);
    helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
}

void MainWindow::createToolbars()
{
    setToolButtonStyle(Qt::ToolButtonFollowStyle);  // necessary?

    QToolBar *tb = addToolBar(tr("Score"));
    const QSize buttonSize(20, 30);

    playButton = new QPushButton("Play", this);
    const QIcon playIcon = style()->standardIcon(QStyle::SP_MediaPlay);
    playButton->setIcon(playIcon);
    playButton->setEnabled(true);
    playButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    playButton->setMinimumSize(buttonSize);
    tb->addWidget(playButton);
    CHECKED_CONNECT(playButton, SIGNAL(clicked()), this, SLOT(playScore()));

    stopButton = new QPushButton("Stop", this);
    const QIcon stopIcon = style()->standardIcon(QStyle::SP_MediaStop);
    stopButton->setIcon(stopIcon);
    stopButton->setEnabled(false);
    stopButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    stopButton->setMinimumSize(buttonSize);
    tb->addWidget(stopButton);
    CHECKED_CONNECT(stopButton, SIGNAL(clicked()), this, SLOT(stopScore()));

#ifdef NOTYET
    recordButton = new QPushButton("Record", this);
    const QIcon recordIcon = QIcon(rsrcPath + "/record.png");
    recordButton->setIcon(recordIcon);
    recordButton->setEnabled(true);
    recordButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    recordButton->setMinimumSize(buttonSize);
    tb->addWidget(recordButton);
    CHECKED_CONNECT(recordButton, SIGNAL(clicked()), this, SLOT(record()));
#endif

    // font family/size popups -------------------------------------

    tb = addToolBar(tr("Text"));
    tb->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    addToolBar(tb);

    comboFont = new QFontComboBox(tb);
    tb->addWidget(comboFont);
    CHECKED_CONNECT(comboFont, QOverload<const QString &>::of(&QComboBox::activated), this, &MainWindow::textFamily);

    comboSize = new QComboBox(tb);
    comboSize->setObjectName("comboSize");
    tb->addWidget(comboSize);
    comboSize->setEditable(true);

    const QList<int> standardSizes = QFontDatabase::standardSizes();
    foreach (int size, standardSizes)
        comboSize->addItem(QString::number(size));
    comboSize->setCurrentIndex(standardSizes.indexOf(QApplication::font().pointSize()));

    CHECKED_CONNECT(comboSize, QOverload<const QString &>::of(&QComboBox::activated), this, &MainWindow::textSize);
}

void MainWindow::setDefaultFont()
{
    QFont textFont("Courier");
    textFont.setStyleHint(QFont::Monospace);
    textFont.setFixedPitch(true);
    textFont.setPointSize(12);
    curEditor->setFont(textFont);
    rtcmixLogView->setFont(textFont);
    updateFontMenus(curEditor->font());
    setTabStops();
}

// NB: plural, in anticipation of tabbed editors
// The member name <curEditor> because of this.
void MainWindow::createEditors()
{
    Editor *e = new Editor(this);
    curEditor = e;
    setCurrentFileName(QString(tr("untitled.sco")));
    setWindowModified(curEditor->document()->isModified());
}

void MainWindow::createVerticalSplitter()
{
//does splitter need to be by itself in a VBox layout?
    splitter = new QSplitter(Qt::Vertical, this);
    setCentralWidget(splitter);
    splitter->addWidget(curEditor);
    splitter->addWidget(rtcmixLogView);
    int edIndex = splitter->indexOf(curEditor);
    int joIndex = splitter->indexOf(rtcmixLogView);
    splitter->setStretchFactor(edIndex, 1);
    splitter->setStretchFactor(joIndex, 0);
    splitter->setCollapsible(edIndex, false);
}

// Do this after setting a new font or size.
void MainWindow::setTabStops()
{
    //TODO: make this a preference
    const int tabStop = 4;
    QString spaces; // more accurate to measure a string of tabStop spaces, instead of one space
    for (int i = 0; i < tabStop; i++)
        spaces += " ";
    QFontMetrics metrics(curEditor->font());
    curEditor->setTabStopWidth(metrics.width(spaces));
}

void MainWindow::textFamily(const QString &f)
{
    QFont font = curEditor->font();
    font.setFamily(f);
    curEditor->setFont(font);
    setTabStops();
    updateFontMenus(curEditor->font());
    font.setPointSize(12);
    rtcmixLogView->setFont(font);
}

void MainWindow::textSize(const QString &p)
{
    qreal pointSize = p.toFloat();
    if (pointSize > 0) {
        QFont font = curEditor->font();
        font.setPointSize(pointSize);
        curEditor->setFont(font);
        setTabStops();
        updateFontMenus(curEditor->font());
        font.setPointSize(logFontSize);
        rtcmixLogView->setFont(font);
    }
}

void MainWindow::clipboardDataChanged()
{
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        actionPaste->setEnabled(md->hasText());
}

void MainWindow::updateFontMenus(const QFont &f)
{
    comboFont->setCurrentIndex(comboFont->findText(QFontInfo(f).family()));
    comboSize->setCurrentIndex(comboSize->findText(QString::number(f.pointSize())));
}

void MainWindow::about()
{
    QString str = tr("<h3>RTcmixShell " VERSION_STR "</h3>"
            "<p>John Gibson, Brad Garton, Doug Scott</p>");
    QMessageBox::about(this, tr("About"), str);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if (maybeSave())
        e->accept();
    else
        e->ignore();
}


// =====================================================================
// File menu functions

void MainWindow::setCurrentFileName(const QString &fileName)
{
    this->fileName = fileName;
    curEditor->document()->setModified(false);

    QString shownName;
    if (fileName.isEmpty())
        shownName = "untitled.sco";
    else
        shownName = QFileInfo(fileName).fileName();

    setWindowTitle(tr("%1 - %2[*]").arg(QCoreApplication::applicationName(), shownName));
    setWindowModified(false);
}

void MainWindow::fileNew()
{
    if (maybeSave()) {
        curEditor->clear();
        setCurrentFileName(QString());
    }
}

bool MainWindow::loadFile(const QString &f)
{
    if (!QFile::exists(f))
        return false;
    QFile file(f);
    if (!file.open(QFile::ReadOnly))
        return false;

    // See syntaxhighlighter example for a simpler way...
    QByteArray data = file.readAll();
    QTextCodec *codec = Qt::codecForHtml(data);
    QString str = codec->toUnicode(data);
    str = QString::fromLocal8Bit(data);
    curEditor->setPlainText(str);
    setCurrentFileName(f);
    return true;
}

void MainWindow::fileOpen()
{
    QFileDialog fileDialog(this, tr("Open File"));
    if (firstFileDialog) {
        fileDialog.setDirectory(QDir::home());
        firstFileDialog = false;
    }
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setNameFilters(QStringList() << "RTcmix score files (*.sco)" << "Text files (*.txt)");
    if (fileDialog.exec() != QDialog::Accepted)
        return;
    const QString fn = fileDialog.selectedFiles().first();
    if (loadFile(fn))
        statusBar()->showMessage(tr("Opened \"%1\"").arg(QDir::toNativeSeparators(fn)));
    else
        statusBar()->showMessage(tr("Could not open \"%1\"").arg(QDir::toNativeSeparators(fn)));
}

bool MainWindow::maybeSave()
{
    if (!curEditor->document()->isModified())
        return true;

    const QMessageBox::StandardButton ret =
        QMessageBox::warning(this, QCoreApplication::applicationName(),
                             tr("The document has been modified.\n"
                                "Do you want to save your changes?"),
                             QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
        return fileSave();
    else if (ret == QMessageBox::Cancel)
        return false;
    return true;
}

bool MainWindow::fileSave()
{
    if (fileName.isEmpty())
        return fileSaveAs();
    if (fileName.startsWith(QStringLiteral(":/")))
        return fileSaveAs();

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Could not write to file \"%1\":\n%2.")
                             .arg(QDir::toNativeSeparators(fileName),
                                  file.errorString()));
        return false;
    }
    QTextStream out(&file);
    out << curEditor->toPlainText();
    curEditor->document()->setModified(false);
    return true;
}

bool MainWindow::fileSaveAs()
{
    QFileDialog fileDialog(this, tr("Save"));
    if (firstFileDialog) {
        fileDialog.setDirectory(QDir::home());
        firstFileDialog = false;
    }
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setNameFilter("RTcmix score files (*.sco)");
    fileDialog.setDefaultSuffix("sco");
    if (fileDialog.exec() != QDialog::Accepted)
        return false;
    const QString fn = fileDialog.selectedFiles().first();
    setCurrentFileName(fn);
    return fileSave();
}


// =====================================================================
// Score menu functions

void rtcmixFinishedCallback(long long frameCount, void *inContext)
{
    (void) frameCount;
    MainWindow *thisclass = reinterpret_cast<MainWindow *>(inContext);
    thisclass->scoreFinished = true;
}

void MainWindow::checkScoreFinished()
{
    if (scoreFinished && (scorePlayMode == Exclusive))
        stopScore();
}

void MainWindow::setScorePlayMode()
{
    if (actionAllowOverlappingScores->isChecked()) {
        scorePlayMode = Overlapping;
        // we do not disable the score-finished callback, in case
        // other useful information gets there in the future
    }
    else {
        scorePlayMode = Exclusive;
        // not sure we should stop score playing here; this setting
        // should affect future plays
//        stopScore();
    }
}

void MainWindow::xableScoreActions(bool starting)
{
    if (scorePlayMode == Exclusive) {
        if (starting) {
            actionPlay->setEnabled(false);
            playButton->setEnabled(false);
            actionStop->setEnabled(true);
            stopButton->setEnabled(true);
        }
        else {
            actionStop->setEnabled(false);
            stopButton->setEnabled(false);
            actionPlay->setEnabled(true);
            playButton->setEnabled(true);
        }
    }
    else {  // Overlapping
        if (starting) {
            actionStop->setEnabled(true);
            stopButton->setEnabled(true);
        }
        else {
            actionStop->setEnabled(false);
            stopButton->setEnabled(false);
        }
    }
}

void MainWindow::playScore()
{
    QString sco = curEditor->document()->toPlainText();
    QByteArray ba = sco.toLatin1();
    char *buf = ba.data();
    if (!buf)
        return;
    const int len = strlen(buf);
    if (len) {
        rtcmixLogView->startLog();
        setScorePrintLevel(5);
        rtcmixLogView->printLogSeparator(this->fileName);
        xableScoreActions(true);
        scoreFinished = false;
        if (!scoreFinishedTimer->isActive())
            scoreFinishedTimer->start(scoreFinishedTimerInterval);
        int result = RTcmix_parseScore(buf, len);
        Q_UNUSED(result);
    }
//qDebug("invoked playScore(), buf len: %d, buffer...", len);
//qDebug("%s", buf);
}

void MainWindow::sendScoreFragment(char *fragment)
{
    int result = RTcmix_parseScore(fragment, strlen(fragment));
    Q_UNUSED(result);
}

void MainWindow::setScorePrintLevel(int level)
{
Q_UNUSED(level);
#ifdef NOTYET // FIXME: test longchain.sco first
    char buf[32];
    snprintf(buf, 32, "print_on(%d)\n", level);
    sendScoreFragment(buf);
#endif
}

void MainWindow::stopScore()
{
    xableScoreActions(false);
    scoreFinishedTimer->stop();
    rtcmixLogView->stopLog();
    setScorePrintLevel(0);
//#define FLUSH_SCORE_ON_STOP
#ifdef FLUSH_SCORE_ON_STOP
    RTcmix_flushScore();
#else
    audio->reInitializeRTcmix();
#endif
}
