QT += core
QT += widgets

TEMPLATE        = app
TARGET          = RTcmixShell

HEADERS         = audio.h \
                  credits.h \
                  editor.h \
                  finddialog.h \
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
                  finddialog.cpp \
                  highlighter.cpp \
                  mainwindow.cpp \
                  main.cpp \
                  pa_ringbuffer.c \
                  preferences.cpp \
                  record.cpp \
                  rtcmixlogview.cpp \
                  utils.cpp

RESOURCES += RTcmixShell.qrc

# NB: removing default app_bundle property because we
# create our bundle in the deploy-build-mac.sh script,
# which we run manually after building in Qt Creator.
build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
    CONFIG += warn_on
    CONFIG -= app_bundle
}

# This defaults to the minimum supported macOS
# version, which is 11 for Qt 6.5.1.
#QMAKE_MACOSX_DEPLOYMENT_TARGET = 11.0

#QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64
#QMAKE_APPLE_DEVICE_ARCHS = x86_64

# install
target.path = build
INSTALLS += target

win32:RC_ICONS += RTcmixShell.ico
macx:ICON = RTcmixShell.icns

#DEPENDPATH += $$PWD/lib/
INCLUDEPATH += $$PWD/

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/win/release/ -lrtcmix_embedded
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/win/debug/ -lrtcmix_embedded
else:macx:LIBS += -L$$PWD/lib/mac/ -lrtcmix_embedded
else:unix:LIBS += -L$$PWD/lib/unix/ -lrtcmix_embedded

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/win/release/ -lportaudio
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/win/debug/ -lportaudio
else:macx:LIBS += -L$$PWD/lib/mac/ -lportaudio
else:unix:LIBS += -L$$PWD/lib/unix/ -lportaudio

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/win/release/ -lsndfile
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/win/debug/ -lsndfile
else:macx:LIBS += -L$$PWD/lib/mac/ -lsndfile
else:unix:LIBS += -L$$PWD/lib/unix/ -lsndfile
