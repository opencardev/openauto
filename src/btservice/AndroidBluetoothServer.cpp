/*
*  This file is part of openauto project.
*  Copyright (C) 2018 f1x.studio (Michal Szwaj)
*
*  openauto is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as publis  void AndroidBluetoothServer::sendMessage(const google::protobuf::Message &msg, uint16_t msgId) {
    LOG_INFO(BLUETOOTH, "Sending message to connected device");d by
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
#include <modern/Logger.hpp>
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
    LOG_INFO(BLUETOOTH, "Initialising Android Bluetooth Server");

    connect(rfcommServer_.get(), &QBluetoothServer::newConnection, this,
            &AndroidBluetoothServer::onClientConnected);

  }

  /// Start Server listening on Address
  uint16_t AndroidBluetoothServer::start(const QBluetoothAddress &address) {
    LOG_DEBUG(BLUETOOTH, "Starting Bluetooth server");
    rfcommServer_->close(); // Close should always be called before listen.
    if (rfcommServer_->listen(address)) {

      return rfcommServer_->serverPort();
    }
    return 0;
  }

  void AndroidBluetoothServer::onError(QBluetoothServer::Error error) {
    LOG_DEBUG(BLUETOOTH, "Bluetooth server error occurred");
  }

  /// Call-Back for when Client Connected
  void AndroidBluetoothServer::onClientConnected() {
    LOG_DEBUG(BLUETOOTH, "Client connected to Bluetooth server");
    if (socket != nullptr) {
      socket->deleteLater();
    }

    socket = rfcommServer_->nextPendingConnection();

    if (socket != nullptr) {
      std::map<std::string, std::string> context = {
          {"peer_name", socket->peerName().toStdString()}
      };
      LOG_DEBUG_CTX(BLUETOOTH, "RFCOMM client connected", context);

      connect(socket, &QBluetoothSocket::readyRead, this, &AndroidBluetoothServer::readSocket);

      aap_protobuf::aaw::WifiVersionRequest versionRequest;
      aap_protobuf::aaw::WifiStartRequest startRequest;
      startRequest.set_ip_address(getIP4_("wlan0"));
      startRequest.set_port(5000);

      sendMessage(versionRequest, aap_protobuf::aaw::MessageId::WIFI_VERSION_REQUEST);
      sendMessage(startRequest, aap_protobuf::aaw::MessageId::WIFI_START_REQUEST);
    } else {
      LOG_ERROR(BLUETOOTH, "Received null socket during client connection");
    }
  }

  /// Read data from Bluetooth Socket
  void AndroidBluetoothServer::readSocket() {
    buffer += socket->readAll();

    LOG_DEBUG(BLUETOOTH, "Reading from socket");

    if (buffer.length() < 4) {
      LOG_DEBUG(BLUETOOTH, "Not enough data, waiting for more");
      return;
    }

    QDataStream stream(buffer);
    uint16_t length;
    stream >> length;

    if (buffer.length() < length + 4) {
      std::map<std::string, std::string> context = {
          {"buffer_length", std::to_string(buffer.length())}
      };
      LOG_DEBUG_CTX(BLUETOOTH, "Not enough data, waiting for more", context);
      return;
    }

    quint16 rawMessageId;
    stream >> rawMessageId;

    aap_protobuf::aaw::MessageId messageId;
    messageId = static_cast<aap_protobuf::aaw::MessageId>(rawMessageId);

    std::map<std::string, std::string> context = {
        {"message_length", std::to_string(length)},
        {"message_id", std::to_string(messageId)}
    };
    LOG_DEBUG_CTX(BLUETOOTH, "Processing message", context);

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
        std::map<std::string, std::string> context = {
            {"message_id", std::to_string(messageId)},
            {"data", ss.str()}
        };
        LOG_DEBUG_CTX(BLUETOOTH, "Unknown message received", context);

        break;
    }

    buffer = buffer.mid(length + 4);
  }

  /// Handles request for WifiInfoRequest by sending a WifiInfoResponse
  /// \param buffer
  /// \param length
  void AndroidBluetoothServer::handleWifiInfoRequest(QByteArray &buffer, uint16_t length) {
    LOG_INFO(BLUETOOTH, "Handling WiFi info request");

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
    LOG_INFO(BLUETOOTH, "Handling WiFi version response");

    aap_protobuf::aaw::WifiVersionResponse response;
    response.ParseFromArray(buffer.data() + 4, length);
    std::map<std::string, std::string> context = {
        {"unknown_param_1", std::to_string(response.unknown_value_a())},
        {"unknown_param_2", std::to_string(response.unknown_value_b())}
    };
    LOG_DEBUG_CTX(BLUETOOTH, "WiFi version response parameters", context);
  }

  /// Listens for WifiStartResponse from MD - usually just a notification with a status
  /// \param buffer
  /// \param length
  void AndroidBluetoothServer::handleWifiStartResponse(QByteArray &buffer, uint16_t length) {
    LOG_INFO(BLUETOOTH, "Handling WiFi start response");

    aap_protobuf::aaw::WifiStartResponse response;
    response.ParseFromArray(buffer.data() + 4, length);
    std::map<std::string, std::string> context = {
        {"ip_address", response.ip_address()},
        {"port", std::to_string(response.port())},
        {"status", Status_Name(response.status())}
    };
    LOG_DEBUG_CTX(BLUETOOTH, "WiFi start response details", context);
  }

  /// Handles request for WifiStartRequest by sending a WifiStartResponse
  /// \param buffer
  /// \param length
  void AndroidBluetoothServer::handleWifiConnectionStatus(QByteArray &buffer, uint16_t length) {
    aap_protobuf::aaw::WifiConnectionStatus status;
    status.ParseFromArray(buffer.data() + 4, length);
    std::map<std::string, std::string> context = {
        {"status", Status_Name(status.status())}
    };
    LOG_INFO_CTX(BLUETOOTH, "Handle WiFi connection status", context);
  }

  void AndroidBluetoothServer::sendMessage(const google::protobuf::Message &message, uint16_t type) {
    LOG_INFO(BLUETOOTH, "Sending message to connected device");

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

    std::map<std::string, std::string> context = {
        {"message_type", message.GetTypeName()},
        {"debug_string", message.DebugString()}
    };
    LOG_DEBUG_CTX(BLUETOOTH, "Sending protobuf message", context);

    auto written = socket->write(out);
    if (written > -1) {
      std::map<std::string, std::string> context = {
          {"bytes_written", std::to_string(written)}
      };
      LOG_DEBUG_CTX(BLUETOOTH, "Bytes written to socket", context);
    } else {
      LOG_DEBUG(BLUETOOTH, "Could not write data to socket");
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
