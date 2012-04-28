#include "gui_utils.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

class RegisterWidgetItem : public QTableWidgetItem
{
public:
    int rw_id;
    QString name;

public:
    RegisterWidgetItem(int rw_id)
    : QTableWidgetItem()
    {
        this->rw_id = rw_id;
    }
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(&run_timer, SIGNAL(timeout()), this, SLOT(pumpCPU()));

    ui->registers_table->setColumnCount(2);
    ui->registers_table->setRowCount(0);

    int rows = 11;
    const char *names[] = {"A", "B", "C", "I", "J", "K", "X", "Y", "PC", "SP", "Overflow"};
    for(int row = 0; row < rows; row++)
        addInfoRow(names[row], DCPU16::RW_REGISTER_0 + row);

    info_row_cycles = addInfoRow("Cycles", 0);
    info_row_error  = addInfoRow("Error", 0);

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
}

MainWindow::~MainWindow()
{
    delete ui;
}

int MainWindow::addInfoRow(const char *name, int rw_id)
{
    int row = ui->registers_table->rowCount();
    ui->registers_table->insertRow(row);

    QTableWidgetItem *item = new QTableWidgetItem(name);
    item->setFlags(item->flags() & (~Qt::ItemIsEditable));
    ui->registers_table->setItem(row, 0, item);
    ui->registers_table->setItem(row, 1, new RegisterWidgetItem(rw_id));

    return row;
}

void MainWindow::updateGUI()
{
    for(int row = 0; row < 11; row++) //TODO rows constant
    {
        uint16_t value = debugger.getDCPU().read(DCPU16::RW_REGISTER_0 + row);
        ui->registers_table->item(row, 1)->setText(hexstr(value));
    }

    uint64_t cycles = debugger.getDCPU().getCycles();
    ui->cycles_label->setText(QString::number(cycles));
    ui->registers_table->item(info_row_cycles, 1)->setText(QString::number(cycles));

    const char *err_str = debugger.getDCPU().getErrorString(debugger.getDCPU().getError());
    ui->registers_table->item(info_row_error, 1)->setText(err_str);

    ui->memory_view->updateTexts();
    //TODO only repaint if memory changed.
    //TODO MemoryView should take care of determining if a repaint is necessary when updating texts.
    ui->memory_view->repaint();
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
