QT += widgets
qtHaveModule(printsupport): QT += printsupport

TEMPLATE        = app
TARGET          = RTcmixShell

HEADERS         = textedit.h \
                  highlighter.h
SOURCES         = textedit.cpp \
                  main.cpp \
                  highlighter.cpp

RESOURCES += RTcmixShell.qrc

build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

EXAMPLE_FILES = RTcmixShell.qdoc

# install
target.path = build
INSTALLS += target
