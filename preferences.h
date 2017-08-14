#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QDialog>
#include <QPoint>
#include <QPushButton>
#include <QSettings>
#include <QSize>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QColor;
class QComboBox;
class QDialogButtonBox;
class QFontComboBox;
class QSpinBox;
class QTabWidget;
QT_END_NAMESPACE
class Highlighter;
class MainWindow;
class Preferences;

class SelectColorButton : public QPushButton
{
    Q_OBJECT

public:
    SelectColorButton(QWidget *parent = 0);

    void setColor(const QColor &color);
    const QColor &getColor();

public slots:
    void updateColor();
    void changeColor();

signals:
    void colorChanged(QColor);

private:
    QColor color;
};

#ifdef GENERALTAB
class GeneralTab : public QWidget
{
    Q_OBJECT

public:
    explicit GeneralTab(QWidget *parent = 0);
    void initFromPreferences(Preferences *);
    void applyPreferences(Preferences *);
    void cancelPreferences(Preferences *);
};
#endif

class AudioTab : public QWidget
{
    Q_OBJECT

public:
    explicit AudioTab(QWidget *parent = 0);
    void initFromPreferences(Preferences *);
    void applyPreferences(Preferences *);
    void cancelPreferences(Preferences *);

private slots:
    void conformValuesToSelectedDevice(int);

private:
    void initDeviceMenus();

    QComboBox *outDeviceMenu;
    QComboBox *samplingRateMenu;
    QSpinBox *outChannelsSpin;
    QComboBox *bufferSizeMenu;
    QSpinBox *numBusesSpin;
    QCheckBox *warnOverlappingScores;
    QVector<int> outputDeviceList;
    bool initing;
};

class EditorTab : public QWidget
{
    Q_OBJECT

public:
    explicit EditorTab(QWidget *parent = 0);
    void initFromPreferences(Preferences *);
    void applyPreferences(Preferences *);
    void cancelPreferences(Preferences *);

private slots:
    void logLinkFamilyClicked(bool);
    void linkedEditorFontFamilyChanged(const QString &);

private:
    MainWindow *mainWindow;
    QString prevEditorFontFamily;
    int prevEditorFontSize;
    int prevEditorTabWidth;
    QString prevLogFontFamily;
    int prevLogFontSize;
    bool prevLogLinkFamily;
    QFontComboBox *editorFontFamilyMenu;
    QComboBox *editorFontSizeMenu;
    QSpinBox *editorTabWidthSpin;
    QFontComboBox *logFontFamilyMenu;
    QComboBox *logFontSizeMenu;
    QCheckBox *logLinkFamily;
};

class SyntaxHighlightingTab : public QWidget
{
    Q_OBJECT

public:
    explicit SyntaxHighlightingTab(QWidget *parent = 0);
    void initFromPreferences(Preferences *);
    void applyPreferences(Preferences *);
    void cancelPreferences(Preferences *);

private slots:
    void setHighlighting(bool);
    void setCommentColor(QColor);
    void setFunctionColor(QColor);
    void setNumberColor(QColor);
    void setReservedColor(QColor);
    void setStringColor(QColor);
    void setUnusedColor(QColor);

private:
    Highlighter *highlighter;
    bool prevDoSyntaxHighlighting;
    QColor prevCommentColor;
    QColor prevFunctionColor;
    QColor prevNumberColor;
    QColor prevReservedColor;
    QColor prevStringColor;
    QColor prevUnusedColor;
    QCheckBox *xableHighlighting;
    SelectColorButton *commentButton;
    SelectColorButton *functionButton;
    SelectColorButton *numberButton;
    SelectColorButton *reservedButton;
    SelectColorButton *stringButton;
    SelectColorButton *unusedButton;
};


class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget *parent = 0);
    void initFromPreferences(Preferences *);
    void applyPreferences(Preferences *);
    void cancelPreferences(Preferences *);

private:
    QTabWidget *tabWidget;
#ifdef GENERALTAB
    GeneralTab *generalTab;
#endif
    AudioTab *audioTab;
    EditorTab *editorTab;
    SyntaxHighlightingTab *syntaxHighlightingTab;
    QDialogButtonBox *okCancelButtonBox;
};

class Preferences : public QObject
{
    Q_OBJECT

public:
    Preferences();
    void savePreferences();
    void dump();

    // accessors - the getter methods return a default value if none is saved in settings file

    // MainWindow

    // NB: size/pos getters take a default arg, because that is computed in caller
    QSize mainWindowSize(QSize defaultSize) { return settings->value("mainwindow/size", defaultSize).toSize(); }
    void setMainWindowSize(QSize size) { settings->setValue("mainwindow/size", size); }

    QPoint mainWindowPosition(QPoint defaultPos) { return settings->value("mainwindow/pos", defaultPos).toPoint(); }
    void setMainWindowPosition(QPoint pos) { settings->setValue("mainwindow/pos", pos); }

    // Editor

    QString editorFontFamily() { return settings->value("editor/fontFamily", "Courier New").toString(); }
    void setEditorFontFamily(QString family) { settings->setValue("editor/fontFamily", family); }

