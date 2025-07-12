//
// Created by Simon Dean on 26/11/2024.
//

#include <f1x/openauto/btservice/BluetoothHandler.hpp>
#include <f1x/openauto/btservice/AndroidBluetoothService.hpp>
#include <f1x/openauto/btservice/AndroidBluetoothServer.hpp>
#include <modern/Logger.hpp>
#include "modern/Logger.hpp"

namespace f1x::openauto::btservice {
  BluetoothHandler::BluetoothHandler(btservice::IAndroidBluetoothService::Pointer androidBluetoothService,
                                     autoapp::configuration::IConfiguration::Pointer configuration)
  : configuration_(std::move(configuration)),
    androidBluetoothService_(std::move(androidBluetoothService)),
    androidBluetoothServer_(std::make_unique<btservice::AndroidBluetoothServer>(configuration_)) {

    LOG_INFO(BLUETOOTH, "[BluetoothHandler::BluetoothHandler] Starting Up...");

    QString adapterAddress = QString::fromStdString(configuration_->getBluetoothAdapterAddress());
    QBluetoothAddress address(adapterAddress);
    localDevice_ = std::make_unique<QBluetoothLocalDevice>(QBluetoothAddress());

    if (!localDevice_->isValid()) {
      LOG_ERROR(BLUETOOTH, "Bluetooth adapter is not valid.");
    } else {
      LOG_INFO(BLUETOOTH, "Bluetooth adapter is valid.");
    }

    QObject::connect(localDevice_.get(), &QBluetoothLocalDevice::pairingDisplayPinCode, this, &BluetoothHandler::onPairingDisplayPinCode);
    QObject::connect(localDevice_.get(), &QBluetoothLocalDevice::pairingDisplayConfirmation, this, &BluetoothHandler::onPairingDisplayConfirmation);
    QObject::connect(localDevice_.get(), &QBluetoothLocalDevice::pairingFinished, this, &BluetoothHandler::onPairingFinished);
    QObject::connect(localDevice_.get(), &QBluetoothLocalDevice::hostModeStateChanged, this, &BluetoothHandler::onHostModeStateChanged);
    QObject::connect(localDevice_.get(), &QBluetoothLocalDevice::error, this, &BluetoothHandler::onError);

    // Turn Bluetooth on
    localDevice_->powerOn();

    // Make it visible to others
    localDevice_->setHostMode(QBluetoothLocalDevice::HostDiscoverable);

    uint16_t portNumber = androidBluetoothServer_->start(address);

    if (portNumber == 0) {
      LOG_ERROR(BLUETOOTH, "Server start failed.");
      throw std::runtime_error("Unable to start bluetooth server");
    }

    std::map<std::string, std::string> context = {
        {"address", address.toString().toStdString()},
        {"port", std::to_string(portNumber)}
    };
    LOG_INFO_CTX(BLUETOOTH, "Listening for connections", context);

   if (!androidBluetoothService_->registerService(portNumber, address)) {
      LOG_ERROR(BLUETOOTH, "Service registration failed.");
      throw std::runtime_error("Unable to register btservice");
    } else {
      std::map<std::string, std::string> serviceContext = {
          {"port", std::to_string(portNumber)}
      };
      LOG_INFO_CTX(BLUETOOTH, "Service registered", serviceContext);
    }

    // TODO: Connect to any previously paired devices
  }

  void BluetoothHandler::shutdownService() {
    LOG_INFO(BLUETOOTH, "[BluetoothHandler::shutdownService] Shutdown initiated");
    androidBluetoothService_->unregisterService();
  }

  void BluetoothHandler::onPairingDisplayPinCode(const QBluetoothAddress &address, QString pin) {
    LOG_DEBUG_STREAM(BLUETOOTH, "[BluetoothHandler::onPairingDisplayPinCode] Pairing display PIN code: " + pin.toStdString());
  }

  void BluetoothHandler::onPairingDisplayConfirmation(const QBluetoothAddress &address, QString pin) {
    LOG_DEBUG_STREAM(BLUETOOTH, "[BluetoothHandler::onPairingDisplayConfirmation] Pairing display confirmation: " + pin.toStdString());

    // Here you can implement logic to show this PIN to the user or automatically accept if you trust all devices
    localDevice_->pairingConfirmation(true); // Confirm pairing (for security, you might want to verify the PIN)
  }

  void BluetoothHandler::onPairingFinished(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing) {
    LOG_INFO_STREAM(BLUETOOTH, "[BluetoothHandler::onPairingFinished] pairingFinished, address: " + address.toString().toStdString() + ", pairing: " + std::to_string(pairing));
  }

  void BluetoothHandler::onError(QBluetoothLocalDevice::Error error) {
    LOG_WARN_STREAM(BLUETOOTH, "[BluetoothHandler::onError] Bluetooth error: " + std::to_string(error));
    // ... your logic to handle the error ...
  }

  void BluetoothHandler::onHostModeStateChanged(QBluetoothLocalDevice::HostMode state) {
    LOG_INFO_STREAM(BLUETOOTH, "[BluetoothHandler::onHostModeStateChanged] Host mode state changed: " + std::to_string(state));
    // ... your logic to handle the state change ...
  }
}