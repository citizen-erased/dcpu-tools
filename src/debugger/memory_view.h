#ifndef DEBUGGER_MEMORY_VIEW_H
#define DEBUGGER_MEMORY_VIEW_H

#include <FL/Fl_Group.H>


class Fl_Scrollbar;
class DCPU16;


class MemoryView : public Fl_Group
{
private:
    DCPU16 *dcpu;
    Fl_Scrollbar *scrollbar;

public:
    MemoryView(int X, int Y, int W, int H, DCPU16 *dcpu);

    void draw();

private:
    static void cb_scrollbar(Fl_Widget *w, void *view);
};

#endif /* DEBUGGER_MEMORY_VIEW_H */

