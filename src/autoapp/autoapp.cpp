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
#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>
#include <aasdk/USB/USBHub.hpp>
#include <aasdk/USB/ConnectedAccessoriesEnumerator.hpp>
#include <aasdk/USB/AccessoryModeQueryChain.hpp>
#include <aasdk/USB/AccessoryModeQueryChainFactory.hpp>
#include <aasdk/USB/AccessoryModeQueryFactory.hpp>
#include <aasdk/TCP/TCPWrapper.hpp>
#include <boost/log/utility/setup.hpp>
#include <f1x/openauto/autoapp/App.hpp>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include <f1x/openauto/autoapp/Configuration/RecentAddressesList.hpp>
#include <f1x/openauto/autoapp/Service/AndroidAutoEntityFactory.hpp>
#include <f1x/openauto/autoapp/Service/ServiceFactory.hpp>
#include <f1x/openauto/autoapp/Configuration/Configuration.hpp>
#include <f1x/openauto/autoapp/UI/MainWindow.hpp>
#include <f1x/openauto/autoapp/UI/SettingsWindow.hpp>
#include <f1x/openauto/autoapp/UI/ConnectDialog.hpp>
#include <f1x/openauto/autoapp/UI/WarningDialog.hpp>
#include <f1x/openauto/autoapp/UI/UpdateDialog.hpp>
#include <modern/Logger.hpp>

namespace autoapp = f1x::openauto::autoapp;
using ThreadPool = std::vector<std::thread>;

void startUSBWorkers(boost::asio::io_service& ioService, libusb_context* usbContext, ThreadPool& threadPool)
{
    auto usbWorker = [&ioService, usbContext]() {
        timeval libusbEventTimeout{180, 0};

        while(!ioService.stopped())
        {
            libusb_handle_events_timeout_completed(usbContext, &libusbEventTimeout, nullptr);
        }
    };

    threadPool.emplace_back(usbWorker);
    threadPool.emplace_back(usbWorker);
    threadPool.emplace_back(usbWorker);
    threadPool.emplace_back(usbWorker);
}

void startIOServiceWorkers(boost::asio::io_service& ioService, ThreadPool& threadPool)
{
    auto ioServiceWorker = [&ioService]() {
        ioService.run();
    };

    threadPool.emplace_back(ioServiceWorker);
    threadPool.emplace_back(ioServiceWorker);
    threadPool.emplace_back(ioServiceWorker);
    threadPool.emplace_back(ioServiceWorker);
}

void configureLogging() {
    // Configure modern logger for autoapp
    auto& logger = openauto::modern::Logger::getInstance();
    logger.setLevel(openauto::modern::LogLevel::INFO);
    logger.setAsync(true);
    
    // Check for legacy log config file and warn about migration
    const std::string logIni = "openauto-logs.ini";
    std::ifstream logSettings(logIni);
    if (logSettings.good()) {
        SLOG_WARN(CONFIG, "autoapp", "Legacy log configuration file found - consider migrating to modern logger config");
    }
}

