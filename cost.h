#include <iostream>
#include <chrono>
#include <functional>

template <typename ReturnType, typename... Args>
double measureExecutionTime(std::function<ReturnType(Args...)> func, Args... args) {
    auto start = std::chrono::high_resolution_clock::now();
    func(args...);
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}