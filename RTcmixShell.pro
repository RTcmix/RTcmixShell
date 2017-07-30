QT += widgets
qtHaveModule(printsupport): QT += printsupport

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

EXAMPLE_FILES = RTcmixShell.qdoc

# install
target.path = build
INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/rtcmix/release/ -lrtcmix_embedded
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/rtcmix/debug/ -lrtcmix_embedded
else:macx: LIBS += -L$$PWD/rtcmix/ -lrtcmix_embedded

INCLUDEPATH += $$PWD/rtcmix
DEPENDPATH += $$PWD/rtcmix

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/portaudio/release/ -lportaudio
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/portaudio/debug/ -lportaudio
else:unix: LIBS += -L$$PWD/portaudio/ -lportaudio

INCLUDEPATH += $$PWD/portaudio
DEPENDPATH += $$PWD/portaudio

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/portaudio/release/libportaudio.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/portaudio/debug/libportaudio.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/portaudio/release/portaudio.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/portaudio/debug/portaudio.lib
else:unix: PRE_TARGETDEPS += $$PWD/portaudio/libportaudio.a
