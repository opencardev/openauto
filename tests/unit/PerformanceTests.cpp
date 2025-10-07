#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <future>
#include <algorithm>
#include <numeric>
#include <QtCore/QString>
#include <QtCore/QRect>

// Include comprehensive interfaces for performance testing
#include <f1x/openauto/autoapp/Projection/IVideoOutput.hpp>
#include <f1x/openauto/autoapp/Projection/IAudioOutput.hpp>
#include <f1x/openauto/autoapp/Projection/IAudioInput.hpp>
#include <f1x/openauto/autoapp/Service/IAndroidAutoEntity.hpp>

// Include all existing mocks for performance testing
#include "../mocks/MockVideoOutput.hpp"
#include "../mocks/MockAudioOutput.hpp"
#include "../mocks/MockAudioInput.hpp"
#include "../mocks/MockInputDevice.hpp"
#include "../mocks/MockInputDeviceEventHandler.hpp"
#include "../mocks/MockConfiguration.hpp"
#include "../mocks/MockAndroidAutoEntity.hpp"
#include "../mocks/MockAndroidAutoEntityFactory.hpp"
#include "../mocks/MockAndroidAutoEntityEventHandler.hpp"

using ::testing::_;
using ::testing::Return;
using ::testing::StrictMock;
using ::testing::NiceMock;
using ::testing::InSequence;
using ::testing::AtLeast;
using ::testing::AtMost;
using ::testing::Exactly;
using ::testing::Invoke;
using ::testing::SaveArg;
using ::testing::DoAll;
using ::testing::WithArgs;

// Phase 3: Performance and Stress Testing Suite

namespace f1x::openauto::autoapp::performance {

// Performance metrics and benchmarking fixture
class PerformanceTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // Create mock components optimized for performance testing
        mockVideoOutput = std::make_shared<NiceMock<projection::MockVideoOutput>>();
        mockAudioOutput = std::make_shared<NiceMock<projection::MockAudioOutput>>();
        mockAudioInput = std::make_shared<NiceMock<projection::MockAudioInput>>();
        mockInputDevice = std::make_shared<NiceMock<projection::MockInputDevice>>();
        mockInputHandler = std::make_shared<NiceMock<projection::MockInputDeviceEventHandler>>();
        
        mockAndroidAutoEntity = std::make_shared<NiceMock<service::MockAndroidAutoEntity>>();
        mockEntityFactory = std::make_shared<NiceMock<service::MockAndroidAutoEntityFactory>>();
        mockEntityEventHandler = std::make_shared<NiceMock<service::MockAndroidAutoEntityEventHandler>>();
        mockConfiguration = std::make_shared<NiceMock<f1x::openauto::autoapp::configuration::MockConfiguration>>();
        
