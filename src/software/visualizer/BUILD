load("@bazel_rules_qt//:qt.bzl", "qt_cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "visualizer_wrapper",
    srcs = ["visualizer_wrapper.cpp"],
    hdrs = ["visualizer_wrapper.h"],
    deps = [
        "//software/ai/hl/stp:play_info",
        "//software/backend:robot_status",
        "//software/geom",
        "//software/multithreading:thread_safe_buffer",
        "//software/multithreading:threaded_observer",
        "//software/visualizer/drawing:draw_functions",
        "//software/visualizer/drawing:world",
        "//software/visualizer/widgets:visualizer",
        "//software/world",
        "@qt//:qt_widgets",
    ],
)
