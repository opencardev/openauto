#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <chrono>
#include <modern/EventBus.hpp>
#include <modern/StateMachine.hpp>
#include <thread>

using namespace openauto::modern;
using namespace testing;

class StateMachineTest : public ::testing::Test {
  protected:
    void SetUp() override {
        eventBus = std::make_shared<EventBus>();
        stateMachine = std::make_unique<StateMachine>(eventBus);
    }

    void TearDown() override {
        stateMachine.reset();
        eventBus.reset();
    }

    std::unique_ptr<StateMachine> stateMachine;
    std::shared_ptr<EventBus> eventBus;
};

// Test initial state
TEST_F(StateMachineTest, InitialStateTest) {
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::INITIALIZING);
    EXPECT_FALSE(stateMachine->isInState(SystemState::READY));
    EXPECT_TRUE(stateMachine->isInState(SystemState::INITIALIZING));
}

// Test valid state transitions
TEST_F(StateMachineTest, ValidStateTransitionsTest) {
    // INITIALIZING -> READY
    EXPECT_TRUE(stateMachine->transitionTo(SystemState::READY));
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::READY);

    // READY -> CONNECTED
    EXPECT_TRUE(stateMachine->transitionTo(SystemState::CONNECTED));
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::CONNECTED);

    // CONNECTED -> PROJECTING
    EXPECT_TRUE(stateMachine->transitionTo(SystemState::PROJECTING));
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::PROJECTING);

    // PROJECTING -> CONNECTED (back)
    EXPECT_TRUE(stateMachine->transitionTo(SystemState::CONNECTED));
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::CONNECTED);

    // CONNECTED -> READY (back)
    EXPECT_TRUE(stateMachine->transitionTo(SystemState::READY));
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::READY);
}

// Test error state transitions
TEST_F(StateMachineTest, ErrorStateTransitionsTest) {
    // From any state, should be able to go to ERROR
    EXPECT_TRUE(stateMachine->transitionTo(SystemState::ERROR));
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::ERROR);

    // From ERROR, should be able to go to READY (recovery)
    EXPECT_TRUE(stateMachine->transitionTo(SystemState::READY));
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::READY);
}

// Test shutdown state transitions
TEST_F(StateMachineTest, ShutdownStateTransitionsTest) {
    // Transition to READY first
    stateMachine->transitionTo(SystemState::READY);

    // From any state, should be able to go to SHUTDOWN
    EXPECT_TRUE(stateMachine->transitionTo(SystemState::SHUTDOWN));
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::SHUTDOWN);
}

// Test invalid state transitions
TEST_F(StateMachineTest, InvalidStateTransitionsTest) {
    // Cannot go directly from INITIALIZING to CONNECTED
    EXPECT_FALSE(stateMachine->transitionTo(SystemState::CONNECTED));
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::INITIALIZING);

    // Cannot go directly from INITIALIZING to PROJECTING
    EXPECT_FALSE(stateMachine->transitionTo(SystemState::PROJECTING));
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::INITIALIZING);

    // Transition to READY
    stateMachine->transitionTo(SystemState::READY);

    // Cannot go directly from READY to PROJECTING
    EXPECT_FALSE(stateMachine->transitionTo(SystemState::PROJECTING));
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::READY);
}

// Test state change events
TEST_F(StateMachineTest, StateChangeEventsTest) {
    std::shared_ptr<Event> lastEvent;

    eventBus->subscribe(EventType::STATE_CHANGED,
                        [&lastEvent](std::shared_ptr<Event> event) { lastEvent = event; });

    // Transition should trigger event
    stateMachine->transitionTo(SystemState::READY);

    // Wait for event processing
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    ASSERT_NE(lastEvent, nullptr);
    EXPECT_EQ(lastEvent->getType(), EventType::STATE_CHANGED);
    EXPECT_EQ(lastEvent->getSource(), "state_machine");

    auto fromState = lastEvent->getData<int>("from_state");
    auto toState = lastEvent->getData<int>("to_state");

    ASSERT_TRUE(fromState.has_value());
    ASSERT_TRUE(toState.has_value());
    EXPECT_EQ(fromState.value(), static_cast<int>(SystemState::INITIALIZING));
    EXPECT_EQ(toState.value(), static_cast<int>(SystemState::READY));
}

// Test state history
TEST_F(StateMachineTest, StateHistoryTest) {
    // Perform several transitions
    stateMachine->transitionTo(SystemState::READY);
    stateMachine->transitionTo(SystemState::CONNECTED);
    stateMachine->transitionTo(SystemState::PROJECTING);
    stateMachine->transitionTo(SystemState::ERROR);
    stateMachine->transitionTo(SystemState::READY);

    auto history = stateMachine->getStateHistory();

    // Should have at least the transitions we made
    EXPECT_GE(history.size(), 5);

    // Check the last few states
    auto it = history.rbegin();
    EXPECT_EQ(it->state, SystemState::READY);
    ++it;
    EXPECT_EQ(it->state, SystemState::ERROR);
    ++it;
    EXPECT_EQ(it->state, SystemState::PROJECTING);
}

