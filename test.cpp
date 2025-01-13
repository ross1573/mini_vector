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
    foo(int n) : value(n) { ++constructor_cnt; }
    foo(foo const& o) : value(o.value) { ++copy_cnt; }
    foo(foo&& o) noexcept : value(o.value) { ++move_cnt; }
    ~foo() { ++destructor_cnt; }
    
    foo &operator=(foo const& o) { value = o.value; ++assign_cnt; return *this; }
    foo &operator=(foo&& o) { value = o.value; ++move_assign_cnt; return *this; }
    
    bool operator==(foo const &other) { return value == other.value; }
    bool operator!=(foo const &other) { return value != other.value; }
};

struct foo_debug {
    int value;
    
    foo_debug() : foo_debug(42) {}
    foo_debug(int n) : value(n) { std::cout << "constructor " << n << " called\n"; }
    foo_debug(foo_debug const& o) : value(o.value) { std::cout << "copy constructor " << o.value << " called\n"; }
    foo_debug(foo_debug && o) noexcept : value(o.value) { std::cout << "move constructor " << o.value << " called\n"; }
    ~foo_debug() { std::cout << "destructor called\n"; }
    
    foo_debug &operator=(foo_debug const& o)
        { value = o.value; std::cout << "assgin operator " << o.value <<" called\n"; return *this; }
    foo_debug &operator=(foo_debug&& o)
        { value = o.value; std::cout << "move assign operator " << o.value << " called\n"; return *this; }
    
    bool operator==(foo_debug const &other) { return value == other.value; }
    bool operator!=(foo_debug const &other) { return value != other.value; }
};



template <typename T>
using benchmark_t = std::tuple<std::string, int, void(*)(T&, int)>;

void debug([[maybe_unused]] auto& vec) {
//    vec.push_back({1});
//    vec.insert(vec.begin(), {2});
//    vec.insert(vec.begin(), {3});;
//    vec.emplace_back(4);
//    vec.emplace_back(5);
//    vec.emplace_back(6);
//    vec.emplace_back(7);
//    vec.push_back({8});
//    vec.push_back({9});
//    vec.push_back({10});
//    vec.push_back({11});
//    vec.push_back({12});
//    vec.push_back({13});
//    vec.push_back({14});
}

template <typename T>
std::array benchmark = {    
    benchmark_t<T>{ "reserve", 10000,
        [](auto& vec, [[maybe_unused]] int cnt) -> void { vec.reserve((std::size_t)cnt); }},
    
    benchmark_t<T>{ "resize", 1000,
        [](auto& vec, [[maybe_unused]] int cnt) -> void { vec.resize((std::size_t)cnt * 1000); }},
    
    benchmark_t<T>{ "resize_args", 1000,
        [](auto& vec, [[maybe_unused]] int cnt) -> void { vec.resize((std::size_t)(cnt + 100) * 1000, {32}); }},
    
    benchmark_t<T>{ "push_back", 1000000,
        [](auto& vec, [[maybe_unused]] int cnt) -> void { vec.push_back({cnt}); }},
    
    benchmark_t<T>{ "insert", 1000,
        [](auto& vec, [[maybe_unused]] int cnt) -> void { vec.insert(vec.begin() + cnt * 2, {42}); }},
    
    benchmark_t<T>{ "modify", 1000000,
        [](auto& vec, [[maybe_unused]] int cnt) -> void { vec[(std::size_t) + 1000].value = cnt * 200; }},
    
    benchmark_t<T>{ "erase", 1000,
        [](auto& vec, [[maybe_unused]] int cnt) -> void { vec.erase(vec.begin()); }},
    
    benchmark_t<T>{ "emplace_back", 1000000,
        [](auto& vec, [[maybe_unused]] int cnt) -> void { vec.emplace_back(cnt); }},
    
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
            std::cout << "ERROR: Element mismatch detected in " << idx << std::endl;
            return 0;
        }
    }
}
