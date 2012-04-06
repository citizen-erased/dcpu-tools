#include <cstring>
#include <algorithm>
#include <iostream>
#include <iomanip>

#include "dcpu16.h"


DCPU16::DCPU16()
{
    for(int i = 0; i < 16; i++)
        op_cycles[i] = 0;

    for(int i = 0; i < 0x20; i++)
        operand_cycles[i] = 0;

    op_cycles[SET] = 1;
    op_cycles[ADD] = 2;
    op_cycles[SUB] = 2;
    op_cycles[MUL] = 2;
    op_cycles[DIV] = 3;
    op_cycles[MOD] = 3;
    op_cycles[SHL] = 2;
    op_cycles[SHR] = 2;
    op_cycles[AND] = 1;
    op_cycles[BOR] = 1;
    op_cycles[XOR] = 1;
    op_cycles[IFE] = 2;
    op_cycles[IFN] = 2;
    op_cycles[IFG] = 2;
    op_cycles[IFB] = 2;

    for(int i = 0x10; i <= 0x17; i++)
        operand_cycles[i] = 1;

    operand_cycles[0x1E] = 1;
    operand_cycles[0x1F] = 1;

    reset();
}

void DCPU16::reset()
{
    pc = 0;
    sp = 0;
    overflow = 0;
    clock = 0;
    last_instruction = 0;
    error = ERROR_NONE;
    skip_next = false;

    std::fill(mem, mem+MEMORY_SIZE, 0);
    std::fill(mem_flags, mem_flags+MEMORY_SIZE, 0);
    std::fill(reg, mem+NUM_REGISTERS, 0);
}

void DCPU16::loadProgram(const uint16_t *words, uint16_t num_words)
{
    reset();
    std::copy(words, words+num_words, mem);
}

void DCPU16::tick()
{
    if(error)
        return;

    uint16_t inst = mem[pc++];
    last_instruction = inst;

    uint16_t op = (inst >> INST_OP_SHIFT) & INST_OP_MASK;
    uint16_t va = (inst >> INST_VA_SHIFT) & INST_VA_MASK;
    uint16_t vb = (inst >> INST_VB_SHIFT) & INST_VB_MASK;

    uint16_t a, *aptr, b, *bptr;
    processOperand(va, &aptr, &a);
    processOperand(vb, &bptr, &b);

    if(skip_next)
    {
        skip_next = false;
        return;
    }

    int32_t a32 = a, b32 = b, r32 = 0;

    switch(op)
    {
    case SET: 
        writePtr(aptr, b);
        break;

    case ADD:
        r32 = a32 + b32;
        //if(r32 > UINT16_MAX) //FIXME why isn't UINT16_MAX defined included?
        if(r32 > 0xFFFF)
        {
            writePtr(aptr, r32 % 0xFFFF);
            overflow = 0x0001;
        }
        else
        {
            writePtr(aptr, r32);
            overflow = 0x0000;
        }
    break;

    case SUB:
        r32 = a32 - b32;
        if(r32 < 0)
        {
            writePtr(aptr, r32 + 0xFFFF);
            overflow = 0xFFFF;
        }
        else
        {
            writePtr(aptr, r32);
            overflow = 0x0000;
        }
        break;

    case MUL:
        r32 = a32 * b32;
        writePtr(aptr, r32);
        overflow = (r32 >> 16) & 0xFFFF;
        break;

    case DIV:
        if(b == 0)
        {
            writePtr(aptr, 0);
            overflow = 0;
        }
        else
        {
            r32 = a32 / b32;
            writePtr(aptr, r32);
            overflow = ((a32 << 16) / b32) & 0xFFFF;
        }
        break;

    case MOD:
        if(b32 == 0)
            writePtr(aptr, 0);
        else
            writePtr(aptr, a % b);
        break;

    case SHL:
        writePtr(aptr, a << b);
        overflow = ((a32 << b32) >> 16) & 0xFFFF;
        break;

    case SHR:
        writePtr(aptr, a >> b);
        overflow = ((a32 << 16) >> b32) & 0xFFFF;
        break;

    case AND:
        writePtr(aptr, a & b);
        break;

    case BOR:
        writePtr(aptr, a | b);
        break;

    case XOR:
        writePtr(aptr, a ^ b);
        break;

    case IFE:
        skip_next = !(a == b);
        break;

    case IFN:
        skip_next = !(a != b);
        break;

    case IFG:
        skip_next = !(a > b);
        break;

    case IFB:
        skip_next = !((a & b) != 0);
        break;

    case EXT:
        switch(va)
        {
        case JSR:
            mem[--sp] = pc;
            pc = b;
            break;
        }
        break;

    default:
        setError(ERROR_OPCODE_INVALID);
        break;
    }

    //TODO cycles for ext instructions
    clock += op_cycles[op];
    //TODO don't add cycles when a has an extended opcode in it
    if(va < 0x20) clock += operand_cycles[va];
    if(vb < 0x20) clock += operand_cycles[vb];
    if(skip_next) clock += 2;
}

void DCPU16::processOperand(uint16_t operand, uint16_t **ptr, uint16_t *value)
{
    *ptr = NULL;

    if(operand <= 0x07) //TODO check overflow works correctly here
        *ptr = reg + operand;
    else if(operand <= 0x0F)
        *ptr = mem + reg[operand - 0x8];
    else if(operand <= 0x17)
        *ptr = mem + mem[pc++] + reg[operand - 0x10];
    else if(0x18 == operand)
        *ptr = mem + sp++;
    else if(0x19 == operand)
        *ptr = mem + sp;
    else if(0x1a == operand)
        *ptr = mem + --sp;
    else if(0x1b == operand)
        *ptr = &sp;
    else if(0x1c == operand)
        *ptr = &pc;
    else if(0x1d == operand)
        *ptr = &overflow;
    else if(0x1e == operand)
        *ptr = mem + mem[pc++];
    else if(0x1f == operand)
        *ptr = mem + pc++;
    else
        *value = operand - 0x20;

    if(*ptr) *value = **ptr;
}

bool DCPU16::writePtr(uint16_t *ptr, uint16_t value)
{
    if(!ptr)
    {
        //silently fail
        //TODO print warning?
        return false;
    }

    if(mem <= ptr && ptr < mem + MEMORY_SIZE)
    {
        //TODO check memory flags
    }

    *ptr = value;
    return true;
}

int DCPU16::getError() const
{
    return error;
}

void DCPU16::setError(int err)
{
    //TODO print info?
    error = err;
}

void DCPU16::printState()
{
    std::cout << "clock           = " << std::dec << clock << "\n";
    std::cout << "program counter = " << std::dec << pc << "\n";
    std::cout << "instruction     = " << std::hex << last_instruction << "\n";
    std::cout << "stack pointer   = " << sp << "\n";
    std::cout << "stack peek      = " << mem[sp] << "\n";
    std::cout << "overflow        = " << overflow << "\n";
    std::cout << "skip            = " << skip_next << "\n";
    std::cout << "error           = " << error << "\n";
    for(int i = 0; i < NUM_REGISTERS; i++)
        std::cout << "register[" << i << "]    = " << reg[i] << "\n";

    for(int i = 0; i < MEMORY_SIZE; i++)
    {
        if(i > 0 && (i % 32) == 0)
            std::cout << "\n";
        if((i % 32) == 0)
            std::cout << std::setfill('0') << std::setw(4) << std::hex << i << ":";
        std::cout << std::setfill('0') << std::setw(4) << std::hex << mem[i] << " ";
    }
    std::cout << "\n";
}

