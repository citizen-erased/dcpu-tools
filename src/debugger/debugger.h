#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <vector>
#include <deque>
#include "../dcpu16/dcpu16.h"

class Debugger
{
private:
    DCPU16 dcpu;
    DCPU16 initial_state;
    std::deque<DCPU16> history;

public:
    void loadProgram(uint16_t *words, uint16_t num_words);
    void run();
    void step(int steps);
    void reset();

    void setRegister(uint16_t register, uint16_t value);

    const DCPU16& getDCPU() const;

    void pushHistory();
    void popHistory(int n);
};

#endif /* DEBUGGER_H */

