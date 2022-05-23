#include <std_include.hpp>
#include "../steam.hpp"

namespace steam
{
	uint64_t screenshots::WriteScreenshot(void* pubRGB, uint32_t cubRGB, int nWidth, int nHeight)
	{
		return 0;
	}

	uint64_t screenshots::AddScreenshotToLibrary(const char* pchFilename, const char* pchThumbnailFilename, int nWidth,
	                                             int nHeight)
	{
		return 0;
	}

	void screenshots::TriggerScreenshot()
	{
	}

	void screenshots::HookScreenshots(bool bHook)
	{
	}

	bool screenshots::SetLocation(uint64_t hScreenshot, const char* pchLocation)
	{
		return false;
	}

	bool screenshots::TagUser(uint64_t hScreenshot, steam_id steamID)
	{
		return false;
	}

	bool screenshots::TagPublishedFile(uint64_t hScreenshot, uint64_t unPublishedFileID)
	{
		return false;
	}
}
