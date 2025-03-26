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

#pragma once

#include <f1x/openauto/autoapp/Service/IServiceFactory.hpp>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>

namespace f1x {
  namespace openauto {
    namespace autoapp {
      namespace service {

        class ServiceFactory : public IServiceFactory {
        public:
          ServiceFactory(boost::asio::io_service &ioService, configuration::IConfiguration::Pointer configuration);
          ServiceList create(aasdk::messenger::IMessenger::Pointer messenger) override;

        private:
          IService::Pointer createBluetoothService(aasdk::messenger::IMessenger::Pointer messenger);
          IService::Pointer createGenericNotificationService(aasdk::messenger::IMessenger::Pointer messenger);
          IService::Pointer createInputService(aasdk::messenger::IMessenger::Pointer messenger);
          IService::Pointer createMediaBrowserService(aasdk::messenger::IMessenger::Pointer messenger);
          IService::Pointer createMediaPlaybackStatusService(aasdk::messenger::IMessenger::Pointer messenger);

          void createMediaSinkServices(ServiceList &serviceList, aasdk::messenger::IMessenger::Pointer messenger);
          void createMediaSourceServices(ServiceList &serviceList, aasdk::messenger::IMessenger::Pointer messenger);

          IService::Pointer createNavigationStatusService(aasdk::messenger::IMessenger::Pointer messenger);
          IService::Pointer createPhoneStatusService(aasdk::messenger::IMessenger::Pointer messenger);
          IService::Pointer createRadioService(aasdk::messenger::IMessenger::Pointer messenger);
          IService::Pointer createSensorService(aasdk::messenger::IMessenger::Pointer messenger);
          IService::Pointer createVendorExtensionService(aasdk::messenger::IMessenger::Pointer messenger);
          IService::Pointer createWifiProjectionService(aasdk::messenger::IMessenger::Pointer messenger);

          boost::asio::io_service &ioService_;
          configuration::IConfiguration::Pointer configuration_;
        };

      }
    }
  }
}
