#pragma once

#include <string>
#include "nt.hpp"

namespace utils::image
{
	struct image
	{
		size_t width;
		size_t height;
		std::string data;
	};

	class object
	{
	public:
		object() = default;

		object(const HGDIOBJ h)
			: handle_(h)
		{
		}

		~object()
		{
			if (*this)
			{
				DeleteObject(this->handle_);
				this->handle_ = nullptr;
			}
		}

		object(const object&) = delete;
		object& operator=(const object&) = delete;

		object(object&& obj) noexcept
			: object()
		{
			this->operator=(std::move(obj));
		}

		object& operator=(object&& obj) noexcept
		{
			if (this != &obj)
			{
				this->~object();
				this->handle_ = obj.handle_;
				obj.handle_ = nullptr;
			}

			return *this;
		}

		object& operator=(HANDLE h) noexcept
		{
			this->~object();
			this->handle_ = h;

			return *this;
		}

		HGDIOBJ get() const
		{
			return this->handle_;
		}

		operator bool() const
		{
			return this->handle_ != nullptr;
		}

		operator HGDIOBJ() const
		{
			return this->handle_;
		}

		operator LPARAM() const
		{
			return reinterpret_cast<LPARAM>(this->handle_);
		}

	private:
		HGDIOBJ handle_{nullptr};
	};

	image load_image(const std::string& data);
	object create_bitmap(const image& img);
}
