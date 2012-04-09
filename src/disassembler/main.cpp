#include <cstdio>
#include "disassembler.h"

int main()
{
    uint16_t prog[32] = {
        0x7c01, 0x0030, 0x7de1, 0x1000, 0x0020, 0x7803, 0x1000, 0xc00d,
        0x7dc1, 0x001a, 0xa861, 0x7c01, 0x2000, 0x2161, 0x2000, 0x8463,
        0x806d, 0x7dc1, 0x000d, 0x9031, 0x7c10, 0x0018, 0x7dc1, 0x001a,
        0x9037, 0x61c1, 0x7dc1, 0x001a
    };

    Disassembler d;
    d.disassemble(prog, 28);

    for(uint16_t i = 0; i < d.getInstructionCount(); i++)
    {
        const Disassembler::Instruction *inst = d.getInstruction(i);
        printf("%s ",  inst->address_str);
        printf("%s ",  inst->operation_str);
        printf("%s ",  inst->operand_a_str);
        printf("%s\n", inst->operand_b_str);
    }
}

