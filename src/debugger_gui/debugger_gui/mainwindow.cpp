#include "gui_utils.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

class InfoWidgetItem : public QTableWidgetItem
{
protected:
    Debugger *debugger;

public:
    InfoWidgetItem(Debugger *debugger)
    {
        this->debugger = debugger;
        setWritable(false);
    }

    virtual void update() {}
    virtual void onEdit() {}

protected:
    void setWritable(bool write)
    {
        if(write)
            setFlags(flags() | Qt::ItemIsEditable);
        else
            setFlags(flags() & ~Qt::ItemIsEditable);
    }
};

class ReadOnlyWidgetItem : public InfoWidgetItem
{
public:
    int rw_id;

public:
    ReadOnlyWidgetItem(Debugger *debugger, int rw_id)
        : InfoWidgetItem(debugger)
    {
        this->rw_id = rw_id;
    }

    virtual void update()
    {
        uint16_t value = debugger->getDCPU().read(rw_id);
        setText(hexstr(value));
    }
};

class ReadWriteWidgetItem : public ReadOnlyWidgetItem
{
public:
    ReadWriteWidgetItem(Debugger *debugger, int rw_id)
        : ReadOnlyWidgetItem(debugger, rw_id)
    {
        setWritable(true);
    }

    void onEdit()
    {
        QString str = text();
        int integer = 0;
        bool ok = false;

        if(str.startsWith("0b"))
            integer = str.remove(0, 2).toInt(&ok, 2);
        else
            integer = str.toInt(&ok, 0);

        if(ok && 0 <= integer && integer <= 0xFFFF)
        {
            debugger->getDCPU().write(rw_id, integer);
            //print confirmation to gui console box thing
        }
        else
        {
            //print a warning to gui console box thing
        }
    }
};

class CyclesWidgetItem : public InfoWidgetItem
{
public:
    CyclesWidgetItem(Debugger *debugger)
        : InfoWidgetItem(debugger)
    {

    }

    void update()
    {
        uint64_t value = debugger->getDCPU().getCycles();
        setText(QString::number(value));
    }
};

class ErrorWidgetItem : public InfoWidgetItem
{
private:
    QColor no_error_color;
    QColor error_color;

public:
    ErrorWidgetItem(Debugger *debugger)
        : InfoWidgetItem(debugger)
    {
        no_error_color.setRgb(0xC7FEBE);
        error_color.setRgb(0xDE6770);
    }

