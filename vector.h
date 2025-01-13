#ifndef _VECTOR_H
#define _VECTOR_H

#include <new>
#include "utility.h"


template <typename T>
class vector {
public:
    typedef T value_type;
    typedef T* pointer_type;
    typedef T const* const_pointer_type;
    typedef T& reference_type;
    typedef T const& const_reference_type;
    typedef unsigned long size_type;
    
private:
    value_type *__arr;
    size_type __size;
    size_type __capacity;
    
public:
    vector();
    explicit vector(size_type const size);
    vector(vector const &vec);
    vector(vector &&vec);
    ~vector();
    
    void push_back(value_type const &value);
    void push_back(value_type &&value);
    void pop_back();
    template <typename... Args>
    reference_type emplace_back(Args&&... args);
    
    void insert(const_pointer_type pos, value_type const& value);
    void insert(const_pointer_type pos,
                pointer_type begin, pointer_type end);
    
    void erase(const_pointer_type pos);
    void erase(const_pointer_type begin, const_pointer_type end);
    
    void resize(size_type size);
    void resize(size_type size, value_type const& value);
    void swap(vector& other);
    void clear();
    
    void reserve(size_type capacity);
    bool empty() const noexcept;
    size_type size() const noexcept;
    size_type capacity() const noexcept;
    
    pointer_type data();
    const_pointer_type data() const;
    
    pointer_type begin();
    pointer_type end();
    reference_type front();
    reference_type back();
    reference_type at(size_type index);
    const_pointer_type begin() const;
    const_pointer_type end() const;
    const_reference_type front() const;
    const_reference_type back() const;
    const_reference_type at(size_type index) const;
    
    reference_type operator[](size_type index);
    const_reference_type operator[](size_type index) const;
    vector& operator=(vector const &vec);
    vector& operator=(vector &&vec);
    
private:
    [[nodiscard]] pointer_type __allocate(size_type capacity);
    void __deallocate(const_pointer_type pos);
    
    template <typename... Args>
    void __construct(const_pointer_type pos, Args&&... args);
    template <typename... Args>
    void __construct_range(const_pointer_type begin, const_pointer_type end, Args&&... args);
    
    void __copy_construct_range(const_pointer_type dst,
                                const_pointer_type begin, const_pointer_type end);
    void __move_construct_range(const_pointer_type dst,
                                pointer_type begin, pointer_type end);
    void __move_construct_backward(const_pointer_type dst,
                                   pointer_type rbegin, pointer_type rend);
    
    void __destruct(pointer_type pos);
    void __destruct_range(pointer_type begin, pointer_type end);
    
    void __copy_range(pointer_type dst,
                      const_pointer_type begin, const_pointer_type end);
    void __move_range(pointer_type dst,
                      const_pointer_type begin, const_pointer_type end);
    void __move_backward(pointer_type dst,
                         const_pointer_type rbegin, const_pointer_type rend);
};


template <typename T>
vector<T>::vector() 
    : vector(0)
{}

template <typename T>
vector<T>::vector(size_type const size)
    : __arr(__allocate(size))
    , __size(size)
    , __capacity(size)
{}

template <typename T>
vector<T>::vector(vector const &vec)
    : __size(vec.__size)
    , __capacity(vec.__capacity)
    , __arr(__allocate()) {
    __copy_range(__arr, vec.begin(), vec.end());
}

