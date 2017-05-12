import qbs 1.0

QtcPlugin {
    name: "Rust"

    Depends { name: "Qt.widgets" }
    Depends { name: "Utils" }

    Depends { name: "Core" }
    Depends { name: "ProjectExplorer" }
    Depends { name: "TextEditor" }

    files: [
        "buildconfiguration.cpp",
        "buildconfiguration.h",
        "buildconfigurationwidget.ui",
        "buildstep.cpp",
        "buildstep.h",
        "buildstepconfigwidget.ui",
        "editors.h",
        "indenter.cpp",
        "indenter.h",
        "lexer.cpp",
        "lexer.h",
        "mimetypes.h",
        "product.h",
        "racercompletionassist.cpp",
        "racercompletionassist.h",
        "runconfiguration.cpp",
        "runconfiguration.h",
        "rust.qrc",
        "cargo.cpp",
        "cargo.h",
        "plugin.cpp",
        "plugin.h",
        "project.h",
        "project.cpp",
        "projectmanager.h",
        "projectmanager.cpp",
        "projectnode.h",
        "projectnode.cpp",
        "rustautocompleter.cpp",
        "rustautocompleter.h",
        "rustcparser.cpp",
        "rustcparser.hpp",
        "rusteditorfactory.cpp",
        "rusteditorfactory.h",
        "rustsourcelayout.cpp",
        "rustsourcelayout.h",
        "syntaxhighlighter.cpp",
        "syntaxhighlighter.h",
        "token.h",
    ]

    Group {
        name: "Unit tests"
        condition: qtc.testsEnabled
        prefix: "tests/"
        files: [
            "lexer_test.cpp",
            "lexer_test.h",
        ]
    }
}
