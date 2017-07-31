QT += widgets

TEMPLATE        = app
TARGET          = RTcmixShell

HEADERS         = audio.h \
                  highlighter.h \
                  portaudio.h \
                  RTcmix_API.h \
                  textedit.h

SOURCES         = audio.cpp \
                  highlighter.cpp \
                  main.cpp \
                  textedit.cpp

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

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/portaudio/release/ -lportaudio.2
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/portaudio/debug/ -lportaudio.2
else:unix: LIBS += -L$$PWD/portaudio/ -lportaudio.2

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.
