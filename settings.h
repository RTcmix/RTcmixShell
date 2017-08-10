#ifndef SETTINGS_H
#define SETTINGS_H

#include <QPoint>
#include <QSettings>
#include <QSize>

class Settings
{
public:
    Settings();
    void saveSettings();

    // accessors - the getter methods return a default value if none is saved in settings file

    // MainWindow

    // NB: size/pos getters take a default arg, because that is computed in caller
    QSize mainWindowSize(QSize defaultSize) { return settings->value("mainwindow/size", defaultSize).toSize(); }
    void setMainWindowSize(QSize size) { settings->setValue("mainwindow/size", size); }

    QPoint mainWindowPosition(QPoint defaultPos) { return settings->value("mainwindow/pos", defaultPos).toPoint(); }
    void setMainWindowPosition(QPoint pos) { settings->setValue("mainwindow/pos", pos); }

    // Editor

    QString editorFontName() { return settings->value("editor/fontName", "Courier New").toString(); }
    void setEditorFontName(QString name) { settings->setValue("editor/fontName", name); }

    double editorFontSize() { return settings->value("editor/fontSize", 14.0).toDouble(); }
    void setEditorFontSize(double size) { settings->setValue("editor/fontSize", size); }

    int editorTabWidth() { return settings->value("editor/tabWidth", 4).toInt(); }
    void setEditorTabWidth(int width) { settings->setValue("editor/tabWidth", width); }

    bool editorDoSyntaxHighlighting() { return settings->value("editor/doSyntaxHighlighting", true).toBool(); }
    void setEditorDoSyntaxHighlighting(bool doSyntaxHighlight) { settings->setValue("editor/doSyntaxHighlighting", doSyntaxHighlight); }

    bool editorShowLineNumbers() { return settings->value("editor/showLineNumbers", false).toBool(); }
    void setEditorShowLineNumbers(bool showLineNumbers) { settings->setValue("editor/showLineNumbers", showLineNumbers); }

    // Log

    bool logShowOnLaunch() { return settings->value("log/showOnLaunch", true).toBool(); }
    void setLogShowOnLaunch(bool showOnLaunch) { settings->setValue("log/showOnLaunch", showOnLaunch); }

    QString logFontName() { return settings->value("log/fontName", "Courier New").toString(); }
    void setLogFontName(QString name) { settings->setValue("log/fontName", name); }

    double logFontSize() { return settings->value("log/fontSize", 12.0).toDouble(); }
    void setLogFontSize(double size) { settings->setValue("log/fontSize", size); }

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

    int audioBlockSize() { return settings->value("audio/blockSize", 512).toInt(); }
    void setAudioBlockSize(int size) { settings->setValue("audio/blockSize", size); }

    int audioNumBuses() { return settings->value("audio/numBuses", 32).toInt(); }
    void setAudioNumBuses(int numBuses) { settings->setValue("audio/numBuses", numBuses); }

#ifdef MAYBE_NEVER // might not be a good idea
    bool audioAllowOverlappingScores() { return settings->value("audio/allowOverlappingScores", false).toBool(); }
    void setAudioAllowOverlappingScores(bool allow) { settings->setValue("audio/allowOverlappingScores", allow); }
#endif

private:
    void reportError();

    QSettings *settings;
};

#endif // SETTINGS_H
