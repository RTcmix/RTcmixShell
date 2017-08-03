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

#define VERSION_STR	"v1.0b2"

void rtcmixPrintCallback(const char *printBuffer, void *inContext);

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    bool loadFile(const QString &f);

protected:
    void closeEvent(QCloseEvent *e) override;

private slots:
    void about();
    void fileNew();
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
    void createMenus();
    void createToolbars();
    void createEditors();
    void createJobOutputView();
    void createVerticalSplitter();
    void setCurrentFileName(const QString &fileName);
    bool maybeSave();
    void fontChanged(const QFont &f);
    void setTabStops();

    Audio *audio;
    QSplitter *splitter;
    QString fileName;
    QTextEdit *editor;
    QTextEdit *jobOutputView;
    QAction *actionNewFile;
    QAction *actionOpenFile;
    QAction *actionSaveFile;
    QAction *actionSaveFileAs;
    QAction *actionUndo;
    QAction *actionRedo;
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;
    QAction *actionPlay;
    QAction *actionStop;
    QAction *actionRecord;
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *scoreMenu;
    QFontComboBox *comboFont;
    QComboBox *comboSize;

    bool firstFileDlog;
};

#endif // MAINWINDOW_H
