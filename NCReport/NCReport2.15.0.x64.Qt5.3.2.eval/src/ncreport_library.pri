unix {
	!macx {
		target.path = /usr/local/NCReport/bin
	} else {
		QMAKE_LFLAGS += -F$$NCREPORT_LIBPATH
	}
}

CONFIG(release, debug|release) {
	win32-g++: LIBS += $$NCREPORT_LIBPATH/libNCReport2.a
	else:win32: LIBS += $$NCREPORT_LIBPATH/NCReport2.lib
	unix:!macx: LIBS += -L$$NCREPORT_LIBPATH -lNCReport
	else:macx: LIBS += -framework NCReport
}
CONFIG(debug, debug|release) {
	win32-g++: LIBS += $$NCREPORT_LIBPATH/libNCReportDebug2.a
	else:win32: LIBS += $$NCREPORT_LIBPATH/NCReportDebug2.lib
	unix:!macx: LIBS += -L$$NCREPORT_LIBPATH -lNCReportDebug
	else:macx: LIBS += -framework NCReportDebug
}
