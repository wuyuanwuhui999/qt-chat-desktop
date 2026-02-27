#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include "ui/LeftPanel.h"
#include "ui/RightPanel.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    QSplitter *splitter;
    LeftPanel *leftPanel;
    RightPanel *rightPanel;
};

#endif // MAINWINDOW_H
