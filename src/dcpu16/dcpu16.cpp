#include <cmath>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <iomanip>

#include "dcpu16.h"


InstructionData::InstructionData()
{
    instruction = 0;
    instruction_address = 0;
    op = oa = ob = 0;
    a = b = 0;
    aptr = bptr = NULL;
    cycles = 0;
}


DCPU16::DCPU16()
{
    reset();
}

void DCPU16::reset()
{
    pc = 0;
    sp = 0;
    ex = 0;
    ia = 0;
    clock = 0;
    error = ERROR_NONE;
    last_instruction = InstructionData();

    std::fill(mem, mem+MEMORY_SIZE, 0);
    std::fill(mem_flags, mem_flags+MEMORY_SIZE, 0);
    std::fill(reg, mem+NUM_REGISTERS, 0);
}

void DCPU16::loadProgram(const uint16_t *words, uint16_t num_words)
{
    reset();
    std::copy(words, words+num_words, mem);
}

void DCPU16::step()
{
    if(error)
        return;

    if(interrupt_count > 0)
        beginInterrupt(interrupt_queue[--interrupt_count]);

    InstructionData instruction = nextInstruction();
    last_instruction = instruction;

    uint16_t op    = instruction.op;
    uint16_t ob    = instruction.ob;
    uint16_t a     = instruction.a;
    uint16_t b     = instruction.b;
    uint16_t *aptr = instruction.aptr;
    uint16_t *bptr = instruction.bptr;

    /* true if a conditional fails. */
    bool skip_next = false;

    if(op != EXT)
        doOpcode(op, a, b, bptr, &skip_next);
    else if(ob != EXT)
        doOpcodeExt0(ob, a, b, aptr);
    else
        setError(ERROR_OPCODE_INVALID);

    clock += instruction.cycles;

    if(skip_next)
    {
        nextInstruction();
        clock += 2;
    }
}

void DCPU16::doOpcode(uint16_t op, uint16_t a, uint16_t b, uint16_t *bptr, bool *skip_next)
{
    /* signed 64 bit values to perform intermediate operation with. */
    int64_t a64 = a, b64 = b;
    int16_t sa = a;
    int16_t sb = b;

    switch(op)
    {
    case SET: 
        writePtr(bptr, a);
        break;

    case ADD:
        writePtr(bptr, b + a);
        ex = b64 + a64 > 0xFFFF ? 0x0001 : 0x0000;
        break;

    case SUB:
        writePtr(bptr, b - a);
        ex = b64 - a64 < 0 ? 0xFFFF : 0x0000;
        break;

    case MUL:
        writePtr(bptr, b * a);
        ex = ((b64 * a64) >> 16) & 0xFFFF;
        break;

    case MLI:
        writePtr(bptr, sb * sa);
        ex = ((b64 * a64) >> 16) & 0xFFFF;
        break;

    case DIV:
        if(a == 0)
        {
            writePtr(bptr, 0);
            ex = 0x0000;
        }
        else
        {
            writePtr(bptr, b / a);
            ex = ((b64 << 16) / a64) & 0xFFFF;
        }
        break;

    case DVI:
        if(a == 0)
        {
            writePtr(bptr, 0);
            ex = 0x0000;
        }
        else
        {
            int16_t result = sb / sa;
            result = result > 0 ? result : std::ceil(result);
            writePtr(bptr, std::ceil(sb / sa));
            ex = ((b64 << 16) / a64) & 0xFFFF;
        }
        break;

    case MOD:
        writePtr(bptr, a == 0 ? 0 : b % a);
        break;

    case MDI:
        writePtr(bptr, a == 0 ? 0 : b - a * (b/a));
        break;

    case AND:
        writePtr(bptr, b & a);
        break;

    case BOR:
        writePtr(bptr, b | a);
        break;

    case XOR:
        writePtr(bptr, b ^ a);
        break;

    case SHR:
        writePtr(bptr, b >> a);
        ex = ((b64 << 16) >> a64) & 0xFFFF;
        break;

    case ASR:
        writePtr(bptr, arithmeticShift(b, a));
        ex = ((b64 << 16) >> a64) & 0xFFFF;
        break;

    case SHL:
        writePtr(bptr, b << a);
        ex = ((b64 << a64) >> 16) & 0xFFFF;
        break;

    case IFB:
        *skip_next = !((b & a) != 0);
        break;

    case IFC:
        *skip_next = !((b & a) == 0);
        break;

    case IFE:
        *skip_next = !(b == a);
        break;

    case IFN:
        *skip_next = !(b != a);
        break;

    case IFG:
        *skip_next = !(b > a);
        break;

    case IFA:
        *skip_next = !(sb > sa);
        break;

    case IFL:
        *skip_next = !(b < a);
        break;

    case IFU:
        *skip_next = !(sb < sa);
        break;

    case ADX:
        writePtr(bptr, b + a + ex);
        ex = b64 + a64 + ex > 0 ? 0x0001 : 0x0000;
        break;

    case SBX:
        writePtr(bptr, b - a + ex);
        ex = b64 - a64 + ex < 0 ? 0xFFFF : 0x0000;
        break;

    case STI:
        writePtr(bptr, a);
        reg[REG_I]++;
        reg[REG_J]++;
        break;

    case STD:
        writePtr(bptr, a);
        reg[REG_I]--;
        reg[REG_J]--;
        break;

    default:
        setError(ERROR_OPCODE_INVALID);
        break;
    }
}

