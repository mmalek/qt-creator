import qbs 1.0

QtcPlugin {
    name: "Rust"

    Depends { name: "Qt.widgets" }
    Depends { name: "Utils" }

    Depends { name: "Core" }
    Depends { name: "TextEditor" }
    Depends { name: "ProjectExplorer" }

    files: [
        "mimetypes.h",
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
    ]
}