        setupPerformanceDefaults();
        resetPerformanceCounters();
    }

    void setupPerformanceDefaults() {
        // Ultra-high performance video settings
        ON_CALL(*mockVideoOutput, open()).WillByDefault(Return(true));
        ON_CALL(*mockVideoOutput, init()).WillByDefault(Return(true));
        ON_CALL(*mockVideoOutput, getVideoFPS())
            .WillByDefault(Return(aap_protobuf::service::media::sink::message::VIDEO_FPS_60));
        ON_CALL(*mockVideoOutput, getVideoResolution())
            .WillByDefault(Return(aap_protobuf::service::media::sink::message::VIDEO_1920x1080));
        ON_CALL(*mockVideoOutput, getScreenDPI()).WillByDefault(Return(480));  // Ultra high DPI
        ON_CALL(*mockVideoOutput, getVideoMargins()).WillByDefault(Return(QRect(0, 0, 1920, 1080)));
        
        // High-fidelity audio settings
        ON_CALL(*mockAudioOutput, open()).WillByDefault(Return(true));
        ON_CALL(*mockAudioOutput, getSampleRate()).WillByDefault(Return(192000));  // Ultra high quality
        ON_CALL(*mockAudioOutput, getChannelCount()).WillByDefault(Return(8));     // 7.1 surround
        ON_CALL(*mockAudioOutput, getSampleSize()).WillByDefault(Return(32));      // 32-bit float
        
        // Professional audio input
        ON_CALL(*mockAudioInput, open()).WillByDefault(Return(true));
        ON_CALL(*mockAudioInput, isActive()).WillByDefault(Return(true));
        ON_CALL(*mockAudioInput, getSampleRate()).WillByDefault(Return(96000));   // Professional quality
        ON_CALL(*mockAudioInput, getChannelCount()).WillByDefault(Return(4));     // Quad input
        ON_CALL(*mockAudioInput, getSampleSize()).WillByDefault(Return(24));      // 24-bit
        
        // High-performance service factory
        ON_CALL(*mockEntityFactory, create(::testing::Matcher<aasdk::usb::IAOAPDevice::Pointer>(::testing::_)))
            .WillByDefault(Return(mockAndroidAutoEntity));
        
        // Performance configuration
        ON_CALL(*mockConfiguration, getVideoFPS())
            .WillByDefault(Return(aap_protobuf::service::media::sink::message::VIDEO_FPS_60));
        ON_CALL(*mockConfiguration, getVideoResolution())
            .WillByDefault(Return(aap_protobuf::service::media::sink::message::VIDEO_1920x1080));
        ON_CALL(*mockConfiguration, getScreenDPI()).WillByDefault(Return(480));
        ON_CALL(*mockConfiguration, getVideoMargins()).WillByDefault(Return(QRect(0, 0, 1920, 1080)));
    }

    void resetPerformanceCounters() {
        operationCount = 0;
        totalLatency = 0;
        maxLatency = 0;
        minLatency = std::numeric_limits<uint64_t>::max();
        operationLatencies.clear();
    }

    uint64_t measureOperationLatency(std::function<void()> operation) {
        auto start = std::chrono::high_resolution_clock::now();
        operation();
        auto end = std::chrono::high_resolution_clock::now();
        
        uint64_t latency = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        
        // Update performance statistics
        operationCount++;
        totalLatency += latency;
        
        uint64_t currentMax = maxLatency.load();
        while (latency > currentMax && !maxLatency.compare_exchange_weak(currentMax, latency)) {
            // Retry if another thread updated maxLatency
        }
        
        uint64_t currentMin = minLatency.load();
        while (latency < currentMin && !minLatency.compare_exchange_weak(currentMin, latency)) {
            // Retry if another thread updated minLatency
        }
        
        operationLatencies.push_back(latency);
        
        return latency;
    }

    double getAverageLatency() const {
        return operationCount > 0 ? static_cast<double>(totalLatency) / operationCount : 0.0;
    }

    double getLatencyStandardDeviation() const {
        if (operationCount < 2) return 0.0;
        
        double mean = getAverageLatency();
        double variance = 0.0;
        
        for (uint64_t latency : operationLatencies) {
            double diff = static_cast<double>(latency) - mean;
            variance += diff * diff;
        }
        
        variance /= (operationCount - 1);
        return std::sqrt(variance);
    }

    uint64_t getPercentileLatency(double percentile) const {
        if (operationLatencies.empty()) return 0;
        
        std::vector<uint64_t> sortedLatencies = operationLatencies;
        std::sort(sortedLatencies.begin(), sortedLatencies.end());
        
        size_t index = static_cast<size_t>(std::ceil(percentile * sortedLatencies.size() / 100.0)) - 1;
        index = std::min(index, sortedLatencies.size() - 1);
        
        return sortedLatencies[index];
    }

    void TearDown() override {
        mockVideoOutput.reset();
        mockAudioOutput.reset();
        mockAudioInput.reset();
        mockInputDevice.reset();
        mockInputHandler.reset();
        mockAndroidAutoEntity.reset();
        mockEntityFactory.reset();
        mockEntityEventHandler.reset();
        mockConfiguration.reset();
    }

    std::shared_ptr<projection::MockVideoOutput> mockVideoOutput;
    std::shared_ptr<projection::MockAudioOutput> mockAudioOutput;
    std::shared_ptr<projection::MockAudioInput> mockAudioInput;
    std::shared_ptr<projection::MockInputDevice> mockInputDevice;
    std::shared_ptr<projection::MockInputDeviceEventHandler> mockInputHandler;
    std::shared_ptr<service::MockAndroidAutoEntity> mockAndroidAutoEntity;
    std::shared_ptr<service::MockAndroidAutoEntityFactory> mockEntityFactory;
    std::shared_ptr<service::MockAndroidAutoEntityEventHandler> mockEntityEventHandler;
    std::shared_ptr<f1x::openauto::autoapp::configuration::MockConfiguration> mockConfiguration;

    // Performance metrics
    std::atomic<uint32_t> operationCount{0};
    std::atomic<uint64_t> totalLatency{0};
    std::atomic<uint64_t> maxLatency{0};
    std::atomic<uint64_t> minLatency{UINT64_MAX};
    std::vector<uint64_t> operationLatencies;
};

