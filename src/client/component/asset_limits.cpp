#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"

#include <utils/hook.hpp>
#include <utils/io.hpp>

#include <rapidjson/document.h>

namespace asset_limits
{
	namespace
	{
		bool is_enabled()
		{
			const auto path = std::filesystem::path("boiii_players") / "user" / "launcher_settings.json";
			std::string data;
			if (utils::io::read_file(path.string(), &data) && !data.empty())
			{
				rapidjson::Document doc;
				if (!doc.Parse(data.c_str()).HasParseError() && doc.IsObject())
				{
					auto it = doc.FindMember("asset_limits_enabled");
					if (it != doc.MemberEnd() && it->value.IsString())
					{
						return std::string(it->value.GetString()) == "1";
					}
				}
			}
			// Default: enabled
			return true;
		}

		void reallocate_asset_pool(const game::XAssetType type, const unsigned int new_size)
		{
			const auto entry_size = game::DB_GetXAssetTypeSize(type);
			auto* pool = &game::DB_XAssetPool[type];

			const auto new_pool = calloc(new_size, entry_size);
			if (!new_pool)
			{
				printf("Failed to allocate asset pool for type %d (size: %u)\n",
				       static_cast<int>(type), new_size);
				return;
			}

			// Copy existing entries
			memcpy(new_pool, pool->pool,
			       pool->itemAllocCount * static_cast<size_t>(entry_size));

			// Rebuild free list for new entries
			pool->freeHead = reinterpret_cast<game::AssetLink*>(
				static_cast<char*>(new_pool) +
				static_cast<size_t>(entry_size) * pool->itemAllocCount);

			for (auto i = pool->itemAllocCount; i < static_cast<int>(new_size) - 1; i++)
			{
				auto* current = reinterpret_cast<game::AssetLink*>(
					static_cast<char*>(new_pool) + static_cast<size_t>(entry_size) * i);
				current->next = reinterpret_cast<game::AssetLink*>(
					static_cast<char*>(new_pool) + static_cast<size_t>(entry_size) * (i + 1));
			}

			// Last entry points to null
			auto* last = reinterpret_cast<game::AssetLink*>(
				static_cast<char*>(new_pool) +
				static_cast<size_t>(entry_size) * (new_size - 1));
			last->next = nullptr;

			pool->pool = new_pool;
			pool->itemAllocCount = static_cast<int>(new_size);

			printf("Reallocated asset pool type %d: %d -> %u entries\n",
			       static_cast<int>(type), pool->itemCount, new_size);
		}
	}

	class component final : public client_component
	{
	public:
		void post_unpack() override
		{
			if (!is_enabled())
			{
				printf("Asset pool expansion disabled by user settings\n");
				return;
			}

			// Increase limits for commonly exhausted pools
			reallocate_asset_pool(game::ASSET_TYPE_XMODEL, 2048);
			reallocate_asset_pool(game::ASSET_TYPE_IMAGE, 8192);
			reallocate_asset_pool(game::ASSET_TYPE_MATERIAL, 8192);
			reallocate_asset_pool(game::ASSET_TYPE_STRINGTABLE, 128);
			reallocate_asset_pool(game::ASSET_TYPE_RAWFILE, 2048);
			reallocate_asset_pool(game::ASSET_TYPE_SCRIPTPARSETREE, 2048);
		}
	};
}

REGISTER_COMPONENT(asset_limits::component)
