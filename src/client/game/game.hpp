#pragma once

#include "structs.hpp"
#include "loader/component_loader.hpp"

namespace game
{
#define Com_Error(code, fmt, ...) \
		Com_Error_(__FILE__, __LINE__, code, fmt, ##__VA_ARGS__)

	int Conbuf_CleanText(const char* source, char* target);
	game::eModes Com_SessionMode_GetMode();

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
			return reinterpret_cast<T*>(get_game_address(this->address_));
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

	// Global game definitions
	constexpr auto CMD_MAX_NESTING = 8;
}

#include "symbols.hpp"
