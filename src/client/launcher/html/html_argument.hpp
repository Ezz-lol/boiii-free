#pragma once

#pragma once

class html_argument final
{
public:
	html_argument() = default;
	html_argument(VARIANT* val);
	html_argument(CComVariant val);

	bool is_empty() const;

	bool is_string() const;
	bool is_number() const;
	bool is_bool() const;

	std::string get_string() const;
	int get_number() const;
	bool get_bool() const;

	const CComVariant& get() const
	{
		return this->value_;
	}

	void copy_to(VARIANT& var) const;
	void move_to(VARIANT* var);

private:
	CComVariant value_{};
};
