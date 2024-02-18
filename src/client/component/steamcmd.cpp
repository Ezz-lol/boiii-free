#include <std_include.hpp>
#include <utils/io.hpp>
#include "game/game.hpp"
#include "workshop.hpp"
#include "steamcmd.hpp"
#include <curl/curl.h>
#include "unzip.h"
#include <zlib.h>
#include <zip.h>

namespace steamcmd
{
    int start_new_process(const char* exePath, bool Hide_Window, bool waittill_done, const char* arguments)
	{
		std::string commandLine = std::string(exePath) + " " + std::string(arguments);

		STARTUPINFO startupInfo;
		PROCESS_INFORMATION pi;

		ZeroMemory(&startupInfo, sizeof(startupInfo));
		startupInfo.cb = sizeof(startupInfo);
		if (Hide_Window)
		{
			startupInfo.dwFlags |= STARTF_USESHOWWINDOW;
			startupInfo.wShowWindow = SW_HIDE;
		}

		if (CreateProcess(
			NULL,
			(LPSTR)commandLine.c_str(),
			NULL,
			NULL,
			FALSE,
			0,
			NULL,
			NULL,
			&startupInfo,
			&pi
		))
		{
			if (waittill_done)
			{
				WaitForSingleObject(pi.hProcess, INFINITE);
			}

			DWORD exitCode;
			if (GetExitCodeProcess(pi.hProcess, &exitCode))
			{
				if (exitCode == -1073741510)
				{
					return 1;
				}

				if (exitCode == 0)
				{
					printf("SteamCMD process completed successfully.\n");
				}
				else
				{
					printf("SteamCMD process exited with code: %ld\n", exitCode);
				}
			}
			else
			{
				printf("Error - %lu\n", GetLastError());
			}

			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
		else
		{
			printf("Error - %lu\n", GetLastError());
		}

		return 0;
	}
	
	int extract_steamcmd()
	{
		const char* zipFileName = "steamcmd.zip"; 
		const char* extractDir = "";

		unzFile zipFile = unzOpen(zipFileName);
		if (zipFile == nullptr)
		{
			printf("Failed to open ZIP file: %s\n", zipFileName);
			return 1;
		}

		if (unzGoToFirstFile(zipFile) != UNZ_OK)
		{
			printf("No files found in the ZIP archive.\n");
			unzClose(zipFile);
			return 1;
		}

		do
		{
			char fileName[256];
			unz_file_info fileInfo;
			if (unzGetCurrentFileInfo(zipFile, &fileInfo, fileName, sizeof(fileName), nullptr, 0, nullptr, 0) != UNZ_OK)
			{
				printf("Failed to get file info.\n");
				unzClose(zipFile);
				return 1;
			}

			printf("Extracting: %s\n", fileName);

			if (unzOpenCurrentFile(zipFile) != UNZ_OK)
			{
				printf("Failed to open file inside ZIP: %s\n", fileName);
				unzClose(zipFile);
				return 1;
			}

			char buffer[1024];
			FILE* outFile = nullptr;
			if (extractDir[0] != '\0')
			{
				char outputPath[512];
				snprintf(outputPath, sizeof(outputPath), "%s/%s", extractDir, fileName);

				if (fopen_s(&outFile, outputPath, "wb") != 0)
				{
					printf("Failed to open output file: %s\n", outputPath);
					unzCloseCurrentFile(zipFile);
					unzClose(zipFile);
					return 1;
				}
			}
			else
			{
				if (fopen_s(&outFile, fileName, "wb") != 0)
				{
					printf("Failed to open output file: %s\n", fileName);
					unzCloseCurrentFile(zipFile);
					unzClose(zipFile);
					return 1;
				}
			}

			int bytesRead;
			do
			{
				bytesRead = unzReadCurrentFile(zipFile, buffer, sizeof(buffer));
				if (bytesRead > 0)
				{
					fwrite(buffer, 1, bytesRead, outFile);
				}
			} while (bytesRead > 0);

			fclose(outFile);
			unzCloseCurrentFile(zipFile);
		} while (unzGoToNextFile(zipFile) == UNZ_OK);

		unzClose(zipFile);
		return 0;
	}

	size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream)
	{
		return fwrite(ptr, size, nmemb, stream);
	}

