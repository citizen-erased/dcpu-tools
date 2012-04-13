src_dcpu = [
    "dcpu16/dcpu16.cpp",
    "dcpu16/main.cpp",
]

src_assembler = [
    "assembler/assemble.cpp",
    "assembler/main.cpp",
]

src_disassembler = [
    "dcpu16/dcpu16.o",
    "disassembler/disassembler.cpp",
    "disassembler/main.cpp",
]

src_debugger = [
    "dcpu16/dcpu16.o",
    "disassembler/disassembler.o",
    "debugger/memory_view.cpp",
    "debugger/disassembly_view.cpp",
    "debugger/gui.cpp",
    "debugger/main.cpp",
]

libs_debugger = [
    "fltk",
]

VariantDir("build", "src", duplicate=0)

cpp_flags = ["-Wall", "-Wextra", "-g"]
#cpp_flags = ["-Wall", "-Wextra", "-O3"]

env = Environment(
    CCFLAGS     = cpp_flags,
)

env.Program("dcpu", src_dcpu, srcdir="build")
#env.Program(src_assembler, srcdir="build")
env.Program("disassembler", src_disassembler, srcdir="build")


cpp_flags = ["-Wall", "-Wextra", "-g"]
env = Environment(
    CCFLAGS     = cpp_flags,
)

env.Program("debugger", src_debugger, srcdir="build", LIBS=libs_debugger)

