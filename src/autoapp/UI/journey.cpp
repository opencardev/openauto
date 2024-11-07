#include "journey.h"
#include "ui_journey.h"

journey::journey(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::journey)
{
    ui->setupUi(this);
}

journey::~journey()
{
    delete ui;
}
