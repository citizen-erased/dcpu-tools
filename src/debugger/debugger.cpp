#include "debugger.h"


void Debugger::loadProgram(uint16_t *words, uint16_t num_words)
{
    initial_state = DCPU16();
    initial_state.loadProgram(words, num_words);
    reset();
}

void Debugger::run()
{
    while(!dcpu.getError())
        step(INT_MAX);
}

void Debugger::step(int steps)
{
    if(steps > 0)
    {
        for(int i = 0; i < steps && !dcpu.getError(); i++)
        {
            pushHistory();
            dcpu.step();
        }
    }
    else if(steps < 0)
    {
        popHistory(steps);
    }
}

void Debugger::reset()
{
    dcpu = initial_state;
    history.clear();
}

void Debugger::setRegister(uint16_t register, uint16_t value)
{

}

DCPU16& Debugger::getDCPU()
{
    return dcpu;
}

const DCPU16& Debugger::getDCPU() const
{
    return dcpu;
}

void Debugger::pushHistory()
{
    //TODO serialize and compress
    history.push_back(dcpu);

    //TODO max history size
    if(history.size() > 100)
        history.pop_front();
}

void Debugger::popHistory(int n)
{
    if(history.empty())
        return;

    dcpu = history.back();
    history.pop_back();
}

