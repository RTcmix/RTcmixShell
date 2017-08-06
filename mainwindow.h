#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QAction;
class QComboBox;
class QFontComboBox;
class QMenu;
class QPushButton;
class QSplitter;
class QTextEdit;
class QTimer;
QT_END_NAMESPACE
class Audio;
class Editor;
class RTcmixLogView;

#define VERSION_STR	"v1.0b2"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void fileNew();
    bool loadFile(const QString &f);
    bool scoreFinished;

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
    void checkScoreFinished();
    void setScorePlayMode();

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
    void xableScoreActions(bool);
    void setScorePrintLevel(int);
    void sendScoreFragment(char *);

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
    QFontComboBox *comboFont;
    QComboBox *comboSize;
    QTimer *scoreFinishedTimer;

    enum ScorePlayMode {
        Exclusive = 0,   // playing a new score not permitted until prev one stops
        Overlapping = 1
    };
    ScorePlayMode scorePlayMode;
    bool firstFileDialog;
};

#endif // MAINWINDOW_H
