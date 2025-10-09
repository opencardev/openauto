//
// Created by Simon Dean on 26/11/2024.
//

#include <f1x/openauto/btservice/BluetoothHandler.hpp>
#include <f1x/openauto/btservice/AndroidBluetoothService.hpp>
#include <f1x/openauto/btservice/AndroidBluetoothServer.hpp>
#include <openauto/Common/ModernLogger.hpp>

namespace f1x::openauto::btservice {
  BluetoothHandler::BluetoothHandler(btservice::IAndroidBluetoothService::Pointer androidBluetoothService,
                                     autoapp::configuration::IConfiguration::Pointer configuration)
  : configuration_(std::move(configuration)),
    androidBluetoothService_(std::move(androidBluetoothService)),
    androidBluetoothServer_(std::make_unique<btservice::AndroidBluetoothServer>(configuration_)) {

    OPENAUTO_LOG_INFO(BLUETOOTH, "[BluetoothHandler::BluetoothHandler] Starting Up...");

    QString adapterAddress = QString::fromStdString(configuration_->getBluetoothAdapterAddress());
    QBluetoothAddress address(adapterAddress);
    localDevice_ = std::make_unique<QBluetoothLocalDevice>(QBluetoothAddress());

    if (!localDevice_->isValid()) {
      OPENAUTO_LOG_ERROR(BLUETOOTH, "[BluetoothHandler] Bluetooth adapter is not valid.");
    } else {
      OPENAUTO_LOG_INFO(BLUETOOTH, "[BluetoothHandler] Bluetooth adapter is valid.");
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
      OPENAUTO_LOG_ERROR(BLUETOOTH, "[BluetoothHandler::BluetoothHandler] Server start failed.");
      throw std::runtime_error("Unable to start bluetooth server");
    }

    OPENAUTO_LOG_INFO(BLUETOOTH, (std::stringstream() << "[BluetoothHandler::BluetoothHandler] Listening for connections, address: " << address.toString().toStdString()
                       << ", port: " << portNumber).str());

   if (!androidBluetoothService_->registerService(portNumber, address)) {
      OPENAUTO_LOG_ERROR(BLUETOOTH, "[BluetoothHandler::BluetoothHandler] Service registration failed.");
      throw std::runtime_error("Unable to register btservice");
    } else {
      OPENAUTO_LOG_INFO(BLUETOOTH, (std::stringstream() << "[BluetoothHandler::BluetoothHandler] Service registered, port: " << portNumber).str());
    }

    // TODO: Connect to any previously paired devices
  }

  void BluetoothHandler::shutdownService() {
    OPENAUTO_LOG_INFO(BLUETOOTH, "[BluetoothHandler::shutdownService] Shutdown initiated");
    androidBluetoothService_->unregisterService();
  }

  void BluetoothHandler::onPairingDisplayPinCode(const QBluetoothAddress &address, QString pin) {
    OPENAUTO_LOG_DEBUG(BLUETOOTH, (std::stringstream() << "[BluetoothHandler::onPairingDisplayPinCode] Pairing display PIN code: " << pin.toStdString()).str());
  }

  void BluetoothHandler::onPairingDisplayConfirmation(const QBluetoothAddress &address, QString pin) {
    OPENAUTO_LOG_DEBUG(BLUETOOTH, (std::stringstream() << "[BluetoothHandler::onPairingDisplayConfirmation] Pairing display confirmation: " << pin.toStdString()).str());

    // Here you can implement logic to show this PIN to the user or automatically accept if you trust all devices
    localDevice_->pairingConfirmation(true); // Confirm pairing (for security, you might want to verify the PIN)
  }

  void BluetoothHandler::onPairingFinished(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing) {
    OPENAUTO_LOG_INFO(BLUETOOTH, (std::stringstream() << "[BluetoothHandler::onPairingFinished] pairingFinished, address: " << address.toString().toStdString()
                       << ", pairing: " << pairing).str());
  }

  void BluetoothHandler::onError(QBluetoothLocalDevice::Error error) {
    OPENAUTO_LOG_WARN(BLUETOOTH, (std::stringstream() << "[BluetoothHandler::onError] Bluetooth error: " << error).str());
    // ... your logic to handle the error ...
  }

  void BluetoothHandler::onHostModeStateChanged(QBluetoothLocalDevice::HostMode state) {
    OPENAUTO_LOG_INFO(BLUETOOTH, (std::stringstream() << "[BluetoothHandler::onHostModeStateChanged] Host mode state changed: " << state).str());
    // ... your logic to handle the state change ...
  }
}