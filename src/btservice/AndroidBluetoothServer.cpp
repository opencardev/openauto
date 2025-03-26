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
#include <iostream>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/unknown_field_set.h>

using namespace google::protobuf;
using namespace google::protobuf::io;

// 39171FDJG002WHhandleWifiVersionRequest

namespace f1x::openauto::btservice {

  AndroidBluetoothServer::AndroidBluetoothServer(autoapp::configuration::IConfiguration::Pointer configuration)
      : rfcommServer_(std::make_unique<QBluetoothServer>(QBluetoothServiceInfo::RfcommProtocol, this)),
        configuration_(std::move(configuration)) {
    OPENAUTO_LOG(info) << "[AndroidBluetoothServer::AndroidBluetoothServer] Initialising";

    connect(rfcommServer_.get(), &QBluetoothServer::newConnection, this,
            &AndroidBluetoothServer::onClientConnected);

  }

  /// Start Server listening on Address
  uint16_t AndroidBluetoothServer::start(const QBluetoothAddress &address) {
    OPENAUTO_LOG(debug) << "[AndroidBluetoothServer::start]";
    rfcommServer_->close(); // Close should always be called before listen.
    if (rfcommServer_->listen(address)) {

      return rfcommServer_->serverPort();
    }
    return 0;
  }

  void AndroidBluetoothServer::onError(QBluetoothServer::Error error) {
    OPENAUTO_LOG(debug) << "[AndroidBluetoothServer::onError]";
  }

  /// Call-Back for when Client Connected
  void AndroidBluetoothServer::onClientConnected() {
    OPENAUTO_LOG(debug) << "[AndroidBluetoothServer::onClientConnected]";
    if (socket != nullptr) {
      socket->deleteLater();
    }

    socket = rfcommServer_->nextPendingConnection();

    if (socket != nullptr) {
      OPENAUTO_LOG(debug) << "[AndroidBluetoothServer] rfcomm client connected, peer name: "
                         << socket->peerName().toStdString();

      connect(socket, &QBluetoothSocket::readyRead, this, &AndroidBluetoothServer::readSocket);

      aap_protobuf::aaw::WifiVersionRequest versionRequest;
      aap_protobuf::aaw::WifiStartRequest startRequest;
      startRequest.set_ip_address(getIP4_("wlan0"));
      startRequest.set_port(5000);

      sendMessage(versionRequest, aap_protobuf::aaw::MessageId::WIFI_VERSION_REQUEST);
      sendMessage(startRequest, aap_protobuf::aaw::MessageId::WIFI_START_REQUEST);
    } else {
      OPENAUTO_LOG(error) << "[AndroidBluetoothServer] received null socket during client connection.";
    }
  }

  /// Read data from Bluetooth Socket
  void AndroidBluetoothServer::readSocket() {
    buffer += socket->readAll();

    OPENAUTO_LOG(debug) << "[AndroidBluetoothServer::readSocket] Reading from socket.";

    if (buffer.length() < 4) {
      OPENAUTO_LOG(debug) << "[AndroidBluetoothServer::readSocket] Not enough data, waiting for more.";
      return;
    }

    QDataStream stream(buffer);
    uint16_t length;
    stream >> length;

    if (buffer.length() < length + 4) {
      OPENAUTO_LOG(debug) << "[AndroidBluetoothServer::readSocket] Not enough data, waiting for more: " << buffer.length();
      return;
    }

    quint16 rawMessageId;
    stream >> rawMessageId;

    aap_protobuf::aaw::MessageId messageId;
    messageId = static_cast<aap_protobuf::aaw::MessageId>(rawMessageId);

    OPENAUTO_LOG(debug) << "[AndroidBluetoothServer::readSocket] Message length: " << length << " MessageId: " << messageId;

    switch (messageId) {

      case aap_protobuf::aaw::MessageId::WIFI_INFO_REQUEST: // WifiInfoRequest - Respond with a WifiInfoResponse
        handleWifiInfoRequest(buffer, length);
        break;
      case aap_protobuf::aaw::MessageId::WIFI_VERSION_RESPONSE: // WifiVersionRequest - Send a Version Request
        handleWifiVersionResponse(buffer, length);// do something
        break;
      case aap_protobuf::aaw::MessageId::WIFI_CONNECTION_STATUS: // WifiStartResponse  - Receive a confirmation
        handleWifiConnectionStatus(buffer, length);
        break;
      case aap_protobuf::aaw::MessageId::WIFI_START_RESPONSE: // WifiStartResponse  - Receive a confirmation
        handleWifiStartResponse(buffer, length);
        break;
      case aap_protobuf::aaw::MessageId::WIFI_START_REQUEST:      // These are not received from the MD.
      case aap_protobuf::aaw::MessageId::WIFI_INFO_RESPONSE:      // These are not received from the MD.
      case aap_protobuf::aaw::MessageId::WIFI_VERSION_REQUEST:    // These are not received from the MD.
      default:
        QByteArray messageData = buffer.mid(stream.device()->pos(), length - 2);

        // Convert QByteArray to std::string
        std::string protoData = messageData.toStdString();

        // Pass it to your function
        this->DecodeProtoMessage(protoData);

        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (auto &&val: buffer) {
          ss << std::setw(2) << static_cast<unsigned>(val);
        }
        OPENAUTO_LOG(debug) << "[AndroidBluetoothServer::readSocket] Unknown message: " << messageId;
        OPENAUTO_LOG(debug) << "[AndroidBluetoothServer::readSocket] Data " << ss.str();

        break;
    }

    buffer = buffer.mid(length + 4);
  }

