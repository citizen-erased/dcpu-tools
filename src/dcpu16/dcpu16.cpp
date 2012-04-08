#include <cstring>
#include <algorithm>
#include <iostream>
#include <iomanip>

#include "dcpu16.h"


DCPU16::DCPU16()
{
    reset();
}

void DCPU16::reset()
{
    pc = 0;
    sp = 0;
    overflow = 0;
    clock = 0;
    //last_instruction = 0; //TODO initialize last instruction or make sure it has a constructor
    error = ERROR_NONE;

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

    InstructionData instruction = nextInstruction();
    last_instruction = instruction;

    uint16_t op    = instruction.op;
    uint16_t oa    = instruction.oa;
    uint16_t a     = instruction.a;
    uint16_t b     = instruction.b;
    uint16_t *aptr = instruction.aptr;

    /* true if a conditional fails. */
    bool skip_next = false;

    /* signed 32 bit values to perform intermediate operation with. */
    int64_t a64 = a, b64 = b;

    switch(op)
    {
    case SET: 
        writePtr(aptr, b);
        break;

    case ADD:
        writePtr(aptr, a + b);
        overflow = a64 + b64 > 0xFFFF ? 0x0001 : 0x0000;
        break;

    case SUB:
        writePtr(aptr, a - b);
        overflow = a64 - b64 < 0 ? 0xFFFF : 0x0000;
        break;

    case MUL:
        writePtr(aptr, a * b);
        overflow = ((a64 * b64) >> 16) & 0xFFFF;
        break;

    case DIV:
        if(b == 0)
        {
            writePtr(aptr, 0);
            overflow = 0x0000;
        }
        else
        {
            writePtr(aptr, a / b);
            overflow = ((a64 << 16) / b64) & 0xFFFF;
        }
        break;

    case MOD:
            writePtr(aptr, b == 0 ? 0 : a % b);
        break;

    case SHL:
        writePtr(aptr, a << b);
        overflow = ((a64 << b64) >> 16) & 0xFFFF;
        break;

    case SHR:
        writePtr(aptr, a >> b);
        overflow = ((a64 << 16) >> b64) & 0xFFFF;
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
        switch(oa)
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

    clock += instruction.cycles;

    if(skip_next)
    {
        nextInstruction();
        clock += 2;
    }
}

/*
 * Reads the next instruction, processes its operands, and advances the program
 * counter.
 *
 * Note: No cycles are added.
 */
InstructionData DCPU16::nextInstruction()
{
    InstructionData data;

    data.instruction_address = pc;
    data.instruction         = mem[pc++];
    data.cycles              = getInstructionCycles(data.instruction);
    splitInstruction(data.instruction, &data.op, &data.oa, &data.ob);


    if(data.op != EXT)
    {
        processOperand(data.oa, &data.aptr, &data.a);
        processOperand(data.ob, &data.bptr, &data.b);
    }
    else
    {
        data.a = 0;
        data.aptr = NULL;
        processOperand(data.ob, &data.bptr, &data.b);
    }

    return data;
}

void DCPU16::splitInstruction(uint16_t instruction, uint16_t *op, uint16_t *oa, uint16_t *ob)
{
    *op = (instruction >> INST_OP_SHIFT) & INST_OP_MASK;
    *oa = (instruction >> INST_VA_SHIFT) & INST_VA_MASK;
    *ob = (instruction >> INST_VB_SHIFT) & INST_VB_MASK;
}

void DCPU16::processOperand(uint16_t operand, uint16_t **ptr, uint16_t *value)
{
    /*
     * When indexing registers it's possible to compute the index as (operand %
     * NUM_REGISTERS). This works since register indices are aligned to
     * NUM_REGISTERS boundaries.
     */

    *ptr = NULL;

    if(operand <= OPERAND_REGISTER) //TODO check overflow works correctly here
        *ptr = reg + operand;
    else if(operand <= OPERAND_REGISTER_PTR)
        *ptr = mem + reg[operand % NUM_REGISTERS];
    else if(operand <= OPERAND_REGISTER_NEXT_WORD_PTR)
        *ptr = mem + mem[pc++] + reg[operand % NUM_REGISTERS];
    else if(OPERAND_POP == operand)
        *ptr = mem + sp++;
    else if(OPERAND_PEEK == operand)
        *ptr = mem + sp;
    else if(OPERAND_PUSH == operand)
        *ptr = mem + --sp;
    else if(OPERAND_SP == operand)
        *ptr = &sp;
    else if(OPERAND_PC == operand)
        *ptr = &pc;
    else if(OPERAND_OVERFLOW == operand)
        *ptr = &overflow;
    else if(OPERAND_NEXT_WORD_PTR == operand)
        *ptr = mem + mem[pc++];
    else if(OPERAND_NEXT_WORD_LITERAL == operand)
        *ptr = mem + pc++;
    else
        *value = operand - OPERAND_LITERAL;

    if(*ptr) *value = **ptr;
}

/*
 * Gets the total cycle count for an instruction. This includes the cycles
 * required by the the instruction's operation and operands.
 *
 * @param instruction Instruction to get the cycle count for.
 *
 * @return The number of cycles the instruction takes.
 */
int DCPU16::getInstructionCycles(uint16_t instruction)
{
    uint16_t op, oa, ob;
    splitInstruction(instruction, &op, &oa, &ob);

    int cycles = getOperationCycles(instruction);
    cycles    += getOperationCycles(ob);
    cycles    += op == EXT ? 0 : getOperandCycles(oa);

    return cycles;
}

/*
 * @param instruction The instruction to get the operaion cycle count for.
 *
 * @return The number of cycles the instruction's operation takes.
 */
int DCPU16::getOperationCycles(uint16_t instruction)
{
    uint16_t op, oa, ob;
    splitInstruction(instruction, &op, &oa, &ob);

    switch(op)
    {
    case SET: return 1;
    case ADD: return 2;
    case SUB: return 2;
    case MUL: return 2;
    case DIV: return 3;
    case MOD: return 3;
    case SHL: return 2;
    case SHR: return 2;
    case AND: return 1;
    case BOR: return 1;
    case XOR: return 1;
    case IFE: return 2;
    case IFN: return 2;
    case IFG: return 2;
    case IFB: return 2;
    default:
        switch(oa)
        {
            case JSR: return 2;
            default:  return 0;
        }
    }

    return 0;
}

/*
 * @param operand Operand extracted from an instruction.
 *
 * @return The number of cycles the operand takes.
 */
int DCPU16::getOperandCycles(uint16_t operand)
{
    if(OPERAND_REGISTER_PTR < operand && operand <= OPERAND_REGISTER_NEXT_WORD_PTR)
        return 1;
    if(OPERAND_NEXT_WORD_PTR == operand)
        return 1;
    if(OPERAND_LITERAL == operand)
        return 1;
    return 0;
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
    std::cout << "instruction     = " << std::hex << last_instruction.instruction << "\n";
    std::cout << "stack pointer   = " << sp << "\n";
    std::cout << "stack peek      = " << mem[sp] << "\n";
    std::cout << "overflow        = " << overflow << "\n";
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

