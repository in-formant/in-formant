#ifndef RPCXX_H
#define RPCXX_H

#if USING_RPMALLOC
#  include "rpmalloc.h"
#endif

#include "new_allocator.h"

#ifndef __APPLE__
#  include "aligned_new_allocator.h"
#endif

#include <cstddef>

#include <vector>
#include <deque>
#include <list>
#include <set>
#include <map>
#include <unordered_map>

namespace rpm {

#ifndef __APPLE__
    template<typename T>
    using allocator = rpm::aligned_new_allocator<T>;
#else
    template<typename T>
    using allocator = rpm::new_allocator<T>;
#endif

    template<typename T>
    using vector = std::vector<T, rpm::allocator<T>>;

    template<typename T>
    using deque = std::deque<T, rpm::allocator<T>>;

    template<typename T>
    using list = std::list<T, rpm::allocator<T>>;

    template<typename K, typename Compare = std::less<K>>
    using set = std::set<K, Compare, rpm::allocator<K>>;

    template<typename K, typename T, typename Compare = std::less<K>>
    using map = std::map<K, T, Compare, rpm::allocator<std::pair<const K, T>>>;

    template<typename K, typename T, typename Hash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    using unordered_map = std::unordered_map<K, T, Hash, KeyEqual, rpm::allocator<std::pair<const K, T>>>;

}

#endif // RPCXX_H