  /// Handles request for WifiInfoRequest by sending a WifiInfoResponse
  /// \param buffer
  /// \param length
  void AndroidBluetoothServer::handleWifiInfoRequest(QByteArray &buffer, uint16_t length) {
    OPENAUTO_LOG(info) << "[AndroidBluetoothServer::handleWifiInfoRequest] Handling wifi info request";

    aap_protobuf::aaw::WifiInfoResponse response;

    response.set_ssid(configuration_->getParamFromFile("/etc/hostapd/hostapd.conf", "ssid").toStdString());
    response.set_password(
        configuration_->getParamFromFile("/etc/hostapd/hostapd.conf", "wpa_passphrase").toStdString());
    response.set_bssid(QNetworkInterface::interfaceFromName("wlan0").hardwareAddress().toStdString());
    // TODO: AAP uses different values than WiFiProjection....
    response.set_security_mode(
        aap_protobuf::service::wifiprojection::message::WifiSecurityMode::WPA2_ENTERPRISE);
    response.set_access_point_type(aap_protobuf::service::wifiprojection::message::AccessPointType::STATIC);

    sendMessage(response, 3);
  }

  /// Listens for a WifiVersionResponse from the MD - usually just a notification
  /// \param buffer
  /// \param length
  void AndroidBluetoothServer::handleWifiVersionResponse(QByteArray &buffer, uint16_t length) {
    OPENAUTO_LOG(info) << "[AndroidBluetoothServer::handleWifiVersionResponse] Handling wifi version response";

    aap_protobuf::aaw::WifiVersionResponse response;
    response.ParseFromArray(buffer.data() + 4, length);
    OPENAUTO_LOG(debug) << "[AndroidBluetoothServer::handleWifiVersionResponse] Unknown Param 1: " << response.unknown_value_a() << " Unknown Param 2: " << response.unknown_value_b();
  }

  /// Listens for WifiStartResponse from MD - usually just a notification with a status
  /// \param buffer
  /// \param length
  void AndroidBluetoothServer::handleWifiStartResponse(QByteArray &buffer, uint16_t length) {
    OPENAUTO_LOG(info) << "[AndroidBluetoothServer::handleWifiStartResponse] Handling wifi start response";

    aap_protobuf::aaw::WifiStartResponse response;
    response.ParseFromArray(buffer.data() + 4, length);
    OPENAUTO_LOG(debug) << "[AndroidBluetoothServer::handleWifiStartResponse] " << response.ip_address() << " port " << response.port() << " status " << Status_Name(response.status());
  }

  /// Handles request for WifiStartRequest by sending a WifiStartResponse
  /// \param buffer
  /// \param length
  void AndroidBluetoothServer::handleWifiConnectionStatus(QByteArray &buffer, uint16_t length) {
    aap_protobuf::aaw::WifiConnectionStatus status;
    status.ParseFromArray(buffer.data() + 4, length);
    OPENAUTO_LOG(info) << "[AndroidBluetoothServer::handleWifiConnectionStatus] Handle wifi connection status, received: " << Status_Name(status.status());
  }

  void AndroidBluetoothServer::sendMessage(const google::protobuf::Message &message, uint16_t type) {
    OPENAUTO_LOG(info) << "[AndroidBluetoothServer::sendMessage] Sending message to connected device";

    int byteSize = message.ByteSizeLong();
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

    OPENAUTO_LOG(debug) << message.GetTypeName() << " - " + message.DebugString();

    auto written = socket->write(out);
    if (written > -1) {
      OPENAUTO_LOG(debug) << "[AndroidBluetoothServer::sendMessage] Bytes written: " << written;
    } else {
      OPENAUTO_LOG(debug) << "[AndroidBluetoothServer::sendMessage] Could not write data";
    }
  }

  const ::std::string AndroidBluetoothServer::getIP4_(const QString intf) {
    for (const QNetworkAddressEntry &address: QNetworkInterface::interfaceFromName(intf).addressEntries()) {
      if (address.ip().protocol() == QAbstractSocket::IPv4Protocol)
        return address.ip().toString().toStdString();
    }
    return "";
  }

  /// Decode Proto Messages to their constituent components
  /// \param proto_data
  void AndroidBluetoothServer::DecodeProtoMessage(const std::string& proto_data) {
    UnknownFieldSet set;

    // Create streams
    ArrayInputStream raw_input(proto_data.data(), proto_data.size());
    CodedInputStream input(&raw_input);

    // Decode the message
    if (!set.MergeFromCodedStream(&input)) {
      std::cerr << "Failed to decode the message." << std::endl;
      return;
    }

    // Iterate over the fields
    for (int i = 0; i < set.field_count(); ++i) {
      const UnknownField& field = set.field(i);
      switch (field.type()) {
        case UnknownField::TYPE_VARINT:
          std::cout << "Field number " << field.number() << " is a varint: " << field.varint() << std::endl;
          break;
        case UnknownField::TYPE_FIXED32:
          std::cout << "Field number " << field.number() << " is a fixed32: " << field.fixed32() << std::endl;
          break;
        case UnknownField::TYPE_FIXED64:
          std::cout << "Field number " << field.number() << " is a fixed64: " << field.fixed64() << std::endl;
          break;
        case UnknownField::TYPE_LENGTH_DELIMITED:
          std::cout << "Field number " << field.number() << " is length-delimited: ";
          for (char ch : field.length_delimited()) {
            std::cout << std::hex << (int)(unsigned char)ch;
          }
          std::cout << std::dec << std::endl;
          break;
        case UnknownField::TYPE_GROUP:  // Deprecated in modern Protobuf
          std::cout << "Field number " << field.number() << " is a group." << std::endl;
          break;
      }
    }
  }
}