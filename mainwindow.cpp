
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontComboBox>
#include <QFontDatabase>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPlainTextEdit>
#include <QSplitter>
#include <QStatusBar>
#include <QTextCodec>
#include <QTextEdit>
#include <QTimer>
#include <QToolBar>
#include <QtDebug>

#include "audio.h"
#include "highlighter.h"
#include "mainwindow.h"
#include "RTcmix_API.h"

void rtcmixPrintCallback(const char *printBuffer, void *inContext);

#ifdef Q_OS_MAC
const QString rsrcPath = ":/images/mac";
#else
const QString rsrcPath = ":/images/win";
#endif

const int ringBufferNumChars = 1024 * 128;
const int jobOutputTimerInterval = 100; // msec

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
#ifdef Q_OS_OSX
    setUnifiedTitleAndToolBarOnMac(true);
#endif
    setWindowTitle(QCoreApplication::applicationName());
    firstFileDlog = true;

    createActions();    // must come before editors
    createMenus();
    createToolbars();

    createEditors();
    createJobOutputView();
    setDefaultFont();
    createVerticalSplitter();

    setAcceptDrops(true);
    jobOutputView->viewport()->setAcceptDrops(false);

    audio = new Audio;
}

void MainWindow::createActions()
{
    createFileActions();
    createEditActions();
    createScoreActions();
    createTextActions();
}

void MainWindow::createFileActions()
{
    actionNewFile = new QAction(tr("&New"), this);
    actionNewFile->setShortcut(QKeySequence::New);
    actionNewFile->setStatusTip(tr("Create a new score"));
    connect(actionNewFile, &QAction::triggered, this, &MainWindow::fileNew);

    actionOpenFile = new QAction(tr("&Open"), this);
    actionOpenFile->setShortcut(QKeySequence::Open);
    actionOpenFile->setStatusTip(tr("Open an existing score file"));
    connect(actionOpenFile, &QAction::triggered, this, &MainWindow::fileOpen);

    actionSaveFile = new QAction(tr("&Save"), this);
    actionSaveFile->setShortcut(QKeySequence::Save);
    actionSaveFile->setStatusTip(tr("Save the edited score to disk"));
    actionSaveFile->setEnabled(false);
    connect(actionSaveFile, &QAction::triggered, this, &MainWindow::fileSave);

    actionSaveFileAs = new QAction(tr("Save &As..."), this);
    actionSaveFileAs->setShortcut(QKeySequence::SaveAs);
    actionSaveFileAs->setStatusTip(tr("Save the edited score to disk"));
    connect(actionSaveFileAs, &QAction::triggered, this, &MainWindow::fileSaveAs);

    actionQuit = new QAction(tr("&Quit"), this);
    actionQuit->setShortcut(Qt::CTRL + Qt::Key_Q);
    connect(actionQuit, &QAction::triggered, this, &QWidget::close);
}

void MainWindow::createEditActions()
{
    actionUndo = new QAction(tr("&Undo"), this);
    actionUndo->setShortcut(QKeySequence::Undo);
    actionUndo->setStatusTip(tr("Undo the last operation"));

    actionRedo = new QAction(tr("&Redo"), this);
    actionRedo->setShortcut(QKeySequence::Redo);
    actionRedo->setStatusTip(tr("Redo the last operation"));

    actionCut = new QAction(tr("Cu&t"), this);
    actionCut->setShortcut(QKeySequence::Cut);
    actionCut->setStatusTip(tr("Cut the current selection to the clipboard"));
    actionCut->setEnabled(false);

    actionCopy = new QAction(tr("&Copy"), this);
    actionCopy->setShortcut(QKeySequence::Copy);
    actionCopy->setStatusTip(tr("Copy the current selection to the clipboard"));
    actionCopy->setEnabled(false);

    actionPaste = new QAction(tr("&Paste"), this);
    actionPaste->setShortcut(QKeySequence::Paste);
    actionPaste->setStatusTip(tr("Paste the clipboard into the current selection"));

    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &MainWindow::clipboardDataChanged);
}

