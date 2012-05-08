#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <vector>
#include "../../debugger/debugger.h"

namespace Ui {
class MainWindow;
}


class InfoWidgetItem;


class MainWindow : public QMainWindow
{
    Q_OBJECT
    
private:
    std::vector<InfoWidgetItem*> info_items;
    bool updating_gui;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    void doStep(int step);
    void updateGUI();
    int addInfoRow(const char *name, InfoWidgetItem *data_item);

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
    void cellChanged(int row, int column);
};

#endif // MAINWINDOW_H
