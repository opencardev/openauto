#pragma once

#include <gmock/gmock.h>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>

namespace f1x::openauto::autoapp::configuration {

class MockConfiguration : public IConfiguration {
public:
    MOCK_METHOD(void, load, (), (override));
    MOCK_METHOD(void, reset, (), (override));
    MOCK_METHOD(void, save, (), (override));
    MOCK_METHOD(bool, hasTouchScreen, (), (const, override));
    MOCK_METHOD(void, setHandednessOfTrafficType, (HandednessOfTrafficType value), (override));
    MOCK_METHOD(HandednessOfTrafficType, getHandednessOfTrafficType, (), (const, override));
    MOCK_METHOD(void, showClock, (bool value), (override));
    MOCK_METHOD(bool, showClock, (), (const, override));
    MOCK_METHOD(void, showBigClock, (bool value), (override));
    MOCK_METHOD(bool, showBigClock, (), (const, override));
    MOCK_METHOD(void, oldGUI, (bool value), (override));
    MOCK_METHOD(bool, oldGUI, (), (const, override));
    MOCK_METHOD(void, setAlphaTrans, (size_t value), (override));
    MOCK_METHOD(size_t, getAlphaTrans, (), (const, override));
    MOCK_METHOD(void, hideMenuToggle, (bool value), (override));
    MOCK_METHOD(bool, hideMenuToggle, (), (const, override));
    MOCK_METHOD(void, hideAlpha, (bool value), (override));
    MOCK_METHOD(bool, hideAlpha, (), (const, override));
    MOCK_METHOD(QString, getCSValue, (const QString& key), (const, override));
    MOCK_METHOD(QString, readFileContent, (const QString& fileName), (const, override));
};

}  // namespace f1x::openauto::autoapp::configuration