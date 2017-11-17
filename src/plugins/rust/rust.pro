include(../../qtcreatorplugin.pri)

DEFINES += \
    RUST_LIBRARY

HEADERS += \
    rustautocompleter.h \
    rustbuildconfiguration.h \
    rustbuildstep.h \
    rustcompileroutputparser.h \
    rusteditorfactory.h \
    rusteditors.h \
    rusteditorwidget.h \
    rustgrammar.h \
    rusthighlighter.h \
    rustindenter.h \
    rustkitinformation.h \
    rustkitconfigwidget.h \
    rustlexer.h \
    rustmanifest.h \
    rustmimetypes.h \
    rustplugin.h \
    rustproduct.h \
    rustproject.h \
    rustprojectnode.h \
    rustracer.h \
    rustracercompletionassist.h \
    rustrunconfiguration.h \
    rustsettings.h \
    rustslice.h \
    rustsourcelayout.h \
    rusttargetarchinformation.h \
    rusttargetarchwidget.h \
    rusttoken.h \
    rusttoolchainmanager.h \
    rusttoolsoptionspage.h

SOURCES += \
    rustautocompleter.cpp \
    rustbuildconfiguration.cpp \
    rustbuildstep.cpp \
    rustcompileroutputparser.cpp \
    rusteditorfactory.cpp \
    rusteditorwidget.cpp \
    rusthighlighter.cpp \
    rustindenter.cpp \
    rustkitconfigwidget.cpp \
    rustkitinformation.cpp \
    rustlexer.cpp \
    rustmanifest.cpp \
    rustplugin.cpp \
    rustproject.cpp \
    rustprojectnode.cpp \
    rustracer.cpp \
    rustracercompletionassist.cpp \
    rustrunconfiguration.cpp \
    rustsettings.cpp \
    rustsourcelayout.cpp \
    rusttargetarchinformation.cpp \
    rusttargetarchwidget.cpp \
    rusttoolchainmanager.cpp \
    rusttoolsoptionspage.cpp

FORMS +=  \
    rustbuildconfigurationwidget.ui \
    rustbuildstepconfigwidget.ui \
    rusttoolsoptionspage.ui

RESOURCES += \
    rust.qrc
