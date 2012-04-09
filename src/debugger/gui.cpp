#include <cassert>
#include <cmath>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Pack.H>

#include "../disassembler/disassembler.h"
#include "memory_view.h"
#include "disassembly_view.h"
#include "gui.h"



void gridByRows(Fl_Group *g, int x, int y, int rows, int widget_height, int xborder, int yborder, int xpad, int ypad)
{
    g->position(x, y);

    int row = 0;
    int max_x = 0;
    int total_col_widths = 0;
    int wx = x + xborder, wy = y + yborder;

    for(int i = 0; i < g->children(); i++)
    {
        Fl_Widget *c = g->child(i);
        c->resize(wx, wy, c->w(), widget_height);

        wy += widget_height + ypad;
        max_x = std::max(max_x, c->w() + xpad);

        row++;

        if(row >= rows)
        {
            total_col_widths += max_x;
            wx += max_x + xpad;
            max_x = 0;
            wy = y + yborder;
            row = 0;
        }
    }

    Fl_Widget *resizable = g->resizable();
    g->resizable(NULL);
    g->size(total_col_widths + xborder*2, widget_height*rows + (ypad * (rows-1)) + yborder*2);
    g->resizable(resizable);
}

/*
void resizeR(Fl_Group *widget, int x, int y, int w, int h)
{
    int offx = x - widget->x();
    int offy = y - widget->y();
    widget->resize(x, y, w, h);

    for(int i = 0; i < widget->children(); i++)
    {
        Fl_Widget *c = widget->child(i);
        resizeR(c, offx + c->x(), offy + c->y(), c->w(), c->h());
    }
}

void horizontalLayout(Fl_Group *g, int x, int y, int w, int h, int xborder, int yborder, int pad, bool scaley)
{
    int wx = x + xborder;
    int max_y = 0;

    for(int i = 0; i < g->children(); i++)
    {
        Fl_Widget *c = g->child(i);
        int wh = scaley ? h - yborder*2 : c->h();
        resizeR(c, wx, yborder, c->w(), wh);
        wx += c->w() + pad;
        max_y = std::max(max_y, c->h());
    }

    g->resize(x, y, wx - pad + xborder, max_y + yborder*2);
}

void verticalLayout(Fl_Group *g, int x, int y, int w, int h, int xborder, int yborder, int pad, bool scalex)
{
    int wy = y + yborder;
    int max_x = 0;

    for(int i = 0; i < g->children(); i++)
    {
        Fl_Widget *c = g->child(i);
        int ww = scalex ? w - xborder*2 : c->w();
        resizeR(c, xborder, wy, ww, c->h());
        wy += c->h() + pad;
        max_x = std::max(max_x, c->w());
    }

    g->resize(x, y, max_x + xborder*2, wy - pad + yborder);
}

void gridLayout(Fl_Group *g, int x, int y, int w, int h, int rows, int cols, int xpad, int ypad, int xborder, int yborder)
{

}

void gridLayoutScale(Fl_Group *g, int x, int y, int w, int h, int rows, int cols, int xpad, int ypad, int xborder, int yborder)
{
    assert(rows > 0 || cols > 0);
    assert(!(rows > 0 && cols > 0));

    g->resize(x, y, w, h);

    int xbegin = x + xborder, ybegin = y + yborder;
    int ww = 0, wh = 0;

    if(cols > 0)
    {
        ww = w - xborder*2 - ((cols-1) * xpad);
        rows = std::ceil((float)g->children() / (float)cols);
        wh = h - yborder*2 - ((rows-1) * ypad);
    }
    else if(rows > 0)
    {
    }

    int row = 0, col = 0;
    int wx = xbegin, wy = ybegin;

    for(int i = 0; i < g->children(); i++)
    {
        Fl_Widget *w = g->child(i);
        w->resize(wx, wy, ww, wh);

        wx += ww + xpad;
        col++;

        if(row > rows)
        {
            row++;
            col = 0;
            wx = xbegin;
            wy = ybegin;
        }
    }
}
*/







DebuggerGUI::DebuggerGUI(int argc, char *argv[])
{
    running = false;

    uint16_t prog[32] = {
        0x7c01, 0x0030, 0x7de1, 0x1000, 0x0020, 0x7803, 0x1000, 0xc00d,
        0x7dc1, 0x001a, 0xa861, 0x7c01, 0x2000, 0x2161, 0x2000, 0x8463,
        0x806d, 0x7dc1, 0x000d, 0x9031, 0x7c10, 0x0018, 0x7dc1, 0x001a,
        0x9037, 0x61c1, 0x7dc1, 0x001a, 0x0000, 0x0000, 0x0000, 0x0000
    };

    dcpu.loadProgram(prog, 28);
    disassembler.disassemble(prog, 28);

    init(argc, argv);
    updateState();
    updateState();
}

