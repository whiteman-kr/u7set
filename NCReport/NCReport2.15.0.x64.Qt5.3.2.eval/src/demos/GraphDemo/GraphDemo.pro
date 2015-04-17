QT = core gui sql
TEMPLATE = app
TARGET = GraphDemo
include(../demos.pri)
NCREPORT_LIBPATH = $$OUT_PWD/../../../lib
include($$PWD/../demos.pri)
include($$PWD/../../ncreport_library.pri)

HEADERS += \
$$PWD/graph.h \
$$PWD/../../runner/ncrtestform.h \
$$PWD/../../runner/testdatasource.h \
$$PWD/../../runner/testitemrendering.h

SOURCES += \ 
$$PWD/main.cpp \
$$PWD/graph.cpp \
$$PWD/../../runner/ncrtestform.cpp \
$$PWD/../../runner/testdatasource.cpp \
$$PWD/../../runner/testitemrendering.cpp

FORMS += \
$$PWD/../../runner/ncrtestform.ui




