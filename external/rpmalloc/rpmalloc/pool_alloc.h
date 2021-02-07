#ifndef POOL_ALLOC_H
#define POOL_ALLOC_H

#include <cstddef>

namespace rpm {

    class pool_alloc_base {
    protected:
        enum { _S_align = 8 };
        enum { _S_max_bytes = 128 };
        enum { _S_free_list_size = (size_t) _S_max_bytes / (size_t) _S_align };

        union _Obj {
            union _Obj* _M_free_list_link;
            char        _M_client_data[1];
        };

        static _Obj* volatile _S_free_list[_S_free_list_size];

        static char* _S_start_free;
        static char* _S_end_free;
        static size_t _S_heap_size;

        size_t _M_round_up(size_t __bytes) {
            return ((__bytes + (size_t)_S_align - 1) & ~((size_t)_S_align - 1));
        }

        const _Obj* volatile* _M_get_free_list(size_t __bytes) throw();

        std::mutex& _M_get_mutex() throw();
      
        void* _M_refill(size_t __n);
      
        char* _M_allocate_chunk(size_t __n, int& __nobjs);
    };

    template<typename _Tp>
    class pool_alloc : private pool_alloc_base
    {
    private:
        static _Atomic_word _S_force_new;
    }
}

#endif // POOL_ALLOC_H
