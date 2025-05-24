# signal

**signal** --- is a simple and lightweight C++20 library that brings Godot-like awaitable signals to native C++.

It only depends on my another lightweight library [delegate](https://github.com/Nikitf777/delegate), that provides a reusable wrapper for a collection of callables.

### Usage
```C++
#include <iostream>
#include <thread>
#include <chrono>

#include "signal.hpp"
#include "task.hpp" // only needed when you want to await a signal

struct MyClass {
    void Print(int x) {
        std::cout << "MyClass::Print(" << x << ")\n";
    }
};

void FreeFunction(int x) {
    std::cout << "FreeFunction(" << x << ")\n";
}

int main() {
    Signal<void(int)> mySignal;

    MyClass obj;

	// Connect callables
    mySignal.Connect(std::bind(&MyClass::Print, &obj, std::placeholders::_1));
    mySignal.Connect([](int x) { std::cout << "Lambda(" << x << ")\n"; });
    mySignal.Connect(FreeFunction);

    auto task1 = [&]() -> Task {
        while (true) {
            std::cout << "[Coroutine 1] Awaiting signal...\n";
            auto [value] = co_await mySignal;
            std::cout << "[Coroutine 1] Resumed with value: " << value << "\n";
        }
    }();

    auto task2 = [&]() -> Task {
        while (true) {
            std::cout << "[Coroutine 2] Awaiting signal...\n";
            auto [value] = co_await mySignal;
            std::cout << "[Coroutine 2] Resumed with value: " << value << "\n";
        }
    }();

	 // start awaiting
    task1.resume();
    task2.resume();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Emitting first signal...\n";
    mySignal(42);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Emitting second signal...\n";
    mySignal(84);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Emitting third signal...\n";
    mySignal(126);

    return 0;
}
```