#include <QMessageBox>
#include <f1x/openauto/autoapp/UI/ConnectDialog.hpp>
#include "ui_connectdialog.h"
#include <QFileInfo>
#include <QTextStream>
#include <fstream>
#include <QNetworkInterface>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace ui
{

ConnectDialog::ConnectDialog(boost::asio::io_service& ioService, aasdk::tcp::ITCPWrapper& tcpWrapper, openauto::autoapp::configuration::IRecentAddressesList& recentAddressesList, QWidget *parent)
    : QDialog(parent)
    , ioService_(ioService)
    , tcpWrapper_(tcpWrapper)
    , recentAddressesList_(recentAddressesList)
    , ui_(new Ui::ConnectDialog)
{
    qRegisterMetaType<aasdk::tcp::ITCPEndpoint::SocketPointer>("aasdk::tcp::ITCPEndpoint::SocketPointer");
    qRegisterMetaType<std::string>("std::string");

    ui_->setupUi(this);
    connect(ui_->pushButtonCancel, &QPushButton::clicked, this, &ConnectDialog::close);
    connect(ui_->pushButtonConnect, &QPushButton::clicked, this, &ConnectDialog::onConnectButtonClicked);
    connect(this, &ConnectDialog::connectionSucceed, this, &ConnectDialog::onConnectionSucceed);
    connect(this, &ConnectDialog::connectionFailed, this, &ConnectDialog::onConnectionFailed);
    connect(ui_->pushButtonUpdate, &QPushButton::clicked, this, &ConnectDialog::onUpdateButtonClicked);

    this->loadRecentList();

    ui_->progressBarConnect->hide();
    ui_->lineEditIPAddress->setFocus();
}

ConnectDialog::~ConnectDialog()
{
    delete ui_;
}

void ConnectDialog::onConnectButtonClicked()
{
    this->setControlsEnabledStatus(false);

    const auto& ipAddress = ui_->lineEditIPAddress->text().toStdString();
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(ioService_);
    ui_->progressBarConnect->show();
    try
    {
        tcpWrapper_.asyncConnect(*socket, ipAddress, 5277, std::bind(&ConnectDialog::connectHandler, this, std::placeholders::_1, ipAddress, socket));
    }
    catch(const boost::system::system_error& se)
    {
        emit connectionFailed(QString(se.what()));
    }
}

void ConnectDialog::onUpdateButtonClicked()
{
    system("/usr/local/bin/autoapp_helper updaterecent");
    loadClientList();
}

void ConnectDialog::connectHandler(const boost::system::error_code& ec, const std::string& ipAddress, aasdk::tcp::ITCPEndpoint::SocketPointer socket)
{
    if(!ec)
    {
        emit connectionSucceed(std::move(socket), ipAddress);
        this->close();
    }
    else
    {
        emit connectionFailed(QString::fromStdString(ec.message()));
    }
}

void ConnectDialog::onConnectionSucceed(aasdk::tcp::ITCPEndpoint::SocketPointer, const std::string& ipAddress)
{
    ui_->progressBarConnect->hide();
    //this->insertIpAddress(ipAddress);
    this->setControlsEnabledStatus(true);
}

void ConnectDialog::onConnectionFailed(const QString& message)
{
    this->setControlsEnabledStatus(true);

    ui_->progressBarConnect->hide();
    QMessageBox errorMessage(QMessageBox::Critical, "Connect error", message, QMessageBox::Ok);
    //errorMessage.setWindowFlags(Qt::WindowStaysOnTopHint);
    errorMessage.exec();
}

void ConnectDialog::onRecentAddressClicked(const QModelIndex& index)
{
    const auto& recentAddressesList = recentAddressesList_.getList();
}

void ConnectDialog::setControlsEnabledStatus(bool status)
{
    ui_->pushButtonConnect->setVisible(status);
    ui_->pushButtonCancel->setEnabled(status);
    ui_->lineEditIPAddress->setEnabled(status);
}

void ConnectDialog::loadRecentList()
{
    QStringList stringList;
    const auto& configList = recentAddressesList_.getList();

    for(const auto& element : configList)
    {
        stringList.append(QString::fromStdString(element));
    }

    recentAddressesModel_.setStringList(stringList);
}

void ConnectDialog::loadClientList()
{
    int cleaner = ui_->listWidgetClients->count();
    while (cleaner > -1) {
        ui_->listWidgetClients->takeItem(cleaner);
        cleaner--;
    }

    if (std::ifstream("/tmp/hotspot_active")) {
        ui_->listWidgetClients->show();
        ui_->pushButtonUpdate->show();
        if (std::ifstream("/tmp/temp_recent_list")) {
            QFile versionFile(QString("/tmp/temp_recent_list"));
            versionFile.open(QIODevice::ReadOnly);
            QTextStream data(&versionFile);
            while (!data.atEnd())
            {
                QString ip = data.readLine().trimmed();
                if (ip != "") {
                    ui_->listWidgetClients->addItem(ip);
                    ui_->lineEditIPAddress->setText(ip);
                    //ConnectDialog::insertIpAddress(ip.toStdString());
                }
            }
            versionFile.close();
            if (ui_->listWidgetClients->count() == 1) {
                this->setControlsEnabledStatus(false);
                const auto& ipAddress = ui_->lineEditIPAddress->text().toStdString();
                auto socket = std::make_shared<boost::asio::ip::tcp::socket>(ioService_);
                ui_->progressBarConnect->show();
                try
                {
                    tcpWrapper_.asyncConnect(*socket, ipAddress, 5277, std::bind(&ConnectDialog::connectHandler, this, std::placeholders::_1, ipAddress, socket));
                }
                catch(const boost::system::system_error& se)
                {
                    emit connectionFailed(QString(se.what()));
                }
            }
        } else {
            ui_->lineEditIPAddress->setText("");
        }
    } else {
        ui_->listWidgetClients->hide();
        if (std::ifstream("/tmp/gateway_wlan0")) {
            ui_->pushButtonUpdate->hide();
            QFile gatewayData(QString("/tmp/gateway_wlan0"));
            gatewayData.open(QIODevice::ReadOnly);
            QTextStream gateway_date(&gatewayData);
            QString linedate = gateway_date.readAll();
            gatewayData.close();
            if (linedate != "") {
                ui_->lineEditIPAddress->setText(linedate.simplified());
                ui_->listWidgetClients->addItem(linedate.simplified());
                if (ui_->listWidgetClients->count() == 1) {
                    this->setControlsEnabledStatus(false);
                    const auto& ipAddress = ui_->lineEditIPAddress->text().toStdString();
                    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(ioService_);
                    ui_->progressBarConnect->show();
                    try
                    {
                        tcpWrapper_.asyncConnect(*socket, ipAddress, 5277, std::bind(&ConnectDialog::connectHandler, this, std::placeholders::_1, ipAddress, socket));
                    }
                    catch(const boost::system::system_error& se)
                    {
                        emit connectionFailed(QString(se.what()));
                    }
                }
            }
        } else {
            ui_->lineEditIPAddress->setText("");
        }
    }
}

void ConnectDialog::insertIpAddress(const std::string& ipAddress)
{
    recentAddressesList_.insertAddress(ipAddress);
    this->loadRecentList();
}

void f1x::openauto::autoapp::ui::ConnectDialog::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape)
    {
        ConnectDialog::close();
    }
}

}
}
}
}
