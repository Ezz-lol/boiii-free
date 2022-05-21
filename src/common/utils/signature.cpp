#include "signature.hpp"
#include <thread>
#include <mutex>

#include <intrin.h>

namespace utils::hook
{
	void signature::load_pattern(const std::string& pattern)
	{
		this->mask_.clear();
		this->pattern_.clear();

		uint8_t nibble = 0;
		auto has_nibble = false;

		for (auto val : pattern)
		{
			if (val == ' ') continue;
			if (val == '?')
			{
				this->mask_.push_back(val);
				this->pattern_.push_back(0);
			}
			else
			{
				if ((val < '0' || val > '9') && (val < 'A' || val > 'F') && (val < 'a' || val > 'f'))
				{
					throw std::runtime_error("Invalid pattern");
				}

				char str[] = {val, 0};
				const auto current_nibble = static_cast<uint8_t>(strtol(str, nullptr, 16));

				if (!has_nibble)
				{
					has_nibble = true;
					nibble = current_nibble;
				}
				else
				{
					has_nibble = false;
					const uint8_t byte = current_nibble | (nibble << 4);

					this->mask_.push_back('x');
					this->pattern_.push_back(byte);
				}
			}
		}

		while (!this->mask_.empty() && this->mask_.back() == '?')
		{
			this->mask_.pop_back();
			this->pattern_.pop_back();
		}

		if (this->has_sse_support())
		{
			while (this->pattern_.size() < 16)
			{
				this->pattern_.push_back(0);
			}
		}

		if (has_nibble)
		{
			throw std::runtime_error("Invalid pattern");
		}
	}

	std::vector<size_t> signature::process_range(uint8_t* start, const size_t length) const
	{
		if (this->has_sse_support()) return this->process_range_vectorized(start, length);
		return this->process_range_linear(start, length);
	}

	std::vector<size_t> signature::process_range_linear(uint8_t* start, const size_t length) const
	{
		std::vector<size_t> result;

		for (size_t i = 0; i < length; ++i)
		{
			const auto address = start + i;

			size_t j = 0;
			for (; j < this->mask_.size(); ++j)
			{
				if (this->mask_[j] != '?' && this->pattern_[j] != address[j])
				{
					break;
				}
			}

			if (j == this->mask_.size())
			{
				result.push_back(size_t(address));
			}
		}

		return result;
	}

	std::vector<size_t> signature::process_range_vectorized(uint8_t* start, const size_t length) const
	{
		std::vector<size_t> result;
		__declspec(align(16)) char desired_mask[16] = {0};

		for (size_t i = 0; i < this->mask_.size(); i++)
		{
			desired_mask[i / 8] |= (this->mask_[i] == '?' ? 0 : 1) << i % 8;
		}

		const auto mask = _mm_load_si128(reinterpret_cast<const __m128i*>(desired_mask));
		const auto comparand = _mm_loadu_si128(reinterpret_cast<const __m128i*>(this->pattern_.data()));

		for (size_t i = 0; i < length; ++i)
		{
			const auto address = start + i;
			const auto value = _mm_loadu_si128(reinterpret_cast<const __m128i*>(address));
			const auto comparison = _mm_cmpestrm(value, 16, comparand, static_cast<int>(this->mask_.size()),
			                                     _SIDD_CMP_EQUAL_EACH);

			const auto matches = _mm_and_si128(mask, comparison);
			const auto equivalence = _mm_xor_si128(mask, matches);

			if (_mm_test_all_zeros(equivalence, equivalence))
			{
				result.push_back(size_t(address));
			}
		}

		return result;
	}

	signature::signature_result signature::process() const
	{
		const auto range = this->length_ - this->mask_.size();
		const auto cores = std::max(1u, std::thread::hardware_concurrency());

		if (range <= cores * 10ull) return this->process_serial();
		return this->process_parallel();
	}

	signature::signature_result signature::process_serial() const
	{
		const auto sub = this->has_sse_support() ? 16 : this->mask_.size();
		return {this->process_range(this->start_, this->length_ - sub)};
	}

	signature::signature_result signature::process_parallel() const
	{
		const auto sub = this->has_sse_support() ? 16 : this->mask_.size();
		const auto range = this->length_ - sub;
		const auto cores = std::max(1u, std::thread::hardware_concurrency() / 2);
		// Only use half of the available cores
		const auto grid = range / cores;

		std::mutex mutex;
		std::vector<size_t> result;
		std::vector<std::thread> threads;

		for (auto i = 0u; i < cores; ++i)
		{
			const auto start = this->start_ + (grid * i);
			const auto length = (i + 1 == cores) ? (this->start_ + this->length_ - sub) - start : grid;
			threads.emplace_back([&, start, length]()
			{
				auto local_result = this->process_range(start, length);
				if (local_result.empty()) return;

				std::lock_guard _(mutex);
				for (const auto& address : local_result)
				{
					result.push_back(address);
				}
			});
		}

		for (auto& t : threads)
		{
			if (t.joinable())
			{
				t.join();
			}
		}

		std::sort(result.begin(), result.end());
		return {std::move(result)};
	}

	bool signature::has_sse_support() const
	{
		if (this->mask_.size() <= 16)
		{
			int cpu_id[4];
			__cpuid(cpu_id, 0);

			if (cpu_id[0] >= 1)
			{
				__cpuidex(cpu_id, 1, 0);
				return (cpu_id[2] & (1 << 20)) != 0;
			}
		}

		return false;
	}
}

utils::hook::signature::signature_result operator"" _sig(const char* str, const size_t len)
{
	return utils::hook::signature(std::string(str, len)).process();
}
