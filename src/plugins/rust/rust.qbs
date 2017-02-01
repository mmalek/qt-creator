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
        "mimetypes.h",
        "product.h",
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
        "rustcparser.cpp",
        "rustcparser.hpp",
    ]
}
