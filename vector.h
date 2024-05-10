#ifndef _VECTOR_H
#define _VECTOR_H

#include <new>


template <typename T>
class vector {
public:
    typedef T value_type;
    typedef T *pointer_type;
    typedef unsigned long size_type;
    
private:
    value_type *__arr;
    size_type __size;
    size_type __capacity;
    
public:
    vector();
    vector(size_type const size);
    vector(vector const &vec);
    vector(vector &&vec);
    ~vector();
    
    auto push_back(value_type const &value) -> void;
    auto push_back(value_type &&value) -> void;
    auto pop_back() -> void;
    
    auto insert(value_type *const pos, value_type const &value) -> void;
    auto insert(value_type *const pos,
                value_type const *begin, value_type const *end) -> void;
    
    auto erase(value_type *const pos) -> void;
    auto erase(value_type *const begin, value_type *const end) -> void;
    
    auto resize(size_type size) -> void;
    auto swap(vector& other) -> void;
    auto clear() -> void;
    
    auto reserve(size_type capacity) -> void;
    auto empty() const noexcept -> bool;
    auto size() const noexcept -> decltype(auto);
    auto capacity() const noexcept -> decltype(auto);
    
    auto data() -> value_type*;
    auto data() const -> value_type const*;
    
    auto begin() const -> decltype(auto);
    auto end() const -> decltype(auto);
    auto front() -> value_type&;
    auto back() -> value_type&;
    auto at(size_type index) -> value_type&;
    
    auto operator[](size_type index) -> value_type&;
    auto operator=(vector const &vec) -> decltype(auto);
    auto operator=(vector &&vec) -> decltype(auto);
    
private:
    [[nodiscard]] auto __allocate(size_t capacity) -> pointer_type;
    auto __deallocate(value_type *const pos) -> void;
    
    auto __construct(value_type *const pos) -> void;
    auto __construct(value_type *const pos, value_type const &value) -> void;
    auto __construct(value_type *const pos, value_type &&value) -> void;
    auto __construct_range(value_type *begin, value_type *end) -> void;
    auto __construct_range(value_type *dst, 
                           value_type *begin, value_type *end) -> void;
    auto __construct_backward(value_type *dst, 
                              value_type *rbegin, value_type *rend) -> void;
    
    auto __destruct(value_type *pos) -> void;
    auto __destruct_range(value_type *begin, value_type *end) -> void;
    
    auto __copy_range(value_type *dst, 
                      value_type const *begin, value_type const *end) -> void;
    auto __move_range(value_type *dst,
                      value_type *begin, value_type *end) -> void;
    auto __move_backward(value_type *dst,
                         value_type *rbegin, value_type *rend) -> void;
};


template <typename T>
vector<T>::vector() 
: vector(0)
{}

template <typename T>
vector<T>::vector(size_type const size)
: __arr(__allocate(size)), __size(size), __capacity(size)
{}

template <typename T>
vector<T>::vector(vector const &vec)
: __size(vec.__size), __capacity(vec.__capacity), __arr(__allocate()) {
    __copy_range(__arr, vec.begin(), vec.end());
}

template <typename T>
vector<T>::vector(vector &&vec)
: __arr(vec.__arr), __size(vec.__size), __capacity(vec.__capacity) {
    vec.__size     = 0;
    vec.__capacity = 0;
    vec.__arr      = nullptr;
}

template <typename T>
vector<T>::~vector() {
    __destruct_range(begin(), end());
    __deallocate(__arr);
    __arr = nullptr;
}

template <typename T>
auto vector<T>::push_back(const value_type &value) -> void {
    if (__size < __capacity) {
        __construct(__arr + __size++, value);
    }
    else {
        // allocate new array
        auto new_cap = __capacity == 0 ? 1 : __capacity * 2;
        auto dst     = __allocate(new_cap);
        
        // memmove
        __construct(dst + __size + 1, value);
        __construct_range(dst, begin(), end());
        
        // delete old
        __destruct_range(begin(), end());
        __deallocate(begin());
        
        __arr      = dst;
        __capacity = new_cap;
        ++__size;
    }
}

