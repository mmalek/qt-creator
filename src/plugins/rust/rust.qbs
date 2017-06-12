import qbs 1.0

QtcPlugin {
    name: "Rust"

    Depends { name: "Qt.widgets" }
    Depends { name: "Utils" }

    Depends { name: "Core" }
    Depends { name: "ProjectExplorer" }
    Depends { name: "TextEditor" }

    files: [
        "rust.qrc",
        "rustautocompleter.cpp",
        "rustautocompleter.h",
        "rustbuildconfiguration.cpp",
        "rustbuildconfiguration.h",
        "rustbuildconfigurationwidget.ui",
        "rustbuildstep.cpp",
        "rustbuildstep.h",
        "rustbuildstepconfigwidget.ui",
        "rustcargo.cpp",
        "rustcargo.h",
        "rustcompileroutputparser.cpp",
        "rustcompileroutputparser.h",
        "rusteditorfactory.cpp",
        "rusteditorfactory.h",
        "rusteditors.h",
        "rusteditorwidget.cpp",
        "rusteditorwidget.h",
        "rustgrammar.h",
        "rusthighlighter.cpp",
        "rusthighlighter.h",
        "rustindenter.cpp",
        "rustindenter.h",
        "rustkitconfigwidget.cpp",
        "rustkitconfigwidget.h",
        "rustlexer.cpp",
        "rustlexer.h",
        "rustmimetypes.h",
        "rustplugin.cpp",
        "rustplugin.h",
        "rustproduct.h",
        "rustproject.cpp",
        "rustproject.h",
        "rustprojectmanager.cpp",
        "rustprojectmanager.h",
        "rustprojectnode.cpp",
        "rustprojectnode.h",
        "rustracer.cpp",
        "rustracer.h",
        "rustracercompletionassist.cpp",
        "rustracercompletionassist.h",
        "rustrunconfiguration.cpp",
        "rustrunconfiguration.h",
        "rustslice.h",
        "rustsourcelayout.cpp",
        "rustsourcelayout.h",
        "rusttoken.h",
        "rusttoolchainmanager.cpp",
        "rusttoolchainmanager.h",
        "rusttoolsoptionspage.cpp",
        "rusttoolsoptionspage.h",
        "rusttoolsoptionspage.ui",
    ]

    Group {
        name: "Unit tests"
        condition: qtc.testsEnabled
        prefix: "tests/"
        files: [
            "rustlexer_test.cpp",
            "rustlexer_test.h",
        ]
    }
}
