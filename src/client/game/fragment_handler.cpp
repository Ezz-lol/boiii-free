#include <std_include.hpp>
#include "fragment_handler.hpp"

namespace game::fragment_handler
{
	namespace
	{
		constexpr size_t MAX_FRAGMENTS = 100;

		using fragments = std::unordered_map<size_t, std::string>;

		struct fragmented_packet
		{
			size_t fragment_count{0};
			fragments fragments{};
			std::chrono::high_resolution_clock::time_point insertion_time = std::chrono::high_resolution_clock::now();
		};

		using id_fragment_map = std::unordered_map<uint64_t, fragmented_packet>;
		using address_fragment_map = std::unordered_map<netadr_t, id_fragment_map>;

		utils::concurrency::container<address_fragment_map> global_map{};

		std::vector<std::string> construct_fragments(const void* data, const size_t length)
		{
			std::vector<std::string> fragments{};

			constexpr size_t max_fragment_size = 0x400;

			for (size_t i = 0; i < length; i += max_fragment_size)
			{
				const auto current_fragment_size = std::min(length - i, max_fragment_size);

				std::string fragment(static_cast<const char*>(data) + i, current_fragment_size);
				fragments.push_back(std::move(fragment));
			}

			return fragments;
		}
	}

	bool handle(const netadr_t& target, utils::byte_buffer& buffer, std::string& final_packet)
	{
		const auto fragment_id = buffer.read<uint64_t>();
		const size_t fragment_count = buffer.read<uint32_t>();
		const size_t fragment_index = buffer.read<uint32_t>();

		auto fragment_data = buffer.get_remaining_data();

		if (fragment_index > fragment_count || !fragment_count || fragment_count > MAX_FRAGMENTS)
		{
			return false;
		}

		return global_map.access<bool>([&](address_fragment_map& map)
		{
			auto& user_map = map[target];
			if (!user_map.contains(fragment_id) && user_map.size() > MAX_FRAGMENTS)
			{
				return false;
			}

			auto& packet_queue = user_map[fragment_id];

			if (packet_queue.fragment_count == 0)
			{
				packet_queue.fragment_count = fragment_count;
			}

			if (packet_queue.fragment_count != fragment_count)
			{
				return false;
			}

			if (packet_queue.fragments.size() + 1 < fragment_count)
			{
				packet_queue.fragments[fragment_index] = std::move(fragment_data);
				return false;
			}

			final_packet.clear();

			for (size_t i = 0; i < fragment_count; ++i)
			{
				if (i == fragment_index)
				{
					final_packet.append(fragment_data);
				}
				else
				{
					final_packet.append(packet_queue.fragments.at(i));
				}
			}

			return true;
		});
	}

	void clean()
	{
		global_map.access([](address_fragment_map& map)
		{
			for (auto i = map.begin(); i != map.end();)
			{
				auto& user_map = i->second;

				for (auto j = user_map.begin(); j != user_map.end();)
				{
					const auto now = std::chrono::high_resolution_clock::now();
					const auto diff = now - j->second.insertion_time;

					if (diff > 5s)
					{
						j = user_map.erase(j);
					}
					else
					{
						++j;
					}
				}

				if (user_map.empty())
				{
					i = map.erase(i);
				}
				else
				{
					++i;
				}
			}
		});
	}

	void fragment_handler::fragment_data(const void* data, const size_t size,
	                                     const std::function<void(const utils::byte_buffer& buffer)>& callback)
	{
		static std::atomic_uint64_t current_id{0};
		const auto id = current_id++;

		const auto fragments = construct_fragments(data, size);

		for (size_t i = 0; i < fragments.size(); ++i)
		{
			utils::byte_buffer buffer{};
			buffer.write(id);
			buffer.write(static_cast<uint32_t>(fragments.size()));
			buffer.write(static_cast<uint32_t>(i));

			auto& fragment = fragments.at(i);
			buffer.write(fragment.data(), fragment.size());

			callback(buffer);
		}
	}
}