void MainWindow::createScoreActions()
{
    const QIcon playIcon = QIcon::fromTheme("media-playback-start", QIcon(rsrcPath + "/play.png"));
//    const QIcon playIcon = style()->standardIcon(QStyle::SP_MediaPlay);
    actionPlay = new QAction(playIcon, tr("&Play"), this);
    actionPlay->setShortcut(Qt::CTRL + Qt::Key_P);
    actionPlay->setStatusTip(tr("Play the score"));
    connect(actionPlay, &QAction::triggered, this, &MainWindow::playScore);

    const QIcon stopIcon = QIcon::fromTheme("media-playback-stop", QIcon(rsrcPath + "/stop.png"));
 //   const QIcon stopIcon = style()->standardIcon(QStyle::SP_MediaStop);
    actionStop = new QAction(stopIcon, tr("&Stop"), this);
    actionStop->setShortcut(Qt::CTRL + Qt::Key_Period);
    actionStop->setStatusTip(tr("Stop playing the score"));
    connect(actionStop, &QAction::triggered, this, &MainWindow::stopScore);

#ifdef NOTYET
    const QIcon recordIcon = QIcon::fromTheme("media-record", QIcon(rsrcPath + "/record.png"));
    actionRecord = new QAction(recordIcon, tr("&Record"), this);
    actionRecord->setShortcut(Qt::CTRL + Qt::Key_R);
    actionRecord->setShortcut(QKeySequence::Record);
    actionRecord->setStatusTip(tr("Record the sound that's playing to a sound file"));
    connect(actionRecord, &QAction::triggered, this, &MainWindow::record);
#endif
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

    QMenu *helpMenu = menuBar()->addMenu(tr("Help"));
    helpMenu->addAction(tr("About"), this, &MainWindow::about);
    helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
}

void MainWindow::createToolbars()
{
//TODO: look into QToolButton as a better solution for play/stop, etc. Leave icons out of the menus.
    setToolButtonStyle(Qt::ToolButtonFollowStyle);

    QToolBar *tb = addToolBar(tr("Score"));
    tb->addAction(actionPlay);
    tb->addAction(actionStop);
#ifdef NOTYET
    tb->addAction(actionRecord);
#endif

    tb = addToolBar(tr("Text"));
    tb->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    addToolBar(tb);

    comboFont = new QFontComboBox(tb);
    tb->addWidget(comboFont);
    connect(comboFont, QOverload<const QString &>::of(&QComboBox::activated), this, &MainWindow::textFamily);

    comboSize = new QComboBox(tb);
    comboSize->setObjectName("comboSize");
    tb->addWidget(comboSize);
    comboSize->setEditable(true);

    const QList<int> standardSizes = QFontDatabase::standardSizes();
    foreach (int size, standardSizes)
        comboSize->addItem(QString::number(size));
    comboSize->setCurrentIndex(standardSizes.indexOf(QApplication::font().pointSize()));

    connect(comboSize, QOverload<const QString &>::of(&QComboBox::activated), this, &MainWindow::textSize);
}

void MainWindow::setDefaultFont()
{
    QFont textFont("Courier");
    textFont.setStyleHint(QFont::Monospace);
    textFont.setFixedPitch(true);
    textFont.setPointSize(12);
    editor->setFont(textFont);
    jobOutputView->setFont(textFont);
    updateFontMenus(editor->font());
    setTabStops();
}

