src_dcpu = [
    "dcpu16/dcpu16.cpp",
    "dcpu16/main.cpp",
]

src_assembler = [
    "assembler/assemble.cpp",
    "assembler/assemble.cpp",
]

VariantDir("build", "src", duplicate=0)

cpp_flags = ["-Wall", "-Wextra"]
env = Environment(
    CCFLAGS     = cpp_flags,
)

env.Program("cpu", src_dcpu, srcdir="build")
#env.Program(src_assembler, srcdir="build")

