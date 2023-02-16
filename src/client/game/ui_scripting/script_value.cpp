#include <std_include.hpp>
#include "execution.hpp"
#include "types.hpp"
#include "script_value.hpp"

namespace ui_scripting
{
	hks_object::hks_object(const game::hks::HksObject& value)
	{
		this->assign(value);
	}

	hks_object::hks_object(const hks_object& other) noexcept
	{
		this->operator=(other);
	}

	hks_object::hks_object(hks_object&& other) noexcept
	{
		this->operator=(std::move(other));
	}

	hks_object& hks_object::operator=(const hks_object& other) noexcept
	{
		if (this != &other)
		{
			this->release();
			this->assign(other.value_);
		}

		return *this;
	}

	hks_object& hks_object::operator=(hks_object&& other) noexcept
	{
		if (this != &other)
		{
			this->release();
			this->value_ = other.value_;
			other.value_.t = game::hks::TNONE;
		}

		return *this;
	}

	hks_object::~hks_object()
	{
		this->release();
	}

	const game::hks::HksObject& hks_object::get() const
	{
		return this->value_;
	}

	void hks_object::assign(const game::hks::HksObject& value)
	{
		this->value_ = value;

		const auto state = *game::hks::lua_state;
		const auto top = state->m_apistack.top;

		push_value(this->value_);
		this->ref_ = game::hks::hksi_luaL_ref(*game::hks::lua_state, -10000);
		state->m_apistack.top = top;
	}

	void hks_object::release()
	{
		if (this->ref_)
		{
			game::hks::hksi_luaL_unref(*game::hks::lua_state, -10000, this->ref_);
			this->value_.t = game::hks::TNONE;
		}
	}

	/***************************************************************
	 * Constructors
	 **************************************************************/

	script_value::script_value(const game::hks::HksObject& value)
		: value_(value)
	{
	}

	script_value::script_value(const int value)
	{
		game::hks::HksObject obj{};
		obj.t = game::hks::TNUMBER;
		obj.v.number = static_cast<float>(value);

		this->value_ = obj;
	}

	script_value::script_value(const unsigned int value)
	{
		game::hks::HksObject obj{};
		obj.t = game::hks::TNUMBER;
		obj.v.number = static_cast<float>(value);

		this->value_ = obj;
	}

	script_value::script_value(const bool value)
	{
		game::hks::HksObject obj{};
		obj.t = game::hks::TBOOLEAN;
		obj.v.boolean = value;

		this->value_ = obj;
	}

	script_value::script_value(const float value)
	{
		game::hks::HksObject obj{};
		obj.t = game::hks::TNUMBER;
		obj.v.number = static_cast<float>(value);

		this->value_ = obj;
	}

	script_value::script_value(const double value)
		: script_value(static_cast<float>(value))
	{
	}

	script_value::script_value(const char* value, const std::size_t len)
	{
		game::hks::HksObject obj{};

		const auto state = *game::hks::lua_state;
		if (state == nullptr)
		{
			return;
		}

		const auto top = state->m_apistack.top;
		game::hks::hksi_lua_pushlstring(state, value, static_cast<std::uint32_t>(len));
		obj = state->m_apistack.top[-1];
		state->m_apistack.top = top;

		this->value_ = obj;
	}

	script_value::script_value(const char* value)
		: script_value(value, std::strlen(value))
	{
	}

	script_value::script_value(const std::string& value)
		: script_value(value.data(), value.size())
	{
	}

	script_value::script_value(const lightuserdata& value)
	{
		game::hks::HksObject obj{};
		obj.t = game::hks::TLIGHTUSERDATA;
		obj.v.ptr = value.ptr;

		this->value_ = obj;
	}

	script_value::script_value(const userdata& value)
	{
		game::hks::HksObject obj{};
		obj.t = game::hks::TUSERDATA;
		obj.v.ptr = value.ptr;

		this->value_ = obj;
	}

	script_value::script_value(const table& value)
	{
		game::hks::HksObject obj{};
		obj.t = game::hks::TTABLE;
		obj.v.ptr = value.ptr;

		this->value_ = obj;
	}