template <typename T>
auto vector<T>::push_back(value_type &&value) -> void {
    if (__size < __capacity) {
        __construct(__arr + __size++, static_cast<value_type &&>(value));
    }
    else {
        // allocate new array
        auto new_cap = __capacity == 0 ? 1 : __capacity * 2;
        auto dst     = __allocate(new_cap);
        
        // memmove
        __construct(dst + __size + 1, static_cast<value_type &&>(value));
        __construct_range(dst, begin(), end());
        
        // delete old
        __destruct_range(begin(), end());
        __deallocate(begin());
        
        __arr      = dst;
        __capacity = new_cap;
        ++__size;
    }
}

template <typename T>
auto vector<T>::pop_back() -> void {
    __destruct(__arr + --__size);
}

template <typename T>
auto vector<T>::insert(value_type *const pos, value_type const &value) -> void {
    if (pos == end()) {
        push_back(value);
    }
    else {
        insert(pos, &value, &value + 1);
    }
}

template <typename T>
auto vector<T>::insert(value_type *const pos,
                       value_type const *begin, value_type const *end) -> void {
    pointer_type dst = __arr;
    
    size_type new_cap  = __capacity;
    size_type new_size = __size + size_type(end - begin);
    
    // get new capacity
    if (new_size > __capacity) {
        new_cap = new_size < __capacity * 2 ? __capacity * 2 : new_size;
        dst     = __allocate(new_cap);
        
        // move construct front
        __construct_range(dst, this->begin(), pos);
    }
    
    // move construct back
    if (pos < this->end()) {
        __move_backward(dst + new_size - 1, this->end() - 1, pos - 1);
    }
    
    // construct inserting items
    __copy_range(dst + size_type(pos - this->begin()), begin, end);
    
    // delete temporary only on realloc
    if (dst != __arr) {
        __destruct_range(this->begin(), this->end());
        __deallocate(__arr);
        
        __arr      = dst;
        __capacity = new_cap;
    }
    
    __size = new_size;
}

template <typename T>
auto vector<T>::erase(value_type *const pos) -> void {
    __destruct(pos);
    __move_range(pos, pos + 1, end());
    --__size;
}

template <typename T>
auto vector<T>::erase(value_type *const begin, value_type *const end) -> void {
    __destruct_range(begin, end);
    __move_range(begin, end, this->end());
    __size -= (size_type)(end - begin);
}

template <typename T>
auto vector<T>::reserve(size_type capacity) -> void {
    if (__capacity >= capacity) {
        return;
    }
    
    // allocate new array
    auto dst = __allocate(capacity);
    
    // construct old elements
    __construct_range(dst, begin(), end());
    
    // delete old
    __destruct_range(begin(), end());
    __deallocate(begin());
    
    __arr = dst;
    __capacity = capacity;
}

template <typename T>
auto vector<T>::resize(size_type size) -> void {
    if (size == __size) {
        return;
    }
    
    if (size > __size) {
        if (size > __capacity) {
            auto new_cap = __capacity * 2 > size ? __capacity * 2 : size;
            auto dst = __allocate(new_cap);
            
            __construct_range(dst, begin(), end());
            __construct_range(dst + __size, dst + size);
            
            __destruct_range(begin(), end());
            __deallocate(begin());
            
            __arr = dst;
            __capacity = new_cap;
        }
        else {
            __construct_range(end(), begin() + size);
        }
    }
    else {
        __destruct_range(begin() + size, end());
    }
    
    __size = size;
}

template <typename T>
auto vector<T>::swap(vector<T> &other) -> void {
    auto temp_size = __size;
    auto temp_capacity = __capacity;
    auto temp_arr = __arr;
    
    __size = other.__size;
    __capacity = other.__capacity;
    __arr = other.__arr;
    
    other.__size = temp_size;
    other.__capacity = temp_capacity;
    other.__arr = temp_arr;
}

template <typename T>
auto vector<T>::clear() -> void {
    __size = 0;
    __destruct_range(begin(), end());
}

template <typename T>
auto vector<T>::data() -> value_type* {
    return __arr;
}

template <typename T>
auto vector<T>::data() const -> value_type const* {
    return __arr;
}

template <typename T>
auto vector<T>::empty() const noexcept -> bool {
    return !__size;
}

