#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QAction;
class QComboBox;
class QFontComboBox;
class QMenu;
class QSplitter;
class QTextEdit;
QT_END_NAMESPACE
class Audio;
class RTcmixLogView;

#define VERSION_STR	"v1.0b2"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void fileNew();
    bool loadFile(const QString &f);

protected:
    void closeEvent(QCloseEvent *e) override;

private slots:
    void about();
    void fileOpen();
    bool fileSave();
    bool fileSaveAs();
    void playScore();
    void stopScore();
    void textFamily(const QString &f);
    void textSize(const QString &p);
    void clipboardDataChanged();
    void cursorPositionChanged();

private:
    void createActions();
    void createFileActions();
    void createEditActions();
    void createScoreActions();
    void createTextActions();
    void createMenus();
    void createToolbars();
    void setDefaultFont();
    void createEditors();
    void createVerticalSplitter();
    void setCurrentFileName(const QString &fileName);
    bool maybeSave();
    void updateFontMenus(const QFont &f);
    void setTabStops();
    void setScorePrintLevel(int);
    void sendScoreFragment(char *);

    Audio *audio;
    RTcmixLogView *rtcmixLogView;
    QSplitter *splitter;
    QString fileName;
    QTextEdit *editor;
    QAction *actionNewFile;
    QAction *actionOpenFile;
    QAction *actionSaveFile;
    QAction *actionSaveFileAs;
    QAction *actionQuit;
    QAction *actionUndo;
    QAction *actionRedo;
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;
    QAction *actionPlay;
    QAction *actionStop;
    QAction *actionRecord;
    QAction *actionClearLog;
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *scoreMenu;
    QFontComboBox *comboFont;
    QComboBox *comboSize;

    bool firstFileDlog;
};

// NB: This comes from the Spectrum Qt example.
// Macro which connects a signal to a slot, and which causes application to
// abort if the connection fails.  This is intended to catch programming errors
// such as mis-typing a signal or slot name.  It is necessary to write our own
// macro to do this - the following idiom
//     Q_ASSERT(connect(source, signal, receiver, slot));
// will not work because Q_ASSERT compiles to a no-op in release builds.

#define CHECKED_CONNECT(source, signal, receiver, slot) \
    if (!connect(source, signal, receiver, slot)) \
        qt_assert_x(Q_FUNC_INFO, "CHECKED_CONNECT failed", __FILE__, __LINE__);


#endif // MAINWINDOW_H
