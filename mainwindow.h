#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "pa_ringbuffer.h"

QT_BEGIN_NAMESPACE
class QAction;
class QComboBox;
class QFontComboBox;
class QMenu;
class QPlainTextEdit;
class QSplitter;
class QTextEdit;
class QTimer;
QT_END_NAMESPACE
class Audio;

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
    void clearJobOutput();
    void checkJobOutput();

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
    void createJobOutputView();
    void startJobOutput();
    void printJobOutputSeparator();
    void stopJobOutput();
    void createVerticalSplitter();
    void setCurrentFileName(const QString &fileName);
    bool maybeSave();
    void updateFontMenus(const QFont &f);
    void setTabStops();
    void sendScoreFragment(char *);
    void setScorePrintLevel(int);

    Audio *audio;
    QSplitter *splitter;
    QString fileName;
    QTextEdit *editor;
    QPlainTextEdit *jobOutputView;
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
    QAction *actionClearJobOutput;
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *scoreMenu;
    QFontComboBox *comboFont;
    QComboBox *comboSize;
    QTimer *jobOutputTimer;

    PaUtilRingBuffer jobOutputRingBuffer;
    char *ringBufferBlock;
    char *tmpJobOutputBlock;

    bool firstFileDlog;
};

#endif // MAINWINDOW_H
