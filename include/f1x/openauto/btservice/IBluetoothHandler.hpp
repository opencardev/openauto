//
// Created by Simon Dean on 26/11/2024.
//

#ifndef OPENAUTO_IBLUETOOTHHANDLER_HPP
#define OPENAUTO_IBLUETOOTHHANDLER_HPP

#include <QBluetoothAddress>
#include <QBluetoothLocalDevice>

namespace f1x::openauto::btservice {

  class IBluetoothHandler
  {
  public:
    virtual ~IBluetoothHandler() = default;

  };

}



#endif //OPENAUTO_IBLUETOOTHHANDLER_HPP