	int setup_steamcmd()
	{
		int tries = 0;
		int max_tries = 3;
		if (!std::filesystem::exists("steamcmd"))
		{
			std::filesystem::create_directory("steamcmd");
		}

		if (!utils::io::file_exists("steamcmd.zip") && !utils::io::file_exists("steamcmd/steamcmd.exe"))
		{
			CURL* curl = curl_easy_init();
			if (!curl)
			{
				printf("CURL initialization failed.\n");
				return 1;
			}

			const char url[] = "https://steamcdn-a.akamaihd.net/client/installer/steamcmd.zip"; //official steamcmd download link
			const char outfilename[] = "./steamcmd.zip";

			curl_version_info_data* vinfo = curl_version_info(CURLVERSION_NOW);

			if (vinfo->features & CURL_VERSION_SSL)
			{
				printf("CURL: SSL enabled\n");
			}
			else
			{
				printf("CURL: SSL not enabled\n");
			}

			FILE* outFile = NULL;
			errno_t err = fopen_s(&outFile, outfilename, "wb");
			if (err != 0 || outFile == NULL)
			{
				printf("Error opening file for writing.\n");
				return 1;
			}

			curl_easy_setopt(curl, CURLOPT_URL, url);
			curl_easy_setopt(curl, CURLOPT_CAINFO, "./ca-bundle.crt");
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, outFile);

			CURLcode res = curl_easy_perform(curl);
			if (res != CURLE_OK && tries < max_tries)
			{
				++tries;
				printf("CURL failed: %s\n", curl_easy_strerror(res));
				fclose(outFile);
				remove(outfilename); // Delete the file in case of failure and try again untill max tries used
				Sleep(2000);
				return setup_steamcmd();
			}
			else if (res != CURLE_OK && tries >= max_tries)
			{
				printf("[ERROR] Could not download steamcmd! \nMax tries used.");
				return 1;
			}

			fclose(outFile);
			curl_easy_cleanup(curl);

			if (extract_steamcmd() == 1)
			{
				printf("[ERROR] Could not extract steamcmd! \n");
				return 1;
			}

		}
		else if (!utils::io::file_exists("steamcmd/steamcmd.exe") && utils::io::file_exists("steamcmd.zip"))
		{
			if (extract_steamcmd() == 1)
			{
				printf("[ERROR] Could not extract steamcmd! \n");
				return 1;
			}
		}
		if (!utils::io::file_exists("steamcmd/steamcmd.exe") && utils::io::file_exists("steamcmd.exe"))
		{
			std::filesystem::rename("steamcmd.exe", "steamcmd/steamcmd.exe");
		}

		try
		{
			if (utils::io::file_exists("steamcmd/steamcmd.exe") && std::filesystem::is_regular_file("steamcmd/steamcmd.exe"))
			{
				std::uintmax_t file_size = std::filesystem::file_size("steamcmd/steamcmd.exe");
				if (file_size < 3 * 1024 * 1024)
				{
					printf("Install / update SteamCMD \n");
					start_new_process("./steamcmd/steamcmd.exe", false, true, "+quit");
				}
			}
		}
		catch (std::filesystem::filesystem_error& e)
		{
			printf( e.what() + '\n');
		}

