#pragma once
#include "game/game.hpp"

#include <utils/string.hpp>

namespace ui_scripting
{
	class lightuserdata;
	class userdata_value;
	class userdata;
	class table_value;
	class table;
	class function;
	class script_value;

	template <typename T>
	std::string get_typename()
	{
		auto& info = typeid(T);

		if (info == typeid(std::string) ||
			info == typeid(const char*))
		{
			return "string";
		}

		if (info == typeid(lightuserdata))
		{
			return "lightuserdata";
		}

		if (info == typeid(userdata))
		{
			return "userdata";
		}

		if (info == typeid(table))
		{
			return "table";
		}

		if (info == typeid(function))
		{
			return "function";
		}

		if (info == typeid(int) ||
			info == typeid(float) ||
			info == typeid(unsigned int))
		{
			return "number";
		}

		if (info == typeid(bool))
		{
			return "boolean";
		}

		return info.name();
	}

	class hks_object
	{
	public:
		hks_object() = default;
		hks_object(const game::hks::HksObject& value);
		hks_object(const hks_object& other) noexcept;
		hks_object(hks_object&& other) noexcept;

		hks_object& operator=(const hks_object& other) noexcept;
		hks_object& operator=(hks_object&& other) noexcept;

		~hks_object();

		const game::hks::HksObject& get() const;

	private:
		void assign(const game::hks::HksObject& value);
		void release();

		game::hks::HksObject value_{game::hks::TNONE, {}};
		int ref_{};
	};

	using arguments = std::vector<script_value>;
	using event_arguments = std::unordered_map<std::string, script_value>;

	class script_value
	{
	public:
		script_value() = default;
		script_value(const game::hks::HksObject& value);

		script_value(int value);
		script_value(unsigned int value);
		script_value(bool value);

		script_value(float value);
		script_value(double value);

		script_value(const char* value, std::size_t len);
		script_value(const char* value);
		script_value(const std::string& value);

		script_value(const lightuserdata& value);
		script_value(const userdata& value);
		script_value(const table& value);
		script_value(const function& value);

		template <template<class, class> class C, class T, typename TableType = table>
		script_value(const C<T, std::allocator<T>>& container)
		{
			TableType table_{};
			int index = 1;

			for (const auto& value : container)
			{
				table_.set(index++, value);
			}

			game::hks::HksObject obj{};
			obj.t = game::hks::TTABLE;
			obj.v.ptr = table_.ptr;

			this->value_ = obj;
		}

		template <typename F>
		script_value(F f)
			: script_value(function(f))
		{
		}

		bool operator==(const script_value& other) const;

		[[maybe_unused]] arguments operator()() const;
		[[maybe_unused]] arguments operator()(const arguments& arguments) const;

		template <class ...T>
		[[maybe_unused]] arguments operator()(T ... arguments) const
		{
			return this->as<function>().call({arguments...});
		}

		template <size_t Size>
		table_value operator[](const char (&key)[Size]) const
		{
			return {this->as<table>(), key};
		}

		template <typename T = script_value>
		table_value operator[](const T& key) const
		{
			return {this->as<table>(), key};
		}

		template <typename T>
		[[nodiscard]] bool is() const;

		template <typename T>
		T as() const
		{
			if (!this->is<T>())
			{
				const auto hks_typename = game::hks::s_compilerTypeName[this->get_raw().t + 2];
				const auto typename_ = get_typename<T>();

				throw std::runtime_error(utils::string::va("%s expected, got %s", typename_.data(), hks_typename));
			}

			return get<T>();
		}

		template <typename T>
		operator T() const
		{
			return this->as<T>();
		}

		[[nodiscard]] const game::hks::HksObject& get_raw() const;

		hks_object value_{};

	private:
		template <typename T>
		T get() const;
	};

	class variadic_args : public arguments
	{
	};

	class function_argument
	{
	public:
		function_argument(const arguments& args, const script_value& value, const int index);

		template <typename T>
		T as() const
		{
			try
			{
				return this->value_.as<T>();
			}
			catch (const std::exception& e)
			{
				throw std::runtime_error(utils::string::va("bad argument #%d (%s)", this->index_ + 1, e.what()));
			}
		}

		template <>
		variadic_args as() const
		{
			variadic_args args{};
			for (auto i = this->index_; i < this->values_.size(); i++)
			{
				args.push_back(this->values_[i]);
			}
			return args;
		}

		template <typename T>
		operator T() const
		{
			return this->as<T>();
		}

	private:
		arguments values_{};
		script_value value_{};
		int index_{};
	};

	class function_arguments
	{
	public:
		function_arguments(const arguments& values);

		function_argument operator[](const int index) const
		{
			if (static_cast<std::size_t>(index) >= values_.size())
			{
				return {values_, {}, index};
			}

			return {values_, values_[index], index};
		}

	private:
		arguments values_{};
	};
}
