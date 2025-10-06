#include <gtest/gtest.h>
#include <QApplication>
#include <QThread>
#include <memory>
#include <boost/asio/io_service.hpp>

std::unique_ptr<QApplication> app;
boost::asio::io_service ioService;
std::unique_ptr<QThread> ioServiceThread;

// Custom test environment for setting up and tearing down global test resources
class IntegrationTestEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        // Start io_service in a separate thread for async operations
        ioServiceThread = std::make_unique<QThread>();
        QObject::connect(ioServiceThread.get(), &QThread::started, []() {
            boost::asio::io_service::work work(ioService);
            ioService.run();
        });
        ioServiceThread->start();
    }

    void TearDown() override {
        // Cleanup resources
        ioService.stop();
        if (ioServiceThread && ioServiceThread->isRunning()) {
            ioServiceThread->quit();
            ioServiceThread->wait(3000); // Wait 3 seconds max
            if (ioServiceThread->isRunning()) {
                ioServiceThread->terminate();
            }
        }
    }
};

int main(int argc, char *argv[]) {
    // Initialize QApplication for tests that require Qt functionality
    app = std::make_unique<QApplication>(argc, argv);
    
    ::testing::InitGoogleTest(&argc, argv);
    
    // Add the global test environment
    ::testing::AddGlobalTestEnvironment(new IntegrationTestEnvironment);
    
    return RUN_ALL_TESTS();
}