// NB: plural, in anticipation of tabbed editors
void MainWindow::createEditors()
{
    editor = new QTextEdit(this);
    Highlighter *h = new Highlighter(editor->document());
    Q_UNUSED(h);

    connect(actionUndo, &QAction::triggered, editor, &QTextEdit::undo);
    connect(actionRedo, &QAction::triggered, editor, &QTextEdit::redo);
    connect(actionCut, &QAction::triggered, editor, &QTextEdit::cut);
    connect(actionCopy, &QAction::triggered, editor, &QTextEdit::copy);
    connect(actionPaste, &QAction::triggered, editor, &QTextEdit::paste);

    connect(editor->document(), &QTextDocument::modificationChanged, actionSaveFile, &QAction::setEnabled);
    connect(editor->document(), &QTextDocument::modificationChanged, this, &QWidget::setWindowModified);
    connect(editor->document(), &QTextDocument::undoAvailable, actionUndo, &QAction::setEnabled);
    connect(editor->document(), &QTextDocument::redoAvailable, actionRedo, &QAction::setEnabled);
    connect(editor, &QTextEdit::cursorPositionChanged, this, &MainWindow::cursorPositionChanged);

    setWindowModified(editor->document()->isModified());
    actionSaveFile->setEnabled(editor->document()->isModified());
    actionUndo->setEnabled(editor->document()->isUndoAvailable());
    actionRedo->setEnabled(editor->document()->isRedoAvailable());
    actionCut->setEnabled(false);
    actionCopy->setEnabled(false);

    editor->setFocus();
    setCurrentFileName(QString(tr("untitled.sco")));
}

// This is called from one of the RTcmix threads, incl. the high-priority
// audio thread. Since Qt GUI widgets cannot operate in more than one thread,
// and since we don't want to put GUI code in the high-priority thread, we
// use a ring buffer to ship the job output text from here to the main GUI
// thread before printing to the window.
void rtcmixPrintCallback(const char *printBuffer, void *inContext)
{
    // This is complicated, because we don't know how large printBuffer is,
    // only that it comprises any number of C-strings laid end-to-end.
    PaUtilRingBuffer *ringBuf = reinterpret_cast<PaUtilRingBuffer *>(inContext);
    const char *pbuf = printBuffer;
    char str[1024];
    //FIXME: need to seek into first non-null char -- maybe that never happens, though
    int len = strlen(pbuf); // not incl. term. null
    while (len > 0) {
        strncpy(str, pbuf, 1024);
        str[len] = 0; // strncpy does not guarantee null termination
        ring_buffer_size_t rbCount = PaUtil_GetRingBufferWriteAvailable(ringBuf);
        if (rbCount < len) {
            qDebug("rtcmixPrintCallback: not enough ring buffer space for incoming data");
            str[rbCount-1] = 0;
        }
        rbCount = PaUtil_WriteRingBuffer(ringBuf, str, len);
//qDebug("PRINT (%d): %s", rbCount, str);
        pbuf += (len + 1);  // skip to next C-string
        len = strlen(pbuf);
    }
}

void MainWindow::createJobOutputView()
{
    jobOutputView = new QPlainTextEdit(this);
    jobOutputView->setReadOnly(true);

    // set background to very light gray
    QPalette p = jobOutputView->palette();
    p.setColor(QPalette::Active, QPalette::Base, QColor(245, 245, 245));
    p.setColor(QPalette::Inactive, QPalette::Base, QColor(245, 245, 245));
    jobOutputView->setPalette(p);

    ringBufferBlock = (char *) calloc(ringBufferNumChars, sizeof(char));
    PaUtil_InitializeRingBuffer(&jobOutputRingBuffer, sizeof(char), ringBufferNumChars, ringBufferBlock);
    RTcmix_setPrintCallback(rtcmixPrintCallback, &jobOutputRingBuffer);

    jobOutputTimer = new QTimer(this);
    connect(jobOutputTimer, SIGNAL(timeout()), this, SLOT(checkJobOutput()));
}

void MainWindow::startJobOutput()
{
    // this just resets the read and write pointers
    PaUtil_FlushRingBuffer(&jobOutputRingBuffer);

    // clear the block held by the ring buffer
    char *p = ringBufferBlock;
    int len = ringBufferNumChars;
    while (len-- > 0)
        *p++ = 0;

    jobOutputView->clear();
    if (!jobOutputTimer->isActive())
        jobOutputTimer->start(jobOutputTimerInterval);
}

