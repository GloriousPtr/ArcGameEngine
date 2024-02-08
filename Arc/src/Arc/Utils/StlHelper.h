#pragma once

#include <EASTL/hash_map.h>

namespace eastl
{
	template<typename K, typename V>
	inline V try_at(const eastl::hash_map<K, V>& map, K key, V defaultValue)
	{
		auto it = map.find(key);
		if (it != map.end())
			return it->second;
		return defaultValue;
	}
}
