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

#include <f1x/openauto/btservice/AndroidBluetoothService.hpp>
#include <f1x/openauto/Common/Log.hpp>

namespace f1x::openauto::btservice {

  AndroidBluetoothService::AndroidBluetoothService() {
    OPENAUTO_LOG(info) << "[AndroidBluetoothService::AndroidBluetoothService] Initialising";
    const QBluetoothUuid serviceUuid(QLatin1String("4de17a00-52cb-11e6-bdf4-0800200c9a66"));

    QBluetoothServiceInfo::Sequence classId;
    classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::SerialPort));
    serviceInfo_.setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList, classId);
    classId.prepend(QVariant::fromValue(serviceUuid));
    serviceInfo_.setAttribute(QBluetoothServiceInfo::ServiceClassIds, classId);
    serviceInfo_.setAttribute(QBluetoothServiceInfo::ServiceName, "OpenAuto Bluetooth Service");
    serviceInfo_.setAttribute(QBluetoothServiceInfo::ServiceDescription, "AndroidAuto WiFi projection automatic setup");
    serviceInfo_.setAttribute(QBluetoothServiceInfo::ServiceProvider, "f1xstudio.com");
    serviceInfo_.setServiceUuid(serviceUuid);

    QBluetoothServiceInfo::Sequence publicBrowse;
    publicBrowse << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::PublicBrowseGroup));
    serviceInfo_.setAttribute(QBluetoothServiceInfo::BrowseGroupList, publicBrowse);
  }

  bool AndroidBluetoothService::registerService(int16_t portNumber, const QBluetoothAddress &bluetoothAddress) {
    OPENAUTO_LOG(info) << "[AndroidBluetoothService::registerService] Registering Service";

    QBluetoothServiceInfo::Sequence protocolDescriptorList;
    QBluetoothServiceInfo::Sequence protocol;
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::L2cap));
    protocolDescriptorList.append(QVariant::fromValue(protocol));
    protocol.clear();
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::Rfcomm))
             << QVariant::fromValue(quint16(portNumber));
    protocolDescriptorList.append(QVariant::fromValue(protocol));
    serviceInfo_.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList, protocolDescriptorList);

    return serviceInfo_.registerService(bluetoothAddress);
  }

  bool AndroidBluetoothService::unregisterService() {
    OPENAUTO_LOG(info) << "[AndroidBluetoothService::unregisterService] Unregistering";
    return serviceInfo_.unregisterService();
  }

}