// Test video output performance under high load
TEST_F(PerformanceTestFixture, VideoOutputPerformanceBenchmark) {
    const int iterations = 1000;
    
    EXPECT_CALL(*mockVideoOutput, getVideoFPS()).Times(iterations);
    EXPECT_CALL(*mockVideoOutput, getVideoResolution()).Times(iterations);
    EXPECT_CALL(*mockVideoOutput, getScreenDPI()).Times(iterations);
    
    // Benchmark video parameter queries
    for (int i = 0; i < iterations; ++i) {
        measureOperationLatency([this]() {
            auto fps = mockVideoOutput->getVideoFPS();
            auto resolution = mockVideoOutput->getVideoResolution();
            auto dpi = mockVideoOutput->getScreenDPI();
            
            EXPECT_EQ(fps, aap_protobuf::service::media::sink::message::VIDEO_FPS_60);
            EXPECT_EQ(resolution, aap_protobuf::service::media::sink::message::VIDEO_1920x1080);
            EXPECT_EQ(dpi, 480);
        });
    }
    
    // Performance assertions
    EXPECT_EQ(operationCount, iterations);
    EXPECT_GT(getAverageLatency(), 0);
    EXPECT_LT(getAverageLatency(), 100000);  // Less than 100 microseconds average
    
    // Performance statistics
    auto p95Latency = getPercentileLatency(95.0);
    auto p99Latency = getPercentileLatency(99.0);
    
    EXPECT_LT(p95Latency, 500000);  // 95th percentile under 500 microseconds
    EXPECT_LT(p99Latency, 1000000); // 99th percentile under 1 millisecond
    
    // Performance logging (would be visible in test output)
    std::cout << "Video Performance Metrics:\n";
    std::cout << "  Operations: " << operationCount << "\n";
    std::cout << "  Average Latency: " << getAverageLatency() << " ns\n";
    std::cout << "  Standard Deviation: " << getLatencyStandardDeviation() << " ns\n";
    std::cout << "  95th Percentile: " << p95Latency << " ns\n";
    std::cout << "  99th Percentile: " << p99Latency << " ns\n";
}

