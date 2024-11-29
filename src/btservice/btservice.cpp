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

#include <QCoreApplication>
#include <QtBluetooth>
#include <f1x/openauto/Common/Log.hpp>
#include <f1x/openauto/autoapp/Configuration/Configuration.hpp>
#include <f1x/openauto/btservice/BluetoothHandler.hpp>
#include <f1x/openauto/btservice/AndroidBluetoothService.hpp>

namespace btservice = f1x::openauto::btservice;

int main(int argc, char *argv[]) {
  QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth*=true"));
  QCoreApplication qApplication(argc, argv);

  auto configuration = std::make_shared<f1x::openauto::autoapp::configuration::Configuration>();

  try {
    auto androidBluetoothService = std::make_shared<btservice::AndroidBluetoothService>();
    btservice::BluetoothHandler bluetoothHandler(androidBluetoothService, configuration);
    QCoreApplication::exec();
  } catch (std::runtime_error& e) {
    std::cerr << "Exception caught: " << e.what() << std::endl;
  }

  OPENAUTO_LOG(info) << "stop";

  return 0;
}