// this runs periodically while user plays a score
void MainWindow::checkJobOutput()
{
    char buf[1024], str[1024];
    while (1) {
        bzero(buf, 1024);
        ring_buffer_size_t rbCount = PaUtil_GetRingBufferReadAvailable(&jobOutputRingBuffer);
        if (rbCount <= 0)
            break;
        PaUtil_ReadRingBuffer(&jobOutputRingBuffer, buf, rbCount);
        if (buf[rbCount-1] != 0)
            qDebug("buf last char not null: '%c'", buf[rbCount-1]);
qDebug("rbCount: %d, buf: %p, buflen: %ld", rbCount, buf, strlen(buf));
        char *pbuf = buf;
        int len = strlen(pbuf);   // not incl. term. null
        while (len > 0) {
#if 1
            jobOutputView->appendPlainText(QString(pbuf));
#else
            strncpy(str, pbuf, 1024);
            str[len] = 0; // strncpy does not guarantee null termination
            jobOutputView->appendPlainText(QString(str));
#endif
            pbuf += (len + 1);  // skip to next C-string
            len = strlen(pbuf);
        }
    }
    jobOutputView->moveCursor(QTextCursor::End);
}

void MainWindow::stopJobOutput()
{
    jobOutputTimer->stop();
}

void MainWindow::createVerticalSplitter()
{
//does splitter need to be by itself in a VBox layout?
    splitter = new QSplitter(Qt::Vertical, this);
    setCentralWidget(splitter);
    splitter->addWidget(editor);
    splitter->addWidget(jobOutputView);
//    splitter->setResizeMode(jobOutputView, QSplitter::KeepSize);
    int edIndex = splitter->indexOf(editor);
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
    QFontMetrics metrics(editor->font());
    editor->setTabStopWidth(metrics.width(spaces));
}

void MainWindow::textFamily(const QString &f)
{
    QFont font = editor->font();
    font.setFamily(f);
    editor->setFont(font);
    setTabStops();
    updateFontMenus(editor->font());
    font.setPointSize(12);
    jobOutputView->setFont(font);
}

void MainWindow::textSize(const QString &p)
{
    qreal pointSize = p.toFloat();
    if (pointSize > 0) {
        QFont font = editor->font();
        font.setPointSize(pointSize);
        editor->setFont(font);
        setTabStops();
        updateFontMenus(editor->font());
        font.setPointSize(12);
        jobOutputView->setFont(font);
    }
}

void MainWindow::cursorPositionChanged()
{
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
qDebug("closeEvent");   // why is this being called twice?
    if (maybeSave())
        e->accept();
    else
        e->ignore();
}

void MainWindow::setCurrentFileName(const QString &fileName)
{
    this->fileName = fileName;
    editor->document()->setModified(false);

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
        editor->clear();
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
    editor->setPlainText(str);
    setCurrentFileName(f);
    return true;
}

void MainWindow::fileOpen()
{
    QFileDialog fileDialog(this, tr("Open File"));
    if (firstFileDlog) {
        fileDialog.setDirectory(QDir::home());
        firstFileDlog = false;
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
    if (!editor->document()->isModified())
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
    out << editor->toPlainText();
    editor->document()->setModified(false);
    return true;
}

bool MainWindow::fileSaveAs()
{
    QFileDialog fileDialog(this, tr("Save"));
    if (firstFileDlog) {
        fileDialog.setDirectory(QDir::home());
        firstFileDlog = false;
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

void MainWindow::playScore()
{
    QString sco = editor->document()->toPlainText();
    QByteArray ba = sco.toLatin1();
    char *buf = ba.data();
    if (!buf)
        return;
    const int len = strlen(buf);
    if (len) {
        startJobOutput();
        setScorePrintLevel(5);
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
    char buf[32];
    snprintf(buf, 32, "print_on(%d)\n", level);
    sendScoreFragment(buf);
}

void MainWindow::stopScore()
{
    stopJobOutput();
    setScorePrintLevel(0);
//#define FLUSH_SCORE_ON_STOP
#ifdef FLUSH_SCORE_ON_STOP
    RTcmix_flushScore();
#else
    audio->reInitializeRTcmix();
#endif
}
