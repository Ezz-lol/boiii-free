#pragma once
#include "signature.hpp"

#include <asmjit/core/jitruntime.h>
#include <asmjit/x86/x86assembler.h>

using namespace asmjit::x86;

namespace utils::hook
{
	namespace detail
	{
		template <size_t Entries>
		std::vector<size_t(*)()> get_iota_functions()
		{
			if constexpr (Entries == 0)
			{
				std::vector<size_t(*)()> functions;
				return functions;
			}
			else
			{
				auto functions = get_iota_functions<Entries - 1>();
				functions.emplace_back([]()
				{
					return Entries - 1;
				});
				return functions;
			}
		}
	}

	// Gets the pointer to the entry in the v-table.
	// It seems otherwise impossible to get this.
	// This is ugly as fuck and only safely works on x64
	// Example:
	//   ID3D11Device* device = ...
	//   auto entry = get_vtable_entry(device, &ID3D11Device::CreateTexture2D);
	template <size_t Entries = 100, typename Class, typename T, typename... Args>
	void** get_vtable_entry(Class* obj, T (Class::* entry)(Args ...))
	{
		union
		{
			decltype(entry) func;
			void* pointer;
		};

		func = entry;

		auto iota_functions = detail::get_iota_functions<Entries>();
		auto* object = iota_functions.data();

		using fake_func = size_t(__thiscall*)(void* self);
		auto index = static_cast<fake_func>(pointer)(&object);

		void** obj_v_table = *reinterpret_cast<void***>(obj);
		return &obj_v_table[index];
	}

	class assembler : public Assembler
	{
	public:
		using Assembler::Assembler;
		using Assembler::call;
		using Assembler::jmp;

		void pushad64();
		void popad64();

		void prepare_stack_for_call();
		void restore_stack_after_call();

		template <typename T>
		void call_aligned(T&& target)
		{
			this->prepare_stack_for_call();
			this->call(std::forward<T>(target));
			this->restore_stack_after_call();
		}

		asmjit::Error call(void* target);
		asmjit::Error jmp(void* target);
	};

	class detour
	{
	public:
		detour();
		detour(void* place, void* target);
		detour(size_t place, void* target);
		~detour();

		detour(detour&& other) noexcept
		{
			this->operator=(std::move(other));
		}

		detour& operator=(detour&& other) noexcept
		{
			if (this != &other)
			{
				this->clear();

				this->place_ = other.place_;
				this->original_ = other.original_;
				this->moved_data_ = other.moved_data_;

				other.place_ = nullptr;
				other.original_ = nullptr;
				other.moved_data_ = {};
			}

			return *this;
		}

		detour(const detour&) = delete;
		detour& operator=(const detour&) = delete;

		void enable();
		void disable();

		void create(void* place, void* target);
		void create(size_t place, void* target);
		void clear();

		void move();

		void* get_place() const;

		template <typename T>
		T* get() const
		{
			return static_cast<T*>(this->get_original());
		}

		template <typename T = void, typename... Args>
		T invoke(Args ... args)
		{
			return static_cast<T(*)(Args ...)>(this->get_original())(args...);
		}

		[[nodiscard]] void* get_original() const;

	private:
		std::vector<uint8_t> moved_data_{};
		void* place_{};
		void* original_{};

		void un_move();
	};

	std::optional<std::pair<void*, void*>> iat(const nt::library& library, const std::string& target_library,
	                                           const std::string& process, void* stub);

	void nop(void* place, size_t length);
	void nop(size_t place, size_t length);

	void copy(void* place, const void* data, size_t length);
	void copy(size_t place, const void* data, size_t length);

	void copy_string(void* place, const char* str);
	void copy_string(size_t place, const char* str);

	bool is_relatively_far(const void* pointer, const void* data, int offset = 5);

	void call(void* pointer, void* data);
	void call(size_t pointer, void* data);
	void call(size_t pointer, size_t data);

	void jump(void* pointer, void* data, bool use_far = false, bool use_safe = false);
	void jump(size_t pointer, void* data, bool use_far = false, bool use_safe = false);
	void jump(size_t pointer, size_t data, bool use_far = false, bool use_safe = false);

	void* assemble(const std::function<void(assembler&)>& asm_function);

	void inject(void* pointer, const void* data);
	void inject(size_t pointer, const void* data);

	std::vector<uint8_t> move_hook(void* pointer);
	std::vector<uint8_t> move_hook(size_t pointer);

	template <typename T>
	T extract(void* address)
	{
		auto* const data = static_cast<uint8_t*>(address);
		const auto offset = *reinterpret_cast<int32_t*>(data);
		return reinterpret_cast<T>(data + offset + 4);
	}

	void* follow_branch(void* address);

	template <typename T>
	static void set(void* place, T value = false)
	{
		copy(place, &value, sizeof(value));
	}

	template <typename T>
	static void set(const size_t place, T value = false)
	{
		return set<T>(reinterpret_cast<void*>(place), value);
	}

	template <typename T, typename... Args>
	static T invoke(size_t func, Args ... args)
	{
		return reinterpret_cast<T(*)(Args ...)>(func)(args...);
	}

	template <typename T, typename... Args>
	static T invoke(void* func, Args ... args)
	{
		return static_cast<T(*)(Args ...)>(func)(args...);
	}
}
