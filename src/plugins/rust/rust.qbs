import qbs 1.0

QtcPlugin {
    name: "Rust"

    Depends { name: "Qt.widgets" }
    Depends { name: "Utils" }

    Depends { name: "Core" }
    Depends { name: "TextEditor" }
    Depends { name: "ProjectExplorer" }

    Group {
        name: "General"
        files: [
            "rustplugin.cpp", "rustplugin.h",
            "rust.qrc",
        ]
    }
}
