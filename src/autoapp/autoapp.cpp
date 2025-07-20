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
#include <fstream>
#include <iomanip>
#include <ctime>
#include <cstdlib>  // For std::getenv
#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>
#include <QPixmap>
#include <QString>
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
#include <modern/ModernIntegration.hpp>
#include <modern/ConfigurationManager.hpp>
#include <modern/EventBus.hpp>
#include <modern/StateMachine.hpp>
#include <modern/RestApiServer.hpp>

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
    // Check for debug mode from environment or command line
    bool debugMode = false;
    const char* envDebug = std::getenv("OPENAUTO_DEBUG_MODE");
    const char* envLogLevel = std::getenv("OPENAUTO_LOG_LEVEL");
    
    if (envDebug && std::string(envDebug) == "1") {
        debugMode = true;
    }
    
    if (envLogLevel && std::string(envLogLevel) == "DEBUG") {
        debugMode = true;
    }
    
    // Configure modern logger for autoapp
    auto& logger = openauto::modern::Logger::getInstance();
    
    // Set log level based on debug mode
    if (debugMode) {
        logger.setLevel(openauto::modern::LogLevel::DEBUG);
        // Enable debug for all categories
        logger.setCategoryLevel(openauto::modern::LogCategory::ANDROID_AUTO, openauto::modern::LogLevel::DEBUG);
        logger.setCategoryLevel(openauto::modern::LogCategory::SYSTEM, openauto::modern::LogLevel::DEBUG);
        logger.setCategoryLevel(openauto::modern::LogCategory::UI, openauto::modern::LogLevel::DEBUG);
        logger.setCategoryLevel(openauto::modern::LogCategory::CAMERA, openauto::modern::LogLevel::DEBUG);
        logger.setCategoryLevel(openauto::modern::LogCategory::NETWORK, openauto::modern::LogLevel::DEBUG);
        logger.setCategoryLevel(openauto::modern::LogCategory::BLUETOOTH, openauto::modern::LogLevel::DEBUG);
        logger.setCategoryLevel(openauto::modern::LogCategory::AUDIO, openauto::modern::LogLevel::DEBUG);
        logger.setCategoryLevel(openauto::modern::LogCategory::VIDEO, openauto::modern::LogLevel::DEBUG);
        logger.setCategoryLevel(openauto::modern::LogCategory::CONFIG, openauto::modern::LogLevel::DEBUG);
        logger.setCategoryLevel(openauto::modern::LogCategory::API, openauto::modern::LogLevel::DEBUG);
        logger.setCategoryLevel(openauto::modern::LogCategory::EVENT, openauto::modern::LogLevel::DEBUG);
        logger.setCategoryLevel(openauto::modern::LogCategory::STATE, openauto::modern::LogLevel::DEBUG);
    } else {
        logger.setLevel(openauto::modern::LogLevel::INFO);
    }
    
    // Add file sink for logging
    const std::string logFile = "/var/log/openauto/openauto.log";
    try {
        // Create file sink with rotation
        auto fileSink = std::make_shared<openauto::modern::FileSink>(logFile);
        logger.addSink(fileSink);
        
        // Add console sink for immediate feedback
        auto consoleSink = std::make_shared<openauto::modern::ConsoleSink>();
        logger.addSink(consoleSink);
        
        // Use synchronous logging to ensure immediate output
        logger.setAsync(false);
        
        // Test file logging
        std::ofstream testFile(logFile, std::ios::app);
        if (testFile.is_open()) {
            std::time_t now = std::time(nullptr);
            testFile << "[" << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S") 
                    << "] [INFO] [SYSTEM] [autoapp] Logger initialized with file output\n";
            testFile.close();
        }
        
    } catch (const std::exception& e) {
        // Fall back to console-only logging
        auto consoleSink = std::make_shared<openauto::modern::ConsoleSink>();
        logger.addSink(consoleSink);
        logger.setAsync(false);
    }
    
    // Test that logging is working
    if (debugMode) {
        SLOG_DEBUG(SYSTEM, "autoapp", "🔍 DEBUG MODE ENABLED - Verbose logging active");
        SLOG_DEBUG(SYSTEM, "autoapp", "   📊 Log level: DEBUG (all categories)");
        SLOG_DEBUG(SYSTEM, "autoapp", "   🔗 AASDK debug: Enabled via build configuration");
        SLOG_DEBUG(SYSTEM, "autoapp", "   📺 Output: Console + " + logFile);
    } else {
        SLOG_INFO(SYSTEM, "autoapp", "🚀 Modern logging system initialized");
        SLOG_INFO(SYSTEM, "autoapp", "   📊 Log level: INFO");
        SLOG_INFO(SYSTEM, "autoapp", "   📺 Output: Console + " + logFile);
    }
    
    // Check for legacy log config file and warn about migration
    const std::string logIni = "openauto-logs.ini";
    std::ifstream logSettings(logIni);
    if (logSettings.good()) {
        SLOG_WARN(CONFIG, "autoapp", "Legacy log configuration file found - consider migrating to modern logger config");
    }
}

