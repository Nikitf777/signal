#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <future>
#include <chrono>

#include "signal.hpp"
#include "task.hpp"

// Mock class for testing member function connections
class MockClass {
public:
    MOCK_METHOD(void, Print, (int), ());
};

// Test fixture to avoid repetition
class SignalTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset counters before each test
        lambda_call_count = 0;
        free_func_call_count = 0;
    }

    int lambda_call_count = 0;
    int free_func_call_count = 0;
};

// Test that the signal is compatible with different types
TEST_F(SignalTest, SignalIsCompatibleWithDifferentTypes) {
	Signal<void(int)> intSignal;
	Signal<void(float)> floatSignal;
	Signal<void(std::string)> stringSignal;

	int intValue{};
	float floatValue{};
	std::string stringValue{};

	intSignal.connect([&intValue](int x) { intValue = x; });
	floatSignal.connect([&floatValue](float x) { floatValue = x; });
	stringSignal.connect([&stringValue](std::string x) { stringValue = x; });

	intSignal(42);
	floatSignal(3.14f);
	stringSignal("Hello, World!");

	EXPECT_EQ(intValue, 42);
	EXPECT_EQ(floatValue, 3.14f);
	EXPECT_EQ(stringValue, "Hello, World!");
}

// Test that all connected handlers are invoked once per signal emission
TEST_F(SignalTest, AllHandlersAreInvoked) {
    Signal<void(int)> signal;
    MockClass mock;

    // Connect handlers
    signal.connect(std::bind(&MockClass::Print, &mock, std::placeholders::_1));
    signal.connect([this](int x) { lambda_call_count++; });
    signal.connect([this](int x) { free_func_call_count++; });

    // Expectations
    EXPECT_CALL(mock, Print(42)).Times(1);

    // Emit signal
    signal(42);

    // Validate
    EXPECT_EQ(lambda_call_count, 1);
    EXPECT_EQ(free_func_call_count, 1);
}

// Test that handlers are invoked multiple times on multiple emissions
TEST_F(SignalTest, HandlersAreInvokedMultipleTimes) {
    Signal<void(int)> signal;
    MockClass mock;

    signal.connect(std::bind(&MockClass::Print, &mock, std::placeholders::_1));
    signal.connect([this](int x) { lambda_call_count++; });
    signal.connect([this](int x) { free_func_call_count++; });

    // First emission
    EXPECT_CALL(mock, Print(42)).Times(1);
    signal(42);
    EXPECT_EQ(lambda_call_count, 1);
    EXPECT_EQ(free_func_call_count, 1);

    // Second emission
    EXPECT_CALL(mock, Print(84)).Times(1);
    signal(84);
    EXPECT_EQ(lambda_call_count, 2);
    EXPECT_EQ(free_func_call_count, 2);
}

// Test that a coroutine awaiting the signal is resumed and receives the correct value
TEST_F(SignalTest, CoroutineResumesWithValue) {
    Signal<void(int)> signal;
    std::promise<int> promise;
    auto future = promise.get_future();

    auto task = [&]() -> Task {
        auto [value] = co_await signal;
        promise.set_value(value);
    }();

    // Resume the task, which should suspend until the signal is emitted
    task.resume();

    // Emit the signal
    signal(42);

    // Wait for the coroutine to complete
    auto status = future.wait_for(std::chrono::seconds(1));
    EXPECT_EQ(status, std::future_status::ready);
    EXPECT_EQ(future.get(), 42);
}

// Test that multiple coroutines awaiting the same signal are all resumed
TEST_F(SignalTest, MultipleCoroutinesResumed) {
    Signal<void(int)> signal;
    std::promise<int> p1, p2;
    auto f1 = p1.get_future();
    auto f2 = p2.get_future();

    auto task1 = [&]() -> Task {
        auto [value] = co_await signal;
        p1.set_value(value);
    }();

    auto task2 = [&]() -> Task {
        auto [value] = co_await signal;
        p2.set_value(value);
    }();

    task1.resume();
    task2.resume();

    signal(84);

    EXPECT_EQ(f1.get(), 84);
    EXPECT_EQ(f2.get(), 84);
}