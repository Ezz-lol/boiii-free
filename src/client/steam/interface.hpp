#pragma once

#ifdef interface
#undef interface
#endif

namespace steam
{
	struct raw_steam_id final
	{
		unsigned int account_id : 32;
		unsigned int account_instance : 20;
		unsigned int account_type : 4;
		int universe : 8;
	};

	typedef union
	{
		raw_steam_id raw;
		unsigned long long bits;
	} steam_id;

#pragma pack( push, 1 )
	struct raw_game_id final
	{
		unsigned int app_id : 24;
		unsigned int type : 8;
		unsigned int mod_id : 32;
	};

	typedef union
	{
		raw_game_id raw;
		unsigned long long bits;
	} game_id;
#pragma pack( pop )

	class interface final
	{
	public:

		interface();
		interface(void* interface_ptr);

		operator bool() const;

		template <typename T, typename... Args>
		T invoke(const std::string& method_name, Args ... args)
		{
			if (!this->interface_ptr_)
			{
				throw std::runtime_error("Invalid interface pointer");
			}

			const auto method = this->find_method(method_name);
			if (!method)
			{
				throw std::runtime_error("Unable to find method: " + method_name);
			}

			return static_cast<T(__thiscall*)(void*, Args ...)>(method)(this->interface_ptr_, args...);
		}

		template <typename T, typename... Args>
		T invoke(const size_t table_entry, Args ... args)
		{
			if (!this->interface_ptr_)
			{
				throw std::runtime_error("Invalid interface pointer");
			}

			return static_cast<T(__thiscall*)(void*, Args ...)>((*this->interface_ptr_)[table_entry])(
				this->interface_ptr_, args...);
		}

	private:
		void*** interface_ptr_;
		std::unordered_map<std::string, void*> methods_;

		void* find_method(const std::string& name);
		void* search_method(const std::string& name);

		std::string analyze_method(const void* method_ptr);
	};
}
