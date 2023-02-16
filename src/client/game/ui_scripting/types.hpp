#pragma once
#include "game/game.hpp"
#include "script_value.hpp"

#include "component/ui_scripting.hpp"

namespace ui_scripting
{
	class lightuserdata
	{
	public:
		lightuserdata(void*);
		void* ptr;
	};

	class userdata_value;

	class userdata
	{
	public:
		userdata(void*);

		userdata(const userdata& other);
		userdata(userdata&& other) noexcept;

		~userdata();

		userdata& operator=(const userdata& other);
		userdata& operator=(userdata&& other) noexcept;

		script_value get(const script_value& key) const;
		void set(const script_value& key, const script_value& value) const;

		userdata_value operator[](const script_value& key) const;

		void* ptr;

	private:
		void add();
		void release();

		int ref{};
	};

	class userdata_value : public script_value
	{
	public:
		userdata_value(const userdata& table, const script_value& key);
		void operator=(const script_value& value);
		bool operator==(const script_value& value);
	private:
		userdata userdata_;
		script_value key_;
	};

	class table_value;

	class table
	{
	public:
		table();
		table(game::hks::HashTable* ptr_);

		table(const table& other);
		table(table&& other) noexcept;

		~table();

		table& operator=(const table& other);
		table& operator=(table&& other) noexcept;

		[[nodiscard]] script_value get(const script_value& key) const;
		void set(const script_value& key, const script_value& value) const;

		table_value operator[](const script_value& key) const;

		game::hks::HashTable* ptr;

	private:
		void add();
		void release();

		int ref{};
	};

	class table_value : public script_value
	{
	public:
		table_value(const table& table, const script_value& key);
		void operator=(const script_value& value);
		void operator=(const table_value& value);
		bool operator==(const script_value& value);
		bool operator==(const table_value& value);
	private:
		table table_;
		script_value key_;
	};


	class function
	{
	public:
		function(game::hks::lua_function);
		function(game::hks::cclosure*, game::hks::HksObjectType);

		template <typename F>
		function(F f)
		{
			this->ptr = ui_scripting::convert_function(f);
			this->type = game::hks::TCFUNCTION;
		}

		function(const function& other);
		function(function&& other) noexcept;

		~function();

		function& operator=(const function& other);
		function& operator=(function&& other) noexcept;

		arguments call(const arguments& arguments) const;

		arguments operator()(const arguments& arguments) const;

		template<class ...T>
		arguments operator()(T... arguments) const
		{
			return this->call({ arguments... });
		}

		arguments operator()() const;

		game::hks::cclosure* ptr;
		game::hks::HksObjectType type;

	private:
		void add();
		void release();

		int ref{};
	};
}
