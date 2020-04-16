#ifndef RPMALLOC_STL_H
#define RPMALLOC_STL_H

#include <deque>
#include <map>
#include <stack>
#include <vector>

#include "rpmalloc/allocator.h"

namespace rpm {

    template<typename T>
    using deque = std::deque<T, allocator<T>>;

    template<typename K, typename T, typename Comp = std::less<K>>
    using map = std::map<K, T, Comp, allocator<std::pair<const K, T>>>;
    
    template<typename T>
    using stack = std::stack<T, allocator<T>>;
    
    template<typename T>
    using vector = std::vector<T, allocator<T>>;

}

#endif // RPMALLOC_STL_H
