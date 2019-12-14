package(default_visibility = ["//visibility:public"])

genrule(
    name = "arm_select",
    outs = ["@arm_developer_gcc_linux://everything"]
    cmd = "ln -snf external/arm_developer_gcc_linux external/arm_developer_gcc",
    visibility = ["//visibility:private"],
)