// Test audio processing performance under high throughput
TEST_F(PerformanceTestFixture, AudioProcessingPerformanceBenchmark) {
    const int iterations = 500;
    
    EXPECT_CALL(*mockAudioOutput, getSampleRate()).Times(iterations);
    EXPECT_CALL(*mockAudioOutput, getChannelCount()).Times(iterations);
    EXPECT_CALL(*mockAudioOutput, getSampleSize()).Times(iterations);
    EXPECT_CALL(*mockAudioInput, getSampleRate()).Times(iterations);
    EXPECT_CALL(*mockAudioInput, getChannelCount()).Times(iterations);
    EXPECT_CALL(*mockAudioInput, isActive()).Times(iterations);
    
    // Benchmark audio parameter queries and status checks
    for (int i = 0; i < iterations; ++i) {
        measureOperationLatency([this]() {
            auto outputRate = mockAudioOutput->getSampleRate();
            auto outputChannels = mockAudioOutput->getChannelCount();
            auto outputSampleSize = mockAudioOutput->getSampleSize();
            auto inputRate = mockAudioInput->getSampleRate();
            auto inputChannels = mockAudioInput->getChannelCount();
            auto isActive = mockAudioInput->isActive();
            
            EXPECT_EQ(outputRate, 192000);
            EXPECT_EQ(outputChannels, 8);
            EXPECT_EQ(outputSampleSize, 32);
            EXPECT_EQ(inputRate, 96000);
            EXPECT_EQ(inputChannels, 4);
            EXPECT_TRUE(isActive);
        });
    }
    
    // Performance assertions for audio processing
    EXPECT_EQ(operationCount, iterations);
    EXPECT_LT(getAverageLatency(), 50000);  // Less than 50 microseconds average (stricter for audio)
    
    auto p95Latency = getPercentileLatency(95.0);
    auto p99Latency = getPercentileLatency(99.0);
    
    EXPECT_LT(p95Latency, 200000);  // 95th percentile under 200 microseconds
    EXPECT_LT(p99Latency, 500000);  // 99th percentile under 500 microseconds
    
    std::cout << "Audio Performance Metrics:\n";
    std::cout << "  Operations: " << operationCount << "\n";
    std::cout << "  Average Latency: " << getAverageLatency() << " ns\n";
    std::cout << "  Standard Deviation: " << getLatencyStandardDeviation() << " ns\n";
    std::cout << "  95th Percentile: " << p95Latency << " ns\n";
    std::cout << "  99th Percentile: " << p99Latency << " ns\n";
}

