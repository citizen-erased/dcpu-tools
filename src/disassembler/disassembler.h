#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include <vector>
#include "../dcpu16/dcpu16.h"

class Disassembler
{
public:
    struct Instruction
    {
        uint16_t address;
        uint16_t index;
        char address_str[8];
        char operation_str[8];
        char operand_a_str[32];
        char operand_b_str[32];
    };


public:
    static const char* getOperationName(uint16_t op, uint16_t oa);
    static const char* getRegisterName(uint16_t i);


private:
    DCPU16 dcpu;
    std::vector<Instruction> instructions;

public:
    void disassemble(uint16_t *words, uint16_t num_words);
    const Instruction* getInstruction(uint16_t index) const;
    const Instruction* findInstructionFromAddress(uint16_t address) const;
    size_t getInstructionCount() const;

private:
    void getOperandStr(uint16_t operand, uint16_t *ptr, uint16_t value, char *str);
};

#endif /* DISASSEMBLER_H */

