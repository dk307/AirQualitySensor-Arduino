#pragma once

#include <esp32-hal.h>

template <typename T>
class spram_allocator
{
public:
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T *pointer;
    typedef const T *const_pointer;
    typedef T &reference;
    typedef const T &const_reference;
    typedef T value_type;

    spram_allocator() {}
    ~spram_allocator() {}

    template <class U>
    struct rebind
    {
        typedef spram_allocator<U> other;
    };
    template <class U>
    spram_allocator(const spram_allocator<U> &) {}

    pointer address(reference x) const { return &x; }
    const_pointer address(const_reference x) const { return &x; }
    size_type max_size() const throw() { return size_t(-1) / sizeof(value_type); }

    pointer allocate(size_type n, const void *hint = 0)
    {
        return static_cast<pointer>(ps_malloc(n * sizeof(T)));
    }

    void deallocate(pointer p, size_type n)
    {
        free(p);
    }

    template <class U, class... Args>
    void construct(U *p, Args &&...args)
    {
        ::new ((void *)p) U(std::forward<Args>(args)...);
    }

    void destroy(pointer p)
    {
        p->~T();
    }
};