#include "event-bus/event-bus.h"
#include <iostream>

// Example from the task
struct Event {
    int param;
    Event(int p) : param(p) {}
};

int main() {
    EventBus<Event> bus;

    // Subscribe handler and save the subscription ID
    auto handler = [](const Event& event) {
        std::cout << "Received: " << event.param << std::endl;
    };

    auto subscriptionId = bus.Subscribe(handler);
    
    bus.Publish(Event(42));  // Should output "Received: 42"
    
    bus.Unsubscribe(subscriptionId);
    
    bus.Publish(Event(100)); // No output - handler was unsubscribed

    return 0;
}
