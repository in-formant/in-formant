#ifndef RPM_ALIGNED_NEW_ALLOCATOR_H
#define RPM_ALIGNED_NEW_ALLOCATOR_H

#include <cstddef>
#include <type_traits>
#include <memory>

namespace rpm {

    // Simple allocator: defers allocs / deallocs to new / delete.

    template<typename T>
    class aligned_new_allocator {
    public:
        typedef std::size_t     size_type;
        typedef std::ptrdiff_t  difference_type;
        typedef T*              pointer;
        typedef const T*        const_pointer;
        typedef T&              reference;
        typedef const T&        const_reference;
        typedef T               value_type;
        
        template<typename U>
        struct rebind { typedef aligned_new_allocator<U> other; };

#if __cplusplus >= 201103L
        typedef std::true_type propagate_on_container_move_assignment;
#endif

        aligned_new_allocator() noexcept {}
        aligned_new_allocator(const aligned_new_allocator&) noexcept {}

        template<typename U>
        aligned_new_allocator(const aligned_new_allocator<U>&) noexcept {}

        ~aligned_new_allocator() noexcept {}

        pointer address(reference x) const noexcept {
            return std::addressof(x);
        }

        const_pointer address(const_reference x) const noexcept {
            return std::addressof(x);
        }

        pointer allocate(size_type n, const void* = static_cast<const void*>(0)) {
            if (n > max_size())
                throw std::bad_alloc();
       
            if (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
                std::align_val_t al = std::align_val_t(alignof(T));
                return static_cast<T*>(::operator new(n * sizeof(T), al));
            }

            return static_cast<T*>(::operator new(n * sizeof(T)));    
        }

        void deallocate(pointer p, size_type) {
            if (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
                ::operator delete(p, std::align_val_t(alignof(T)));
                return;
            }

            ::operator delete(p);
        }

        constexpr size_type max_size() const noexcept {
            return std::size_t(-1) / sizeof(T);
        }
        
#if __cplusplus >= 201103L
        template<typename U, typename... Args>
        void construct(U* p, Args&&... args) {
            ::new((void*) p) U(std::forward<Args>(args)...);
        }

        template<typename U>
        void destroy(U* p) {
            p->~U();
        }
#else
        void construct(pointer p, const T& val) {
            ::new((void*) p) T(val);
        }

        void destroy(pointer p) {
            p->~T();
        }
#endif
    };

    template<typename T>
    inline bool operator==(const aligned_new_allocator<T>&, const aligned_new_allocator<T>&) { return true; }

    template<typename T>
    inline bool operator!=(const aligned_new_allocator<T>&, const aligned_new_allocator<T>&) { return false; }

}

#endif // RPM_ALIGNED_NEW_ALLOCATOR_H
