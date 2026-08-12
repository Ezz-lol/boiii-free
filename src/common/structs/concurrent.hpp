#pragma once

#include <gtl/phmap.hpp>

template <typename K, typename V>
using concurrent_hash_map = gtl::parallel_node_hash_map<
    K, V, gtl::priv::hash_default_hash<K>, gtl::priv::hash_default_eq<K>,
    std::allocator<std::pair<const K, V>>, 12, std::mutex>;