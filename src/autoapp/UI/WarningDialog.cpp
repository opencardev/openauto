#include <ui_warningdialog.h>
#include <QTimer>
#include <f1x/openauto/autoapp/UI/WarningDialog.hpp>

namespace f1x {
namespace openauto {
namespace autoapp {
namespace ui {

WarningDialog::WarningDialog(QWidget *parent) : QDialog(parent), ui_(new Ui::WarningDialog) {
    ui_->setupUi(this);

    connect(ui_->pushButtonClose, &QPushButton::clicked, this, &WarningDialog::close);
    QTimer::singleShot(5000, this, SLOT(Autoclose()));
}

WarningDialog::~WarningDialog() { delete ui_; }

void WarningDialog::Autoclose() { WarningDialog::close(); }

}  // namespace ui
}  // namespace autoapp
}  // namespace openauto
}  // namespace f1x
