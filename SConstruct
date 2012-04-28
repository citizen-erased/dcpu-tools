import os

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

VariantDir("build", "src", duplicate=0)

cpp_flags = ["-Wall", "-Wextra", "-g"]
#cpp_flags = ["-Wall", "-Wextra", "-O3"]

env = Environment(
    # environment for colorgcc to work
    ENV =       {'PATH' : os.environ['PATH'],
                 'TERM' : os.environ['TERM'],
                 'HOME' : os.environ['HOME']},

    CCFLAGS     = cpp_flags,
)

env.Program("dcpu", src_dcpu, srcdir="build")
#env.Program(src_assembler, srcdir="build")
env.Program("disassembler", src_disassembler, srcdir="build")

