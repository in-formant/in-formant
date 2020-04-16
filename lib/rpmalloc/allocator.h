#ifndef RPMALLOC_STL_ALLOCATOR_H
#define RPMALLOC_STL_ALLOCATOR_H

#include <cstddef>
#include <memory>

#include "rpmalloc.h"

namespace rpm {

    template<class T>
    struct allocator {
        typedef size_t    size_type;
        typedef ptrdiff_t difference_type;
        typedef T*        pointer;
        typedef const T*  const_pointer;
        typedef T&        reference;
        typedef const T&  const_reference;
        typedef T         value_type;

        allocator() {}
        allocator(const allocator&) {}

        pointer allocate(size_type n, const void * = 0) {
            return (T *) rpmalloc(n * sizeof(T));
        }

        void deallocate(void *p, size_type) {
            if(p != nullptr) {
                rpfree(p);
            }
        }

        pointer           address(reference x) const { return &x; }
        const_pointer     address(const_reference x) const { return &x; }
        allocator<T>&     operator=(const allocator&) { return *this; }
        void              construct(pointer p, const T& val) 
                            { new ((T *) p) T(val); }
        void              destroy(pointer p) { p->~T(); }

        size_type         max_size() const { return size_t(-1); }
 
        template <class U>
        struct rebind { typedef allocator<U> other; };

        template <class U>
        allocator(const allocator<U>&) {}

        template <class U>
        allocator& operator=(const allocator<U>&) { return *this; }

    };

    template <class T, class U>
    constexpr bool operator== (const allocator<T>&, const allocator<U>&) noexcept { return true; }

    template <class T, class U>
    constexpr bool operator!= (const allocator<T>&, const allocator<U>&) noexcept { return false; }

}

#endif // RPMALLOC_STL_ALLOCATOR_H
