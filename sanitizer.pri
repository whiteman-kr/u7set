# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
    CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}

# AddressSanitizer for Linux
#
unix {
    #CONFIG(debug, debug|release): CONFIG += sanitizer sanitize_address
    #CONFIG(debug, debug|release): CONFIG += sanitizer sanitize_thread
}

