#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/nt.hpp>
#include <game/game.hpp>
#include <utils/io.hpp>
#include <utils/flags.hpp>
#include <filesystem>
#include <vector>

namespace plugins
{
    struct component final : generic_component
    {
        component()
        {
            namespace fs = std::filesystem;

            clear_log();


            const auto is_server = utils::flags::has_flag("dedicated");

            if (!is_server && !utils::flags::has_flag("plugins"))
            {
                log("Plugin loading is disabled. Enable the Plugins option in the launcher to load plugins.");
                return;
            }

            fs::path user_plugins = game::get_appdata_path() / "plugins";
            load_plugins_from(user_plugins);

            fs::path game_plugins = fs::path("boiii") / "plugins";
            load_plugins_from(game_plugins);

            log("Loaded " + std::to_string(extensions_.size()) + " plugin(s)");
        }

        ~component() override
        {
            log("Unloading " + std::to_string(extensions_.size()) + " plugin(s)...");
            for (auto& lib : extensions_)
                lib.free();
        }

        void post_load() override
        {
            for (auto& lib : extensions_)
            {
                try
                {
                    lib.invoke<void>("post_load");
                }
                catch (...) {} // Plugin might not have this function
            }
        }

        void post_unpack() override
        {
            for (auto& lib : extensions_)
            {
                try
                {
                    lib.invoke<void>("post_unpack");
                }
                catch (...) {} // Plugin might not have this function
            }
        }

        void pre_destroy() override
        {
            for (auto& lib : extensions_)
            {
                try
                {
                    lib.invoke<void>("pre_destroy");
                }
                catch (...) {} // Plugin might not have this function
            }
        }

    private:
        std::vector<utils::nt::library> extensions_{};
        std::filesystem::path log_path_;

        void clear_log()
        {
            if (log_path_.empty())
            {
                log_path_ = std::filesystem::path("boiii_players") / "plugins.log";
            }

            if (std::filesystem::exists(log_path_))
            {
                std::filesystem::remove(log_path_);
            }
        }

        void log(const std::string& message)
        {
            if (log_path_.empty())
            {
                log_path_ = std::filesystem::path("boiii_players") / "plugins.log";
            }

            const auto timestamp = std::chrono::system_clock::now();
            const auto time_t = std::chrono::system_clock::to_time_t(timestamp);
            
            std::tm time_info{};
            localtime_s(&time_info, &time_t);
            
            char time_buffer[32];
            std::strftime(time_buffer, sizeof(time_buffer), "[%Y-%m-%d %H:%M:%S]", &time_info);
            
            const auto log_message = std::string(time_buffer) + " " + message + "\n";
            utils::io::write_file(log_path_.string(), log_message, true);
        }

        void load_plugins_from(const std::filesystem::path& folder)
        {
            if (!std::filesystem::exists(folder))
            {
                log("Folder not found: " + folder.string());
                return;
            }

            bool dll_found = false;

            for (const auto& file : std::filesystem::directory_iterator(folder))
            {
                if (file.path().extension() == ".dll")
                {
                    dll_found = true;

                    try
                    {
                        utils::nt::library lib = utils::nt::library::load(file.path());
                        const char* plugin_name = nullptr;
                        std::string fallback_name;

                        try
                        {
                            plugin_name = lib.invoke<const char*>("p_name");
                        }
                        catch (...)
                        {
                            fallback_name = file.path().filename().string();
                            plugin_name = fallback_name.c_str();
                        }

                        extensions_.push_back(std::move(lib));
                        log("Loaded: " + std::string(plugin_name));
                    }
                    catch (const std::exception& e)
                    {
                        log("Failed to load " + file.path().filename().string() + ": " + e.what());
                    }
                    catch (...)
                    {
                        log("Failed to load " + file.path().filename().string() + ": Unknown error");
                    }
                }
            }

            if (!dll_found)
            {
                log("No plugins found in folder: " + folder.string());
            }
        }
    };
}

REGISTER_COMPONENT(plugins::component)