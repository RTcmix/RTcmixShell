
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QFontComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QMenu>
#include <QMenuBar>
#include <QTextEdit>
#include <QStatusBar>
#include <QToolBar>
#include <QTextCursor>
#include <QtDebug>
#include <QCloseEvent>
#include <QMessageBox>

#include "audio.h"
#include "highlighter.h"
#include "mainwindow.h"
#include "RTcmix_API.h"

#ifdef Q_OS_MAC
const QString rsrcPath = ":/images/mac";
#else
const QString rsrcPath = ":/images/win";
#endif


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
    createVerticalSplitter();

    setAcceptDrops(true);
    jobOutputView->viewport()->setAcceptDrops(false);

    audio = new Audio;
    RTcmix_setPrintCallback(rtcmixPrintCallback, this);
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
}

void MainWindow::createEditActions()
{
    actionUndo = new QAction(tr("&Undo"), this);
    actionUndo->setShortcut(QKeySequence::Undo);
    actionUndo->setStatusTip(tr("Undo the last operation"));
    connect(actionUndo, &QAction::triggered, this, &MainWindow::undo);

    actionRedo = new QAction(tr("&Redo"), this);
    actionRedo->setShortcut(QKeySequence::Redo);
    actionRedo->setStatusTip(tr("Redo the last operation"));
    connect(actionRedo, &QAction::triggered, this, &MainWindow::redo);

    actionCut = new QAction(tr("Cu&t"), this);
    actionCut->setShortcut(QKeySequence::Cut);
    actionCut->setStatusTip(tr("Cut the current selection to the clipboard"));
    actionCut->setEnabled(false);
    connect(actionCut, &QAction::triggered, this, &MainWindow::cut);

    actionCopy = new QAction(tr("&Copy"), this);
    actionCopy->setShortcut(QKeySequence::Copy);
    actionCopy->setStatusTip(tr("Copy the current selection to the clipboard"));
    actionCopy->setEnabled(false);
    connect(actionCopy, &QAction::triggered, this, &MainWindow::copy);

    actionPaste = new QAction(tr("&Paste"), this);
    actionPaste->setShortcut(QKeySequence::Paste);
    actionPaste->setStatusTip(tr("Paste the clipboard into the current selection"));
    connect(actionPaste, &QAction::triggered, this, &MainWindow::paste);

    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &MainWindow::clipboardDataChanged);
}

void MainWindow::createScoreActions()
{
    const QIcon playIcon = QIcon::fromTheme("media-playback-start", QIcon(rsrcPath + "/play.png"));
//    const QIcon playIcon = style()->standardIcon(QStyle::SP_MediaPlay);
    actionPlay = new QAction(playIcon, tr("&Play"), this);
    actionPlay->setShortcut(Qt::CTRL + Qt::Key_P);
    actionPlay->setStatusTip(tr("Play the score"));
    connect(actionPlay, &QAction::triggered, this, &MainWindow::play);

    const QIcon stopIcon = QIcon::fromTheme("media-playback-stop", QIcon(rsrcPath + "/stop.png"));
 //   const QIcon stopIcon = style()->standardIcon(QStyle::SP_MediaStop);
    actionStop = new QAction(stopIcon, tr("&Stop"), this);
    actionStop->setShortcut(Qt::CTRL + Qt::Key_Period);
    actionStop->setStatusTip(tr("Stop playing the score"));
    connect(actionStop, &QAction::triggered, this, &MainWindow::stop);

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
    fontChanged(editor->font());
    setTabStops();
}

// NB: plural, in anticipation of tabbed editors
void MainWindow::createEditors()
{
    editor = new QTextEdit(this);
    setDefaultFont();
    Highlighter *h = new Highlighter(editor->document());

    connect(editor->document(), &QTextDocument::modificationChanged, actionSaveFile, &QAction::setEnabled);
    connect(editor->document(), &QTextDocument::modificationChanged, this, &QWidget::setWindowModified);
    connect(editor->document(), &QTextDocument::undoAvailable, actionUndo, &QAction::setEnabled);
    connect(editor->document(), &QTextDocument::redoAvailable, actionRedo, &QAction::setEnabled);
    connect(editor, &QTextEdit::cursorPositionChanged, this, &MainWindow::cursorPositionChanged);

    setWindowModified(editor->document()->isModified());
    actionSave->setEnabled(editor->document()->isModified());
    actionUndo->setEnabled(editor->document()->isUndoAvailable());
    actionRedo->setEnabled(editor->document()->isRedoAvailable());
    actionCut->setEnabled(false);
    actionCopy->setEnabled(false);

    textEdit->setFocus();
    setCurrentFileName(QString(tr("untitled.sco")));
}

void MainWindow::createJobOutputView()
{
    jobOutputView = new QTextEdit(this);
    jobOutputView->setPlainText(tr("Output"));
    jobOutputView->setReadOnly(true);
}

void MainWindow::createVerticalSplitter()
{
//does splitter need to be by itself in a VBox layout?
    splitter = new QSplitter;
    setCentralWidget(splitter);
    splitter->addWidget(editor);
    splitter->addWidget(jobOutputView);
    splitter->setResizeMode(jobOutputView, QSplitter::KeepSize);
    splitter->setCollapsible(editor, false);
}

// Do this after setting a new font or size.
void MainWindow::setTabStops()
{
    //TODO: make this a preference
    const int tabStop = 4;
    QString spaces; // more accurate to measure a string of tabStop spaces, instead of one space
    for (int i = 0; i < tabStop; i++)
        spaces += " ";
    QFontMetrics metrics(textEdit->font());
    textEdit->setTabStopWidth(metrics.width(spaces));
}

void MainWindow::textFamily(const QString &f)
{
    QFont font = editor->font();
    font.setFamily(f);
    editor->setFont(font);
    setTabStops();
    fontChanged(editor->font());
}

void MainWindow::textSize(const QString &p)
{
    qreal pointSize = p.toFloat();
    if (pointSize > 0) {
        QFont font = editor->font();
        font.setPointSize(pointSize);
        editor->setFont(font);
        setTabStops();
        fontChanged(editor->font());
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

void MainWindow::fontChanged(const QFont &f)
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

void MainWindow::playScore()
{
    QString sco = textEdit->document()->toPlainText();
    QByteArray ba = sco.toLocal8Bit();
    char *buf = ba.data();
    if (!buf)
        return;
    const int len = strlen(buf);
    if (len) {
        // *** will this return before score plays?
        // do we need to use dyn mem for buf?
        int result = RTcmix_parseScore(buf, len);
        Q_UNUSED(result);
    }
//    qDebug("invoked playScore(), buf len: %d, buffer...", len);
//    qDebug("%s", buf);
}

void MainWindow::stopScore()
{
//#define FLUSH_SCORE_ON_STOP
#ifdef FLUSH_SCORE_ON_STOP
    RTcmix_flushScore();
#else
    audio->reInitializeRTcmix();
#endif
}

void rtcmixPrintCallback(const char *printBuffer, void *inContext)
{
    (void) inContext;
    const char *pbuf = printBuffer;
    char str[1024];
    int len = strlen(pbuf);
    while (len > 0) {
        strncpy(str, pbuf, 1024);
        str[len-1] = 0; // get rid of duplicate line ending
        qDebug("%s", str);
        pbuf += (len + 1);
        len = strlen(pbuf);
    }
}

void MainWindow::setCurrentFileName(const QString &fileName)
{
    this->fileName = fileName;
    textEdit->document()->setModified(false);

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