	script_value::script_value(const function& value)
	{
		game::hks::HksObject obj{};
		obj.t = value.type;
		obj.v.ptr = value.ptr;

		this->value_ = obj;
	}

	/***************************************************************
	 * Integer
	 **************************************************************/

	template <>
	bool script_value::is<int>() const
	{
		const auto number = this->get_raw().v.number;
		return this->get_raw().t == game::hks::TNUMBER && static_cast<int>(number) == number;
	}

	template <>
	bool script_value::is<unsigned int>() const
	{
		return this->is<int>();
	}

	template <>
	int script_value::get() const
	{
		return static_cast<int>(this->get_raw().v.number);
	}

	template <>
	unsigned int script_value::get() const
	{
		return static_cast<unsigned int>(this->get_raw().v.number);
	}

	/***************************************************************
	 * Boolean
	 **************************************************************/

	template <>
	bool script_value::is<bool>() const
	{
		return this->get_raw().t == game::hks::TBOOLEAN;
	}

	template <>
	bool script_value::get() const
	{
		return this->get_raw().v.boolean;
	}

	/***************************************************************
	 * Float
	 **************************************************************/

	template <>
	bool script_value::is<float>() const
	{
		return this->get_raw().t == game::hks::TNUMBER;
	}

	template <>
	bool script_value::is<double>() const
	{
		return this->is<float>();
	}

	template <>
	float script_value::get() const
	{
		return this->get_raw().v.number;
	}

	template <>
	double script_value::get() const
	{
		return this->get_raw().v.number;
	}

	/***************************************************************
	 * String
	 **************************************************************/

	template <>
	bool script_value::is<const char*>() const
	{
		return this->get_raw().t == game::hks::TSTRING;
	}

	template <>
	bool script_value::is<std::string>() const
	{
		return this->is<const char*>();
	}

	template <>
	const char* script_value::get() const
	{
		return this->get_raw().v.str->m_data;
	}

	template <>
	std::string script_value::get() const
	{
		return this->get<const char*>();
	}

	/***************************************************************
	 * Lightuserdata
	 **************************************************************/

	template <>
	bool script_value::is<lightuserdata>() const
	{
		return this->get_raw().t == game::hks::TLIGHTUSERDATA;
	}

	template <>
	lightuserdata script_value::get() const
	{
		return this->get_raw().v.ptr;
	}

	/***************************************************************
	 * Userdata
	 **************************************************************/

	template <>
	bool script_value::is<userdata>() const
	{
		return this->get_raw().t == game::hks::TUSERDATA;
	}

	template <>
	userdata script_value::get() const
	{
		return this->get_raw().v.ptr;
	}

	/***************************************************************
	 * Table
	 **************************************************************/

	template <>
	bool script_value::is<table>() const
	{
		return this->get_raw().t == game::hks::TTABLE;
	}

	template <>
	table script_value::get() const
	{
		return this->get_raw().v.table;
	}

	/***************************************************************
	 * Function
	 **************************************************************/

	template <>
	bool script_value::is<function>() const
	{
		return this->get_raw().t == game::hks::TIFUNCTION
			|| this->get_raw().t == game::hks::TCFUNCTION;
	}

	template <>
	function script_value::get() const
	{
		return { this->get_raw().v.cClosure, this->get_raw().t };
	}

	/***************************************************************
	 *
	 **************************************************************/

	const game::hks::HksObject& script_value::get_raw() const
	{
		return this->value_.get();
	}

	bool script_value::operator==(const script_value& other) const
	{
		if (this->get_raw().t != other.get_raw().t)
		{
			return false;
		}

		if (this->get_raw().t == game::hks::TSTRING)
		{
			return this->get<std::string>() == other.get<std::string>();
		}

		return this->get_raw().v.native == other.get_raw().v.native;
	}

	arguments script_value::operator()() const
	{
		return this->as<function>()();
	}

	arguments script_value::operator()(const arguments& arguments) const
	{
		return this->as<function>()(arguments);
	}

	function_argument::function_argument(const arguments& args, const script_value& value, const int index)
		: values_(args), value_(value), index_(index)
	{
	}

	function_arguments::function_arguments(const arguments& values)
		: values_(values)
	{
	}
}
