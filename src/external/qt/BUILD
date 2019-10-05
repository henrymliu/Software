package(default_visibility = ["//visibility:public"])

genrule(
    name = "qt_build",
    srcs = glob(["**/*"]),
    outs = ["lib/libQt5Core.so", "lib/libQt5Gui.so", "lib/libQtWidgets.so",
             "bin/moc", "bin/uic", "bin/qmake"],
    cmd = "\n".join([
        "./configure -opensource -confirm-license -no-framework -shared -no-rpath -nomake examples -nomake tests",
        "make"
    ]),
    message = "Building Qt...",
)
cc_library(
    name = "qt_core",
    hdrs = glob(["include/QtCore/**"]),
    includes = ["./include"],
    linkopts = [
        "-lQt5Core",
    ],
    srcs = [":qt_build"],
)

cc_library(
    name = "qt_network",
    hdrs = glob(["QtNetwork/**"]),
    includes = ["."],
    linkopts = [
        "-lQt5Network",
    ],
)

cc_library(
    name = "qt_widgets",
    hdrs = glob(["include/QtWidgets/**"]),
    includes = ["./include"],
    linkopts = [
        "-lQt5Widgets",
    ],
    deps = [":qt_core", ":qt_gui"],
)

cc_library(
    name = "qt_quick",
    hdrs = glob(["QtQuick/**"]),
    includes = ["."],
    linkopts = [
        "-lQt5Quick",
    ],
    deps = [
        ":qt_gui",
        ":qt_qml",
    ],
)

cc_library(
    name = "qt_qml",
    hdrs = glob(["QtQml/**"]),
    includes = ["."],
    linkopts = [
        "-lQt5Qml",
    ],
    deps = [
        ":qt_core",
        ":qt_network",
    ],
)

cc_library(
    name = "qt_gui",
    hdrs = glob(["include/QtGui/**"]),
    includes = ["./include"],
    linkopts = [
        "-lQt5Gui",
    ],
    deps = [":qt_core"],
)

cc_library(
    name = "qt_opengl",
    hdrs = glob(["QtOpenGL/**"]),
    includes = ["."],
    linkopts = ["-lQt5OpenGL"],
)
