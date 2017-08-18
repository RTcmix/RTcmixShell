QT += core
QT += widgets

TEMPLATE        = app
TARGET          = RTcmixShell

HEADERS         = audio.h \
                  credits.h \
                  editor.h \
                  highlighter.h \
                  led.h \
                  mainwindow.h \
                  myapp.h \
                  pa_memorybarrier.h \
                  pa_ringbuffer.h \
                  portaudio.h \
                  preferences.h \
                  record.h \
                  RTcmix_API.h \
                  rtcmixlogview.h \
                  sndfile.h \
                  utils.h

SOURCES         = audio.cpp \
                  editor.cpp \
                  highlighter.cpp \
                  mainwindow.cpp \
                  main.cpp \
                  pa_ringbuffer.c \
                  preferences.cpp \
                  record.cpp \
                  rtcmixlogview.cpp \
                  utils.cpp

RESOURCES += RTcmixShell.qrc

build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

# install
target.path = build
INSTALLS += target

ICON = RTcmixShell.icns

DEPENDPATH += $$PWD/lib/
INCLUDEPATH += $$PWD/

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/release/ -lrtcmix_embedded
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/debug/ -lrtcmix_embedded
else:macx: LIBS += -L$$PWD/lib/ -lrtcmix_embedded

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/release/ -lportaudio
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/debug/ -lportaudio
else:macx: LIBS += -L$$PWD/lib/ -lportaudio

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/release/ -lsndfile
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/debug/ -lsndfile
else:macx: LIBS += -L$$PWD/lib/ -lsndfile

