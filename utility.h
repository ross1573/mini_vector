#ifndef _UTILITY_H
#define _UTILITY_H


template <typename T>
struct remove_reference {
    typedef T type;
};

template <typename T>
struct remove_reference<T&> : remove_reference<T>
{};

template <typename T>
struct remove_reference<T&&> : remove_reference<T>
{};


template <typename T>
constexpr T&& forward(typename remove_reference<T>::type &arg) noexcept {
    return static_cast<T&&>(arg);
};

template <typename T>
constexpr T&& forward(typename remove_reference<T>::type &&arg) noexcept {
    return static_cast<T&&>(arg);
};

#endif