int main(int argc, char* argv[])
{
    configureLogging();

    libusb_context* usbContext;
    if(libusb_init(&usbContext) != 0)
    {
        SLOG_ERROR(SYSTEM, "autoapp", "libusb_init failed");
        return 1;
    }

    boost::asio::io_service ioService;
    boost::asio::io_service::work work(ioService);
    std::vector<std::thread> threadPool;
    startUSBWorkers(ioService, usbContext, threadPool);
    startIOServiceWorkers(ioService, threadPool);

    QApplication qApplication(argc, argv);
    int width = QApplication::desktop()->width();
    int height = QApplication::desktop()->height();

    for (QScreen *screen : qApplication.screens()) {
      std::map<std::string, std::string> context = {
          {"screen_name", screen->name().toStdString()},
          {"screen_width", std::to_string(screen->geometry().width())},
          {"physical_width", std::to_string(screen->physicalSize().width())}
      };
      SLOG_INFO(UI, "autoapp", "Screen detected: " + screen->name().toStdString());
    }

    QScreen *primaryScreen = QGuiApplication::primaryScreen();

    // Check if a primary screen was found
    if (primaryScreen) {
      // Get the geometry of the primary screen
      QRect screenGeometry = primaryScreen->geometry();
      width = screenGeometry.width();
      height = screenGeometry.height();
      SLOG_INFO(UI, "autoapp", "Using geometry from primary screen");
    } else {
      SLOG_INFO(UI, "autoapp", "Unable to find primary screen, using default values");
    }

    std::map<std::string, std::string> context = {
        {"display_width", std::to_string(width)},
        {"display_height", std::to_string(height)}
    };
    SLOG_INFO(UI, "autoapp", "Display configuration: " + std::to_string(width) + "x" + std::to_string(height));

    auto configuration = std::make_shared<autoapp::configuration::Configuration>();

    autoapp::ui::MainWindow mainWindow(configuration);
    //mainWindow.setWindowFlags(Qt::WindowStaysOnTopHint);

    autoapp::ui::SettingsWindow settingsWindow(configuration);
    //settingsWindow.setWindowFlags(Qt::WindowStaysOnTopHint);

    settingsWindow.setFixedSize(width, height);
    settingsWindow.adjustSize();

    autoapp::configuration::RecentAddressesList recentAddressesList(7);
    recentAddressesList.read();

    aasdk::tcp::TCPWrapper tcpWrapper;
    autoapp::ui::ConnectDialog connectdialog(ioService, tcpWrapper, recentAddressesList);
    //connectdialog.setWindowFlags(Qt::WindowStaysOnTopHint);
    connectdialog.move((width - 500)/2,(height-300)/2);

    autoapp::ui::WarningDialog warningdialog;
    //warningdialog.setWindowFlags(Qt::WindowStaysOnTopHint);
    warningdialog.move((width - 500)/2,(height-300)/2);

    autoapp::ui::UpdateDialog updatedialog;
    //updatedialog.setWindowFlags(Qt::WindowStaysOnTopHint);
    updatedialog.setFixedSize(500, 260);
    updatedialog.move((width - 500)/2,(height-260)/2);

    QObject::connect(&mainWindow, &autoapp::ui::MainWindow::exit, []() { system("touch /tmp/shutdown"); std::exit(0); });
    QObject::connect(&mainWindow, &autoapp::ui::MainWindow::reboot, []() { system("touch /tmp/reboot"); std::exit(0); });
    QObject::connect(&mainWindow, &autoapp::ui::MainWindow::openSettings, &settingsWindow, &autoapp::ui::SettingsWindow::showFullScreen);
    QObject::connect(&mainWindow, &autoapp::ui::MainWindow::openSettings, &settingsWindow, &autoapp::ui::SettingsWindow::show_tab1);
    QObject::connect(&mainWindow, &autoapp::ui::MainWindow::openSettings, &settingsWindow, &autoapp::ui::SettingsWindow::loadSystemValues);
    QObject::connect(&mainWindow, &autoapp::ui::MainWindow::openConnectDialog, &connectdialog, &autoapp::ui::ConnectDialog::loadClientList);
    QObject::connect(&mainWindow, &autoapp::ui::MainWindow::openConnectDialog, &connectdialog, &autoapp::ui::ConnectDialog::exec);
    QObject::connect(&mainWindow, &autoapp::ui::MainWindow::openUpdateDialog, &updatedialog, &autoapp::ui::UpdateDialog::updateCheck);
    QObject::connect(&mainWindow, &autoapp::ui::MainWindow::openUpdateDialog, &updatedialog, &autoapp::ui::UpdateDialog::exec);

    if (configuration->showCursor() == false) {
        qApplication.setOverrideCursor(Qt::BlankCursor);
    } else {
        qApplication.setOverrideCursor(Qt::ArrowCursor);
    }

    QObject::connect(&mainWindow, &autoapp::ui::MainWindow::cameraHide, [&qApplication]() {
        system("/opt/crankshaft/cameracontrol.py Background &");
        SLOG_DEBUG(CAMERA, "autoapp", "Camera background mode activated");
    });

    QObject::connect(&mainWindow, &autoapp::ui::MainWindow::cameraShow, [&qApplication]() {
        system("/opt/crankshaft/cameracontrol.py Foreground &");
        SLOG_DEBUG(CAMERA, "autoapp", "Camera foreground mode activated");
    });

    QObject::connect(&mainWindow, &autoapp::ui::MainWindow::cameraPosYUp, [&qApplication]() {
        system("/opt/crankshaft/cameracontrol.py PosYUp &");
        SLOG_DEBUG(CAMERA, "autoapp", "Camera position Y up");
    });

    QObject::connect(&mainWindow, &autoapp::ui::MainWindow::cameraPosYDown, [&qApplication]() {
        system("/opt/crankshaft/cameracontrol.py PosYDown &");
        SLOG_DEBUG(CAMERA, "autoapp", "Camera position Y down");
    });

    QObject::connect(&mainWindow, &autoapp::ui::MainWindow::cameraZoomPlus, [&qApplication]() {
        system("/opt/crankshaft/cameracontrol.py ZoomPlus &");
        SLOG_DEBUG(CAMERA, "autoapp", "Camera zoom plus");
    });

    QObject::connect(&mainWindow, &autoapp::ui::MainWindow::cameraZoomMinus, [&qApplication]() {
        system("/opt/crankshaft/cameracontrol.py ZoomMinus &");
        SLOG_DEBUG(CAMERA, "autoapp", "Camera zoom minus");
    });

    QObject::connect(&mainWindow, &autoapp::ui::MainWindow::cameraRecord, [&qApplication]() {
        system("/opt/crankshaft/cameracontrol.py Record &");
        SLOG_DEBUG(CAMERA, "autoapp", "Camera recording started");
    });

    QObject::connect(&mainWindow, &autoapp::ui::MainWindow::cameraStop, [&qApplication]() {
        system("/opt/crankshaft/cameracontrol.py Stop &");
        SLOG_DEBUG(CAMERA, "autoapp", "Camera recording stopped");
    });

    QObject::connect(&mainWindow, &autoapp::ui::MainWindow::cameraSave, [&qApplication]() {
        system("/opt/crankshaft/cameracontrol.py Save &");
        SLOG_DEBUG(CAMERA, "autoapp", "Camera save triggered");
    });

    QObject::connect(&mainWindow, &autoapp::ui::MainWindow::TriggerScriptNight, [&qApplication]() {
        system("/opt/crankshaft/service_daynight.sh app night");
        SLOG_DEBUG(UI, "autoapp", "Night mode activated");
    });

    QObject::connect(&mainWindow, &autoapp::ui::MainWindow::TriggerScriptDay, [&qApplication]() {
        system("/opt/crankshaft/service_daynight.sh app day");
        SLOG_DEBUG(UI, "autoapp", "Day mode activated");
    });

    mainWindow.showFullScreen();
    mainWindow.setFixedSize(width, height);
    mainWindow.adjustSize();

    aasdk::usb::USBWrapper usbWrapper(usbContext);
    aasdk::usb::AccessoryModeQueryFactory queryFactory(usbWrapper, ioService);
    aasdk::usb::AccessoryModeQueryChainFactory queryChainFactory(usbWrapper, ioService, queryFactory);
    autoapp::service::ServiceFactory serviceFactory(ioService, configuration);
    autoapp::service::AndroidAutoEntityFactory androidAutoEntityFactory(ioService, configuration, serviceFactory);

    auto usbHub(std::make_shared<aasdk::usb::USBHub>(usbWrapper, ioService, queryChainFactory));
    auto connectedAccessoriesEnumerator(std::make_shared<aasdk::usb::ConnectedAccessoriesEnumerator>(usbWrapper, ioService, queryChainFactory));
    auto app = std::make_shared<autoapp::App>(ioService, usbWrapper, tcpWrapper, androidAutoEntityFactory, std::move(usbHub), std::move(connectedAccessoriesEnumerator));

    QObject::connect(&connectdialog, &autoapp::ui::ConnectDialog::connectionSucceed, [&app](auto socket) {
        app->start(std::move(socket));
    });

    QObject::connect(&mainWindow, &autoapp::ui::MainWindow::TriggerAppStart, [&app]() {
        SLOG_DEBUG(ANDROID_AUTO, "autoapp", "Manual Android Auto start triggered");
        try {
            app->disableAutostartEntity = false;
            app->resume();
            app->waitForUSBDevice();
        } catch (...) {
            SLOG_ERROR(GENERAL, "autoapp", "[AutoApp] TriggerAppStart: app->waitForUSBDevice()");
        }
    });

    QObject::connect(&mainWindow, &autoapp::ui::MainWindow::TriggerAppStop, [&app]() {
        try {
            if (std::ifstream("/tmp/android_device")) {
                SLOG_DEBUG(GENERAL, "autoapp", "[AutoApp] TriggerAppStop: Manual stop usb android auto.");
                app->disableAutostartEntity = true;
                system("/usr/local/bin/autoapp_helper usbreset");
                usleep(500000);
                try {
                    app->stop();
                    //app->pause();
                } catch (...) {
                    SLOG_ERROR(GENERAL, "autoapp", "[AutoApp] TriggerAppStop: stop()");
                }

            } else {
                SLOG_DEBUG(NETWORK, "autoapp", "[AutoApp] TriggerAppStop: Manual stop wifi android auto.");
                try {
                    app->onAndroidAutoQuit();
                    //app->pause();
                } catch (...) {
                    SLOG_ERROR(GENERAL, "autoapp", "[Autoapp] TriggerAppStop: stop()");
                }

            }
        } catch (...) {
            SLOG_ERROR(GENERAL, "autoapp", "[AutoApp] Exception in manual stop android auto.");
        }
    });

    QObject::connect(&mainWindow, &autoapp::ui::MainWindow::CloseAllDialogs, [&settingsWindow, &connectdialog, &updatedialog, &warningdialog]() {
        settingsWindow.close();
        connectdialog.close();
        warningdialog.close();
        updatedialog.close();
        SLOG_DEBUG(GENERAL, "autoapp", "[AutoApp] Close all possible open dialogs.");
    });

    if (configuration->hideWarning() == false) {
        warningdialog.show();
    }

    app->waitForUSBDevice();

    auto result = qApplication.exec();

    std::for_each(threadPool.begin(), threadPool.end(), std::bind(&std::thread::join, std::placeholders::_1));

    libusb_exit(usbContext);
    return result;
}
