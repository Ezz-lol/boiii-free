
#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/utils.hpp"
#include <utils/io.hpp>
#include <utils/string.hpp>
#include <utils/thread.hpp>
#include <utils/flags.hpp>
#include <game/game.hpp>

namespace workshop_id
{
	namespace
	{
		std::thread workshop_id_wait{};
		std::atomic_bool running{true};

		int write_pubid_to_file(std::string pubID)
		{
			const std::string& config_path = std::filesystem::current_path().string() + "/zone/server_zm.cfg";

			std::vector<std::string> lines;
			bool line_exists = false;

			std::string line_to_find = "set workshop_id";
			std::string new_string = "set workshop_id \"" + pubID + "\"";

			std::ifstream inputFile(config_path);
			if (!inputFile.is_open())
			{
				printf("Error opening server_zm.cfg file.\n");
				return 1;
			}

			std::string line;
			while (std::getline(inputFile, line))
			{
				if (line == new_string)
				{
					printf("Correct dvar already exists.\n");
					return 0;
				}

				if (line.find(line_to_find) != std::string::npos)
				{
					printf("Line replaced with new workshop id in /zone/server_zm.cfg.\n");
					lines.push_back(new_string);
					line_exists = true;
				}
				else
				{
					lines.push_back(line);
				}
			}

			inputFile.close();

			if (!line_exists)
			{
				lines.push_back(new_string);
				printf("The 'workshop_id' dvar added in /zone/server_zm.cfg successfully.\n");
			}

			std::ofstream outputFile(config_path);
			if (!outputFile.is_open())
			{
				printf("Error opening or adding the workshop_id dvar to server_zm.cfg file.\n");
				return 1;
			}

			for (const std::string& updated_line : lines)
			{
				outputFile << updated_line << std::endl;
			}

			outputFile.close();
			MessageBox(NULL, "Loaded workshop item id found! \n\nWriting ID to zone/server_zm.cfg as workshop_id dvar. \n\nPlease restart the server to enable it.", "Info!", MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);

			return 0;
		}

		void read_json_for_id(std::filesystem::path path)
		{
			const auto json_str = utils::io::read_file(path);

			if (json_str.empty())
			{
				printf("[ WorkshopID ] workshop.json has not been found in folder \n");
				return;
			}

			rapidjson::Document doc;
			const rapidjson::ParseResult parse_result = doc.Parse(json_str);

			if (parse_result.IsError() || !doc.IsObject())
			{
				printf("[ WorkshopID ] Unable to parse workshop.json from folder \n");
				return;
			}

			if (!doc.HasMember("PublisherID"))
			{
				printf("[ WorkshopID ] PublisherID not found workshop.json is invalid \n");
				return;
			}

			std::string pubID = doc["PublisherID"].GetString();
			write_pubid_to_file(pubID);
		}

		bool read_json_for_folder_name(std::filesystem::path path, std::string mapname)
		{
			const auto json_str = utils::io::read_file(path);

			if (json_str.empty())
			{
				printf("[ FolderName ] workshop.json has not been found in folder \n");
				return false;
			}

			rapidjson::Document doc;
			const rapidjson::ParseResult parse_result = doc.Parse(json_str);

			if (parse_result.IsError() || !doc.IsObject())
			{
				printf("[ FolderName ] Unable to parse workshop.json from folder \n");
				return false;
			}

			if (!doc.HasMember("FolderName"))
			{
				printf("[ FolderName ] FolderName not found workshop.json is invalid \n");
				return false;
			}

			std::string folder_name = doc["FolderName"].GetString();
			if (mapname == folder_name)
			{
				return true;
			}

			return false;
		}

		void get_map_id_from_json()
		{
			Sleep(20000);//let the custom map load in server
			if (!running)
			{
				return;
			}

			const std::string loaded_mod_id = game::getPublisherIdFromLoadedMod();
			if (utils::string::is_numeric(loaded_mod_id))
			{
				printf(loaded_mod_id.c_str());
				write_pubid_to_file(loaded_mod_id);
			}
			else
			{
				const std::string& usermaps_path = std::filesystem::current_path().string() + "/usermaps";
				std::string mapname = game::get_dvar_string("mapname");

				for (const auto& entry : std::filesystem::directory_iterator(usermaps_path))
				{
					std::filesystem::path workshop_json = entry.path() / "workshop.json";
					std::filesystem::path workshop_json_zone = entry.path() / "zone/workshop.json";

					if (entry.is_directory() && entry.path().filename() == mapname.data())
					{
						if (std::filesystem::exists(workshop_json) && !std::filesystem::is_directory(workshop_json))
						{
							read_json_for_id(workshop_json);
							break;
						}

						if (std::filesystem::exists(workshop_json_zone) && !std::filesystem::is_directory(workshop_json_zone))
						{
							read_json_for_id(workshop_json_zone);
							break;
						}
					}
					else
					{
						if (std::filesystem::exists(workshop_json) && !std::filesystem::is_directory(workshop_json))
						{
							if (read_json_for_folder_name(workshop_json, mapname))
							{
								read_json_for_id(workshop_json);
								break;
							}
						}
						else if (std::filesystem::exists(workshop_json_zone) && !std::filesystem::is_directory(workshop_json_zone))
						{
							if (read_json_for_folder_name(workshop_json_zone, mapname))
							{
								read_json_for_id(workshop_json_zone);
								break;
							}
						}
					}
				}
			}
		}
	}

	class component final : public generic_component
	{
	public:

		void pre_destroy() override
		{
			running = false;
			if (workshop_id_wait.joinable())
			{
				workshop_id_wait.join();
			}
		}

		void post_unpack() override
		{
			//currently only for zombies and custom maps
			//people can set the workshop_id dvar manually in zone/server_zm.cfg

			if (utils::flags::has_flag("dedicated") && game::Com_SessionMode_GetMode() == game::MODE_ZOMBIES)
			{
				workshop_id_wait = utils::thread::create_named_thread("workshop_id_wait", get_map_id_from_json);
				workshop_id_wait.detach();
			}
		}
	};
}

REGISTER_COMPONENT(workshop_id::component)
