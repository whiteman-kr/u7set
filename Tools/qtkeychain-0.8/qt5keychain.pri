# Minimal qmake support.
# This file is provided as is without any warranty.
# It can break at anytime or be removed without notice.

QT5KEYCHAIN_PWD = $$PWD

CONFIG *= depend_includepath
DEFINES += QTKEYCHAIN_NO_EXPORT
DEFINES += USE_CREDENTIAL_STORE
#CONFIG += plaintextstore

INCLUDEPATH += \
    $$PWD/.. \
    $$QT5KEYCHAIN_PWD

HEADERS += \
    $$QT5KEYCHAIN_PWD/keychain_p.h \
    $$QT5KEYCHAIN_PWD/keychain.h

SOURCES *= \
    $$QT5KEYCHAIN_PWD/keychain.cpp

plaintextstore {
    HEADERS += $$QT5KEYCHAIN_PWD/plaintextstore_p.h
    SOURCES += $$QT5KEYCHAIN_PWD/plaintextstore.cpp
} else {
    unix:!macx {
        QT += dbus

        HEADERS += $$QT5KEYCHAIN_PWD/gnomekeyring_p.h

        SOURCES += \
            $$QT5KEYCHAIN_PWD/gnomekeyring.cpp \
            $$QT5KEYCHAIN_PWD/keychain_unix.cpp
    }

    win32 {
        HEADERS += $$QT5KEYCHAIN_PWD/libsecret_p.h

        SOURCES += \
            $$QT5KEYCHAIN_PWD/keychain_win.cpp \
            $$QT5KEYCHAIN_PWD/libsecret.cpp

        #DBUS_INTERFACES += $$PWD/Keychain/org.kde.KWallet.xml
    }

    mac {
        LIBS += "-framework Security" "-framework Foundation"
        SOURCES += $$QT5KEYCHAIN_PWD/keychain_mac.cpp
    }
}