    void update()
    {
        int err = debugger->getDCPU().getError();
        QString err_str = debugger->getDCPU().getErrorString(err);

        if(err_str.startsWith("ERROR_"))
            err_str.remove(0, 6);

        if(err != DCPU16::ERROR_NONE)
            setBackgroundColor(error_color);
        else
            setBackgroundColor(no_error_color);

        setText(err_str);
    }
};


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    updating_gui = true;

    ui->setupUi(this);

    connect(&run_timer, SIGNAL(timeout()), this, SLOT(pumpCPU()));

    ui->registers_table->setColumnCount(2);
    ui->registers_table->setRowCount(0);

    addInfoRow("A",  new ReadWriteWidgetItem(&debugger, DCPU16::RW_REGISTER_0));
    addInfoRow("B",  new ReadWriteWidgetItem(&debugger, DCPU16::RW_REGISTER_1));
    addInfoRow("C",  new ReadWriteWidgetItem(&debugger, DCPU16::RW_REGISTER_2));
    addInfoRow("I",  new ReadWriteWidgetItem(&debugger, DCPU16::RW_REGISTER_3));
    addInfoRow("J",  new ReadWriteWidgetItem(&debugger, DCPU16::RW_REGISTER_4));
    addInfoRow("K",  new ReadWriteWidgetItem(&debugger, DCPU16::RW_REGISTER_5));
    addInfoRow("X",  new ReadWriteWidgetItem(&debugger, DCPU16::RW_REGISTER_6));
    addInfoRow("Y",  new ReadWriteWidgetItem(&debugger, DCPU16::RW_REGISTER_7));
    addInfoRow("PC", new ReadWriteWidgetItem(&debugger, DCPU16::RW_PROGRAM_COUNTER));
    addInfoRow("SP", new ReadWriteWidgetItem(&debugger, DCPU16::RW_STACK_POINTER));
    addInfoRow("EX", new ReadWriteWidgetItem(&debugger, DCPU16::RW_EXCESS));

    addInfoRow("[A]",  new ReadWriteWidgetItem(&debugger, DCPU16::RW_REGISTER_PTR_0));
    addInfoRow("[B]",  new ReadWriteWidgetItem(&debugger, DCPU16::RW_REGISTER_PTR_1));
    addInfoRow("[C]",  new ReadWriteWidgetItem(&debugger, DCPU16::RW_REGISTER_PTR_2));
    addInfoRow("[I]",  new ReadWriteWidgetItem(&debugger, DCPU16::RW_REGISTER_PTR_3));
    addInfoRow("[J]",  new ReadWriteWidgetItem(&debugger, DCPU16::RW_REGISTER_PTR_4));
    addInfoRow("[K]",  new ReadWriteWidgetItem(&debugger, DCPU16::RW_REGISTER_PTR_5));
    addInfoRow("[X]",  new ReadWriteWidgetItem(&debugger, DCPU16::RW_REGISTER_PTR_6));
    addInfoRow("[Y]",  new ReadWriteWidgetItem(&debugger, DCPU16::RW_REGISTER_PTR_7));
    addInfoRow("[PC]", new ReadWriteWidgetItem(&debugger, DCPU16::RW_PROGRAM_COUNTER_PTR));
    addInfoRow("[SP]", new ReadWriteWidgetItem(&debugger, DCPU16::RW_STACK_POINTER_PTR));

    addInfoRow("Cycles", new CyclesWidgetItem(&debugger));
    addInfoRow("Error",  new ErrorWidgetItem(&debugger));

    ui->splitter->setStretchFactor(0, 30);
    ui->splitter->setStretchFactor(1, 1);

    uint16_t prog[32] = {
        0x7c01, 0x0030, 0x7de1, 0x1000, 0x0020, 0x7803, 0x1000, 0xc00d,
        0x7dc1, 0x001a, 0xa861, 0x7c01, 0x2000, 0x2161, 0x2000, 0x8463,
        0x806d, 0x7dc1, 0x000d, 0x9031, 0x7c10, 0x0018, 0x7dc1, 0x001a,
        0x9037, 0x61c1, 0x7dc1, 0x001a
    };

    uint16_t prog_fib[] = {
        0xa031, 0x0c01, 0x7c10, 0x0007, 0x0051, 0x7dc1, 0x002a, 0x8061, 0x180c,
        0x7dc1, 0x0026, 0x8461, 0x180c, 0x7dc1, 0x0028, 0x8861, 0x180c, 0x7dc1,
        0x0028, 0x0061, 0x8463, 0x0071, 0x8873, 0x1da1, 0x1801, 0x7c10, 0x0007,
        0x6071, 0x0031, 0x0da1, 0x1c01, 0x7c10, 0x0007, 0x6031, 0x0041, 0x0c01,
        0x1002, 0x61c1, 0x8001, 0x61c1, 0x8401, 0x61c1, 0x7de1, 0x1000, 0xfff0,
        0x7dc1, 0x1000,
    };


    //debugger.loadProgram(prog, 28);
    debugger.loadProgram(prog_fib, sizeof(prog_fib)/sizeof(prog_fib[0]));

    ui->memory_view->setDebugger(&debugger);
    updateGUI();

    updating_gui = false;
}

MainWindow::~MainWindow()
{
    delete ui;
}

int MainWindow::addInfoRow(const char *name, InfoWidgetItem *info_item)
{
    int row = ui->registers_table->rowCount();
    ui->registers_table->insertRow(row);

    QTableWidgetItem *item = new QTableWidgetItem(name);
    item->setFlags(item->flags() & (~Qt::ItemIsEditable));

    ui->registers_table->setItem(row, 0, item);
    ui->registers_table->setItem(row, 1, info_item);

    info_items.push_back(info_item);

    return row;
}

void MainWindow::updateGUI()
{
    updating_gui = true;

    for(size_t i = 0; i < info_items.size(); i++)
        info_items[i]->update();

    ui->memory_view->updateTexts();
    //TODO only repaint if memory changed.
    //TODO MemoryView should take care of determining if a repaint is necessary when updating texts.
    ui->memory_view->repaint();

    updating_gui = false;
}

void MainWindow::doStep(int step)
{
    debugger.step(step);

    if(debugger.getDCPU().getError())
        stopCPU();

    updateGUI();
}

void MainWindow::runCPU()
{
    run_timer.start(0);
}

void MainWindow::stopCPU()
{
    run_timer.stop();
}

void MainWindow::stepForward()
{
    doStep(1);
}

void MainWindow::stepBackward()
{
    doStep(-1);
}

void MainWindow::pumpCPU()
{
//    doStep(1000);
    doStep(1);
    //doStep(100000);
}

void MainWindow::reset()
{
    debugger.reset();
    updateGUI();
}

void MainWindow::cellChanged(int row, int column)
{
    if(column ==0 || updating_gui)
        return;

    static_cast<InfoWidgetItem*>(ui->registers_table->item(row, column))->onEdit();
    updateGUI();
}

void MainWindow::exit()
{
    QApplication::quit();
}
