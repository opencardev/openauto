#include <gtest/gtest.h>
#include <QApplication>
#include <memory>

std::unique_ptr<QApplication> app;

int main(int argc, char *argv[]) {
    // Initialize QApplication for tests that require Qt functionality
    app = std::make_unique<QApplication>(argc, argv);
    
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}