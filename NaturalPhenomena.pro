TEMPLATE = app
TARGET = NaturalPhenomena

INCLUDEPATH += . /opt/local/include
QT += core widgets opengl gui 
LIBS += -lglut -lGLU -lstdc++fs

HEADERS += ./Atmosphere.h \
    ./QuadRenderer.h \
    ./Shader.h \
    ./Widget.h \
    ./Window.h
SOURCES += ./Atmosphere.cpp \
    ./QuadRenderer.cpp \
    ./Shader.cpp \
    ./Widget.cpp \
    ./Window.cpp \
    ./main.cpp
