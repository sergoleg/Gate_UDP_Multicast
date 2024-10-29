TEMPLATE = app
TARGET = QRecvUdp 
QMAKE_CXXFLAGS += -Wno-expansion-to-defined
QT        += core gui widgets network 

win32:QMAKE_LFLAGS += -static -static-libgcc

HEADERS   += RecvUdp.h
SOURCES   += main.cpp \
    RecvUdp.cpp
FORMS     += RecvUdp.ui    
RESOURCES +=