		return 0;
	}

	void move_downloaded_folder(const std::string& sourceFolderPath, const std::string& destinationFolderPath)
	{
		try
		{
			if (std::filesystem::exists(sourceFolderPath) && std::filesystem::is_directory(sourceFolderPath))
			{
				if (!std::filesystem::exists(destinationFolderPath))
				{
					std::filesystem::create_directories(destinationFolderPath);
				}

				for (const auto& entry : std::filesystem::directory_iterator(sourceFolderPath))
				{
					std::string sourcePath = entry.path().string();
					std::string filename = entry.path().filename().string();
					std::filesystem::path destinationPath = std::filesystem::path(destinationFolderPath) / filename;
					std::filesystem::rename(sourcePath, destinationPath);
				}

				std::filesystem::remove(sourceFolderPath);
				printf("Folder and its contents moved successfully.\n");
			}
			else
			{
				fprintf(stderr, "Source folder does not exist or is not a directory.\n");
			}
		}
		catch (const std::filesystem::filesystem_error& ex)
		{
			fprintf(stderr, "Error: %s\n", ex.what());
		}
	}

	int download_workshop_item(std::string workshop_id, std::string modtype);

	void initialize_download(std::string workshop_id, std::string modtype)
	{
		workshop::downloading_workshop_item = true;
		int result = download_workshop_item(workshop_id.data(), modtype.data());

		if (result == 4)
		{
			MessageBox(NULL, "Problem downloading the workshop item. Max tries used exiting..", "Error!", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
		}
		else if (result == 3)
		{
			MessageBox(NULL, "Cannot install steamcmd. Please try again.", "Error!", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
		}
		else if (result == 2)
		{
			std::string hint_message = "There was problem moving the workshop item to correct folder! \nYou can try moving it manually and joining the server again. \nDownload path: \n" + std::filesystem::current_path().string() + "/steamcmd/steamapps/workshop/downloads/311210/" + workshop_id;
			MessageBox(NULL, hint_message.c_str(), "Error!", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
		}
		else if (result == 1)
		{
			MessageBox(NULL, "Download cancelled!", "Error!", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
		}
		else if (result == 0)
		{
			MessageBox(NULL, "Workshop item download Success! \nYou can now join the server again.", "Done!", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
		}

		//Refresh steam workshop items with command
		game::Cbuf_AddText(0, "userContentReload\n");
		printf("Workshop items refreshed\n");
		workshop::downloading_workshop_item = false;
	}
	
	int download_workshop_item(std::string workshop_id, std::string modtype)
	{
		if (setup_steamcmd() == 1)
		{
			printf("[ERROR] Could not setup steamcmd! \n");
			return 3;
		}

		const char* workshop_id_char = workshop_id.c_str();
		std::string content_folder = "./steamcmd/steamapps/workshop/content/311210/" + workshop_id;
		std::string steamapps_folder = "./steamcmd/steamapps";

		std::string destination_folder;
		if (modtype == "Mod")
		{
			destination_folder = "./mods/" + workshop_id;
		}
		else if (modtype == "Map")
		{
			destination_folder = "./usermaps/" + workshop_id;
		}

		//remove old folder before each download to prevent any issues with steamcmd download
		if (std::filesystem::exists(steamapps_folder))
		{
			std::filesystem::remove_all(steamapps_folder);
			printf("Old steamapps folder removed successfully.\n");
		}

		bool continue_download = false;
		printf("Mod download started.\n");
		int tries = 0;
		int max_tries = 15;
		while (workshop::downloading_workshop_item && tries < max_tries)
		{
			tries++;
			if (std::filesystem::exists(content_folder))
			{
				printf("Download completed \n");
				break;
			}
			if (continue_download)
			{
				printf("Download interrupted... \nResuming download! \n");
			}

			//create new steamcmd download process which will continue from were it was left if the download was interrupted by connection issues
			int result = start_new_process("./steamcmd/steamcmd.exe", false, true, ("+login anonymous app_update 311210 +workshop_download_item 311210 " + std::string(workshop_id_char) + " validate +quit").c_str());

			if (result == 1)
			{
				printf("User interrupted download \n");
				return 1;
			}

			continue_download = true;
		}

		if (tries >= max_tries)
		{
			printf("Problem downloading the workshop item. Max tries used exiting.. \n");
			return 4;
		}

		try
		{
			move_downloaded_folder(content_folder, destination_folder);
			printf("Mod moved to correct folder \n");
		}
		catch (const std::filesystem::filesystem_error& ex)
		{
			printf(ex.what() + '\n');
			return 2;
		}
		
		return 0;
	}
}
