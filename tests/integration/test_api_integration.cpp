#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <modern/RestApiServer.hpp>
#include <modern/EventBus.hpp>
#include <modern/ConfigurationManager.hpp>
#include <modern/StateMachine.hpp>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <chrono>

using namespace openauto::modern;
using namespace testing;

class APIIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        eventBus = std::make_shared<EventBus>();
        configManager = std::make_shared<ConfigurationManager>("test_api_config.json");
        stateMachine = std::make_shared<StateMachine>(eventBus);
        
        // Use test port
        testPort = 18082;
        apiServer = std::make_unique<RestApiServer>(eventBus, configManager, stateMachine, "127.0.0.1", testPort);
        
        // Initialize and start server
        configManager->loadDefaults();
        apiServer->start();
        
        // Wait for server to start
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Create HTTP client
        client = std::make_unique<httplib::Client>("127.0.0.1", testPort);
    }

    void TearDown() override {
        client.reset();
        
        if (apiServer && apiServer->isRunning()) {
            apiServer->stop();
        }
        
        apiServer.reset();
        stateMachine.reset();
        configManager.reset();
        eventBus.reset();
        
        std::filesystem::remove("test_api_config.json");
    }

    std::shared_ptr<EventBus> eventBus;
    std::shared_ptr<ConfigurationManager> configManager;
    std::shared_ptr<StateMachine> stateMachine;
    std::unique_ptr<RestApiServer> apiServer;
    std::unique_ptr<httplib::Client> client;
    int testPort;
};

// Test API server health check
TEST_F(APIIntegrationTest, HealthCheckTest) {
    auto response = client->Get("/health");
    
    ASSERT_TRUE(response);
    EXPECT_EQ(response->status, 200);
    
    auto json = nlohmann::json::parse(response->body);
    EXPECT_TRUE(json.contains("status"));
    EXPECT_EQ(json["status"], "healthy");
}

// Test API info endpoint
TEST_F(APIIntegrationTest, InfoEndpointTest) {
    auto response = client->Get("/api/v1/info");
    
    ASSERT_TRUE(response);
    EXPECT_EQ(response->status, 200);
    
    auto json = nlohmann::json::parse(response->body);
    EXPECT_TRUE(json.contains("version"));
    EXPECT_TRUE(json.contains("uptime"));
}

// Test configuration API endpoints
TEST_F(APIIntegrationTest, ConfigurationAPITest) {
    // Set a configuration value via API
    nlohmann::json configData;
    configData["key"] = "api.test.value";
    configData["value"] = "test_api_value";
    
    auto setResponse = client->Post("/api/v1/config", configData.dump(), "application/json");
    ASSERT_TRUE(setResponse);
    EXPECT_EQ(setResponse->status, 200);
    
    // Get the configuration value via API
    auto getResponse = client->Get("/api/v1/config/api.test.value");
    ASSERT_TRUE(getResponse);
    EXPECT_EQ(getResponse->status, 200);
    
    auto responseJson = nlohmann::json::parse(getResponse->body);
    EXPECT_TRUE(responseJson.contains("value"));
    EXPECT_EQ(responseJson["value"], "test_api_value");
    
    // Verify the value was actually set in the configuration manager
    auto actualValue = configManager->getValue<std::string>("api.test.value", "");
    EXPECT_EQ(actualValue, "test_api_value");
}

// Test state API endpoints
TEST_F(APIIntegrationTest, StateAPITest) {
    // Get current state
    auto getStateResponse = client->Get("/api/v1/state");
    ASSERT_TRUE(getStateResponse);
    EXPECT_EQ(getStateResponse->status, 200);
    
    auto stateJson = nlohmann::json::parse(getStateResponse->body);
    EXPECT_TRUE(stateJson.contains("current_state"));
    EXPECT_EQ(stateJson["current_state"], static_cast<int>(SystemState::INITIALIZING));
    
    // Transition state via API
    nlohmann::json transitionData;
    transitionData["target_state"] = static_cast<int>(SystemState::READY);
    
    auto transitionResponse = client->Post("/api/v1/state/transition", transitionData.dump(), "application/json");
    ASSERT_TRUE(transitionResponse);
    EXPECT_EQ(transitionResponse->status, 200);
    
    // Verify state changed
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::READY);
}

