#include <cassert>
#include <cstdio>
#include "disassembler.h"

void Disassembler::disassemble(uint16_t *words, uint16_t num_words)
{
    instructions.clear();
    dcpu.loadProgram(words, num_words);

    while(dcpu.read(DCPU16::RW_PROGRAM_COUNTER) < num_words)
    {
        Instruction inst;
        inst.address = dcpu.read(DCPU16::RW_PROGRAM_COUNTER);
        inst.index = static_cast<uint16_t>(instructions.size());
        InstructionData data = dcpu.nextInstruction();

        sprintf(inst.address_str, "0x%04X", (int)inst.address);
        sprintf(inst.operation_str, "%s", Disassembler::getOperationName(data.op, data.oa));

        if(DCPU16::EXT != data.op)
        {
            getOperandStr(data.oa, data.aptr, data.a, DCPU16::OPERAND_SOURCE_A, inst.operand_a_str);
            getOperandStr(data.ob, data.bptr, data.b, DCPU16::OPERAND_SOURCE_B, inst.operand_b_str);
        }
        else
        {
            getOperandStr(data.ob, data.bptr, data.b, DCPU16::OPERAND_SOURCE_B, inst.operand_a_str);
            inst.operand_b_str[0] = 0;
        }

        instructions.push_back(inst);
    }
}

void Disassembler::getOperandStr(uint16_t operand, uint16_t *ptr, uint16_t value, char source, char *str)
{
    if(operand <= DCPU16::OPERAND_REGISTER)
        sprintf(str, "%s", getRegisterName(operand));
    else if(operand <= DCPU16::OPERAND_REGISTER_PTR)
        sprintf(str, "[%s]", getRegisterName(operand));
    else if(operand <= DCPU16::OPERAND_REGISTER_NEXT_WORD_PTR)
        sprintf(str, "[0x%04X + %s]", (int)value, getRegisterName(operand));
    else if(DCPU16::OPERAND_PUSH_POP == operand && DCPU16::OPERAND_SOURCE_A == source)
        sprintf(str, "%s", "POP");
    else if(DCPU16::OPERAND_PUSH_POP == operand && DCPU16::OPERAND_SOURCE_B == source)
        sprintf(str, "%s", "PUSH");
    else if(DCPU16::OPERAND_PEEK == operand)
        sprintf(str, "%s", "PEEK");
    else if(DCPU16::OPERAND_SP == operand)
        sprintf(str, "%s", "SP");
    else if(DCPU16::OPERAND_PC == operand)
        sprintf(str, "%s", "PC");
    else if(DCPU16::OPERAND_EX == operand)
        sprintf(str, "%s", "0");
    else if(DCPU16::OPERAND_NEXT_WORD_PTR == operand)
        sprintf(str, "[0x%04X]", (int)(ptr - dcpu.memoryPointer()));
    else if(DCPU16::OPERAND_NEXT_WORD_LITERAL == operand)
        sprintf(str, "0x%04X", (int)value);
    else
        sprintf(str, "0x%04X", (int)value);
}

const Disassembler::Instruction* Disassembler::getInstruction(uint16_t index) const
{
    if(index < instructions.size())
        return &instructions[index];
    return NULL;
}

const Disassembler::Instruction* Disassembler::findInstructionFromAddress(uint16_t address) const
{
    for(size_t i = 0; i < instructions.size(); i++)
        if(address == instructions[i].address)
            return &instructions[i];
    return NULL;
}

size_t Disassembler::getInstructionCount() const
{
    return instructions.size();
}

const char* Disassembler::getOperationName(uint16_t op, uint16_t oa)
{
    switch(op)
    {
    case DCPU16::SET: return "SET";
    case DCPU16::ADD: return "ADD";
    case DCPU16::SUB: return "SUB";
    case DCPU16::MUL: return "MUL";
    case DCPU16::DIV: return "DIV";
    case DCPU16::MOD: return "MOD";
    case DCPU16::SHL: return "SHL";
    case DCPU16::SHR: return "SHR";
    case DCPU16::AND: return "AND";
    case DCPU16::BOR: return "BOR";
    case DCPU16::XOR: return "XOR";
    case DCPU16::IFE: return "IFE";
    case DCPU16::IFN: return "IFN";
    case DCPU16::IFG: return "IFG";
    case DCPU16::IFB: return "IFB";
    case DCPU16::EXT:
        switch(oa)
        {
        case DCPU16::JSR: return "JSR";
        }

    default: break;
    }
    
    return "???";
}

const char* Disassembler::getRegisterName(uint16_t i)
{
    switch(i % DCPU16::NUM_REGISTERS)
    {
        case 0: return "A";
        case 1: return "B";
        case 2: return "C";
        case 3: return "X";
        case 4: return "Y";
        case 5: return "Z";
        case 6: return "I";
        case 7: return "J";
        default: break;
    }

    return "?";
}

