QT = core gui sql
TEMPLATE = app
TARGET = CustomItemDemo
NCREPORT_LIBPATH = $$OUT_PWD/../../../lib
include($$PWD/../demos.pri)
include($$PWD/../../ncreport_library.pri)

HEADERS += \
$$PWD/../../runner/ncrtestform.h \
$$PWD/../../runner/testdatasource.h \
$$PWD/../../runner/testitemrendering.h

SOURCES += \ 
$$PWD/main.cpp \
$$PWD/../../runner/ncrtestform.cpp \
$$PWD/../../runner/testdatasource.cpp \
$$PWD/../../runner/testitemrendering.cpp

FORMS += \
$$PWD/../../runner/ncrtestform.ui



