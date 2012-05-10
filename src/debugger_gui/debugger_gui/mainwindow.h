#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QString>
#include <vector>
#include "../../debugger/debugger.h"

namespace Ui {
class MainWindow;
}


class InfoWidgetItem;


class MainWindow : public QMainWindow
{
    Q_OBJECT
    
/*---------------------------------------------------------------------------
 * Members
 *--------------------------------------------------------------------------*/
private:
    Ui::MainWindow *ui;
    Debugger debugger;
    QTimer run_timer;
    std::vector<InfoWidgetItem*> info_items;
    bool updating_gui;


/*---------------------------------------------------------------------------
 * Construct/Destruct
 *--------------------------------------------------------------------------*/
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
/*---------------------------------------------------------------------------
 * Loading
 *--------------------------------------------------------------------------*/
 private:
    void open(const QString &path);

/*---------------------------------------------------------------------------
 * GUI Manipulation
 *--------------------------------------------------------------------------*/
private:
    void updateGUI();
    int addInfoRow(const char *name, InfoWidgetItem *data_item);

/*---------------------------------------------------------------------------
 * CPU Manipulation
 *--------------------------------------------------------------------------*/
private:
    void reset();
    void runCPU();
    void stopCPU();
    void doStep(int step);

/*---------------------------------------------------------------------------
 * Application
 *--------------------------------------------------------------------------*/
private:
    void exit();

/*---------------------------------------------------------------------------
 * Custom Slots
 *--------------------------------------------------------------------------*/
private slots:
    void pumpCPU();

/*---------------------------------------------------------------------------
 * Automatic Slots
 *--------------------------------------------------------------------------*/
private slots:
    void on_actionOpen_triggered();
    void on_registers_table_cellChanged(int row, int column);
    void on_playButton_clicked();
    void on_stopButton_clicked();
    void on_stepBackwardButton_clicked();
    void on_stepForwardButton_clicked();
    void on_actionReset_triggered();
    void on_actionExit_triggered();
};

#endif // MAINWINDOW_H
