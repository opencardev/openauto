#include <f1x/openauto/autoapp/UI/UpdateDialog.hpp>
#include "ui_updatedialog.h"
#include <QFileInfo>
#include <QTextStream>
#include <QStorageInfo>
#include <fstream>
#include <cstdio>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace ui
{

UpdateDialog::UpdateDialog(QWidget *parent)
    : QDialog(parent)
    , ui_(new Ui::UpdateDialog)
{
    ui_->setupUi(this);
    connect(ui_->pushButtonUpdateCsmt, &QPushButton::clicked, this, &UpdateDialog::on_pushButtonUpdateCsmt_clicked);
    connect(ui_->pushButtonUpdateUdev, &QPushButton::clicked, this, &UpdateDialog::on_pushButtonUpdateUdev_clicked);
    connect(ui_->pushButtonUpdateOpenauto, &QPushButton::clicked, this, &UpdateDialog::on_pushButtonUpdateOpenauto_clicked);
    connect(ui_->pushButtonUpdateSystem, &QPushButton::clicked, this, &UpdateDialog::on_pushButtonUpdateSystem_clicked);
    connect(ui_->pushButtonUpdateCheck, &QPushButton::clicked, this, &UpdateDialog::on_pushButtonUpdateCheck_clicked);
    connect(ui_->pushButtonUpdateCancel, &QPushButton::clicked, this, &UpdateDialog::on_pushButtonUpdateCancel_clicked);
    connect(ui_->pushButtonClose, &QPushButton::clicked, this, &UpdateDialog::close);
    
    ui_->pushButtonClose->setFocus();
    ui_->progressBarCsmt->hide();
    ui_->progressBarUdev->hide();
    ui_->progressBarOpenauto->hide();
    ui_->progressBarSystem->hide();
    ui_->labelSystemReadyInstall->hide();
    ui_->labelUpdateChecking->hide();
    ui_->pushButtonUpdateCancel->hide();
    updateCheck();

    watcher_tmp = new QFileSystemWatcher(this);
    watcher_tmp->addPath("/tmp");
    connect(watcher_tmp, &QFileSystemWatcher::directoryChanged, this, &UpdateDialog::updateCheck);

    watcher_download = new QFileSystemWatcher(this);
    watcher_download->addPath("/media/USBDRIVES/CSSTORAGE");
    connect(watcher_download, &QFileSystemWatcher::directoryChanged, this, &UpdateDialog::downloadCheck);

    QStorageInfo storage("/media/USBDRIVES/CSSTORAGE");
    storage.refresh();
    if (storage.isValid() && storage.isReady()) {
        ui_->system->show();
        ui_->labelNoStorage->hide();
    } else {
        ui_->labelNoStorage->show();
        ui_->system->hide();
    }
}

UpdateDialog::~UpdateDialog()
{
    delete ui_;
}

void f1x::openauto::autoapp::ui::UpdateDialog::on_pushButtonUpdateCsmt_clicked()
{
    ui_->pushButtonUpdateCsmt->hide();
    ui_->progressBarCsmt->show();
    qApp->processEvents();
    system("crankshaft update csmt &");
}

void f1x::openauto::autoapp::ui::UpdateDialog::on_pushButtonUpdateUdev_clicked()
{
    ui_->pushButtonUpdateUdev->hide();
    ui_->progressBarUdev->show();
    qApp->processEvents();
    system("crankshaft update udev &");
}

void f1x::openauto::autoapp::ui::UpdateDialog::on_pushButtonUpdateOpenauto_clicked()
{
    ui_->pushButtonUpdateOpenauto->hide();
    ui_->progressBarOpenauto->show();
    qApp->processEvents();
    system("crankshaft update openauto &");
}

void f1x::openauto::autoapp::ui::UpdateDialog::on_pushButtonUpdateSystem_clicked()
{
    ui_->pushButtonUpdateSystem->hide();
    ui_->progressBarSystem->show();
    ui_->progressBarSystem->setValue(0);
    qApp->processEvents();
    system("crankshaft update system &");
}

void f1x::openauto::autoapp::ui::UpdateDialog::on_pushButtonUpdateCheck_clicked()
{
    ui_->pushButtonUpdateCheck->hide();
    ui_->labelUpdateChecking->show();
    qApp->processEvents();
    system("/usr/local/bin/crankshaft update check");
    updateCheck();
    ui_->labelUpdateChecking->hide();
    ui_->pushButtonUpdateCheck->show();
}

void f1x::openauto::autoapp::ui::UpdateDialog::on_pushButtonUpdateCancel_clicked()
{
    ui_->pushButtonUpdateCancel->hide();
    system("crankshaft update cancel &");
}

void f1x::openauto::autoapp::ui::UpdateDialog::downloadCheck()
{
    QDir directory("/media/USBDRIVES/CSSTORAGE");
    QStringList files = directory.entryList(QStringList() << "*.zip", QDir::AllEntries, QDir::Name);
    foreach(QString filename, files) {
        if (filename != "") {
            ui_->labelDownload->setText(filename);
        }
    }
}

void f1x::openauto::autoapp::ui::UpdateDialog::updateCheck()
{
    if (!std::ifstream("/tmp/csmt_updating")) {
        if (std::ifstream("/tmp/csmt_update_available")) {
            ui_->labelCsmtOK->hide();
            ui_->pushButtonUpdateCsmt->show();
        } else {
            ui_->pushButtonUpdateCsmt->hide();
            ui_->progressBarCsmt->hide();
            ui_->labelCsmtOK->show();
        }
    }

    if (!std::ifstream("/tmp/udev_updating")) {
        if (std::ifstream("/tmp/udev_update_available")) {
            ui_->labelUdevOK->hide();
            ui_->pushButtonUpdateUdev->show();
        } else {
            ui_->pushButtonUpdateUdev->hide();
            ui_->progressBarUdev->hide();
            ui_->labelUdevOK->show();
        }
    }

    if (!std::ifstream("/tmp/openauto_updating")) {
        if (std::ifstream("/tmp/openauto_update_available")) {
            ui_->labelOpenautoOK->hide();
            ui_->pushButtonUpdateOpenauto->show();
        } else {
            ui_->pushButtonUpdateOpenauto->hide();
            ui_->progressBarOpenauto->hide();
            ui_->labelOpenautoOK->show();
        }
    } else {
        ui_->labelOpenautoOK->hide();
        ui_->pushButtonUpdateOpenauto->hide();
    }

    if (std::ifstream("/tmp/system_update_ready")) {
        ui_->labelSystemOK->hide();
        ui_->pushButtonUpdateSystem->hide();
        ui_->progressBarSystem->hide();
        ui_->labelSystemReadyInstall->show();
        ui_->pushButtonUpdateCancel->hide();
    } else {
        ui_->labelSystemReadyInstall->hide();
        if (std::ifstream("/tmp/system_update_available")) {
            ui_->labelSystemOK->hide();
            ui_->progressBarSystem->hide();
            ui_->pushButtonUpdateSystem->show();
        }
        if (std::ifstream("/tmp/system_update_downloading")) {
            ui_->labelSystemOK->hide();
            ui_->pushButtonUpdateSystem->hide();
            ui_->pushButtonUpdateCheck->hide();
            ui_->progressBarSystem->show();
            ui_->pushButtonUpdateCancel->show();

            QFileInfo downloadfile = "/media/USBDRIVES/CSSTORAGE/" + ui_->labelDownload->text();
            if (downloadfile.exists()) {
                qint64 size = downloadfile.size();
                size = size/1024/1024;
                ui_->progressBarSystem->setValue(size);
            }
        } else {
            if (ui_->pushButtonUpdateCheck->isVisible() == false) {
                ui_->pushButtonUpdateCheck->show();
                ui_->labelDownload->setText("");
                ui_->pushButtonUpdateCancel->hide();
            }
        }

        if (!std::ifstream("/tmp/system_update_available") && !std::ifstream("/tmp/system_update_downloading")) {
            ui_->progressBarSystem->hide();
            ui_->labelSystemOK->show();
            ui_->pushButtonUpdateSystem->hide();
        }
    }
}

void f1x::openauto::autoapp::ui::UpdateDialog::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape)
    {
        UpdateDialog::close();
    }
    if (event->key() == Qt::Key_Return) {
        QApplication::postEvent (QApplication::focusWidget(), new QKeyEvent ( QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier));
        QApplication::postEvent (QApplication::focusWidget(), new QKeyEvent ( QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier));
    }    
    if (event->key() == Qt::Key_1) {
        QApplication::postEvent (QApplication::focusWidget(), new QKeyEvent ( QEvent::KeyPress, Qt::Key_Tab, Qt::ShiftModifier));
    }
    if (event->key() == Qt::Key_2) {
        QApplication::postEvent (QApplication::focusWidget(), new QKeyEvent ( QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier));
    }    
}

}
}
}
}

