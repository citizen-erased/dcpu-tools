#include <algorithm>
#include <FL/Fl_Scrollbar.H>
#include <FL/fl_draw.H>

#include "../dcpu16/dcpu16.h"
#include "../disassembler/disassembler.h"
#include "disassembly_view.h"


DisassemblyView::DisassemblyView(int X, int Y, int W, int H, DCPU16 *dcpu, Disassembler *disassembler)
: Fl_Group(X, Y, W, H)
{
    this->dcpu = dcpu;
    this->disassembler = disassembler;

    scrollbar = new Fl_Scrollbar(X + W - 16, Y, 16, H);
    scrollbar->callback(DisassemblyView::cb_scrollbar, this);
    scrollbar->linesize(1);

    end();
}

void DisassemblyView::draw()
{
    fl_push_clip(x(), y(), w(), h());

    fl_font(FL_COURIER, 12);

    int col_height = fl_height();
    int rows = (h() / col_height) + 1;

    scrollbar->bounds(0, std::max((int)disassembler->getInstructionCount() - rows + 1, 0));

    if(scrollbar->minimum() == scrollbar->maximum())
        scrollbar->hide();
    else
        scrollbar->show();


    int start = std::max(0, scrollbar->value());
    int end   = std::min(scrollbar->value() + rows, (int)disassembler->getInstructionCount());


    /*
     * draw background rows
     */
    fl_rectf(x(), y(), w(), h(), 0x93, 0xD8, 0xBC);
    for(int i = start%2; i < rows; i+=2)
    {
        fl_rectf(x(), y() + i*col_height, w(), col_height, 0xB2, 0xD8, 0xC9);
    }

    const Disassembler::Instruction *inst = disassembler->findInstructionFromAddress(dcpu->getProgramCounter());

    if(inst == NULL)
        return;

#if 0
    bool center_view = true;
    for(int i = start; i < end; i++)
    {
        inst = disassembler->getInstruction(i);
        if(inst->address == dcpu->getProgramCounter())
        {
            center_view = false;
            break;
        }
    }

    if(center_view)
    {
        scrollbar->value()
    }
#endif

    fl_color(FL_BLACK);
    int ty = y() + fl_height();
    for(int i = start; i < end; i++)
    {
        inst = disassembler->getInstruction(i);

        if(inst->address == dcpu->getProgramCounter())
            fl_font(FL_COURIER_BOLD, 12);
        else
            fl_font(FL_COURIER, 12);

        int tx = x();
        fl_draw(inst->address_str, tx, ty - fl_descent());
        tx += 60;
        fl_draw(inst->operation_str, tx, ty - fl_descent());
        tx += 40;
        fl_draw(inst->operand_a_str, tx, ty - fl_descent());
        tx += 100;
        fl_draw(inst->operand_b_str, tx, ty - fl_descent());

        ty += col_height;
    }

    fl_pop_clip();

    draw_children();
}


void DisassemblyView::cb_scrollbar(Fl_Widget *w, void *view)
{
    static_cast<DisassemblyView*>(view)->redraw();
}

