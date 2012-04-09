#include <algorithm>
#include <FL/Fl_Scrollbar.H>
#include <FL/fl_draw.H>

#include "../dcpu16/dcpu16.h"
#include "memory_view.h"


MemoryView::MemoryView(int X, int Y, int W, int H, DCPU16 *dcpu)
: Fl_Group(X, Y, W, H)
{
    this->dcpu = dcpu;

    scrollbar = new Fl_Scrollbar(X + W - 16, Y, 16, H);
    scrollbar->callback(MemoryView::cb_scrollbar, this);
    scrollbar->linesize(1);
    scrollbar->bounds(0, DCPU16::MEMORY_SIZE / 8);

    end();
}

void MemoryView::draw()
{
    fl_push_clip(x(), y(), w(), h());

    fl_font(FL_COURIER, 12);

    int columns = 8;
    int col_height = fl_height();
    int rows = (h() / col_height) + 1;

    const uint16_t *mem = dcpu->memoryPointer();
    int mem_offset = std::max(0, scrollbar->value() * columns);

    char str[16];


    /*
     * draw background rows
     */
    fl_rectf(x(), y(), w(), h(), 0x93, 0xD8, 0xBC);
    for(int i = mem_offset % 2; i < rows; i+=2)
    {
        fl_rectf(x(), y() + i*col_height, w(), col_height, 0xB2, 0xD8, 0xC9);
    }

    /*
     * draw address labels
     */
    fl_color(FL_BLACK);
    int mx = x(), my = y() + fl_height();
    int label_width = fl_width("0xFFFF") + 10;
    for(int i = 0; i < rows; i++)
    {
        int address = mem_offset + i*columns;

        if(address >= DCPU16::MEMORY_SIZE)
            break;

        sprintf(str, "0x%04X", address);
        fl_draw(str, mx, my - fl_descent());
        my += col_height;
    }

    /*
     * draw memory
     */
    int tx = x() + label_width, ty = y() + fl_height();
    int w_inc = fl_width("FFFF") + 10;
    fl_color(FL_BLACK);
    for(int i = mem_offset; i < DCPU16::MEMORY_SIZE; i++)
    {
        if(i > mem_offset && i % columns == 0)
        {
            ty += col_height;
            tx = x() + label_width;
        }

        if(ty > y() + h())
            break;

        sprintf(str, "%04X", (unsigned int)mem[i]);
        fl_draw(str, tx, ty - fl_descent());
        tx += w_inc;
    }

    fl_pop_clip();

    draw_children();
}

void MemoryView::cb_scrollbar(Fl_Widget *w, void *view)
{
    static_cast<MemoryView*>(view)->redraw();
}

