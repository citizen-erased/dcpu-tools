#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "../../debugger/debugger.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
private:
    int info_row_cycles;
    int info_row_error;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    void doStep(int step);
    void updateGUI();
    int addInfoRow(const char *name, int rw_id);

private:
    Ui::MainWindow *ui;
    Debugger debugger;
    QTimer run_timer;

public slots:
    void runCPU();
    void stopCPU();
    void stepForward();
    void stepBackward();
    void pumpCPU();
    void reset();
};

#endif // MAINWINDOW_H
