/*
 * Stack:
 * Grows down from 0xFFFF.
 *
 */

#ifndef DCPU16_H_
#define DCPU16_H_

#include "../library/pstdint.h"

class DCPU16
{
/*---------------------------------------------------------------------------
 * Constants
 *--------------------------------------------------------------------------*/
public:
    enum
    {
        EXT = 0x0,
        SET = 0x1,
        ADD = 0x2,
        SUB = 0x3,
        MUL = 0x4,
        DIV = 0x5,
        MOD = 0x6,
        SHL = 0x7,
        SHR = 0x8,
        AND = 0x9,
        BOR = 0xa,
        XOR = 0xB,
        IFE = 0xC,
        IFN = 0xD,
        IFG = 0xE,
        IFB = 0xF,
    };

    enum
    {
        JSR = 0x01,
    };

    enum
    {
        INST_OP_SHIFT = 0,
        INST_OP_MASK  = 0xF,

        INST_VA_SHIFT = 4,
        INST_VA_MASK  = 0x3F,

        INST_VB_SHIFT = 10,
        INST_VB_MASK  = 0x3F,
    };

    enum
    {
        /*
         * Always 0 so conditionals can evaluate to false on no error.
         */
        ERROR_NONE = 0,
        ERROR_STACK_OVERFLOW,
        ERROR_STACK_UNDERFLOW,
        ERROR_OPCODE_INVALID,
    };

    enum
    {
        MEMORY_SIZE = 0x10000,
    };

    enum
    {
        NUM_REGISTERS = 8,
    };


/*---------------------------------------------------------------------------
 * Members
 *--------------------------------------------------------------------------*/
public:
    uint16_t pc;

    /*
     * Stack pointer stats at 0. The first push decreses the value to 0xFFFF.
     */
    uint16_t sp;

    uint16_t overflow;
    uint16_t reg[NUM_REGISTERS];
    uint16_t mem[MEMORY_SIZE];
    uint8_t  mem_flags[MEMORY_SIZE];

    uint64_t clock;
    uint64_t last_instruction;
    int      error;
    bool     skip_next; //TODO make into flags with other state?

    uint16_t op_cycles[16];
    uint16_t ext1_op_cycles[16];
    uint16_t operand_cycles[0x20];
    


/*---------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------*/
public:
    DCPU16();

    void reset();
    void tick();

    void loadProgram(const uint16_t *words, uint16_t num_words);

    void printState();
    int getError() const;
private:
    void setError(int err);


private:
    void processOperand(uint16_t operand, uint16_t **ptr, uint16_t *value);
    bool writePtr(uint16_t *ptr, uint16_t value);
};

#endif /* DCPU16_H_ */