    double editorFontSize() { return settings->value("editor/fontSize", 14.0).toDouble(); }
    void setEditorFontSize(double size) { settings->setValue("editor/fontSize", size); }

    int editorTabWidth() { return settings->value("editor/tabWidth", 4).toInt(); }
    void setEditorTabWidth(int width) { settings->setValue("editor/tabWidth", width); }

    bool editorShowLineNumbers() { return settings->value("editor/showLineNumbers", false).toBool(); }
    void setEditorShowLineNumbers(bool showLineNumbers) { settings->setValue("editor/showLineNumbers", showLineNumbers); }

    // Syntax Highlighting

    bool editorDoSyntaxHighlighting() { return settings->value("editor/doSyntaxHighlighting", true).toBool(); }
    void setEditorDoSyntaxHighlighting(bool doSyntaxHighlight) { settings->setValue("editor/doSyntaxHighlighting", doSyntaxHighlight); }

    QColor editorCommentColor() { return settings->value("editor/commentColor", QColor(Qt::blue)).value<QColor>(); }
    void setEditorCommentColor(QColor commentColor) { settings->setValue("editor/commentColor", commentColor); }

    QColor editorFunctionColor() { return settings->value("editor/functionColor", QColor(Qt::darkGreen)).value<QColor>(); }
    void setEditorFunctionColor(QColor functionColor) { settings->setValue("editor/functionColor", functionColor); }

    QColor editorNumberColor() { return settings->value("editor/numberColor", QColor(Qt::red)).value<QColor>(); }
    void setEditorNumberColor(QColor numberColor) { settings->setValue("editor/numberColor", numberColor); }

    QColor editorReservedColor() { return settings->value("editor/reservedColor", QColor(Qt::magenta)).value<QColor>(); }
    void setEditorReservedColor(QColor reservedColor) { settings->setValue("editor/reservedColor", reservedColor); }

    QColor editorStringColor() { return settings->value("editor/stringColor", QColor(Qt::red)).value<QColor>(); }
    void setEditorStringColor(QColor stringColor) { settings->setValue("editor/stringColor", stringColor); }

    QColor editorUnusedColor() { return settings->value("editor/unusedColor", QColor(Qt::darkGray)).value<QColor>(); }
    void setEditorUnusedColor(QColor unusedColor) { settings->setValue("editor/unusedColor", unusedColor); }

    // Log

    bool logShowOnLaunch() { return settings->value("log/showOnLaunch", true).toBool(); }
    void setLogShowOnLaunch(bool showOnLaunch) { settings->setValue("log/showOnLaunch", showOnLaunch); }

    QString logFontFamily() { return settings->value("log/fontFamily", "Courier New").toString(); }
    void setLogFontFamily(QString family) { settings->setValue("log/fontFamily", family); }

    double logFontSize() { return settings->value("log/fontSize", 12.0).toDouble(); }
    void setLogFontSize(double size) { settings->setValue("log/fontSize", size); }

    bool logLinkFamily() { return settings->value("log/linkFamily", true).toBool(); }
    void setLogLinkFamily(bool linkFamily) { settings->setValue("log/linkFamily", linkFamily); }

    // Audio

    int audioInputDeviceID() { return settings->value("audio/inputDeviceID", 0).toInt(); }
    void setAudioInputDeviceID(int id) { settings->setValue("audio/inputDeviceID", id); }

    int audioOutputDeviceID() { return settings->value("audio/outputDeviceID", 0).toInt(); }
    void setAudioOutputDeviceID(int id) { settings->setValue("audio/outputDeviceID", id); }

    double audioSamplingRate() { return settings->value("audio/samplingRate", 44100.0).toDouble(); }
    void setAudioSamplingRate(double rate) { settings->setValue("audio/samplingRate", rate); }

    int audioNumInputChannels() { return settings->value("audio/numInputChannels", 0).toInt(); }
    void setAudioNumInputChannels(int numChans) { settings->setValue("audio/numInputChannels", numChans); }

    int audioNumOutputChannels() { return settings->value("audio/numOutputChannels", 2).toInt(); }
    void setAudioNumOutputChannels(int numChans) { settings->setValue("audio/numOutputChannels", numChans); }

    int audioBufferSize() { return settings->value("audio/blockSize", 512).toInt(); }
    void setAudioBufferSize(int size) { settings->setValue("audio/blockSize", size); }

    int audioNumBuses() { return settings->value("audio/numBuses", 32).toInt(); }
    void setAudioNumBuses(int numBuses) { settings->setValue("audio/numBuses", numBuses); }

#ifdef MAYBE_NEVER // might not be a good idea
    bool audioAllowOverlappingScores() { return settings->value("audio/allowOverlappingScores", false).toBool(); }
    void setAudioAllowOverlappingScores(bool allow) { settings->setValue("audio/allowOverlappingScores", allow); }
#endif

    bool audioShowOverlappingScoresWarning() { return settings->value("audio/showOverlappingScoresWarning", true).toBool(); }
    void setAudioShowOverlappingScoresWarning(bool show) { settings->setValue("audio/showOverlappingScoresWarning", show); }

public slots:
    void showPreferencesDialog();

private:
    void reportError();

    QSettings *settings;
};

#endif // PREFERENCES_H
