#pragma once

#include "structs.hpp"

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
			: address_(reinterpret_cast<T*>(address))
		{
		}

		T* get() const
		{
			return address_;
		}

		operator T* () const
		{
			return this->get();
		}

		T* operator->() const
		{
			return this->get();
		}

	private:
		T* address_;
	};

	// Global game definitions
	constexpr auto CMD_MAX_NESTING = 8;
}

#include "symbols.hpp"
