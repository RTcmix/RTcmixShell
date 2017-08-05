QT += widgets

TEMPLATE        = app
TARGET          = RTcmixShell

HEADERS         = audio.h \
                  highlighter.h \
                  mainwindow.h \
                  pa_memorybarrier.h \
                  pa_ringbuffer.h \
                  portaudio.h \
                  RTcmix_API.h \
                  rtcmixlogview.h

SOURCES         = audio.cpp \
                  highlighter.cpp \
                  mainwindow.cpp \
                  main.cpp \
                  pa_ringbuffer.c \
                  rtcmixlogview.cpp

RESOURCES += RTcmixShell.qrc

build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

# install
target.path = build
INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/rtcmix/release/ -lrtcmix_embedded
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/rtcmix/debug/ -lrtcmix_embedded
else:macx: LIBS += -L$$PWD/rtcmix/ -lrtcmix_embedded

DEPENDPATH += $$PWD/rtcmix

win32:CONFIG(release, debug|release): LIBS += -L/usr/local/lib/release/ -lportaudio
else:win32:CONFIG(debug, debug|release): LIBS += -L/usr/local/lib/debug/ -lportaudio
else:unix: LIBS += -L/usr/local/lib/ -lportaudio

INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/include