// Test concurrent operation performance
TEST_F(PerformanceTestFixture, ConcurrentOperationPerformance) {
    const int threadCount = 4;
    const int operationsPerThread = 250;
    
    EXPECT_CALL(*mockVideoOutput, getVideoFPS()).Times(threadCount * operationsPerThread);
    EXPECT_CALL(*mockAudioOutput, getSampleRate()).Times(threadCount * operationsPerThread);
    EXPECT_CALL(*mockAudioInput, isActive()).Times(threadCount * operationsPerThread);
    
    std::vector<std::future<void>> futures;
    std::vector<std::vector<uint64_t>> threadLatencies(threadCount);
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Launch concurrent operations
    for (int t = 0; t < threadCount; ++t) {
        futures.emplace_back(std::async(std::launch::async, [this, t, operationsPerThread, &threadLatencies]() {
            for (int i = 0; i < operationsPerThread; ++i) {
                auto start = std::chrono::high_resolution_clock::now();
                
                auto fps = mockVideoOutput->getVideoFPS();
                auto sampleRate = mockAudioOutput->getSampleRate();
                auto isActive = mockAudioInput->isActive();
                
                auto end = std::chrono::high_resolution_clock::now();
                uint64_t latency = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                threadLatencies[t].push_back(latency);
                
                EXPECT_EQ(fps, aap_protobuf::service::media::sink::message::VIDEO_FPS_60);
                EXPECT_EQ(sampleRate, 192000);
                EXPECT_TRUE(isActive);
            }
        }));
    }
    
    // Wait for all threads to complete
    for (auto& future : futures) {
        future.wait();
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    
    // Aggregate results from all threads
    std::vector<uint64_t> allLatencies;
    for (const auto& threadLatency : threadLatencies) {
        allLatencies.insert(allLatencies.end(), threadLatency.begin(), threadLatency.end());
    }
    
    // Calculate performance metrics
    uint64_t totalOps = allLatencies.size();
    uint64_t sumLatency = std::accumulate(allLatencies.begin(), allLatencies.end(), 0ULL);
    double avgLatency = static_cast<double>(sumLatency) / totalOps;
    
    std::sort(allLatencies.begin(), allLatencies.end());
    uint64_t p95 = allLatencies[static_cast<size_t>(0.95 * allLatencies.size())];
    uint64_t p99 = allLatencies[static_cast<size_t>(0.99 * allLatencies.size())];
    
    // Performance assertions for concurrent operations
    EXPECT_EQ(totalOps, threadCount * operationsPerThread);
    EXPECT_LT(totalTime, 1000);  // Complete within 1 second
    EXPECT_LT(avgLatency, 100000);  // Average under 100 microseconds
    EXPECT_LT(p95, 500000);     // 95th percentile under 500 microseconds
    EXPECT_LT(p99, 1000000);    // 99th percentile under 1 millisecond
    
    std::cout << "Concurrent Performance Metrics:\n";
    std::cout << "  Threads: " << threadCount << "\n";
    std::cout << "  Total Operations: " << totalOps << "\n";
    std::cout << "  Total Time: " << totalTime << " ms\n";
    std::cout << "  Throughput: " << (totalOps * 1000.0 / totalTime) << " ops/sec\n";
    std::cout << "  Average Latency: " << avgLatency << " ns\n";
    std::cout << "  95th Percentile: " << p95 << " ns\n";
    std::cout << "  99th Percentile: " << p99 << " ns\n";
}

// Test memory allocation performance and cleanup
TEST_F(PerformanceTestFixture, MemoryManagementPerformance) {
    const int iterations = 100;
    
    EXPECT_CALL(*mockVideoOutput, open()).Times(iterations).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockVideoOutput, init()).Times(iterations).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockVideoOutput, stop()).Times(iterations);
    EXPECT_CALL(*mockAudioOutput, open()).Times(iterations).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockAudioOutput, start()).Times(iterations);
    EXPECT_CALL(*mockAudioOutput, stop()).Times(iterations);
    
    // Benchmark memory-intensive operations (create/destroy cycles)
    for (int i = 0; i < iterations; ++i) {
        measureOperationLatency([this]() {
            // Simulate resource allocation and cleanup cycle
            EXPECT_TRUE(mockVideoOutput->open());
            EXPECT_TRUE(mockVideoOutput->init());
            EXPECT_TRUE(mockAudioOutput->open());
            mockAudioOutput->start();
            
            // Immediate cleanup
            mockAudioOutput->stop();
            mockVideoOutput->stop();
        });
    }
    
    // Performance assertions for memory management
    EXPECT_EQ(operationCount, iterations);
    EXPECT_LT(getAverageLatency(), 1000000);  // Less than 1 millisecond average
    
    auto p95Latency = getPercentileLatency(95.0);
    auto p99Latency = getPercentileLatency(99.0);
    
    EXPECT_LT(p95Latency, 5000000);   // 95th percentile under 5 milliseconds
    EXPECT_LT(p99Latency, 10000000);  // 99th percentile under 10 milliseconds
    
    std::cout << "Memory Management Performance Metrics:\n";
    std::cout << "  Create/Destroy Cycles: " << operationCount << "\n";
    std::cout << "  Average Latency: " << getAverageLatency() << " ns\n";
    std::cout << "  Standard Deviation: " << getLatencyStandardDeviation() << " ns\n";
    std::cout << "  95th Percentile: " << p95Latency << " ns\n";
    std::cout << "  99th Percentile: " << p99Latency << " ns\n";
}

