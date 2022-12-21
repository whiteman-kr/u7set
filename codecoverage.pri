
# Add test coverage options.
#
unix {
	CODE_COVERAGE {
	    QMAKE_CXXFLAGS += --coverage
		QMAKE_LFLAGS += --coverage
		LIBS += -lgcov
	}
}