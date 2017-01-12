import qbs 1.0

QtcPlugin {
    name: "Rust"

    Depends { name: "Qt.widgets" }
    Depends { name: "Utils" }

    Depends { name: "Core" }
    Depends { name: "TextEditor" }
    Depends { name: "ProjectExplorer" }

    files: [
        "buildconfiguration.cpp",
        "buildconfiguration.h",
        "buildconfigurationfactory.cpp",
        "buildconfigurationfactory.h",
        "buildstep.cpp",
        "buildstep.h",
        "buildstepfactory.cpp",
        "buildstepfactory.h",
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