void DebuggerGUI::init(int argc, char *argv[])
{
    main_window = new Fl_Double_Window(1024, 768, "DCPU-16 Debugger");

    initMenu();
    initRegisters();
    initState();
    initDisassembly();
    initMemoryViewer();
    initFlowControls();

    main_window->end();
    main_window->show(argc, argv);
}

void DebuggerGUI::initMenu()
{
    Fl_Menu_Item items[] = {
        { "&File",              0, 0, 0, FL_SUBMENU, 0, 0, 0, 0 },
          { "&Open File...",    FL_COMMAND + 'o', 0, 0, 0, 0, 0, 0, 0 },
          { "E&xit",            FL_COMMAND + 'q', 0, 0, 0, 0, 0, 0, 0 },
          { 0, 0, 0, 0, 0, 0, 0, 0, 0 },

        { "&Run",          0, 0, 0, FL_SUBMENU, 0, 0, 0, 0, },
          { "&Run",        FL_COMMAND + 'z', cb_runButton, this, 0, 0, 0, 0, 0 },
          { "&Step",       FL_COMMAND + 'x', cb_stepButton, this, 0, 0, 0, 0, 0 },
          { "&Stop",       FL_COMMAND + 'c', cb_pauseButton, this, 0, 0, 0, 0, 0 },
          { 0, 0, 0, 0, 0, 0, 0, 0, 0 },

      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    }; 

    Fl_Menu_Bar *m = new Fl_Menu_Bar(0, 0, 1024, 30);
    m->copy(items);
}

void DebuggerGUI::initRegisters()
{
    int gx = 5, gy = 650;
    int sw = 64, sh = 24;

    Fl_Group *register_group = new Fl_Group(gx, gy, 0, 0);

    register_group->box(FL_UP_BOX);
    register_group->labelfont(FL_BOLD+FL_ITALIC);
    register_group->labelsize(36);
    register_group->labeltype(FL_SHADOW_LABEL);

    for(int i = 0; i < DCPU16::NUM_REGISTERS; i++)
    {
        Fl_Input *s = new Fl_Input(0, 0, sw, sh);
        s->color(FL_RED);
        s->labelfont(FL_COURIER);
        s->copy_label(Disassembler::getRegisterName(i));
        register_inputs[i] = s;
    }
    register_group->end();

    gridByRows(register_group, gx, gy, 4, sh, 16, 4, 12, 1);
}

void DebuggerGUI::initState()
{
    int gx = 250, gy = 650;
    int sw = 64, sh = 24;

    Fl_Group *state_group = new Fl_Group(gx, gy, 0, 0);

    pc_input = new Fl_Input(0, 0, sw, sh);
    pc_input->copy_label("PC");

    sp_input = new Fl_Input(0, 0, sw, sh);
    sp_input->copy_label("SP");

    overflow_input = new Fl_Input(0, 0, sw, sh);
    overflow_input->copy_label("O");

    clock_label = new Fl_Box(0, 0, sw, sh);
    clock_label->label("Clock: 0");

    state_group->end();

    gridByRows(state_group, gx, gy, 4, sh, 4, 4, 1, 1);
}

void DebuggerGUI::initDisassembly()
{
    int tx = 0, ty = 30, tw = 300, th = 200;

    disassembly_view = new DisassemblyView(tx, ty, tw, th, &dcpu, &disassembler);

#if 0
    Fl_Browser *b = new Fl_Browser(tx,ty,tw,th);
    int widths[] = { 50, 50, 100, 100, 0 };
    b->column_widths(widths);
    b->column_char('\t');
    b->type(FL_MULTI_BROWSER);
    b->textfont(FL_COURIER);

    uint16_t prog[32] = {
        0x7c01, 0x0030, 0x7de1, 0x1000, 0x0020, 0x7803, 0x1000, 0xc00d,
        0x7dc1, 0x001a, 0xa861, 0x7c01, 0x2000, 0x2161, 0x2000, 0x8463,
        0x806d, 0x7dc1, 0x000d, 0x9031, 0x7c10, 0x0018, 0x7dc1, 0x001a,
        0x9037, 0x61c1, 0x7dc1, 0x001a
    };


    Disassembler d;
    d.disassemble(prog, 28);
    
    for(uint16_t i = 0; i < d.getInstructionCount(); i++)
    {
        const Disassembler::Instruction *inst = d.getInstruction(i);
        printf("%s ",  inst->address_str);
        printf("%s ",  inst->operation_str);
        printf("%s ",  inst->operand_a_str);
        printf("%s\n", inst->operand_b_str);
        char str[128];
        sprintf(str, "%s\t%s\t%s\t%s", inst->address_str, inst->operation_str, inst->operand_a_str, inst->operand_b_str);
        b->add(str);
    }
#endif
}

