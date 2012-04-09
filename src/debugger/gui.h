#ifndef DEBUGGER_GUI_H
#define DEBUGGER_GUI_H

#include <deque>
#include "../dcpu16/dcpu16.h"
#include "../disassembler/disassembler.h"

class Fl_Widget;
class Fl_Double_Window;
class Fl_Group;
class Fl_Spinner;
class Fl_Box;
class Fl_Input;
class MemoryView;
class DisassemblyView;


class DebuggerGUI
{
private:
    DCPU16              dcpu;
    Disassembler        disassembler;
    Fl_Double_Window    *main_window;
    Fl_Input            *pc_input;
    Fl_Input            *sp_input;
    Fl_Input            *overflow_input;
    Fl_Input            *register_inputs[DCPU16::NUM_REGISTERS];
    Fl_Box              *clock_label;
    MemoryView          * memory_view;
    DisassemblyView     *disassembly_view;

    std::deque<DCPU16> history;
    bool running;

public:
    DebuggerGUI(int argc, char *argv[]);

private:
    void init(int argc, char *argv[]);
    void initMenu();
    void initRegisters();
    void initState();
    void initDisassembly();
    void initMemoryViewer();
    void initScreen();
    void initFlowControls();

    void updateState();

    void setRunning(bool run);
    void step(int num_steps);

    void pushHistory();
    void popHistory(int n);

private:
    static void cb_pauseButton(Fl_Widget *w, void *gui);
    static void cb_stepButton(Fl_Widget *w, void *gui);
    static void cb_stepBackButton(Fl_Widget *w, void *gui);
    static void cb_runButton(Fl_Widget *w, void *gui);
    static void cb_runIdle(void *gui);
};

#endif /* DEBUGGER_GUI_H */

