#ifndef JOURNEY_H
#define JOURNEY_H

#include <QMainWindow>

namespace Ui {
class journey;
}

class journey : public QMainWindow
{
    Q_OBJECT

public:
    explicit journey(QWidget *parent = nullptr);
    ~journey();

private:
    Ui::journey *ui;
};

#endif // JOURNEY_H