void DebuggerGUI::initMemoryViewer()
{
    int tx = 1024-400, ty = 30, tw = 400, th = 738;
    //Fl_Text_Display *td = new Fl_Text_Display(tx, ty, tw, th);
    //Fl_Text_Buffer *buf = new Fl_Text_Buffer(0xFFFF*10); //TODO delete buf
    //td->buffer(buf);

    memory_view = new MemoryView(tx, ty, tw, th, &dcpu);
}

void DebuggerGUI::initScreen()
{

}

void DebuggerGUI::initFlowControls()
{
    int gx = 400, gy = 30;
    int bw = 24, bh = 24;
    Fl_Button *b;

    Fl_Group *group = new Fl_Group(0, 0, 0, 0);

    b = new Fl_Button(0, 0, bw, bh, "@#|<");
    b->callback(cb_stepBackButton, this);

    b = new Fl_Button(0, 0, bw, bh, "@#||");
    b->callback(cb_pauseButton, this);

    b = new Fl_Button(0, 0, bw, bh, "@#>");
    b->callback(cb_runButton, this);

    b = new Fl_Button(0, 0, bw, bh, "@#>|");
    b->callback(cb_stepButton, this);

    group->end();

    gridByRows(group, gx, gy, 1, bh, 0, 0, 0, 0);
}

void DebuggerGUI::updateState()
{
    char str[64];
    int pc = dcpu.getProgramCounter();
    int sp = dcpu.getStackPointer();
    int o  = dcpu.getOverflow();

    //pc_input->textcolor(pc == pc_spinner->value() ? FL_BLACK : FL_RED);
    sprintf(str, "0x%04X", pc);
    pc_input->value(str);

    //sp_input->textcolor(sp == sp_spinner->value() ? FL_BLACK : FL_RED);
    sprintf(str, "0x%04X", sp);
    sp_input->value(str);

    //overflow_input->textcolor(o == overflow_spinner->value() ? FL_BLACK : FL_RED);
    sprintf(str, "0x%04X", o);
    overflow_input->value(str);

    for(int i = 0; i < DCPU16::NUM_REGISTERS; i++)
    {
        uint16_t v = dcpu.getRegister(i);

        Fl_Input *in = register_inputs[i];
        sprintf(str, "0x%04X", (int)v);
        in->value(str);
    }

    sprintf(str, "Clock: %d", dcpu.getCycles());
    clock_label->copy_label(str);

    memory_view->redraw();
    disassembly_view->redraw();
}

void DebuggerGUI::setRunning(bool run)
{
    if(running == run)
        return;

    if(run)
        Fl::add_idle(cb_runIdle, this);
    else
        Fl::remove_idle(cb_runIdle, this);

    running = run;
}

void DebuggerGUI::step(int num_steps)
{
    for(int i = 0; i < num_steps && !dcpu.getError(); i++)
    {
        pushHistory();
        dcpu.step();
    }

    if(dcpu.getError())
    {
        setRunning(false);
    }

    updateState();
}

void DebuggerGUI::pushHistory()
{
    //TODO serialize
    history.push_back(dcpu);

    if(history.size() > 100)
        history.pop_front();
}

void DebuggerGUI::popHistory(int n)
{
    if(history.empty())
        return;

    dcpu = history.back();
    history.pop_back();
    updateState();
}

void DebuggerGUI::cb_pauseButton(Fl_Widget *w, void *gui)
{
    static_cast<DebuggerGUI*>(gui)->setRunning(false);
}

void DebuggerGUI::cb_stepButton(Fl_Widget *w, void *gui)
{
    static_cast<DebuggerGUI*>(gui)->step(1);
}

void DebuggerGUI::cb_stepBackButton(Fl_Widget *w, void *gui)
{
    static_cast<DebuggerGUI*>(gui)->popHistory(1);
}

void DebuggerGUI::cb_runButton(Fl_Widget *w, void *gui)
{
    static_cast<DebuggerGUI*>(gui)->setRunning(true);
}

void DebuggerGUI::cb_runIdle(void *gui)
{
    static_cast<DebuggerGUI*>(gui)->step(1);
}

