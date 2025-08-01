#include <QDialog>

namespace Ui {
class WarningDialog;
}

namespace f1x {
namespace openauto {
namespace autoapp {
namespace ui {

class WarningDialog : public QDialog {
    Q_OBJECT

  public:
    explicit WarningDialog(QWidget* parent = nullptr);
    ~WarningDialog() override;

  private:
    Ui::WarningDialog* ui_;

  private slots:
    void Autoclose();
};

}  // namespace ui
}  // namespace autoapp
}  // namespace openauto
}  // namespace f1x
