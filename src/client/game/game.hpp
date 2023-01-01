#pragma once

#include "structs.hpp"

namespace game
{
	size_t get_base();
	bool is_server();

	inline size_t relocate(const size_t val)
	{
		const auto base = get_base();
		return base + (val - 0x140000000);
	}

	inline size_t derelocate(const size_t val)
	{
		const auto base = get_base();
		return (val - base) + 0x140000000;
	}

	inline size_t derelocate(const void* val)
	{
		return derelocate(reinterpret_cast<size_t>(val));
	}

	template <typename T>
	class symbol
	{
	public:
		symbol(const size_t address)
			: address_(address)
		{
		}

		T* get() const
		{
			return reinterpret_cast<T*>(relocate(this->address_));
		}

		operator T*() const
		{
			return this->get();
		}

		T* operator->() const
		{
			return this->get();
		}

	private:
		size_t address_;
	};
}

inline size_t operator"" _g(const size_t val)
{
	return game::relocate(val);
}

#include "symbols.hpp"
