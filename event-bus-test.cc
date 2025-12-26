#include "event-bus/event-bus.h"
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <cassert>
#include <atomic>

// Test event structure
struct Event {
    int param;
    Event(int p = 0) : param(p) {}
};

// Test basic subscribe and publish
void TestBasicUsage() {
    std::cout << "=== Test Basic Usage ===" << std::endl;
    
    EventBus<Event> bus;
    std::atomic<int> received_value{0};

    auto handler = [&received_value](const Event& event) {
        std::cout << "Received: " << event.param << std::endl;
        received_value = event.param;
    };

    auto id = bus.Subscribe(handler);
    bus.Publish(Event(42));
    
    assert(received_value == 42);
    
    bus.Unsubscribe(id);
    bus.Publish(Event(100));
    
    // Value should not change after unsubscribe
    assert(received_value == 42);
    
    std::cout << "Test Basic Usage PASSED" << std::endl << std::endl;
}

// Test multiple subscribers
void TestMultipleSubscribers() {
    std::cout << "=== Test Multiple Subscribers ===" << std::endl;
    
    EventBus<Event> bus;
    std::atomic<int> call_count{0};

    auto id1 = bus.Subscribe([&call_count](const Event&) {
        call_count++;
        std::cout << "Handler 1 called" << std::endl;
    });

    auto id2 = bus.Subscribe([&call_count](const Event&) {
        call_count++;
        std::cout << "Handler 2 called" << std::endl;
    });

    auto id3 = bus.Subscribe([&call_count](const Event&) {
        call_count++;
        std::cout << "Handler 3 called" << std::endl;
    });

    bus.Publish(Event(1));
    assert(call_count == 3);

    bus.Unsubscribe(id2);
    call_count = 0;
    bus.Publish(Event(2));
    assert(call_count == 2);

    bus.Unsubscribe(id1);
    bus.Unsubscribe(id3);
    call_count = 0;
    bus.Publish(Event(3));
    assert(call_count == 0);

    std::cout << "Test Multiple Subscribers PASSED" << std::endl << std::endl;
}

// Test concurrent publish from multiple threads
void TestConcurrentPublish() {
    std::cout << "=== Test Concurrent Publish ===" << std::endl;
    
    EventBus<Event> bus;
    std::atomic<int> total_received{0};
    const int num_threads = 10;
    const int events_per_thread = 100;

    bus.Subscribe([&total_received](const Event& event) {
        total_received += event.param;
    });

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&bus, events_per_thread]() {
            for (int j = 0; j < events_per_thread; ++j) {
                bus.Publish(Event(1));
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    assert(total_received == num_threads * events_per_thread);
    std::cout << "Total received: " << total_received << std::endl;
    std::cout << "Test Concurrent Publish PASSED" << std::endl << std::endl;
}

// Test concurrent subscribe/unsubscribe
void TestConcurrentSubscribeUnsubscribe() {
    std::cout << "=== Test Concurrent Subscribe/Unsubscribe ===" << std::endl;
    
    EventBus<Event> bus;
    std::atomic<bool> running{true};
    std::atomic<int> call_count{0};

    // Publisher thread
    std::thread publisher([&bus, &running]() {
        while (running) {
            bus.Publish(Event(1));
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    });

    // Subscribe/unsubscribe threads
    std::vector<std::thread> sub_threads;
    for (int i = 0; i < 5; ++i) {
        sub_threads.emplace_back([&bus, &call_count, i]() {
            for (int j = 0; j < 50; ++j) {
                auto id = bus.Subscribe([&call_count, i](const Event&) {
                    call_count++;
                });
                std::this_thread::sleep_for(std::chrono::microseconds(50));
                bus.Unsubscribe(id);
            }
        });
    }

    for (auto& t : sub_threads) {
        t.join();
    }

    running = false;
    publisher.join();

    std::cout << "Call count: " << call_count << std::endl;
    std::cout << "Test Concurrent Subscribe/Unsubscribe PASSED" << std::endl << std::endl;
}

// Test UnsubscribeAndWait
void TestUnsubscribeAndWait() {
    std::cout << "=== Test UnsubscribeAndWait ===" << std::endl;
    
    EventBus<Event> bus;
    std::atomic<bool> handler_started{false};
    std::atomic<bool> handler_finished{false};

    auto id = bus.Subscribe([&](const Event&) {
        handler_started = true;
        std::cout << "Handler started, sleeping..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        handler_finished = true;
        std::cout << "Handler finished" << std::endl;
    });

    // Start publishing in another thread
    std::thread publisher([&bus]() {
        bus.Publish(Event(1));
    });

    // Wait a bit for handler to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    assert(handler_started == true);
    assert(handler_finished == false);
    
    std::cout << "Calling UnsubscribeAndWait while handler is running..." << std::endl;
    
    // This should wait until the handler completes
    auto start = std::chrono::steady_clock::now();
    bus.UnsubscribeAndWait(id);
    auto end = std::chrono::steady_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "UnsubscribeAndWait took " << duration << "ms" << std::endl;
    
    // Handler should have finished by now
    assert(handler_finished == true);
    
    publisher.join();
    
    std::cout << "Test UnsubscribeAndWait PASSED" << std::endl << std::endl;
}

// Test with different event types
struct StringEvent {
    std::string message;
    StringEvent(const std::string& msg) : message(msg) {}
};

void TestDifferentEventTypes() {
    std::cout << "=== Test Different Event Types ===" << std::endl;
    
    EventBus<StringEvent> string_bus;
    std::string received;

    string_bus.Subscribe([&received](const StringEvent& event) {
        received = event.message;
        std::cout << "String event: " << event.message << std::endl;
    });

    string_bus.Publish(StringEvent("Hello, EventBus!"));
    assert(received == "Hello, EventBus!");

    std::cout << "Test Different Event Types PASSED" << std::endl << std::endl;
}

// Test subscriber count
void TestSubscriberCount() {
    std::cout << "=== Test Subscriber Count ===" << std::endl;
    
    EventBus<Event> bus;
    assert(bus.SubscriberCount() == 0);

    auto id1 = bus.Subscribe([](const Event&) {});
    assert(bus.SubscriberCount() == 1);

    auto id2 = bus.Subscribe([](const Event&) {});
    auto id3 = bus.Subscribe([](const Event&) {});
    assert(bus.SubscriberCount() == 3);

    bus.Unsubscribe(id2);
    assert(bus.SubscriberCount() == 2);

    bus.Unsubscribe(id1);
    bus.Unsubscribe(id3);
    assert(bus.SubscriberCount() == 0);

    std::cout << "Test Subscriber Count PASSED" << std::endl << std::endl;
}

int main() {
    std::cout << "EventBus Test Suite" << std::endl;
    std::cout << "===================" << std::endl << std::endl;

    TestBasicUsage();
    TestMultipleSubscribers();
    TestConcurrentPublish();
    TestConcurrentSubscribeUnsubscribe();
    TestUnsubscribeAndWait();
    TestDifferentEventTypes();
    TestSubscriberCount();

    std::cout << "===================" << std::endl;
    std::cout << "All tests PASSED!" << std::endl;

    return 0;
}
