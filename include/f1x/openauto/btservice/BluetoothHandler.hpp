//
// Created by Simon Dean on 26/11/2024.
//

#ifndef OPENAUTO_BLUETOOTHHANDLER_HPP
#define OPENAUTO_BLUETOOTHHANDLER_HPP

#include <f1x/openauto/btservice/IBluetoothHandler.hpp>
#include <f1x/openauto/btservice/IAndroidBluetoothServer.hpp>
#include <f1x/openauto/btservice/IAndroidBluetoothService.hpp>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include <QObject>

namespace f1x::openauto::btservice {

  class BluetoothHandler : public QObject, public IBluetoothHandler {
    Q_OBJECT
  public:
    BluetoothHandler(btservice::IAndroidBluetoothService::Pointer androidBluetoothService,
                     autoapp::configuration::IConfiguration::Pointer configuration);

    void shutdownService() override;

  private slots:
    void onPairingDisplayPinCode(const QBluetoothAddress &address, QString pin);

    void onPairingDisplayConfirmation(const QBluetoothAddress &address, QString pin);

    void onPairingFinished(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing);

    void onError(QBluetoothLocalDevice::Error error);

    void onHostModeStateChanged(QBluetoothLocalDevice::HostMode state);


  private:
    std::unique_ptr<QBluetoothLocalDevice> localDevice_;
    autoapp::configuration::IConfiguration::Pointer configuration_;
    btservice::IAndroidBluetoothService::Pointer androidBluetoothService_;
    btservice::IAndroidBluetoothServer::Pointer androidBluetoothServer_;
  };
}


#endif //OPENAUTO_BLUETOOTHHANDLER_HPP
