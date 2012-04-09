#ifndef DEBUGGER_DISASSEMBLY_VIEW_H
#define DEBUGGER_DISASSEMBLY_VIEW_H

#include <FL/Fl_Group.H>

class Fl_Scroll;
class DCPU16;
class Disassembler;

class DisassemblyView : public Fl_Group
{
private:
    DCPU16       *dcpu;
    Disassembler *disassembler;
    Fl_Scrollbar *scrollbar;

public:
    DisassemblyView(int X, int Y, int W, int H, DCPU16 *dcpu, Disassembler *disassembler);

    void draw();

private:
    static void cb_scrollbar(Fl_Widget *w, void *view);
};

#endif /* DEBUGGER_DISASSEMBLY_VIEW_H */

