#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"

#include <utils/hook.hpp>

namespace asset_limits
{
	namespace
	{
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
