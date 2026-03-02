#ifndef HOMEWINDOW_H
#define HOMEWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include "ui/LeftPanel.h"
#include "ui/RightPanel.h"

class HomeWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit HomeWindow(QWidget *parent = nullptr);

private:
    QSplitter *splitter;
    LeftPanel *leftPanel;
    RightPanel *rightPanel;
};

#endif // HOMEWINDOW_H