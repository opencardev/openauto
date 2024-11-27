/*
*  This file is part of openauto project.
*  Copyright (C) 2018 f1x.studio (Michal Szwaj)
*
*  openauto is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 3 of the License, or
*  (at your option) any later version.

*  openauto is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with openauto. If not, see <http://www.gnu.org/licenses/>.
*/

#include <QApplication>
#include <f1x/openauto/Common/Log.hpp>
#include <f1x/openauto/autoapp/Projection/LocalBluetoothDevice.hpp>
#include <QtBluetooth>

namespace f1x::openauto::autoapp::projection {

  LocalBluetoothDevice::LocalBluetoothDevice(const QString &adapterAddress, QObject *parent) : QObject(parent) {
    qRegisterMetaType<IBluetoothDevice::PairingPromise::Pointer>("PairingPromise::Pointer");

    this->moveToThread(QApplication::instance()->thread());

    QMetaObject::invokeMethod(this, "createBluetoothLocalDevice", Qt::BlockingQueuedConnection,
                              Q_ARG(QString, adapterAddress));

  }

  void LocalBluetoothDevice::createBluetoothLocalDevice(const QString &adapterAddress) {
    OPENAUTO_LOG(info) << "[LocalBluetoothDevice] create.";


    QBluetoothAddress address(adapterAddress);
    localDevice_ = std::make_unique<QBluetoothLocalDevice>(address);

    // Pairing signals are being handled by btservice

  }

  void LocalBluetoothDevice::stop() {
    std::lock_guard<decltype(mutex_)> lock(mutex_);

  }

  bool LocalBluetoothDevice::isPaired(const std::string &address) const {
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    return localDevice_->pairingStatus(QBluetoothAddress(QString::fromStdString(address))) !=
           QBluetoothLocalDevice::Unpaired;
  }

  std::string LocalBluetoothDevice::getAdapterAddress() const {
    std::lock_guard<decltype(mutex_)> lock(mutex_);
    return localDevice_->isValid() ? localDevice_->address().toString().toStdString() : "";
  }

  bool LocalBluetoothDevice::isAvailable() const {
    std::lock_guard<decltype(mutex_)> lock(mutex_);
    return localDevice_->isValid();
  }
}



