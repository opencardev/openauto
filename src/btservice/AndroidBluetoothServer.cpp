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

#include <boost/algorithm/hex.hpp>
#include <f1x/openauto/Common/Log.hpp>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include <f1x/openauto/btservice/AndroidBluetoothServer.hpp>
#include <QString>
#include <QtCore/QDataStream>
#include <QNetworkInterface>


namespace f1x::openauto::btservice {

  AndroidBluetoothServer::AndroidBluetoothServer(autoapp::configuration::IConfiguration::Pointer configuration)
      : rfcommServer_(std::make_unique<QBluetoothServer>(QBluetoothServiceInfo::RfcommProtocol, this)),
        configuration_(std::move(configuration)) {
    connect(rfcommServer_.get(), &QBluetoothServer::newConnection, this,
            &AndroidBluetoothServer::onClientConnected);
  }

  uint16_t AndroidBluetoothServer::start(const QBluetoothAddress &address) {
    if (rfcommServer_->listen(address)) {
      return rfcommServer_->serverPort();
    }
    return 0;
  }

  void AndroidBluetoothServer::onClientConnected() {
    if (socket != nullptr) {
      socket->deleteLater();
    }

    socket = rfcommServer_->nextPendingConnection();

    if (socket != nullptr) {
      OPENAUTO_LOG(info) << "[AndroidBluetoothServer] rfcomm client connected, peer name: "
                         << socket->peerName().toStdString();

      connect(socket, &QBluetoothSocket::readyRead, this, &AndroidBluetoothServer::readSocket);

      aap_protobuf::service::wifiprojection::message::WifiCredentialsRequest request;
      // TODO: How do we ping back the Wireless Port and IP?
      //aap_protobuf::service::::WifiInfoRequest request;
      request.set_ip_address(getIP4_("wlan0"));
      getIP4_()
      //request.set_port(5000);

      sendMessage(request, 1);
    } else {
      OPENAUTO_LOG(error) << "[AndroidBluetoothServer] received null socket during client connection.";
    }
  }

  void AndroidBluetoothServer::readSocket() {
    buffer += socket->readAll();

    OPENAUTO_LOG(info) << "Received message";

    if (buffer.length() < 4) {
      OPENAUTO_LOG(debug) << "Not enough data, waiting for more";
      return;
    }

    QDataStream stream(buffer);
    uint16_t length;
    stream >> length;

    if (buffer.length() < length + 4) {
      OPENAUTO_LOG(info) << "Not enough data, waiting for more: " << buffer.length();
      return;
    }

    aap_protobuf::service::wifiprojection::WifiProjectionMessageId messageId;
    //uint16_t messageId;
    stream >> messageId;

    //OPENAUTO_LOG(info) << "[AndroidBluetoothServer] " << length << " " << messageId;
    OPENAUTO_LOG(debug) << messageId;

        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (auto &&val: buffer) {
          ss << std::setw(2) << static_cast<unsigned>(val);
        }
        OPENAUTO_LOG(info) << "Unknown message: " << messageId;
        OPENAUTO_LOG(info) << ss.str();

    buffer = buffer.mid(length + 4);
  }

  void AndroidBluetoothServer::handleWifiInfoRequest(QByteArray &buffer, uint16_t length) {
    aap_protobuf::service::wifiprojection::message::WifiCredentialsRequest msg;
    msg.ParseFromArray(buffer.data() + 4, length);
    OPENAUTO_LOG(info) << "WifiInfoRequest: " << msg.DebugString();

    aap_protobuf::service::wifiprojection::message::WifiCredentialsResponse response;

    //response.set_ip_address(getIP4_("wlan0"));
    //response.set_port(5000);
    //response.set_status(aap_protobuf::service::control::WifiInfoResponse_Status_STATUS_SUCCESS);

    sendMessage(response, 7);
  }

  void AndroidBluetoothServer::handleWifiSecurityRequest(QByteArray &buffer, uint16_t length) {
    OPENAUTO_LOG(info) << "[AndroidBluetoothServer] WifiSecurityRequest:";
    aap_protobuf::service::wifiprojection::message::WifiCredentialsResponse response;

    response.set_car_wifi_security_mode(
        aap_protobuf::service::wifiprojection::message::WifiSecurityMode::WPA2_PERSONAL);
    response.set_car_wifi_ssid(configuration_->getParamFromFile("/etc/hostapd/hostapd.conf", "ssid").toStdString());
    response.set_car_wifi_password(
        configuration_->getParamFromFile("/etc/hostapd/hostapd.conf", "wpa_passphrase").toStdString());
    response.set_access_point_type(aap_protobuf::service::wifiprojection::message::AccessPointType::STATIC);
    response.add_supported_wifi_channels(1);


    //response.set_ssid(configuration_->getParamFromFile("/etc/hostapd/hostapd.conf","ssid").toStdString());
    //response.set_bssid(QNetworkInterface::interfaceFromName("wlan0").hardwareAddress().toStdString());
    //response.set_key(configuration_->getParamFromFile("/etc/hostapd/hostapd.conf","wpa_passphrase").toStdString());
    //response.set_security_mode(aap_protobuf::messages::WifiSecurityReponse_SecurityMode_WPA2_PERSONAL);
    //response.set_access_point_type(aap_protobuf::messages::WifiSecurityReponse_AccessPointType_STATIC);

    sendMessage(response, 3);
  }

  void AndroidBluetoothServer::sendMessage(const google::protobuf::Message &message, uint16_t type) {
    int byteSize = message.ByteSize();
    QByteArray out(byteSize + 4, 0);
    QDataStream ds(&out, QIODevice::ReadWrite);
    ds << (uint16_t) byteSize;
    ds << type;
    message.SerializeToArray(out.data() + 4, byteSize);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (auto &&val: out) {
      ss << std::setw(2) << static_cast<unsigned>(val);
    }
    //OPENAUTO_LOG(info) << "Writing message: " << ss.str();
    OPENAUTO_LOG(debug) << message.GetTypeName() << " - " + message.DebugString();

    auto written = socket->write(out);
    if (written > -1) {
      OPENAUTO_LOG(info) << "Bytes written: " << written;
    } else {
      OPENAUTO_LOG(info) << "Could not write data";
    }
  }

  void AndroidBluetoothServer::handleWifiInfoRequestResponse(QByteArray &buffer, uint16_t length) {
    OPENAUTO_LOG(info) << "[AndroidBluetoothServer] WifiInfoRequestResponse";
    aap_protobuf::service::wifiprojection::message::WifiCredentialsResponse msg;
    msg.ParseFromArray(buffer.data() + 4, length);
    OPENAUTO_LOG(info) << "WifiInfoResponse: " << msg.DebugString();
  }

  const ::std::string AndroidBluetoothServer::getIP4_(const QString intf) {
    for (const QNetworkAddressEntry &address: QNetworkInterface::interfaceFromName(intf).addressEntries()) {
      if (address.ip().protocol() == QAbstractSocket::IPv4Protocol)
        return address.ip().toString().toStdString();
    }
    return "";
  }
}