void validatePngAssets()
{
    SLOG_INFO(UI, "autoapp", "🔧 Validating PNG assets for runtime warnings...");
    
    // List of PNG files that are embedded in Qt resources
    const std::vector<std::string> pngResources = {
        ":/ico_warning.png", ":/ico_info.png", ":/aausb-hot.png", ":/aawifi-hot.png",
        ":/cursor-hot.png", ":/power-hot.png", ":/settings-hot.png", ":/sleep-hot.png",
        ":/wifi-hot.png", ":/brightness-hot.png", ":/camera-hot.png", ":/day-hot.png",
        ":/night-hot.png", ":/record-hot.png", ":/stop-hot.png", ":/save-hot.png",
        ":/reboot-hot.png", ":/back-hot.png", ":/rearcam-hot.png", ":/recordactive-hot.png",
        ":/lock-hot.png", ":/volume-hot.png", ":/bug-hot.png", ":/eye-hot.png",
        ":/skin-hot.png", ":/mp3-hot.png", ":/play-hot.png", ":/prev-hot.png",
        ":/next-hot.png", ":/pause-hot.png", ":/prevbig-hot.png", ":/nextbig-hot.png",
        ":/list-hot.png", ":/home-hot.png", ":/player-hot.png", ":/coverlogo.png",
        ":/black.png", ":/album-hot.png"
    };
    
    int validatedCount = 0;
    int problematicCount = 0;
    
    for (const auto& resourcePath : pngResources) {
        QPixmap pixmap(QString::fromStdString(resourcePath));
        if (!pixmap.isNull()) {
            validatedCount++;
            SLOG_DEBUG(UI, "autoapp", "✅ PNG asset validated: " + resourcePath);
        } else {
            problematicCount++;
            SLOG_WARN(UI, "autoapp", "⚠️  PNG asset failed to load: " + resourcePath);
            SLOG_WARN(UI, "autoapp", "   📄 This may indicate a corrupted or missing PNG file");
            SLOG_WARN(UI, "autoapp", "   🔧 Consider running: cmake -B build && make -C build");
        }
    }
    
    std::map<std::string, std::string> context = {
        {"validated_pngs", std::to_string(validatedCount)},
        {"problematic_pngs", std::to_string(problematicCount)},
        {"total_pngs", std::to_string(pngResources.size())}
    };
    
    if (problematicCount == 0) {
        SLOG_INFO(UI, "autoapp", "✅ All PNG assets validated successfully (" + std::to_string(validatedCount) + " files)");
        SLOG_INFO(UI, "autoapp", "   📝 No libpng warnings expected from embedded resources");
    } else {
        SLOG_ERROR(UI, "autoapp", "🚨 PNG Asset Validation Failed!");
        SLOG_ERROR(UI, "autoapp", "   📊 " + std::to_string(problematicCount) + " out of " + std::to_string(pngResources.size()) + " PNG assets have issues");
        SLOG_ERROR(UI, "autoapp", "   ⚠️  This may cause 'libpng warning' messages at runtime");
        SLOG_ERROR(UI, "autoapp", "   🔧 Rebuild the project to fix: cmake -B build && make -C build");
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

    // Runtime PNG validation
    validatePngAssets();

    auto configuration = std::make_shared<autoapp::configuration::Configuration>();

    // Initialize modern architecture components
    openauto::modern::EventBus* eventBus = nullptr;
    std::shared_ptr<openauto::modern::ConfigurationManager> configManager;
    std::shared_ptr<openauto::modern::StateMachine> stateMachine;
    std::unique_ptr<openauto::modern::RestApiServer> restApiServer;
    
    try {
        // Get EventBus instance (singleton reference)
        eventBus = &openauto::modern::EventBus::getInstance();
        SLOG_INFO(SYSTEM, "autoapp", "EventBus initialized");
        
        // Create ConfigurationManager
        configManager = std::make_shared<openauto::modern::ConfigurationManager>();
        SLOG_INFO(SYSTEM, "autoapp", "ConfigurationManager initialized");
        
        // Create StateMachine
        stateMachine = std::make_shared<openauto::modern::StateMachine>();
        SLOG_INFO(SYSTEM, "autoapp", "StateMachine initialized");
        
        // Create and start REST API Server (if enabled)
        bool enableRestApi = configManager->getValue<bool>("modern_api.enable_rest_api", true);
        if (enableRestApi) {
            int apiPort = configManager->getValue<int>("modern_api.rest_api_port", 8080);
            
            // Convert EventBus* to shared_ptr for RestApiServer
            std::shared_ptr<openauto::modern::EventBus> eventBusPtr(
                eventBus, [](openauto::modern::EventBus*){} // No-op deleter for singleton
            );
            
            restApiServer = std::make_unique<openauto::modern::RestApiServer>(
                apiPort, eventBusPtr, stateMachine, configManager);
            
            // Start the REST API server in a separate thread
            std::thread apiThread([&restApiServer]() {
                try {
                    restApiServer->start();
                } catch (const std::exception& e) {
                    SLOG_ERROR(API, "autoapp", "REST API server error: " + std::string(e.what()));
                }
            });
            apiThread.detach();
            
            SLOG_INFO(API, "autoapp", "REST API server started on port " + std::to_string(apiPort));
        }
        
        // Set initial system state using transition
        stateMachine->transition(openauto::modern::Trigger::SYSTEM_START);
        SLOG_INFO(STATE, "autoapp", "System state transition to IDLE");
        
    } catch (const std::exception& e) {
        SLOG_ERROR(SYSTEM, "autoapp", "Failed to initialize modern components: " + std::string(e.what()));
    }

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

    // Shutdown modern components gracefully
    try {
        if (restApiServer) {
            restApiServer->stop();
            SLOG_INFO(API, "autoapp", "REST API server stopped");
        }
        
        if (stateMachine) {
            stateMachine->transition(openauto::modern::Trigger::SHUTDOWN_REQUEST);
            SLOG_INFO(STATE, "autoapp", "System state transitioned to SHUTTING_DOWN");
        }
        
        SLOG_INFO(SYSTEM, "autoapp", "Modern components shutdown complete");
    } catch (const std::exception& e) {
        SLOG_ERROR(SYSTEM, "autoapp", "Error during modern components shutdown: " + std::string(e.what()));
    }

    std::for_each(threadPool.begin(), threadPool.end(), std::bind(&std::thread::join, std::placeholders::_1));

    libusb_exit(usbContext);
    return result;
}
