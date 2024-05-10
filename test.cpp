#include <iostream>
#include <vector>
#include <chrono>
#include <array>
#include <tuple>
#include <string>

#include "vector.h"


std::size_t constructor_cnt;
std::size_t destructor_cnt;
std::size_t copy_cnt;
std::size_t move_cnt;
std::size_t assign_cnt;
std::size_t move_assign_cnt;


struct foo {
    int value;
    long dummy[12] = { 0, };
    
    foo() : foo(42) {}
    foo(int num) : value(num) { ++constructor_cnt; }
    foo(foo const&) { ++copy_cnt; }
    foo(foo &&) noexcept { ++move_cnt; }
    ~foo() { ++destructor_cnt; }
    
    foo &operator=(foo const&) { ++assign_cnt; return *this; }
    foo &operator=(foo &&) { ++move_assign_cnt; return *this; }
    
    bool operator==(foo const &other) { return value == other.value; }
    bool operator!=(foo const &other) { return value != other.value; }
};

struct foo_debug {
    int value;
    
    foo_debug() : foo_debug(42) {}
    foo_debug(int num) : value(num) { std::cout << "constructor called\n"; }
    foo_debug(foo const&) { std::cout << "copy constructor called\n"; }
    foo_debug(foo &&) noexcept { std::cout << "move constructor called\n"; }
    ~foo_debug() { std::cout << "destructor called\n"; }
    
    foo_debug &operator=(foo_debug const&) { std::cout << "assgin operator called\n"; return *this; }
    foo_debug &operator=(foo_debug &&) { std::cout << "move assign operator called\n"; return *this; }
    
    bool operator==(foo_debug const &other) { return value == other.value; }
    bool operator!=(foo_debug const &other) { return value != other.value; }
};



template <typename T>
using benchmark_t = std::tuple<std::string, int, void(*)(T&, int)>;

void debug([[maybe_unused]] auto& vec) {
//    vec.push_back(1);
//    vec.push_back(2);
//    
//    for (std::size_t i = 0; i < 10; ++i) {
//        vec.resize(i+2);
//    }
}

template <typename T>
std::array benchmark = {
    benchmark_t<T>{ {}, 1000,
        [](auto& vec, [[maybe_unused]] int cnt) -> void { vec.push_back(cnt); }},
    
    benchmark_t<T>{ "reserve", 10000,
        [](auto& vec, [[maybe_unused]] int cnt) -> void { vec.reserve((std::size_t)cnt); }},
    
    benchmark_t<T>{ "resize", 100,
        [](auto& vec, [[maybe_unused]] int cnt) -> void { vec.resize((std::size_t)cnt * 10000); }},
    
    benchmark_t<T> { {}, 1,
        [](auto& vec, [[maybe_unused]] int cnt) -> void { T temp; vec.swap(temp); }},
    
    benchmark_t<T>{ "push_back", 1000000,
        [](auto& vec, [[maybe_unused]] int cnt) -> void { vec.push_back({cnt}); }},
    
    benchmark_t<T>{ "insert", 1000000,
        [](auto& vec, [[maybe_unused]] int cnt) -> void { vec.insert(vec.begin() + cnt, {42}); }},
    
    benchmark_t<T>{ "modify", 1000000,
        [](auto& vec, [[maybe_unused]] int cnt) -> void { vec[(std::size_t)cnt].value = cnt * 200; }},
    
    benchmark_t<T>{ "erase", 10000,
        [](auto& vec, [[maybe_unused]] int cnt) -> void { vec.erase(vec.begin()); }},
    
    benchmark_t<T>{ {}, 1,
        [](auto& vec, [[maybe_unused]] int cnt) -> void { vec.resize(10000000); }},
    
    benchmark_t<T>{ "at", 100000,
        [](auto& vec, [[maybe_unused]] int cnt) -> void { vec.at((std::size_t)cnt * 3); }}
};


template <typename T>
auto benchmark_impl(T& vec) {
    for (auto& benchmark_func : benchmark<T>) {
        constructor_cnt = 0;
        copy_cnt        = 0;
        move_cnt        = 0;
        destructor_cnt  = 0;
        assign_cnt      = 0;
        move_assign_cnt = 0;
        
        auto target_name = std::get<0>(benchmark_func);
        auto target_iter = std::get<1>(benchmark_func);
        auto target_func = std::get<2>(benchmark_func);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < target_iter; ++i) {
            target_func(vec, i);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        if (target_name.empty()) {
            continue;
        }
        
        std::cout << "Function: " << target_name << "\n\n"
            << "chrono: " << diff.count() << '\n'
            << "constructor: " << constructor_cnt << '\n'
            << "copy: " << copy_cnt << '\n'
            << "move: " << move_cnt << '\n'
            << "detructor: " << destructor_cnt << '\n'
            << "assign: " << assign_cnt << '\n'
            << "move_assign: " << move_assign_cnt << "\n\n";
    }
}


auto main() -> int {
    vector<foo> vec;
    vector<foo_debug> vec_debug;
    std::cout << "Custome impl\n\n\n";
    {
        debug(vec_debug);
        benchmark_impl(vec);
    }
    std::cout << "\n\n";
    
    std::vector<foo> std_vec;
    std::vector<foo_debug> std_vec_debug;
    std::cout << "Standard impl\n\n\n";
    {
        debug(std_vec_debug);
        benchmark_impl(std_vec);
    }
    std::cout << "\n\n";
    
    
    if (vec.size() != std_vec.size()) {
        std::cout << "ERROR: Vector size mismatch!\n";
        return 0;
    }
    
    for (std::size_t idx = 0; idx < std_vec.size(); ++idx) {
        if (vec[idx] != std_vec[idx]) {
            std::cout << "ERROR: Element mismatch detected!\n";
            return 0;
        }
    }
}