// Test concurrent state transitions
TEST_F(StateMachineTest, ConcurrentTransitionsTest) {
    // Move to READY state first
    stateMachine->transitionTo(SystemState::READY);

    std::atomic<int> successfulTransitions{0};
    std::atomic<int> failedTransitions{0};

    std::vector<std::thread> threads;

    // Create multiple threads trying to transition simultaneously
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([this, &successfulTransitions, &failedTransitions]() {
            if (stateMachine->transitionTo(SystemState::CONNECTED)) {
                successfulTransitions++;
            } else {
                failedTransitions++;
            }
        });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Only one transition should succeed
    EXPECT_EQ(successfulTransitions.load(), 1);
    EXPECT_EQ(failedTransitions.load(), 9);
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::CONNECTED);
}

// Test state timeout functionality
TEST_F(StateMachineTest, StateTimeoutTest) {
    // Move to READY
    stateMachine->transitionTo(SystemState::READY);

    auto history = stateMachine->getStateHistory();
    ASSERT_FALSE(history.empty());

    auto currentStateEntry = history.back();
    auto entryTime = currentStateEntry.timestamp;

    // Wait a bit
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Check if we can calculate time in state
    auto now = std::chrono::steady_clock::now();
    auto timeInState = std::chrono::duration_cast<std::chrono::milliseconds>(now - entryTime);

    EXPECT_GE(timeInState.count(), 100);
}

// Test state validation
TEST_F(StateMachineTest, StateValidationTest) {
    // Test canTransitionTo
    EXPECT_TRUE(stateMachine->canTransitionTo(SystemState::READY));
    EXPECT_FALSE(stateMachine->canTransitionTo(SystemState::CONNECTED));
    EXPECT_FALSE(stateMachine->canTransitionTo(SystemState::PROJECTING));
    EXPECT_TRUE(stateMachine->canTransitionTo(SystemState::ERROR));
    EXPECT_TRUE(stateMachine->canTransitionTo(SystemState::SHUTDOWN));

    // After transitioning to READY
    stateMachine->transitionTo(SystemState::READY);

    EXPECT_FALSE(stateMachine->canTransitionTo(SystemState::INITIALIZING));
    EXPECT_TRUE(stateMachine->canTransitionTo(SystemState::CONNECTED));
    EXPECT_FALSE(stateMachine->canTransitionTo(SystemState::PROJECTING));
    EXPECT_TRUE(stateMachine->canTransitionTo(SystemState::ERROR));
    EXPECT_TRUE(stateMachine->canTransitionTo(SystemState::SHUTDOWN));
}

// Test force transition (for emergency scenarios)
TEST_F(StateMachineTest, ForceTransitionTest) {
    // Normal transition that would fail
    EXPECT_FALSE(stateMachine->transitionTo(SystemState::CONNECTED));

    // Force transition should work
    stateMachine->forceTransition(SystemState::CONNECTED);
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::CONNECTED);
}

// Test state callbacks
TEST_F(StateMachineTest, StateCallbacksTest) {
    bool onEnterCalled = false;
    bool onExitCalled = false;
    SystemState enteredState = SystemState::INITIALIZING;
    SystemState exitedState = SystemState::INITIALIZING;

    // Set up callbacks
    stateMachine->setStateCallback(
        SystemState::READY,
        [&onEnterCalled, &enteredState](SystemState state) {
            onEnterCalled = true;
            enteredState = state;
        },
        [&onExitCalled, &exitedState](SystemState state) {
            onExitCalled = true;
            exitedState = state;
        });

    // Transition to READY
    stateMachine->transitionTo(SystemState::READY);

    EXPECT_TRUE(onEnterCalled);
    EXPECT_EQ(enteredState, SystemState::READY);

    // Transition away from READY
    stateMachine->transitionTo(SystemState::CONNECTED);

    EXPECT_TRUE(onExitCalled);
    EXPECT_EQ(exitedState, SystemState::READY);
}

// Test reset functionality
TEST_F(StateMachineTest, ResetTest) {
    // Make several transitions
    stateMachine->transitionTo(SystemState::READY);
    stateMachine->transitionTo(SystemState::CONNECTED);
    stateMachine->transitionTo(SystemState::PROJECTING);

    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::PROJECTING);

    // Reset should go back to INITIALIZING
    stateMachine->reset();
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::INITIALIZING);

    // History should be cleared
    auto history = stateMachine->getStateHistory();
    EXPECT_EQ(history.size(), 1);  // Only the current INITIALIZING state
}
