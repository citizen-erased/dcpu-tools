#ifndef MEMORY_VIEW_H
#define MEMORY_VIEW_H

#include <QWidget>
#include <QStaticText>
#include <QColor>
#include "../../dcpu16/dcpu16.h"

class Debugger;


class MemoryView : public QWidget
{
    //Q_OBJECT

private:
    Debugger *debugger;
    int row_offset;
    int columns;
    QColor color_row0;
    QColor color_row1;
    QColor color_address_overlay;
    QColor color_column_overlay;
    QColor color_program_counter;
    QColor color_stack_pointer;
    QStaticText mem_text[DCPU16::MEMORY_SIZE];
    uint16_t known_memory[DCPU16::MEMORY_SIZE];

public:
    MemoryView(QWidget *parent = NULL);

    void setDebugger(Debugger *debugger);

    void updateTexts(bool force=false);

protected:
    void paintEvent(QPaintEvent *evt);
};

#endif // MEMORY_VIEW_H
