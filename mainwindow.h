#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QPushButton;
class QSettings;
class QSplitter;
class QPlainTextEdit;
class QTimer;
QT_END_NAMESPACE
class Audio;
class Led;
class RTcmixLogView;
class Preferences;
#include "editor.h"
#include "highlighter.h"


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void fileNew();
    bool loadFile(const QString &);
    void reinitializeAudio();
    Highlighter *getHighlighter() { return curEditor->getHighlighter(); }

    bool scoreFinished;

protected:
    void closeEvent(QCloseEvent *e) override;

public slots:
    void editorFontFamily(const QString &);
    void editorFontSize(const QString &);
    void editorTabWidth(const int &);
    void logFontFamily(const QString &);
    void logFontSize(const QString &);
    void fileOpenNoDialog(const QString &);
    void stopScore();
    void showClipping(int);

private slots:
    void about();
    void fileOpen();
    bool fileSave();
    bool fileSaveAs();
    void playScore();
    void record();
    void clipboardDataChanged();
    void checkScoreFinished();
    void setScorePlayMode();

private:
    void createPreferences();
    void createActions();
    void createFileActions();
    void createEditActions();
    void createScoreActions();
    void createTextActions();
    void createMenus();
    void createToolbars();
    void initFonts();
    void createEditors();
    void createVerticalSplitter();
    void setCurrentFileName(const QString &);
    bool maybeSave();
    void setTabStops();
    void xableScoreActions(bool);
    void setScorePrintLevel(int);
    void stopScoreNoReinit();
    void sendScoreFragment(char *);
    bool chooseRecordFilename(QString &);
    void loadSettings();
    void saveSettings();
    void debug();

    Audio *audio;
    Editor *curEditor;
    RTcmixLogView *rtcmixLogView;
    QSplitter *splitter;
    QString fileName;
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
    QAction *actionShowLineNumbers;
    QAction *actionPrefs;
    QAction *actionPlay;
    QAction *actionStop;
    QAction *actionRecord;
    QAction *actionAllowOverlappingScores;
    QAction *actionClearLog;
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *scoreMenu;
    QPushButton *playButton;
    QPushButton *stopButton;
    QPushButton *recordButton;
    Led *clippingIndicator;
    QTimer *scoreFinishedTimer;

    enum ScorePlayMode {
        Exclusive = 0,   // playing a new score not permitted until prev one stops
        Overlapping = 1
    };
    ScorePlayMode scorePlayMode;
    bool playing;
    bool recording;
    bool reinitRTcmixOnPlay;
    bool firstFileDialog;
    int tabWidth;

    Preferences *mainWindowPreferences;
};

#endif // MAINWINDOW_H
