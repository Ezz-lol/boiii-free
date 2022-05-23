#pragma once

namespace steam
{
	class screenshots
	{
	public:
		~screenshots() = default;

		virtual uint64_t WriteScreenshot(void* pubRGB, uint32_t cubRGB, int nWidth, int nHeight);
		virtual uint64_t AddScreenshotToLibrary(const char* pchFilename, const char* pchThumbnailFilename, int nWidth,
		                                        int nHeight);
		virtual void TriggerScreenshot();
		virtual void HookScreenshots(bool bHook);
		virtual bool SetLocation(uint64_t hScreenshot, const char* pchLocation);
		virtual bool TagUser(uint64_t hScreenshot, steam_id steamID);
		virtual bool TagPublishedFile(uint64_t hScreenshot, uint64_t unPublishedFileID);
	};
}
