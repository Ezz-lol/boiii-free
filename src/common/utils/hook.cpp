#include "hook.hpp"
#include "string.hpp"

#include <MinHook.h>

namespace utils::hook
{
	namespace
	{
		[[maybe_unused]] class _
		{
		public:
			_()
			{
				if (MH_Initialize() != MH_OK)
				{
					throw std::runtime_error("Failed to initialize MinHook");
				}
			}

			~_()
			{
				MH_Uninitialize();
			}
		} __;
	}

	void assembler::pushad64()
	{
		this->push(rax);
		this->push(rcx);
		this->push(rdx);
		this->push(rbx);
		this->push(rsp);
		this->push(rbp);
		this->push(rsi);
		this->push(rdi);

		this->sub(rsp, 0x40);
	}

	void assembler::popad64()
	{
		this->add(rsp, 0x40);

		this->pop(rdi);
		this->pop(rsi);
		this->pop(rbp);
		this->pop(rsp);
		this->pop(rbx);
		this->pop(rdx);
		this->pop(rcx);
		this->pop(rax);
	}

	void assembler::prepare_stack_for_call()
	{
		const auto reserve_callee_space = this->newLabel();
		const auto stack_unaligned = this->newLabel();

		this->test(rsp, 0xF);
		this->jnz(stack_unaligned);

		this->sub(rsp, 0x8);
		this->push(rsp);

		this->push(rax);
		this->mov(rax, ptr(rsp, 8, 8));
		this->add(rax, 0x8);
		this->mov(ptr(rsp, 8, 8), rax);
		this->pop(rax);

		this->jmp(reserve_callee_space);

		this->bind(stack_unaligned);
		this->push(rsp);

		this->bind(reserve_callee_space);
		this->sub(rsp, 0x40);
	}

	void assembler::restore_stack_after_call()
	{
		this->lea(rsp, ptr(rsp, 0x40));
		this->pop(rsp);
	}

	asmjit::Error assembler::call(void* target)
	{
		return Assembler::call(size_t(target));
	}

	asmjit::Error assembler::jmp(void* target)
	{
		return Assembler::jmp(size_t(target));
	}

	detour::detour(const size_t place, void* target) : detour(reinterpret_cast<void*>(place), target)
	{
	}

	detour::detour(void* place, void* target)
	{
		this->create(place, target);
	}

	detour::~detour()
	{
		this->clear();
	}

	void detour::enable() const
	{
		MH_EnableHook(this->place_);
	}

	void detour::disable() const
	{
		MH_DisableHook(this->place_);
	}

	void detour::create(void* place, void* target)
	{
		this->clear();
		this->place_ = place;

		if (MH_CreateHook(this->place_, target, &this->original_) != MH_OK)
		{
			throw std::runtime_error(string::va("Unable to create hook at location: %p", this->place_));
		}

		this->enable();
	}

	void detour::create(const size_t place, void* target)
	{
		this->create(reinterpret_cast<void*>(place), target);
	}

	void detour::clear()
	{
		if (this->place_)
		{
			MH_RemoveHook(this->place_);
		}

		this->place_ = nullptr;
		this->original_ = nullptr;
	}

	void* detour::get_original() const
	{
		return this->original_;
	}

	bool iat(const nt::library& library, const std::string& target_library, const std::string& process, void* stub)
	{
		if (!library.is_valid()) return false;

		auto* const ptr = library.get_iat_entry(target_library, process);
		if (!ptr) return false;

		DWORD protect;
		VirtualProtect(ptr, sizeof(*ptr), PAGE_EXECUTE_READWRITE, &protect);

		*ptr = stub;

		VirtualProtect(ptr, sizeof(*ptr), protect, &protect);
		return true;
	}

	void nop(void* place, const size_t length)
	{
		DWORD old_protect{};
		VirtualProtect(place, length, PAGE_EXECUTE_READWRITE, &old_protect);

		std::memset(place, 0x90, length);

		VirtualProtect(place, length, old_protect, &old_protect);
		FlushInstructionCache(GetCurrentProcess(), place, length);
	}

	void nop(const size_t place, const size_t length)
	{
		nop(reinterpret_cast<void*>(place), length);
	}

	void copy(void* place, const void* data, const size_t length)
	{
		DWORD old_protect{};
		VirtualProtect(place, length, PAGE_EXECUTE_READWRITE, &old_protect);

		std::memmove(place, data, length);

		VirtualProtect(place, length, old_protect, &old_protect);
		FlushInstructionCache(GetCurrentProcess(), place, length);
	}

	void copy(const size_t place, const void* data, const size_t length)
	{
		copy(reinterpret_cast<void*>(place), data, length);
	}

	bool is_relatively_far(const void* pointer, const void* data, const int offset)
	{
		const int64_t diff = size_t(data) - (size_t(pointer) + offset);
		const auto small_diff = int32_t(diff);
		return diff != int64_t(small_diff);
	}

	void call(void* pointer, void* data)
	{
		if (is_relatively_far(pointer, data))
		{
			throw std::runtime_error("Too far away to create 32bit relative branch");
		}

		auto* patch_pointer = PBYTE(pointer);
		set<uint8_t>(patch_pointer, 0xE8);
		set<int32_t>(patch_pointer + 1, int32_t(size_t(data) - (size_t(pointer) + 5)));
	}

	void call(const size_t pointer, void* data)
	{
		return call(reinterpret_cast<void*>(pointer), data);
	}

	void call(const size_t pointer, const size_t data)
	{
		return call(pointer, reinterpret_cast<void*>(data));
	}

	void jump(void* pointer, void* data, const bool use_far)
	{
		static const unsigned char jump_data[] = {
			0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0xff, 0xe0
		};

		if (!use_far && is_relatively_far(pointer, data))
		{
			throw std::runtime_error("Too far away to create 32bit relative branch");
		}

		auto* patch_pointer = PBYTE(pointer);

		if (use_far)
		{
			copy(patch_pointer, jump_data, sizeof(jump_data));
			copy(patch_pointer + 2, &data, sizeof(data));
		}
		else
		{
			set<uint8_t>(patch_pointer, 0xE9);
			set<int32_t>(patch_pointer + 1, int32_t(size_t(data) - (size_t(pointer) + 5)));
		}
	}

	void jump(const size_t pointer, void* data, const bool use_far)
	{
		return jump(reinterpret_cast<void*>(pointer), data, use_far);
	}

	void jump(const size_t pointer, const size_t data, const bool use_far)
	{
		return jump(pointer, reinterpret_cast<void*>(data), use_far);
	}

	void* assemble(const std::function<void(assembler&)>& asm_function)
	{
		static asmjit::JitRuntime runtime;

		asmjit::CodeHolder code;
		code.init(runtime.environment());

		assembler a(&code);

		asm_function(a);

		void* result = nullptr;
		runtime.add(&result, &code);

		return result;
	}

	void inject(void* pointer, const void* data)
	{
		if (is_relatively_far(pointer, data, 4))
		{
			throw std::runtime_error("Too far away to create 32bit relative branch");
		}

		set<int32_t>(pointer, int32_t(size_t(data) - (size_t(pointer) + 4)));
	}

	void inject(const size_t pointer, const void* data)
	{
		return inject(reinterpret_cast<void*>(pointer), data);
	}

	void* follow_branch(void* address)
	{
		auto* const data = static_cast<uint8_t*>(address);
		if (*data != 0xE8 && *data != 0xE9)
		{
			throw std::runtime_error("No branch instruction found");
		}

		return extract<void*>(data + 1);
	}
}
