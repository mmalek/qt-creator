import qbs
import qbs.FileInfo

QtcAutotest {
    name: "Rust autotest"

    property path rustDir: FileInfo.joinPaths(project.ide_source_tree, "src", "plugins", "rust")

    cpp.includePaths: base.concat([rustDir])

    Group {
        name: "Sources from Rust plugin"
        prefix: rustDir + "/"
        files: [
            "rustlexer.cpp",
            "rustlexer.h",
        ]
    }

    files: [
        "tst_rustlexer.cpp",
    ]
}
