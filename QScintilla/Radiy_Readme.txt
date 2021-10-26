Current QScintilla version is 2.13.0,

downloaded from

https://www.riverbankcomputing.com/software/qscintilla/download

After downloading, following changed were made in qscintilla.pro:

1. staticlib Added

CONFIG += ... staticlib

2. DESTDIR section added
 
# DESTDIR
#
win32 {
	CONFIG(debug, debug|release): DESTDIR = ../../bin/debug
	CONFIG(release, debug|release): DESTDIR = ../../bin/release
}
unix {
	CONFIG(debug, debug|release): DESTDIR = ../../bin_unix/debug
	CONFIG(release, debug|release): DESTDIR = ../../bin_unix/release
}

3. Target name has been changed

#CONFIG(debug, debug|release) {
#    mac: {
#        TARGET = qscintilla2_qt$${QT_MAJOR_VERSION}_debug
#    } else {
#        win32: {
#            TARGET = qscintilla2_qt$${QT_MAJOR_VERSION}d
#        } else {
#            TARGET = qscintilla2_qt$${QT_MAJOR_VERSION}
#        }
#    }
#} else {
#    TARGET = qscintilla2_qt$${QT_MAJOR_VERSION}
#}
TARGET = QScintilla
