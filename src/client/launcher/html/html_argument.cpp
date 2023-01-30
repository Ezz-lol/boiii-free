#include <std_include.hpp>
#include "html_argument.hpp"

html_argument::html_argument(VARIANT* val)
	: html_argument(*val)
{
}

html_argument::html_argument(CComVariant val)
	: value_(std::move(val))
{
}

bool html_argument::is_empty() const
{
	return this->value_.vt == VT_EMPTY;
}

bool html_argument::is_string() const
{
	if (this->is_empty()) return false;
	return this->value_.vt == VT_BSTR;
}

bool html_argument::is_number() const
{
	if (this->is_empty()) return false;
	return this->value_.vt == VT_I4;
}

bool html_argument::is_bool() const
{
	if (this->is_empty()) return false;
	return this->value_.vt == VT_BOOL;
}

std::string html_argument::get_string() const
{
	if (!this->is_string()) return {};
	std::wstring wide_string(this->value_.bstrVal);
	return std::string(wide_string.begin(), wide_string.end());
}

int html_argument::get_number() const
{
	if (!this->is_number()) return 0;
	return this->value_.intVal;
}

bool html_argument::get_bool() const
{
	if (!this->is_bool()) return false;
	return this->value_.boolVal != FALSE;
}

void html_argument::copy_to(VARIANT& var) const
{
	if (this->is_empty())
	{
		VariantInit(&var);
	}
	else
	{
		(void)VariantCopy(&var, &this->value_);
	}
}

void html_argument::move_to(VARIANT* var)
{
	if (!var)
	{
		return;
	}

	VARIANT& src_var = this->value_;
	memcpy(var, &src_var, sizeof(*var));

	VariantInit(&this->value_);
}
