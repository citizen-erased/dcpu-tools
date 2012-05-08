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
     * 16 bit instruction. Bits are used as follows:
     *   aaaaaabbbbbooooo
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


    InstructionData();
};


class DCPU16
{
/*---------------------------------------------------------------------------
 * Constants
 *--------------------------------------------------------------------------*/
public:
    enum
    {
        EXT = 0x00,
        SET = 0x01,
        ADD = 0x02,
        SUB = 0x03,
        MUL = 0x04,
        MLI = 0x05,
        DIV = 0x06,
        DVI = 0x07,
        MOD = 0x08,
        MDI = 0x09,
        AND = 0x0A,
        BOR = 0x0B,
        XOR = 0x0C,
        SHR = 0x0D,
        ASR = 0x0E,
        SHL = 0x0F,
        IFB = 0x10,
        IFC = 0x11,
        IFE = 0x12,
        IFN = 0x13,
        IFG = 0x14,
        IFA = 0x15,
        IFL = 0x16,
        IFU = 0x17,
        ADX = 0x1A,
        SBX = 0x1B,
        STI = 0x1E,
        STD = 0x1F,
    };

    enum
    {
        JSR = 0x01,
        INT = 0x08,
        IAG = 0x09,
        IAS = 0x0a,
        RFI = 0x0b,
        IAQ = 0x0c,
        HWN = 0x10,
        HWQ = 0x11,
        HWI = 0x12,
    };

    enum
    {
        INST_OP_SHIFT = 0,
        INST_OP_MASK  = 0x1F,

        INST_VA_SHIFT = 5,
        INST_VA_MASK  = 0x3E0,

        INST_VB_SHIFT = 10,
        INST_VB_MASK  = 0xFC00,
    };

    enum
    {
        /*
         * No error is always 0 so conditionals can evaluate to false on no
         * error.
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
        REG_A = 0x00,
        REG_B = 0x01,
        REG_C = 0x02,
        REG_X = 0x03,
        REG_Y = 0x04,
        REG_Z = 0x05,
        REG_I = 0x06,
        REG_J = 0x07,

        NUM_REGISTERS,
    };

    enum
    {
        OPERAND_REGISTER                = 0x07,
        OPERAND_REGISTER_PTR            = 0x0F,
        OPERAND_REGISTER_NEXT_WORD_PTR  = 0x17,
        OPERAND_PUSH_POP                = 0x18,
        OPERAND_PEEK                    = 0x19,
        OPERAND_PICK                    = 0x1A,
        OPERAND_SP                      = 0x1B,
        OPERAND_PC                      = 0x1C,
        OPERAND_EX                      = 0x1D,
        OPERAND_NEXT_WORD_PTR           = 0x1E,
        OPERAND_NEXT_WORD_LITERAL       = 0x1F,

        /* Offset where literals start. */
        OPERAND_LITERAL                 = 0x20,

        /* Instruction bits an operand was extracted from. */
        OPERAND_SOURCE_A,
        OPERAND_SOURCE_B,
    };

    /*
     * Read/write constants to select what to get back when calling read() ,
     * and what to write to when calling the write().  Since the function
     * accepts memory addresses, constants must not overlap the memory address
     * space.
     *
     * Register constants must be sequential. This allows looping through the
     * CPU's registers and adding an offset to RW_REGISTER_0 to read/write all
     * register values.
     */
    enum
    {
        RW_REGISTER_0 = MEMORY_SIZE,
        RW_REGISTER_1,
        RW_REGISTER_2,
        RW_REGISTER_3,
        RW_REGISTER_4,
        RW_REGISTER_5,
        RW_REGISTER_6,
        RW_REGISTER_7,
        RW_REGISTER_PTR_0,
        RW_REGISTER_PTR_1,
        RW_REGISTER_PTR_2,
        RW_REGISTER_PTR_3,
        RW_REGISTER_PTR_4,
        RW_REGISTER_PTR_5,
        RW_REGISTER_PTR_6,
        RW_REGISTER_PTR_7,
        RW_PROGRAM_COUNTER,
        RW_PROGRAM_COUNTER_PTR,
        RW_STACK_POINTER,
        RW_STACK_POINTER_PTR,
        RW_EXCESS,
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

    uint16_t ex;
    uint16_t ia;
    uint16_t reg[NUM_REGISTERS];
    uint16_t mem[MEMORY_SIZE];
    uint8_t  mem_flags[MEMORY_SIZE];

    uint64_t clock;
    int      error;

    InstructionData last_instruction;
    

/*---------------------------------------------------------------------------
 * Initialization
 *--------------------------------------------------------------------------*/
public:
                        DCPU16();


/*---------------------------------------------------------------------------
 * Instruction Processing
 *--------------------------------------------------------------------------*/
public:
    InstructionData     nextInstruction();
    void                splitInstruction(uint16_t instruction, uint16_t *op, uint16_t *oa, uint16_t *ob) const;

    int                 getInstructionCycles(uint16_t instruction) const;
    int                 getOperationCycles(uint16_t instruction) const;
    int                 getOperandCycles(uint16_t operand) const;

private:
    void                processOperand(uint16_t operand, uint16_t **ptr, uint16_t *value, char source);
    void                doOpcode(uint16_t op, uint16_t a, uint16_t b, uint16_t *bptr, bool *skip_next);
    void                doOpcodeExt0(uint16_t op, uint16_t a, uint16_t b, uint16_t *aptr);


/*---------------------------------------------------------------------------
 * CPU 
 *--------------------------------------------------------------------------*/
public:
    void                loadProgram(const uint16_t *words, uint16_t num_words);
    void                step();
    void                reset();
    uint64_t            getCycles() const;


/*---------------------------------------------------------------------------
 * Memory Operations
 *--------------------------------------------------------------------------*/
public:
    uint16_t            read(uint32_t addr) const;
    void                write(uint32_t addr, uint16_t value);
    const uint16_t*     memoryPointer() const;

private:
    bool                writePtr(uint16_t *ptr, uint16_t value);


/*---------------------------------------------------------------------------
 * Error State
 *--------------------------------------------------------------------------*/
public:
    int                 getError() const;
    const char*         getErrorString(int err) const;

private:
    void                setError(int err);


/*---------------------------------------------------------------------------
 * Serialization
 *--------------------------------------------------------------------------*/
public:
    size_t              serialize(uint8_t *buffer) const;
    void                deserialize(uint8_t *buffer);


/*---------------------------------------------------------------------------
 * Debug
 *--------------------------------------------------------------------------*/
public:
    void                printState() const;
};

#endif /* DCPU16_H_ */

