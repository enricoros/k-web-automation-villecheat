# -------------------------------------------------
# Project created by QtCreator 2009-02-20T21:23:37
# -------------------------------------------------
TARGET = villecheat
TEMPLATE = app
HEADERS += \
    AppWidget.h \
    AbstractGame.h \
    ScreenCapture.h \
    InputUtils.h \
    FVGame.h
SOURCES += \
    main.cpp \
    AbstractGame.cpp \
    AppWidget.cpp \
    ScreenCapture.cpp \
    InputUtils.cpp \
    FVGame.cpp
FORMS += AppWidget.ui
unix:LIBS += -lXtst
win32:CONFIG += console
OBJECTS_DIR = .obj
MOC_DIR = .obj
RCC_DIR = .obj
UI_DIR = .obj
