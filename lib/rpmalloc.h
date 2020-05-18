#ifndef RPMALLOC_STL_H
#define RPMALLOC_STL_H

#include <deque>
#include <set>
#include <map>
#include <stack>
#include <vector>

//#ifdef __EMSCRIPTEN__
// Emscripten doesn't support rpmalloc so we have to fallback to std::allocator.
template<typename T>
using allocator = std::allocator<T>;
//#else
//#include "rpmalloc/allocator.h"
//#endif

namespace rpm {

    template<typename T>
    using deque = std::deque<T, allocator<T>>;

    template<typename K, typename T, typename Comp = std::less<K>>
    using map = std::map<K, T, Comp, allocator<std::pair<const K, T>>>;
    
    template<typename K, typename Comp = std::less<K>>
    using multiset = std::multiset<K, Comp, allocator<K>>;

    template<typename T>
    using stack = std::stack<T, allocator<T>>;
    
    template<typename T>
    using vector = std::vector<T, allocator<T>>;

}

#endif // RPMALLOC_STL_H