void DCPU16::doOpcodeExt0(uint16_t op, uint16_t a, uint16_t b, uint16_t *aptr)
{
    switch(op)
    {
    case JSR:
        mem[--sp] = pc;
        pc = a;
        break;

    case INT:
        if(ia != 0 && interrupt_queueing)
        {
            if(interrupt_count == MAX_INTERRUPTS)
                setError(ERROR_INTERRUPT_QUEUE_FULL);
            else
                interrupt_queue[interrupt_count++] = a;
        }
        else if(ia != 0)
        {
            beginInterrupt(a);
        }
        break;

    case IAG:
        writePtr(aptr, ia);
        break;

    case IAS:
        ia = a;
        break;

    case RFI:
        endInterrupt();
        break;

    case IAQ:
        interrupt_queueing = a == 0 ? false : true;
        break;

    case HWN:
        writePtr(aptr, devices.size());
        break;

    case HWQ:
        //TODO
        if(a < devices.size())
        {
            uint32_t hid = devices[a].getHardwareID();
            uint16_t ver = devices[a].getHardwareVersion();
            uint32_t mid = devices[a].getManufacturerID();

            reg[REG_A] = uint16_t(hid & 0xFFFF);
            reg[REG_B] = uint16_t((hid >> 16) & 0xFFFF);
            reg[REG_C] = ver;
            reg[REG_X] = uint16_t(mid & 0xFFFF);
            reg[REG_Y] = uint16_t((mid >> 16) & 0xFFFF);
        }
        else
        {
            // TODO warn a is invalid
            reg[REG_A] = 0;
            reg[REG_B] = 0;
            reg[REG_C] = 0;
            reg[REG_X] = 0;
            reg[REG_Y] = 0;
        }
        break;

    case HWI:
        if(a < devices.size())
            devices[a].interrupt();
        // TODO warn a is invalid
        break;

    default:
        setError(ERROR_OPCODE_INVALID);
        break;
    }
}

uint16_t DCPU16::arithmeticShift(uint16_t i, uint16_t s)
{
    s = std::max(s, (uint16_t)16);
    uint16_t r = i >> s;

    if(i & 0x8000) /* MSB is set */
    {
        for(int i = 0; i < s; i++)
            r = r | (1 << (15-i));
    }

    return r;
}

void DCPU16::beginInterrupt(uint16_t msg)
{
    interrupt_queueing = true;
    mem[sp--] = pc;
    mem[sp--] = reg[REG_A];
    pc = ia;
    reg[REG_A] = msg;
}

