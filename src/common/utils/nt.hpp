#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// min and max is required by gdi, therefore NOMINMAX won't work
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#include <string>
#include <functional>
#include <filesystem>

namespace utils::nt
{
	class library final
	{
	public:
		static library load(const char* name);
		static library load(const std::string& name);
		static library load(const std::filesystem::path& path);
		static library get_by_address(const void* address);

		library();
		explicit library(const std::string& name);
		explicit library(HMODULE handle);

		library(const library& a) : module_(a.module_)
		{
		}

		bool operator!=(const library& obj) const { return !(*this == obj); };
		bool operator==(const library& obj) const;

		operator bool() const;
		operator HMODULE() const;

		void unprotect() const;
		[[nodiscard]] void* get_entry_point() const;
		[[nodiscard]] size_t get_relative_entry_point() const;

		[[nodiscard]] bool is_valid() const;
		[[nodiscard]] std::string get_name() const;
		[[nodiscard]] std::filesystem::path get_path() const;
		[[nodiscard]] std::filesystem::path get_folder() const;
		[[nodiscard]] std::uint8_t* get_ptr() const;
		void free();

		[[nodiscard]] HMODULE get_handle() const;

		template <typename T>
		[[nodiscard]] T get_proc(const std::string& process) const
		{
			if (!this->is_valid()) T{};
			return reinterpret_cast<T>(GetProcAddress(this->module_, process.data()));
		}

		template <typename T>
		[[nodiscard]] std::function<T> get(const std::string& process) const
		{
			if (!this->is_valid()) return std::function<T>();
			return static_cast<T*>(this->get_proc<void*>(process));
		}

		template <typename T, typename... Args>
		T invoke(const std::string& process, Args ... args) const
		{
			auto method = this->get<T(__cdecl)(Args ...)>(process);
			if (method) return method(args...);
			return T();
		}

		template <typename T, typename... Args>
		T invoke_pascal(const std::string& process, Args ... args) const
		{
			auto method = this->get<T(__stdcall)(Args ...)>(process);
			if (method) return method(args...);
			return T();
		}

		template <typename T, typename... Args>
		T invoke_this(const std::string& process, void* this_ptr, Args ... args) const
		{
			auto method = this->get<T(__thiscall)(void*, Args ...)>(this_ptr, process);
			if (method) return method(args...);
			return T();
		}

		[[nodiscard]] std::vector<PIMAGE_SECTION_HEADER> get_section_headers() const;

		[[nodiscard]] PIMAGE_NT_HEADERS get_nt_headers() const;
		[[nodiscard]] PIMAGE_DOS_HEADER get_dos_header() const;
		[[nodiscard]] PIMAGE_OPTIONAL_HEADER get_optional_header() const;

		[[nodiscard]] void** get_iat_entry(const std::string& module_name, const std::string& proc_name) const;

	private:
		HMODULE module_;
	};

	template <HANDLE InvalidHandle = nullptr>
	class handle
	{
	public:
		handle() = default;

		handle(const HANDLE h)
			: handle_(h)
		{
		}

		~handle()
		{
			if (*this)
			{
				CloseHandle(this->handle_);
				this->handle_ = InvalidHandle;
			}
		}

		handle(const handle&) = delete;
		handle& operator=(const handle&) = delete;

		handle(handle&& obj) noexcept
			: handle()
		{
			this->operator=(std::move(obj));
		}

		handle& operator=(handle&& obj) noexcept
		{
			if (this != &obj)
			{
				this->~handle();
				this->handle_ = obj.handle_;
				obj.handle_ = InvalidHandle;
			}

			return *this;
		}

		handle& operator=(HANDLE h) noexcept
		{
			this->~handle();
			this->handle_ = h;

			return *this;
		}

		operator bool() const
		{
			return this->handle_ != InvalidHandle;
		}

		operator HANDLE() const
		{
			return this->handle_;
		}

	private:
		HANDLE handle_{InvalidHandle};
	};

	bool is_wine();
	bool is_shutdown_in_progress();

	__declspec(noreturn) void raise_hard_exception();
	std::string load_resource(int id);

	void relaunch_self();
	__declspec(noreturn) void terminate(uint32_t code = 0);
}