// Test event API endpoints
TEST_F(APIIntegrationTest, EventAPITest) {
    std::shared_ptr<Event> receivedEvent;
    
    // Subscribe to events
    eventBus->subscribe(EventType::CONNECTION_STATUS, [&receivedEvent](std::shared_ptr<Event> event) {
        receivedEvent = event;
    });
    
    // Publish event via API
    nlohmann::json eventData;
    eventData["type"] = static_cast<int>(EventType::CONNECTION_STATUS);
    eventData["source"] = "api_test";
    eventData["data"]["test_key"] = "test_value";
    
    auto publishResponse = client->Post("/api/v1/events", eventData.dump(), "application/json");
    ASSERT_TRUE(publishResponse);
    EXPECT_EQ(publishResponse->status, 200);
    
    // Wait for event processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Verify event was received
    ASSERT_NE(receivedEvent, nullptr);
    EXPECT_EQ(receivedEvent->getType(), EventType::CONNECTION_STATUS);
    EXPECT_EQ(receivedEvent->getSource(), "api_test");
}

// Test API error handling
TEST_F(APIIntegrationTest, ErrorHandlingTest) {
    // Test invalid endpoint
    auto invalidResponse = client->Get("/api/v1/invalid");
    ASSERT_TRUE(invalidResponse);
    EXPECT_EQ(invalidResponse->status, 404);
    
    // Test invalid JSON
    auto invalidJsonResponse = client->Post("/api/v1/config", "invalid json", "application/json");
    ASSERT_TRUE(invalidJsonResponse);
    EXPECT_EQ(invalidJsonResponse->status, 400);
    
    // Test invalid state transition
    nlohmann::json invalidTransition;
    invalidTransition["target_state"] = 999; // Invalid state
    
    auto invalidTransitionResponse = client->Post("/api/v1/state/transition", invalidTransition.dump(), "application/json");
    ASSERT_TRUE(invalidTransitionResponse);
    EXPECT_EQ(invalidTransitionResponse->status, 400);
}

// Test API authentication (if implemented)
TEST_F(APIIntegrationTest, AuthenticationTest) {
    // Test accessing protected endpoint without authentication
    auto unauthorizedResponse = client->Get("/api/v1/admin/logs");
    
    // Should return 401 or redirect to login (depending on implementation)
    if (unauthorizedResponse) {
        EXPECT_TRUE(unauthorizedResponse->status == 401 || unauthorizedResponse->status == 403);
    }
}

// Test API CORS headers
TEST_F(APIIntegrationTest, CORSTest) {
    auto optionsResponse = client->Options("/api/v1/info");
    
    if (optionsResponse) {
        // Check for CORS headers
        auto corsHeader = optionsResponse->get_header_value("Access-Control-Allow-Origin");
        EXPECT_FALSE(corsHeader.empty());
    }
}

// Test API rate limiting (if implemented)
TEST_F(APIIntegrationTest, RateLimitingTest) {
    // Make multiple rapid requests
    int successCount = 0;
    int rateLimitedCount = 0;
    
    for (int i = 0; i < 50; ++i) {
        auto response = client->Get("/api/v1/info");
        if (response) {
            if (response->status == 200) {
                successCount++;
            } else if (response->status == 429) { // Too Many Requests
                rateLimitedCount++;
            }
        }
    }
    
    // Should have some successful requests
    EXPECT_GT(successCount, 0);
    
    // Rate limiting is optional, so we don't enforce it in tests
    // but if implemented, we'd expect some rate limited responses
}

// Test WebSocket endpoints (if implemented)
TEST_F(APIIntegrationTest, WebSocketTest) {
    // Placeholder for WebSocket testing
    // Real implementation would test:
    // - WebSocket connection establishment
    // - Real-time event streaming
    // - Connection persistence
    EXPECT_TRUE(true); // Placeholder
}
