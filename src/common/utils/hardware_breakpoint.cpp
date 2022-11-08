#include "hardware_breakpoint.hpp"
#include "thread.hpp"

namespace utils::hardware_breakpoint
{
	namespace
	{
		void set_bits(uint64_t& value, const uint32_t bit_index, const uint32_t bits, const uint64_t new_value)
		{
			const uint64_t range_mask = (1ull << bits) - 1ull;
			const uint64_t full_mask = ~(range_mask << bit_index);
			value = (value & full_mask) | (new_value << bit_index);
		}

		void validate_index(const uint32_t index)
		{
			if (index >= 4)
			{
				throw std::runtime_error("Invalid index");
			}
		}

		uint32_t translate_length(const uint32_t length)
		{
			if (length != 1 && length != 2 && length != 4)
			{
				throw std::runtime_error("Invalid length");
			}

			return length - 1;
		}

		class debug_context
		{
		public:
			debug_context(uint32_t thread_id)
				: handle_(thread_id, THREAD_SET_CONTEXT | THREAD_GET_CONTEXT)
			{
				if (!this->handle_)
				{
					throw std::runtime_error("Unable to access thread");
				}

				this->context_.ContextFlags = CONTEXT_DEBUG_REGISTERS;

				if (!GetThreadContext(this->handle_, &this->context_))
				{
					throw std::runtime_error("Unable to get thread context");
				}
			}

			~debug_context()
			{
				SetThreadContext(this->handle_, &this->context_);
			}

			debug_context(const debug_context&) = delete;
			debug_context& operator=(const debug_context&) = delete;

			debug_context(debug_context&& obj) noexcept = delete;
			debug_context& operator=(debug_context&& obj) noexcept = delete;

			CONTEXT* operator->()
			{
				return &this->context_;
			}

			operator CONTEXT&()
			{
				return this->context_;
			}

		private:
			thread::handle handle_;
			CONTEXT context_{};
		};

		uint32_t find_free_index(const CONTEXT& context)
		{
			for (uint32_t i = 0; i < 4; ++i)
			{
				if ((context.Dr7 & (1ull << (i << 1ull))) == 0)
				{
					return i;
				}
			}

			throw std::runtime_error("No free index");
		}
	}

	void set_branch_tracing(const bool enabled, CONTEXT& context)
	{
		set_bits(context.Dr7, 8, 1, enabled ? 1 : 0);
	}

	void set_branch_tracing(const bool enabled, const uint32_t thread_id)
	{
		debug_context context(thread_id);
		set_branch_tracing(enabled, context);
	}

	uint32_t activate(const uint64_t address, uint32_t length, const condition cond, CONTEXT& context)
	{
		const auto index = find_free_index(context);
		length = translate_length(length);

		(&context.Dr0)[index] = address;

		set_bits(context.Dr7, 16 + (index << 2ull), 2, cond);
		set_bits(context.Dr7, 18 + (index << 2ull), 2, length);
		set_bits(context.Dr7, index << 1ull, 1, 1);

		return index;
	}

	uint32_t activate(void* address, const uint32_t length, const condition cond, const uint32_t thread_id)
	{
		return activate(reinterpret_cast<uint64_t>(address), length, cond, thread_id);
	}

	uint32_t activate(const uint64_t address, const uint32_t length, const condition cond, const uint32_t thread_id)
	{
		debug_context context(thread_id);
		return activate(address, length, cond, context);
	}

	void deactivate_address(const uint64_t address, CONTEXT& context)
	{
		for (auto i = 0; i < 4; ++i)
		{
			if ((&context.Dr0)[i] == address)
			{
				deactivate(i, context);
			}
		}
	}

	void deactivate_address(void* address, const uint32_t thread_id)
	{
		return deactivate_address(reinterpret_cast<uint64_t>(address), thread_id);
	}

	void deactivate_address(const uint64_t address, const uint32_t thread_id)
	{
		debug_context context(thread_id);
		deactivate_address(address, context);
	}

	void deactivate(const uint32_t index, CONTEXT& context)
	{
		validate_index(index);
		set_bits(context.Dr7, index << 1ull, 1, 0);
	}

	void deactivate(const uint32_t index, const uint32_t thread_id)
	{
		debug_context context(thread_id);
		deactivate(index, context);
	}

	void deactivate_all(CONTEXT& context)
	{
		context.Dr7 = 0;
	}

	void deactivate_all(const uint32_t thread_id)
	{
		debug_context context(thread_id);
		deactivate_all(context);
	}
}