template <typename T>
vector<T>::vector(vector &&vec)
    : __arr(vec.__arr)
    , __size(vec.__size)
    , __capacity(vec.__capacity) {
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
void vector<T>::push_back(const value_type &value) {
    if (__size < __capacity) {
        __construct(__arr + __size++, value);
    }
    else {
        size_type new_cap = __capacity == 0 ? 1 : __capacity * 2;
        pointer_type new_arr = __allocate(new_cap);
        
        pointer_type dst = new_arr + __size;
        __construct(dst, value); dst -= 1;
        __move_construct_backward(dst, end() - 1, begin() - 1);
        
        __destruct_range(begin(), end());
        __deallocate(begin());
        
        __arr      = new_arr;
        __capacity = new_cap;
        ++__size;
    }
}

template <typename T>
void vector<T>::push_back(value_type &&value) {
    if (__size < __capacity) {
        __construct(__arr + __size++, static_cast<value_type&&>(value));
    }
    else {
        size_type new_cap = __capacity == 0 ? 1 : __capacity * 2;
        pointer_type new_arr = __allocate(new_cap);
        
        pointer_type dst = new_arr + __size;
        __construct(dst, static_cast<value_type&&>(value)); dst -= 1;
        __move_construct_backward(dst, end() - 1, begin() - 1);
        
        __destruct_range(begin(), end());
        __deallocate(begin());
        
        __arr      = new_arr;
        __capacity = new_cap;
        ++__size;
    }
}

template <typename T>
void vector<T>::pop_back() {
    __destruct(__arr + --__size);
}

template <typename T>
template <typename... Args>
auto vector<T>::emplace_back(Args&&... args) -> reference_type {
    if (__size < __capacity) {
        __construct(__arr + __size++, forward<Args>(args)...);
    }
    else {
        size_type new_cap = __capacity == 0 ? 1 : __capacity * 2;
        pointer_type new_arr = __allocate(new_cap);
        
        pointer_type dst = new_arr + __size;
        __construct(dst, forward<Args>(args)...); dst -= 1;
        __move_construct_backward(dst, end() - 1, begin() - 1);
        
        __destruct_range(begin(), end());
        __deallocate(begin());
        
        __arr      = new_arr;
        __capacity = new_cap;
        ++__size;
    }
    
    return back();
}

template <typename T>
void vector<T>::insert(const_pointer_type pos, value_type const &value) {
    if (pos == end()) {
        push_back(value);
    }
    else {
        pointer_type begin = const_cast<pointer_type>(&value);
        pointer_type end = begin + 1;
        
        insert(pos, begin, end);
    }
}

template <typename T>
void vector<T>::insert(const_pointer_type pos,
                       pointer_type begin, pointer_type end) {
    size_type new_size = size() + size_type(end - begin);
    
    if (new_size > capacity()) {
        size_type new_cap = new_size < capacity() * 2 ? capacity() * 2 : new_size;
        pointer_type new_arr = __allocate(new_cap);
        
        pointer_type dst = new_arr;
        pointer_type old_begin = this->begin();
        pointer_type old_end = this->end();
        size_type diff = size_type(pos - old_begin);
        
        __move_construct_range(dst, old_begin, old_begin + diff); dst += diff;
        __copy_construct_range(dst, begin, end); dst += size_type(end - begin);
        __move_construct_range(dst, old_begin + diff, old_end);
        
        __destruct_range(old_begin, old_end);
        __deallocate(__arr);
        
        __arr      = new_arr;
        __capacity = new_cap;
    }
    else {
        pointer_type loc = this->begin() + (pos - this->begin());
        
        __move_backward(this->begin() + new_size - 1, this->end() - 1, loc - 1);
        __copy_construct_range(loc, begin, end);
    }
    
    __size = new_size;
}

template <typename T>
void vector<T>::erase(const_pointer_type pos) {
    pointer_type loc = begin() + (pos - begin());
    
    __destruct(loc);
    __move_range(loc, loc + 1, end());
    --__size;
}

template <typename T>
void vector<T>::erase(const_pointer_type begin, const_pointer_type end) {
    __destruct_range(begin, end);
    __move_range(begin, end, this->end());
    __size -= (size_type)(end - begin);
}

template <typename T>
void vector<T>::reserve(size_type capacity) {
    if (__capacity >= capacity) {
        return;
    }
    
    pointer_type dst = __allocate(capacity);
    
    __move_construct_range(dst, begin(), end());
    
    __destruct_range(begin(), end());
    __deallocate(begin());
    
    __arr = dst;
    __capacity = capacity;
}

template <typename T>
void vector<T>::resize(size_type size) {
    if (size == __size) {
        return;
    }
    
    if (size > __size) {
        if (size > __capacity) {
            auto new_cap = __capacity * 2 > size ? __capacity * 2 : size;
            auto dst = __allocate(new_cap);
            
            __move_construct_range(dst, begin(), end());
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
void vector<T>::resize(size_type size, value_type const& value) {
    if (size == __size) {
        return;
    }
    
    if (size > __size) {
        if (size > __capacity) {
            auto new_cap = __capacity * 2 > size ? __capacity * 2 : size;
            auto dst = __allocate(new_cap);
            
            __move_construct_range(dst, begin(), end());
            __construct_range(dst + __size, dst + size, value);
            
            __destruct_range(begin(), end());
            __deallocate(begin());
            
            __arr = dst;
            __capacity = new_cap;
        }
        else {
            __construct_range(end(), begin() + size, value);
        }
    }
    else {
        __destruct_range(begin() + size, end());
    }
    
    __size = size;
}

template <typename T>
void vector<T>::swap(vector<T> &other) {
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
void vector<T>::clear() {
    __size = 0;
    __destruct_range(begin(), end());
}

template <typename T>
auto vector<T>::data() -> pointer_type {
    return __arr;
}

template <typename T>
auto vector<T>::data() const -> const_pointer_type {
    return __arr;
}

template <typename T>
bool vector<T>::empty() const noexcept {
    return !__size;
}

template <typename T>
auto vector<T>::size() const noexcept -> size_type {
    return __size;
}

template <typename T>
auto vector<T>::capacity() const noexcept -> size_type {
    return __capacity;
}

template <typename T>
auto vector<T>::begin() -> pointer_type {
    return __arr;
}

template <typename T>
auto vector<T>::end() -> pointer_type {
    return __arr + __size;
}

template <typename T>
auto vector<T>::front() -> reference_type {
    return *__arr;
}

template <typename T>
auto vector<T>::back() -> reference_type {
    return *(end() - 1);
}

template <typename T>
auto vector<T>::at(size_type index) -> reference_type {
    return __arr[index];
}

template <typename T>
auto vector<T>::begin() const -> const_pointer_type {
    return __arr;
}

template <typename T>
auto vector<T>::end() const -> const_pointer_type {
    return __arr + __size;
}

template <typename T>
auto vector<T>::front() const -> const_reference_type {
    return *__arr;
}

template <typename T>
auto vector<T>::back() const -> const_reference_type {
    return *(end() - 1);
}

template <typename T>
auto vector<T>::at(size_type index) const -> const_reference_type {
    return __arr[index];
}

template <typename T>
auto vector<T>::operator[](size_type index) -> reference_type {
    return __arr[index];
}

template <typename T>
auto vector<T>::operator[](size_type index) const -> const_reference_type {
    return __arr[index];
}

template <typename T>
vector<T>& vector<T>::operator=(const vector<T> &vec) {
    auto dst = __allocate(vec.__capacity);
    
    __copy_range(dst, vec.begin(), vec.end());
    __deallocate(__arr);
    
    __size     = vec.__size;
    __capacity = vec.__capacity;
    __arr      = dst;
    return *this;
}

template <typename T>
vector<T>& vector<T>::operator=(vector<T> &&vec) {
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
auto vector<T>::__allocate(size_type capacity) -> pointer_type {
    return static_cast<value_type*>(::operator new(capacity * sizeof(value_type)));
}

template <typename T>
void vector<T>::__deallocate(const_pointer_type pos) {
    ::operator delete(const_cast<void*>(static_cast<const volatile void*>(pos)));
}

template <typename T>
template <typename... Args>
void vector<T>::__construct(const_pointer_type pos, Args&&... args) {
    ::new (const_cast<void*>(static_cast<const volatile void*>(pos))) value_type(forward<Args>(args)...);
}

template <typename T>
template <typename... Args>
void vector<T>::__construct_range(const_pointer_type begin, const_pointer_type end, Args&&... args) {
    for (pointer_type loc = const_cast<pointer_type>(begin); loc != end; ++loc) {
        __construct(loc, forward<Args>(args)...);
    }
}

template <typename T>
void vector<T>::__copy_construct_range(const_pointer_type dst,
                                       const_pointer_type begin, const_pointer_type end) {
    for (pointer_type loc = const_cast<pointer_type>(begin); loc != end; ++loc) {
        __construct(dst, static_cast<const value_type&>(*loc));
    }
}

template <typename T>
void vector<T>::__move_construct_range(const_pointer_type dst,
                                       pointer_type begin, pointer_type end) {
    for (; begin != end; ++dst, ++begin) {
        __construct(dst, static_cast<value_type&&>(*begin));
    }
}

template <typename T>
void vector<T>::__move_construct_backward(const_pointer_type dst,
                                          pointer_type rbegin, pointer_type rend) {
    for (; rbegin != rend; --dst, --rbegin) {
        __construct(dst, static_cast<value_type&&>(*rbegin));
    }
}

template <typename T>
void vector<T>::__destruct(pointer_type pos) {
    pos->~value_type();
}

template <typename T>
void vector<T>::__destruct_range(pointer_type begin, pointer_type end) {
    for (; begin != end; ++begin) {
        begin->~value_type();
    }
}

template <typename T>
void vector<T>::__copy_range(pointer_type dst,
                             const_pointer_type begin, const_pointer_type end) {
    for (; begin != end; ++dst, ++begin) {
        *dst = *begin;
    }
}

template <typename T>
void vector<T>::__move_range(pointer_type dst,
                             const_pointer_type begin, const_pointer_type end) {
    pointer_type pos = __arr + (begin - this->begin());
    for (; pos != end; ++dst, ++pos) {
        *dst = static_cast<value_type&&>(*pos);
    }
}

template <typename T>
void vector<T>::__move_backward(pointer_type dst,
                                const_pointer_type rbegin, const_pointer_type rend) {
    pointer_type pos = __arr + (rbegin - this->begin());
    for (; pos != rend; --dst, --pos) {
        *dst = static_cast<value_type&&>(*pos);
    }
}

#endif /* _VECTOR_H */
