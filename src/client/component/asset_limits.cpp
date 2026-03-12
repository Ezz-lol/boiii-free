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
		struct pool_config
		{
			game::XAssetType type;
			const char* setting_key;
			unsigned int default_size;
		};

		static const pool_config pool_configs[] = {
			{ game::ASSET_TYPE_XMODEL,           "ap_xmodel",          2048 },
			{ game::ASSET_TYPE_IMAGE,            "ap_image",           8192 },
			{ game::ASSET_TYPE_MATERIAL,         "ap_material",        8192 },
			{ game::ASSET_TYPE_XANIMPARTS,       "ap_xanim",           4096 },
			{ game::ASSET_TYPE_SOUND,            "ap_sound",           4096 },
			{ game::ASSET_TYPE_RAWFILE,          "ap_rawfile",         2048 },
			{ game::ASSET_TYPE_SCRIPTPARSETREE,  "ap_scriptparsetree", 2048 },
			{ game::ASSET_TYPE_STRINGTABLE,      "ap_stringtable",     128 },
			{ game::ASSET_TYPE_SCRIPTBUNDLE,     "ap_scriptbundle",    512 },
			{ game::ASSET_TYPE_LOCALIZE_ENTRY,   "ap_localize",        2048 },
			{ game::ASSET_TYPE_FX,               "ap_fx",              1024 },
			{ game::ASSET_TYPE_WEAPON,           "ap_weapon",          512 },
		};

		rapidjson::Document load_settings_doc()
		{
			rapidjson::Document doc;
			const auto path = std::filesystem::path("boiii_players") / "user" / "launcher_settings.json";
			std::string data;
			if (utils::io::read_file(path.string(), &data) && !data.empty())
			{
				if (doc.Parse(data.c_str()).HasParseError() || !doc.IsObject())
				{
					doc.SetObject();
				}
			}
			else
			{
				doc.SetObject();
			}
			return doc;
		}

		std::string get_setting(const rapidjson::Document& doc, const char* key)
		{
			auto it = doc.FindMember(key);
			if (it != doc.MemberEnd() && it->value.IsString())
			{
				return it->value.GetString();
			}
			return {};
		}

		bool is_enabled(const rapidjson::Document& doc)
		{
			// Check master enable flag
			const auto val = get_setting(doc, "asset_limits_enabled");
			if (!val.empty() && val != "1") return false;

			// Check disable_asset_pools flag (inverted)
			const auto disable_val = get_setting(doc, "disable_asset_pools");
			if (disable_val == "1") return false;

			return true;
		}

		unsigned int get_pool_size(const rapidjson::Document& doc, const pool_config& cfg)
		{
			const auto val = get_setting(doc, cfg.setting_key);
			if (!val.empty())
			{
				try
				{
					const auto parsed = std::stoul(val);
					if (parsed >= 32 && parsed <= 65536)
					{
						return static_cast<unsigned int>(parsed);
					}
				}
				catch (...) {}
			}
			return cfg.default_size;
		}

		void reallocate_asset_pool(const game::XAssetType type, const unsigned int new_size)
		{
			if (static_cast<int>(type) < 0 || type >= game::ASSET_TYPE_COUNT)
			{
				printf("[AssetLimits] Invalid asset type %d\n", static_cast<int>(type));
				return;
			}

			const auto entry_size = game::DB_GetXAssetTypeSize(type);
			if (entry_size <= 0)
			{
				printf("[AssetLimits] Invalid entry size for type %d\n", static_cast<int>(type));
				return;
			}

			auto* pool = &game::DB_XAssetPool[type];

			// Skip if pool already meets or exceeds requested size
			if (pool->itemAllocCount >= static_cast<int>(new_size))
			{
				return;
			}

			if (pool->itemCount > 0)
			{
				printf("[AssetLimits] Skipping pool resize for type %d (in-use count: %d)\n",
				       static_cast<int>(type), pool->itemCount);
				return;
			}

			const auto new_pool = calloc(new_size, entry_size);
			if (!new_pool)
			{
				printf("Failed to allocate asset pool for type %d (size: %u)\n",
				       static_cast<int>(type), new_size);
				return;
			}

			// Copy existing entries
			if (pool->pool && pool->itemAllocCount > 0)
			{
				memcpy(new_pool, pool->pool,
				       pool->itemAllocCount * static_cast<size_t>(entry_size));
			}

			// Rebuild free list for ALL entries
			pool->freeHead = reinterpret_cast<game::AssetLink*>(new_pool);

			for (auto i = 0; i < static_cast<int>(new_size) - 1; i++)
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

			const auto old_pool = pool->pool;
			const auto old_alloc = pool->itemAllocCount;
			pool->pool = new_pool;
			pool->itemAllocCount = static_cast<int>(new_size);

			if (old_pool)
			{
				free(old_pool);
			}

			printf("Reallocated asset pool type %d: %d -> %u entries\n",
			       static_cast<int>(type), old_alloc, new_size);
		}
	}

	class component final : public client_component
	{
	public:
		void post_unpack() override
		{
			const auto doc = load_settings_doc();

			if (!is_enabled(doc))
			{
				printf("Asset pool expansion disabled by user settings\n");
				return;
			}

			for (const auto& cfg : pool_configs)
			{
				const auto size = get_pool_size(doc, cfg);
				reallocate_asset_pool(cfg.type, size);
			}
		}
	};
}

REGISTER_COMPONENT(asset_limits::component)