// Test service lifecycle performance under load
TEST_F(PerformanceTestFixture, ServiceLifecyclePerformance) {
    const int cycles = 50;
    
    EXPECT_CALL(*mockAndroidAutoEntity, start(_)).Times(cycles);
    EXPECT_CALL(*mockAndroidAutoEntity, pause()).Times(cycles);
    EXPECT_CALL(*mockAndroidAutoEntity, resume()).Times(cycles);
    EXPECT_CALL(*mockAndroidAutoEntity, stop()).Times(cycles);
    
    // Benchmark complete service lifecycle
    for (int i = 0; i < cycles; ++i) {
        measureOperationLatency([this]() {
            mockAndroidAutoEntity->start(*mockEntityEventHandler);
            mockAndroidAutoEntity->pause();
            mockAndroidAutoEntity->resume();
            mockAndroidAutoEntity->stop();
        });
    }
    
    // Performance assertions for service lifecycle
    EXPECT_EQ(operationCount, cycles);
    EXPECT_LT(getAverageLatency(), 500000);  // Less than 500 microseconds average
    
    auto p95Latency = getPercentileLatency(95.0);
    auto p99Latency = getPercentileLatency(99.0);
    
    EXPECT_LT(p95Latency, 2000000);  // 95th percentile under 2 milliseconds
    EXPECT_LT(p99Latency, 5000000);  // 99th percentile under 5 milliseconds
    
    std::cout << "Service Lifecycle Performance Metrics:\n";
    std::cout << "  Lifecycle Cycles: " << operationCount << "\n";
    std::cout << "  Average Latency: " << getAverageLatency() << " ns\n";
    std::cout << "  Standard Deviation: " << getLatencyStandardDeviation() << " ns\n";
    std::cout << "  95th Percentile: " << p95Latency << " ns\n";
    std::cout << "  99th Percentile: " << p99Latency << " ns\n";
}

// Test configuration adaptation performance
TEST_F(PerformanceTestFixture, ConfigurationAdaptationPerformance) {
    const int adaptations = 200;
    
    EXPECT_CALL(*mockConfiguration, getVideoFPS()).Times(adaptations);
    EXPECT_CALL(*mockConfiguration, getVideoResolution()).Times(adaptations);
    EXPECT_CALL(*mockConfiguration, getScreenDPI()).Times(adaptations);
    EXPECT_CALL(*mockConfiguration, getVideoMargins()).Times(adaptations);
    
    // Benchmark rapid configuration queries
    for (int i = 0; i < adaptations; ++i) {
        measureOperationLatency([this]() {
            auto fps = mockConfiguration->getVideoFPS();
            auto resolution = mockConfiguration->getVideoResolution();
            auto dpi = mockConfiguration->getScreenDPI();
            auto margins = mockConfiguration->getVideoMargins();
            
            EXPECT_EQ(fps, aap_protobuf::service::media::sink::message::VIDEO_FPS_60);
            EXPECT_EQ(resolution, aap_protobuf::service::media::sink::message::VIDEO_1920x1080);
            EXPECT_EQ(dpi, 480);
            EXPECT_EQ(margins, QRect(0, 0, 1920, 1080));
        });
    }
    
    // Performance assertions for configuration adaptation
    EXPECT_EQ(operationCount, adaptations);
    EXPECT_LT(getAverageLatency(), 20000);  // Less than 20 microseconds average (very fast)
    
    auto p95Latency = getPercentileLatency(95.0);
    auto p99Latency = getPercentileLatency(99.0);
    
    EXPECT_LT(p95Latency, 100000);  // 95th percentile under 100 microseconds
    EXPECT_LT(p99Latency, 200000);  // 99th percentile under 200 microseconds
    
    std::cout << "Configuration Adaptation Performance Metrics:\n";
    std::cout << "  Configuration Queries: " << operationCount << "\n";
    std::cout << "  Average Latency: " << getAverageLatency() << " ns\n";
    std::cout << "  Standard Deviation: " << getLatencyStandardDeviation() << " ns\n";
    std::cout << "  95th Percentile: " << p95Latency << " ns\n";
    std::cout << "  99th Percentile: " << p99Latency << " ns\n";
}

} // namespace f1x::openauto::autoapp::performance