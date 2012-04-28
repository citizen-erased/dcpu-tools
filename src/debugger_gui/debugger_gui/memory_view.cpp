#include <QPainter>
#include "../../debugger/debugger.h"
#include "gui_utils.h"
#include "memory_view.h"

MemoryView::MemoryView(QWidget *parent)
: QWidget(parent)
{
    debugger = NULL;
    row_offset = 0;
    columns = 16;

    /*
     * scheme id: 0n31PdQYnbIQe
     *
     * http://colorschemedesigner.com/#0n31PdQYnbIQe
     */
    color_row0.setRgb(0x76DB66);
    color_row1.setRgb(0xC7FEBE);
    color_address_overlay.setRgb(0xDE9567);
    color_address_overlay.setRgb(0x000000);
    color_address_overlay.setAlphaF(0.1);
    color_column_overlay.setRgb(0x000000);
    color_column_overlay.setAlphaF(0.07);

    color_program_counter.setRgb(0x6AA0D8);
    color_stack_pointer.setRgb(0xBF3030);
}

void MemoryView::setDebugger(Debugger *debugger)
{
    this->debugger = debugger;
    updateTexts(true);
}

void MemoryView::updateTexts(bool force)
{
    const uint16_t *mem = debugger->getDCPU().memoryPointer();

    for(int i = 0; i < DCPU16::MEMORY_SIZE; i++)
        if(force|| mem[i] != known_memory[i])
            mem_text[i].setText(hexstr(mem[i]));
}

void MemoryView::paintEvent(QPaintEvent *)
{
    QPainter paint(this);
    paint.setFont(QFont("Terminus"));

    int col_x_pad = 4, col_y_pad = 4;
    int row_height = paint.fontMetrics().height() + col_y_pad*2;
    int col_width = paint.fontMetrics().width("0xFFFF") + col_x_pad*2;

    int rows = height() / row_height + 1;

    for(int i = 0; i < rows; i++)
    {
        QRectF rect(0, row_height*i, width(), row_height);
        paint.fillRect(rect, i%2 ? color_row0 : color_row1);
    }

    for(int i = 0; i < columns; i+=2)
    {
        QRectF rect(col_width*i, 0, col_width, height());
        paint.fillRect(rect, color_column_overlay);
    }

//    paint.setCompositionMode(QPainter::compo);
    paint.fillRect(QRectF(0, 0, col_width, height()), color_address_overlay);

    for(int i = 0; i < rows; i++)
    {
        //TODO static text this?
        int address = columns * (i + row_offset);
        paint.drawText(col_x_pad, col_y_pad + row_height*i, hexstr(uint16_t(address)));
    }

    int pc = debugger->getDCPU().read(DCPU16::RW_PROGRAM_COUNTER);
    int sp = debugger->getDCPU().read(DCPU16::RW_STACK_POINTER);
    int row = 0, col = 0;
    for(int mem_index = columns * row_offset; mem_index < DCPU16::MEMORY_SIZE; mem_index++)
    {
        if(mem_index == pc && mem_index == sp)
        {
            paint.fillRect(QRectF(col_width*(col+1) + col_width/2, row_height*row, col_width/2, row_height), color_program_counter);
            paint.fillRect(QRectF(col_width*(col+1), row_height*row, col_width/2, row_height), color_stack_pointer);
        }
        else if(mem_index == pc)
            paint.fillRect(QRectF(col_width*(col+1), row_height*row, col_width, row_height), color_program_counter);
        else if(mem_index == sp)
            paint.fillRect(QRectF(col_width*(col+1), row_height*row, col_width, row_height), color_stack_pointer);

        //QRectF trect(col_width*(col+1), row_height*row, col_width, row_height);
        //paint.drawStaticText(trect, Qt::AlignCenter, mem_text[mem_index]);

        int tx = col_x_pad + col_width + col_width * col;
        int ty = col_y_pad + row_height * row;
        paint.drawStaticText(tx, ty, mem_text[mem_index]);

        if(++col >= columns)
        {
            col = 0;
            row++;

            if(row > rows)
                break;
        }
    }
}
