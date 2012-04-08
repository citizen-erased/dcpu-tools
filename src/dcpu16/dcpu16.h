/*
 * Stack:
 * Grows down from 0xFFFF.
 *
 */

#ifndef DCPU16_H_
#define DCPU16_H_

#include "../library/pstdint.h"

struct InstructionData
{
    /*
     * 16 bit instruction where:
     *   bbbbbbaaaaaaoooooo
     * o = operation
     * a = operand a
     * b = operand b
     */
    uint16_t instruction;

    /*
     * Address of the instruction in the cpu's memory.
     */
    uint16_t instruction_address;

    /*
     * Operation and two operands extracted from the instruction.
     */
    uint16_t op, oa, ob;

    /*
     * Value each operand resolves to. These can be:
     * - A literal encoded in the operand.
     * - A literal appearing in the words after the instruction.
     * - A value in the cpu's memory.
     * - A register.
     * - The stack pointer.
     * - The program counter.
     * - Overflow.
     * - Undefined if the operand is an extended instruction.
     *
     * If the value is writable a pointer is set to its memory location (in
     * real memory), or NULL otherwise.
     */
    uint16_t a, *aptr, b, *bptr;

    /*
     * The number of cycles the instruction takes. This is the sum of the
     * operation and operand's cycles.
     */
    uint16_t cycles;
};

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

    enum
    {
        OPERAND_REGISTER                = 0x07,
        OPERAND_REGISTER_PTR            = 0x0F,
        OPERAND_REGISTER_NEXT_WORD_PTR  = 0x17,
        OPERAND_POP                     = 0x18,
        OPERAND_PEEK                    = 0x19,
        OPERAND_PUSH                    = 0x1A,
        OPERAND_SP                      = 0x1B,
        OPERAND_PC                      = 0x1C,
        OPERAND_OVERFLOW                = 0x1D,
        OPERAND_NEXT_WORD_PTR           = 0x1E,
        OPERAND_NEXT_WORD_LITERAL       = 0x1F,

        /* Offset where literals start. */
        OPERAND_LITERAL                 = 0x20,
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
    int      error;

    InstructionData last_instruction;
    


/*---------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------*/
public:
    DCPU16();

    void reset();

    void loadProgram(const uint16_t *words, uint16_t num_words);

    void tick();
    InstructionData nextInstruction();
    void splitInstruction(uint16_t instruction, uint16_t *op, uint16_t *oa, uint16_t *ob);

    int getInstructionCycles(uint16_t instruction);
    int getOperationCycles(uint16_t instruction);
    int getOperandCycles(uint16_t operand);

    void printState();
    int getError() const;
private:
    void setError(int err);


private:
    void processOperand(uint16_t operand, uint16_t **ptr, uint16_t *value);
    bool writePtr(uint16_t *ptr, uint16_t value);
};

#endif /* DCPU16_H_ */

