#pragma once

#include <gtl/phmap.hpp>

static constexpr size_t GTL_NODE_HASH_MAP_MAX_BUCKET_COUNT = 12;
template <typename K, typename V>
using concurrent_hash_map =
    gtl::parallel_node_hash_map<K, V, gtl::priv::hash_default_hash<K>,
                                gtl::priv::hash_default_eq<K>,
                                std::allocator<std::pair<const K, V>>,
                                GTL_NODE_HASH_MAP_MAX_BUCKET_COUNT, std::mutex>;