template <typename T>
auto vector<T>::size() const noexcept -> decltype(auto) {
    return __size;
}

template <typename T>
auto vector<T>::capacity() const noexcept -> decltype(auto) {
    return __capacity;
}

template <typename T>
auto vector<T>::begin() const -> decltype(auto) {
    return __arr;
}

template <typename T>
auto vector<T>::end() const -> decltype(auto) {
    return __arr + __size;
}

template <typename T>
auto vector<T>::front() -> value_type& {
    return *__arr;
}

template <typename T>
auto vector<T>::back() -> value_type& {
    return *(end() - 1);
}

template <typename T>
auto vector<T>::at(size_type index) -> value_type& {
    return __arr[index];
}

template <typename T>
auto vector<T>::operator[](size_type index) -> value_type& {
    return __arr[index];
}

template <typename T>
auto vector<T>::operator=(const vector<T> &vec) -> decltype(auto) {
    auto dst = __allocate(vec.__capacity);
    
    __copy_range(dst, vec.begin(), vec.end());
    __deallocate(__arr);
    
    __size     = vec.__size;
    __capacity = vec.__capacity;
    __arr      = dst;
    return *this;
}

template <typename T>
auto vector<T>::operator=(vector<T> &&vec) -> decltype(auto) {
    __deallocate(__arr);
    
    __size     = vec.__size;
    __capacity = vec.__capacity;
    __arr      = vec.__arr;
    
    vec.__size     = 0;
    vec.__capacity = 0;
    vec.__arr      = nullptr;
    return *this;
}

template <typename T>
auto vector<T>::__allocate(size_t capacity) -> pointer_type {
    return static_cast<value_type*>(::operator new(capacity * sizeof(value_type)));
}

template <typename T>
auto vector<T>::__deallocate(value_type *const pos) -> void {
    ::operator delete(static_cast<void *>(pos));
}

template <typename T>
auto vector<T>::__construct(value_type *const pos) -> void {
    ::new (static_cast<void*>(pos)) value_type();
}

template <typename T>
auto vector<T>::__construct(value_type *const pos, value_type const &value) -> void {
    ::new (static_cast<void*>(pos)) value_type(value);
}

template <typename T>
auto vector<T>::__construct(value_type *const pos, value_type &&value) -> void {
    ::new (static_cast<void*>(pos)) value_type(static_cast<value_type &&>(value));
}

template <typename T>
auto vector<T>::__construct_range(value_type *begin, value_type *end) -> void {
    for (; begin != end; ++begin) {
        __construct(begin);
    }
}

template <typename T>
auto vector<T>::__construct_range(value_type *dst,
                                  value_type *begin, value_type *end) -> void {
    for (; begin != end; ++dst, ++begin) {
        __construct(dst, static_cast<value_type&&>(*begin));
    }
}

template <typename T>
auto vector<T>::__construct_backward(value_type *dst,
                                     value_type *rbegin, value_type *rend) -> void {
    for (; rbegin != rend; --dst, --rbegin) {
        __construct(dst, static_cast<value_type&&>(*rbegin));
    }
}

template <typename T>
auto vector<T>::__destruct(value_type *pos) -> void {
    pos->~value_type();
}

template <typename T>
auto vector<T>::__destruct_range(value_type *begin, value_type *end) -> void {
    for (; begin != end; ++begin) {
        __destruct(begin);
    }
}

template <typename T>
auto vector<T>::__copy_range(value_type *dst,
                             value_type const *begin, value_type const *end) -> void {
    for (; begin != end; ++dst, ++begin) {
        *dst = *begin;
    }
}

template <typename T>
auto vector<T>::__move_range(value_type *dst,
                             value_type *begin, value_type *end) -> void {
    for (; begin != end; ++dst, ++begin) {
        *dst = static_cast<value_type&&>(*begin);
    }
}

template <typename T>
auto vector<T>::__move_backward(value_type *dst,
                                value_type *rbegin, value_type *rend) -> void {
    for (; rbegin != rend; --dst, --rbegin) {
        *dst = static_cast<value_type&&>(*rbegin);
    }
}

#endif /* _VECTOR_H */
