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

#include <thread>
#include <aasdk/USB/AOAPDevice.hpp>
#include <aasdk/TCP/TCPEndpoint.hpp>
#include <f1x/openauto/autoapp/App.hpp>
#include <modern/Logger.hpp>

namespace f1x::openauto::autoapp {

  App::App(boost::asio::io_service &ioService, aasdk::usb::USBWrapper &usbWrapper, aasdk::tcp::ITCPWrapper &tcpWrapper,
           service::IAndroidAutoEntityFactory &androidAutoEntityFactory,
           aasdk::usb::IUSBHub::Pointer usbHub,
           aasdk::usb::IConnectedAccessoriesEnumerator::Pointer connectedAccessoriesEnumerator)
      : ioService_(ioService), usbWrapper_(usbWrapper), tcpWrapper_(tcpWrapper), strand_(ioService_),
        androidAutoEntityFactory_(androidAutoEntityFactory), usbHub_(std::move(usbHub)),
        connectedAccessoriesEnumerator_(std::move(connectedAccessoriesEnumerator)),
        acceptor_(ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 5000)), isStopped_(false) {

  }

  void App::waitForUSBDevice() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      try {
        this->waitForDevice();
      }
      catch (...) {
        LOG_ERROR(GENERAL, "[App] waitForUSBDevice() -exception caused by this->waitForDevice()");
      }
      try {
        this->enumerateDevices();
      }
      catch (...) {
        LOG_ERROR(GENERAL, "[App] waitForUSBDevice() exception caused by this->enumerateDevices()");
      }

    });
  }

  void App::start(aasdk::tcp::ITCPEndpoint::SocketPointer socket) {
    strand_.dispatch([this, self = this->shared_from_this(), socket = std::move(socket)]() mutable {
      LOG_INFO(GENERAL, "Start from socket");
      if (androidAutoEntity_ != nullptr) {
//            tcpWrapper_.close(*socket);
//            LOG_WARN(GENERAL, "[App] android auto entity is still running.");
//            return;
        try {
          androidAutoEntity_->stop();
        } catch (...) {
          LOG_ERROR(UI, "[App] onAndroidAutoQuit: exception caused by androidAutoEntity_->stop()");
        }
        try {
          androidAutoEntity_.reset();
        } catch (...) {
          LOG_ERROR(UI, "[App] onAndroidAutoQuit: exception caused by androidAutoEntity_.reset()");
        }
      }

      try {
//            usbHub_->cancel();
//            connectedAccessoriesEnumerator_->cancel();

        auto tcpEndpoint(std::make_shared<aasdk::tcp::TCPEndpoint>(tcpWrapper_, std::move(socket)));
        androidAutoEntity_ = androidAutoEntityFactory_.create(std::move(tcpEndpoint));
        androidAutoEntity_->start(*this);
      }
      catch (const aasdk::error::Error &error) {
        LOG_ERROR(GENERAL, "[App] TCP AndroidAutoEntity create error: " << error.what()");

        //androidAutoEntity_.reset();
        this->waitForDevice();
      }
    });
  }

  void App::stop() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      isStopped_ = true;
      try {
        connectedAccessoriesEnumerator_->cancel();
      } catch (...) {
        LOG_ERROR(GENERAL, "[App] stop: exception caused by connectedAccessoriesEnumerator_->cancel()");
      }
      try {
        usbHub_->cancel();
      } catch (...) {
        LOG_ERROR(GENERAL, "[App] stop: exception caused by usbHub_->cancel()");
      }

      if (androidAutoEntity_ != nullptr) {
        try {
          androidAutoEntity_->stop();
        } catch (...) {
          LOG_ERROR(GENERAL, "[App] stop: exception caused by androidAutoEntity_->stop()");
        }
        try {
          androidAutoEntity_.reset();
        } catch (...) {
          LOG_ERROR(GENERAL, "[App] stop: exception caused by androidAutoEntity_.reset()");
        }
      }
    });

  }

  void App::aoapDeviceHandler(aasdk::usb::DeviceHandle deviceHandle) {
    LOG_INFO(GENERAL, "[App] Device connected.");

    if (androidAutoEntity_ != nullptr) {
      LOG_WARN(GENERAL, "[App] android auto entity is still running.");
      return;
    }

    try {
      // ignore autostart if exit to csng was used
      if (!disableAutostartEntity) {
        LOG_INFO(GENERAL, "[App] Start Android Auto allowed - let's go.");
        connectedAccessoriesEnumerator_->cancel();

        auto aoapDevice(aasdk::usb::AOAPDevice::create(usbWrapper_, ioService_, deviceHandle));
        androidAutoEntity_ = androidAutoEntityFactory_.create(std::move(aoapDevice));
        androidAutoEntity_->start(*this);
      } else {
        LOG_INFO(GENERAL, "[App] Start Android Auto not allowed - skip.");
      }
    }
    catch (const aasdk::error::Error &error) {
      LOG_ERROR(GENERAL, "[App] USB AndroidAutoEntity create error: " << error.what()");

      androidAutoEntity_.reset();
      this->waitForDevice();
    }
  }

  void App::enumerateDevices() {
    auto promise = aasdk::usb::IConnectedAccessoriesEnumerator::Promise::defer(strand_);
    promise->then([this, self = this->shared_from_this()](auto result) {
                    LOG_INFO(GENERAL, "[App] Devices enumeration result: " << result");
                  },
                  [this, self = this->shared_from_this()](auto e) {
                    LOG_ERROR(GENERAL, "[App] Devices enumeration failed: " << e.what()");
                  });

    connectedAccessoriesEnumerator_->enumerate(std::move(promise));
  }

  void App::waitForDevice() {
    LOG_INFO(GENERAL, "[App] Waiting for device...");

    auto promise = aasdk::usb::IUSBHub::Promise::defer(strand_);
    promise->then(std::bind(&App::aoapDeviceHandler, this->shared_from_this(), std::placeholders::_1),
                  std::bind(&App::onUSBHubError, this->shared_from_this(), std::placeholders::_1));
    usbHub_->start(std::move(promise));
    startServerSocket();
  }

  void App::startServerSocket() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      LOG_INFO(NETWORK, "startServerSocket() - Listening for WIFI Clients on Port 5000");
      auto socket = std::make_shared<boost::asio::ip::tcp::socket>(ioService_);
      acceptor_.async_accept(
          *socket,
          std::bind(&App::handleNewClient, this, socket, std::placeholders::_1)
      );
    });
  }

  void
  App::handleNewClient(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const boost::system::error_code &err) {
    LOG_INFO(NETWORK, "handleNewClient() - Handle WIFI Client Connection");
    if (!err) {
      start(std::move(socket));
    }
  }

  void App::pause() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      LOG_INFO(GENERAL, "[App] pause...");
      androidAutoEntity_->pause();
    });
  }

  void App::resume() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      if (androidAutoEntity_ != nullptr) {
        LOG_INFO(GENERAL, "[App] resume...");
        androidAutoEntity_->resume();
      } else {
        LOG_INFO(GENERAL, "[App] Ignore resume -> no androidAutoEntity_ ...");
      }
    });
  }

  void App::onAndroidAutoQuit() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      LOG_INFO(UI, "[App] onAndroidAutoQuit()");

      //acceptor_.close();

      if (androidAutoEntity_ != nullptr) {
        try {
          androidAutoEntity_->stop();
        } catch (...) {
          LOG_ERROR(UI, "[App] onAndroidAutoQuit: exception caused by androidAutoEntity_->stop()");
        }
        try {
          androidAutoEntity_.reset();
        } catch (...) {
          LOG_ERROR(UI, "[App] onAndroidAutoQuit: exception caused by androidAutoEntity_.reset()");
        }
      }

      if (!isStopped_) {
        try {
          this->waitForDevice();
        } catch (...) {
          LOG_ERROR(UI, "[App] onAndroidAutoQuit: exception caused by this->waitForDevice()");
        }
      }
    });
  }

  void App::onUSBHubError(const aasdk::error::Error &error) {
    LOG_ERROR(GENERAL, "[App] onUSBHubError(): " << error.what()");

//    if(error != aasdk::error::ErrorCode::OPERATION_ABORTED &&
//       error != aasdk::error::ErrorCode::OPERATION_IN_PROGRESS)
//    {
//        try {
//            this->waitForDevice();
//        } catch (...) {
//            LOG_ERROR(GENERAL, "[App] onUSBHubError: exception caused by this->waitForDevice()");
//        }
//    }
  }

}


