QT = core

CONFIG += c++17 cmdline

QMAKE_TARGET_COPYRIGHT = "\\251 Wouldy Mods 2026"
VERSION = 2.2.3.28

INCLUDEPATH += $$PWD/include
LIBS += -L$$PWD/lib/
LIBS += -lfmod64
LIBS += -lfsbank64

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    bank_extract.cpp \
    extract_worker.cpp \
    fileio.cpp \
    main.cpp \
    rebuild_worker.cpp \

HEADERS += \
    bank_extract.h \
    bank_header.h \
    extract_worker.h \
    fileio.h \
    rebuild_worker.h \
    version.h

CONFIG += lrelease

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RC_ICONS = resource\fmod_bank_tools.ico

DISTFILES += \
    readme/title.webp
