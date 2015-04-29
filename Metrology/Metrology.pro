#-------------------------------------------------
#
# Project created by QtCreator 2014-09-12T12:56:12
#
#-------------------------------------------------

QT       += core gui qml network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport concurrent sql printsupport

TARGET = Metrology
TEMPLATE = app

include(../qtpropertybrowser/src/qtpropertybrowser.pri)


# DESTDIR
#
win32 {
	CONFIG(debug, debug|release): DESTDIR = ../bin/debug
	CONFIG(release, debug|release): DESTDIR = ../bin/release
}
unix {
	CONFIG(debug, debug|release): DESTDIR = ../bin_unix/debug
	CONFIG(release, debug|release): DESTDIR = ../bin_unix/release
}

# Force prebuild version control info
#
# for creating version.h at first build
win32:system(IF NOT EXIST version.h (echo int VERSION_H = 0; > version.h))
unix:system([ -e ./version.h ] || touch ./version.h)
# for any build
versionTarget.target = version.h
versionTarget.depends = FORCE
win32 {
        contains(QMAKE_TARGET.arch, x86_64){
            versionTarget.commands = chdir $$PWD/../GetGitProjectVersion & \
            qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
            nmake & \
            chdir $$PWD & \
            $$PWD/../bin_Win64/GetGitProjectVersion.exe $$PWD/Metrology.pro
        }
        else{
            versionTarget.commands = chdir $$PWD/../GetGitProjectVersion & \
            qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
            nmake & \
            chdir $$PWD & \
            $$PWD/../bin_Win32/GetGitProjectVersion.exe $$PWD/Metrology.pro
        }
}
unix {
    versionTarget.commands = cd $$PWD/../GetGitProjectVersion; \
        qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\"; \
        make; \
        cd $$PWD; \
        $$PWD/../bin_unix/GetGitProjectVersion $$PWD/Metrology.pro
}
PRE_TARGETDEPS += version.h
QMAKE_EXTRA_TARGETS += versionTarget


SOURCES += \
    MainWindow.cpp \
    Measure.cpp \
    Calibrator.cpp \
    CalibratorBase.cpp \
    OptionsDialog.cpp \
    Options.cpp \
    OptionsPointsDialog.cpp \
    main.cpp \
    MeasureThread.cpp \
    CalibratorManager.cpp \
    MeasureViewHeader.cpp \
    MeasureView.cpp \
    OptionsMvhDialog.cpp \
    MeasureBase.cpp \
    ExportMeasure.cpp \
    Delegate.cpp \
    FindMeasure.cpp \
    FolderPropertyManager.cpp \
    Database.cpp \
    ReportView.cpp \
    Conversion.cpp \
    Calculator.cpp \
    ../lib/Crc.cpp \
    ../lib/DbStruct.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/ModuleConfiguration.cpp \
    ../lib/ProtoSerialization.cpp \
    ../lib/Signal.cpp \
    ../lib/SignalMask.cpp


HEADERS  += \
    MainWindow.h \
    Measure.h \
    Calibrator.h \
    CalibratorBase.h \
    OptionsDialog.h \
    Options.h \
    OptionsPointsDialog.h \
    MeasureThread.h \
    CalibratorManager.h \
    MeasureViewHeader.h \
    MeasureView.h \
    OptionsMvhDialog.h \
    MeasureBase.h \
    ExportMeasure.h \
    Delegate.h \
    FindMeasure.h \
    FolderPropertyManager.h \
    Database.h \
    ReportView.h \
    version.h \
    Conversion.h \
    Calculator.h \
    ../include/Signal.h \
    ../include/CUtils.h \
    ../include/Crc.h \
    ../include/Factory.h \
    ../include/DeviceObject.h \
    ../include/DbStruct.h \
    ../include/ModuleConfiguration.h \
    ../include/ProtoSerialization.h \
    ../include/SignalMask.h \
    ../include/Types.h \
    ../include/OrderedHash.h \
    Stable.h


FORMS    +=

RESOURCES += \
    Resources.qrc

TRANSLATIONS = translations/Metrology_ru.ts \
                translations/Metrology_uk.ts
OTHER_FILES += \
    translations/Metrology_ru.ts \
    translations/Metrology_uk.ts \
    reports/Linearity.ncr \
    reports/Comparators.ncr \
    reports/ComplexComparators.ncr \
    reports/LinearityCertification.ncr \
    reports/LinearityDetailEl.ncr \
    reports/LinearityDetailPh.ncr

CONFIG(debug, debug|release): DEFINES += Q_DEBUG

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

unix:QMAKE_CXXFLAGS += -std=c++11

# Remove Protobuf 4996 warning, Can't remove it in sources, don't know why
#
win32:QMAKE_CXXFLAGS += -D_SCL_SECURE_NO_WARNINGS

#protobuf
#
win32 {
                LIBS += -L$$DESTDIR -lprotobuf

                INCLUDEPATH += ./../Protobuf
}
unix {
                LIBS += -lprotobuf
}

# Visual Leak Detector
#
win32{
        contains(QMAKE_TARGET.arch, x86_64){
                LIBS += -L"C:/Program Files/Visual Leak Detector/lib/Win64"
                LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
        }
        else{
                LIBS += -L"C:/Program Files/Visual Leak Detector/lib/Win32"
                LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win32"
        }

        INCLUDEPATH += "C:/Program Files/Visual Leak Detector/include"
        INCLUDEPATH += "C:/Program Files (x86)/Visual Leak Detector/include"
}

# NCReport
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files/NCReport/2.13.0.VS2013.Qt5.3.2.eval/lib/" -lNCReportDebug2
    else: CONFIG(release, debug|release) : LIBS += -L"C:/Program Files/NCReport/2.13.0.VS2013.Qt5.3.2.eval/lib/" -lNCReport2

    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/NCReport/2.13.0.VS2013.Qt5.3.2.eval/lib/" -lNCReportDebug2
    else: CONFIG(release, debug|release) : LIBS += -L"C:/Program Files (x86)/NCReport/2.13.0.VS2013.Qt5.3.2.eval/lib/" -lNCReport2

    INCLUDEPATH += "C:/Program Files/NCReport/2.13.0.VS2013.Qt5.3.2.eval/include"
    INCLUDEPATH += "C:/Program Files (x86)/NCReport/2.13.0.VS2013.Qt5.3.2.eval/include"
}

unix {
        contains(QMAKE_HOST.arch, x86_64) {
            CONFIG(debug, debug|release): LIBS += -L"$$PWD/../NCReport/NCReport2.15.0.x64.Qt5.3.2.eval/lib" -lNCReportDebug
            else: CONFIG(release, debug|release) : LIBS += -L"$$PWD/../NCReport/NCReport2.15.0.x64.Qt5.3.2.eval/lib" -lNCReport
            INCLUDEPATH += "$$PWD/../NCReport/NCReport2.15.0.x64.Qt5.3.2.eval/include"
        }
        else{
            CONFIG(debug, debug|release): LIBS += -L"$$PWD/../NCReport/NCReport2.15.0.x86.Qt5.3.2.eval/lib" -lNCReportDebug
            else: CONFIG(release, debug|release) : LIBS += -L"$$PWD/../NCReport/NCReport2.15.0.x86.Qt5.3.2.eval/lib" -lNCReport
            INCLUDEPATH += "$$PWD/../NCReport/NCReport2.15.0.x86.Qt5.3.2.eval/include"
        }
}



