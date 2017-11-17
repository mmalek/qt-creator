QT = core
include(../qttest.pri)

RUSTDIR = $$IDE_SOURCE_TREE/src/plugins/rust

INCLUDEPATH += $$RUSTDIR

SOURCES += \
    tst_rustlexer.cpp \
    $$RUSTDIR/rustlexer.cpp