void DCPU16::endInterrupt()
{
    interrupt_queueing = false;
    pc = mem[sp++];
    reg[REG_A] = mem[sp++];
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
        processOperand(data.oa, &data.aptr, &data.a, OPERAND_SOURCE_A);
        processOperand(data.ob, &data.bptr, &data.b, OPERAND_SOURCE_B);
    }
    else
    {
        data.a = 0;
        data.aptr = NULL;
        processOperand(data.ob, &data.bptr, &data.b, OPERAND_SOURCE_B);
    }

    return data;
}

void DCPU16::splitInstruction(uint16_t instruction, uint16_t *op, uint16_t *oa, uint16_t *ob) const
{
    *op = (instruction & INST_OP_MASK) >> INST_OP_SHIFT;
    *oa = (instruction & INST_VA_MASK) >> INST_VA_SHIFT;
    *ob = (instruction & INST_VB_MASK) >> INST_VB_SHIFT;
}

void DCPU16::processOperand(uint16_t operand, uint16_t **ptr, uint16_t *value, char source)
{
    /*
     * When indexing registers it's possible to compute the index as (operand %
     * NUM_REGISTERS). This works since register indices are aligned to
     * NUM_REGISTERS boundaries.
     */

    *ptr = NULL;

    if(operand <= OPERAND_REGISTER)
        *ptr = reg + operand;
    else if(operand <= OPERAND_REGISTER_PTR)
        *ptr = mem + reg[operand % NUM_REGISTERS];
    else if(operand <= OPERAND_REGISTER_NEXT_WORD_PTR)
        *ptr = mem + mem[pc++] + reg[operand % NUM_REGISTERS];
    else if(OPERAND_PUSH_POP == operand && OPERAND_SOURCE_A == source) // POP
        *ptr = mem + sp++;
    else if(OPERAND_PUSH_POP == operand && OPERAND_SOURCE_B == source) // PUSH
        *ptr = mem + --sp;
    else if(OPERAND_PEEK == operand)
        *ptr = mem + sp;
    else if(OPERAND_SP == operand)
        *ptr = &sp;
    else if(OPERAND_PC == operand)
        *ptr = &pc;
    else if(OPERAND_EX == operand)
        *ptr = &ex;
    else if(OPERAND_NEXT_WORD_PTR == operand)
        *ptr = mem + mem[pc++];
    else if(OPERAND_NEXT_WORD_LITERAL == operand)
        *value = mem[pc++];
    else
        /* literals in the range [-1, 30] */
        *value = operand - OPERAND_LITERAL - 1;

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
int DCPU16::getInstructionCycles(uint16_t instruction) const
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
int DCPU16::getOperationCycles(uint16_t instruction) const
{
    uint16_t op, oa, ob;
    splitInstruction(instruction, &op, &oa, &ob);

    if(op != EXT)
    {
        switch(op)
        {
            case SET: return 1;
            case ADD: return 2;
            case SUB: return 2;
            case MUL: return 2;
            case MLI: return 2;
            case DIV: return 3;
            case DVI: return 3;
            case MOD: return 3;
            case MDI: return 3;
            case AND: return 1;
            case BOR: return 1;
            case XOR: return 1;
            case SHR: return 1;
            case ASR: return 1;
            case SHL: return 1;
            case IFB: return 2;
            case IFC: return 2;
            case IFE: return 2;
            case IFN: return 2;
            case IFG: return 2;
            case IFA: return 2;
            case IFL: return 2;
            case IFU: return 2;
            case ADX: return 3;
            case SBX: return 3;
            case STI: return 2;
            case STD: return 2;
            default:  break;
        }
    }
    else if(ob != EXT)
    {
        switch(ob)
        {
            case JSR: return 3;
            case INT: return 4;
            case IAG: return 1;
            case IAS: return 1;
            case RFI: return 3;
            case IAQ: return 2;
            case HWN: return 2;
            case HWQ: return 4;
            case HWI: return 4;
            default:  break;
        }
    }

    return 0;
}

/*
 * @param operand Operand extracted from an instruction.
 *
 * @return The number of cycles the operand takes.
 */
int DCPU16::getOperandCycles(uint16_t operand) const
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

bool DCPU16::attachDevice(Device device, uint16_t *device_id)
{
    if(devices.size() >= MAX_DEVICES)
        return false;

    devices.push_back(device);
    *device_id = uint16_t(devices.size() - 1);
    return true;
}

void DCPU16::detachAllDevices()
{
    devices.clear();
}

int DCPU16::getError() const
{
    return error;
}

const char* DCPU16::getErrorString(int err) const
{
    switch(err)
    {
        case ERROR_NONE:                    return "ERROR_NONE";
        case ERROR_STACK_OVERFLOW:          return "ERROR_STACK_OVERFLOW";
        case ERROR_STACK_UNDERFLOW:         return "ERROR_STACK_UNDERFLOW";
        case ERROR_OPCODE_INVALID:          return "ERROR_OPCODE_INVALID";
        case ERROR_INTERRUPT_QUEUE_FULL:    return "ERROR_INTERRUPT_QUEUE_FULL";
        default: break;
    }

    return "UNKNOWN";
}

void DCPU16::setError(int err)
{
    //TODO print info to print object thing?
    error = err;
    std::cout << getErrorString(error);
}

uint16_t DCPU16::read(uint32_t addr) const
{
    if(addr < MEMORY_SIZE)
        return mem[addr];
    else if(RW_REGISTER_0 <= addr && addr <= RW_REGISTER_7)
        return reg[addr - RW_REGISTER_0];
    else if(RW_REGISTER_PTR_0 <= addr && addr <= RW_REGISTER_PTR_7)
        return mem[reg[addr - RW_REGISTER_PTR_0]];
    else if(RW_PROGRAM_COUNTER == addr)
        return pc;
    else if(RW_PROGRAM_COUNTER_PTR == addr)
        return mem[pc];
    else if(RW_STACK_POINTER == addr)
        return sp;
    else if(RW_STACK_POINTER_PTR == addr)
        return mem[sp];
    else if(RW_EXCESS == addr)
        return ex;
    else if(RW_INTERRUPT_ADDRESS == addr)
        return ia;

    return 0;
}

void DCPU16::write(uint32_t addr, uint16_t value) 
{
    if(addr < MEMORY_SIZE)
        mem[addr] = value;
    else if(RW_REGISTER_0 <= addr && addr <= RW_REGISTER_7)
        reg[addr - RW_REGISTER_0] = value;
    else if(RW_REGISTER_PTR_0 <= addr && addr <= RW_REGISTER_PTR_7)
        mem[reg[addr - RW_REGISTER_PTR_0]] = value;
    else if(RW_PROGRAM_COUNTER == addr)
        pc = value;
    else if(RW_PROGRAM_COUNTER_PTR == addr)
        mem[pc] = value;
    else if(RW_STACK_POINTER == addr)
        sp = value;
    else if(RW_STACK_POINTER_PTR == addr)
        mem[sp] = value;
    else if(RW_EXCESS == addr)
        ex = value;
    else if(RW_INTERRUPT_ADDRESS == addr)
        ia = value;
}

const uint16_t* DCPU16::memoryPointer() const
{
    return mem;
}

uint64_t DCPU16::getCycles() const
{
    return clock;
}

size_t DCPU16::serialize(uint8_t *buffer) const
{
    //TODO
    return 0;
}

void DCPU16::deserialize(uint8_t *buffer)
{
    //TODO
}

void DCPU16::printState() const
{
    std::cout << "clock           = " << std::dec << clock << "\n";
    std::cout << "program counter = " << std::dec << pc << "\n";
    std::cout << "instruction     = " << std::hex << last_instruction.instruction << "\n";
    std::cout << "stack pointer   = " << sp << "\n";
    std::cout << "stack peek      = " << mem[sp] << "\n";
    std::cout << "ex              = " << ex << "\n";
    std::cout << "ia              = " << ia << "\n";
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

