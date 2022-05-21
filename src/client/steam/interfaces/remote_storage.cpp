#include <std_include.hpp>
#include "../steam.hpp"

namespace steam
{
	bool remote_storage::FileWrite(const char* pchFile, const void* pvData, int cubData)
	{
		return true;
	}

	int remote_storage::FileRead(const char* pchFile, void* pvData, int cubDataToRead)
	{
		return 0;
	}

	bool remote_storage::FileForget(const char* pchFile)
	{
		return true;
	}

	bool remote_storage::FileDelete(const char* pchFile)
	{
		return true;
	}

	unsigned long long remote_storage::FileShare(const char* pchFile)
	{
		return 0;
	}

	bool remote_storage::SetSyncPlatforms(const char* pchFile, unsigned int eRemoteStoragePlatform)
	{
		return true;
	}

	unsigned long long remote_storage::FileWriteStreamOpen(const char* pchFile)
	{
		return 0;
	}

	int remote_storage::FileWriteStreamWriteChunk(unsigned long long hStream, const void* pvData, int cubData)
	{
		return 1;
	}

	int remote_storage::FileWriteStreamClose(unsigned long long hStream)
	{
		return 1;
	}

	int remote_storage::FileWriteStreamCancel(unsigned long long hStream)
	{
		return 1;
	}

	bool remote_storage::FileExists(const char* pchFile)
	{
		return true;
	}

	bool remote_storage::FilePersisted(const char* pchFile)
	{
		return true;
	}

	int remote_storage::GetFileSize(const char* pchFile)
	{
		return 0;
	}

	long long remote_storage::GetFileTimestamp(const char* pchFile)
	{
		return 0;
	}

	unsigned remote_storage::GetSyncPlatforms(const char* pchFile)
	{
		return 0;
	}

	int remote_storage::GetFileCount()
	{
		return 0;
	}

	const char* remote_storage::GetFileNameAndSize(int iFile, int* pnFileSizeInBytes)
	{
		*pnFileSizeInBytes = 0;
		return "";
	}

	bool remote_storage::GetQuota(int* pnTotalBytes, int* puAvailableBytes)
	{
		*pnTotalBytes = 0x10000000;
		*puAvailableBytes = 0x10000000;
		return false;
	}

	bool remote_storage::IsCloudEnabledForAccount()
	{
		return false;
	}

	bool remote_storage::IsCloudEnabledForApp()
	{
		return false;
	}

	void remote_storage::SetCloudEnabledForApp(bool bEnabled)
	{
	}

	unsigned long long remote_storage::UGCDownload(unsigned long long hContent, unsigned int uUnk)
	{
		return 0;
	}

	bool remote_storage::GetUGCDownloadProgress(unsigned long long hContent, unsigned int* puDownloadedBytes,
	                                            unsigned int* puTotalBytes)
	{
		return false;
	}

	bool remote_storage::GetUGCDetails(unsigned long long hContent, unsigned int* pnAppID, char** ppchName,
	                                   int* pnFileSizeInBytes, steam_id* pSteamIDOwner)
	{
		return false;
	}

	int remote_storage::UGCRead(unsigned long long hContent, void* pvData, int cubDataToRead, unsigned int uOffset)
	{
		return 0;
	}

	int remote_storage::GetCachedUGCCount()
	{
		return 0;
	}

	unsigned long long remote_storage::GetCachedUGCHandle(int iCachedContent)
	{
		return 0;
	}

	unsigned long long remote_storage::PublishWorkshopFile(const char* pchFile, const char* pchPreviewFile,
	                                                       unsigned int nConsumerAppId, const char* pchTitle,
	                                                       const char* pchDescription, unsigned int eVisibility,
	                                                       int* pTags, unsigned int eWorkshopFileType)
	{
		return 0;
	}

	unsigned long long remote_storage::CreatePublishedFileUpdateRequest(unsigned long long unPublishedFileId)
	{
		return 0;
	}

	bool remote_storage::UpdatePublishedFileFile(unsigned long long hUpdateRequest, const char* pchFile)
	{
		return false;
	}

	bool remote_storage::UpdatePublishedFilePreviewFile(unsigned long long hUpdateRequest, const char* pchPreviewFile)
	{
		return false;
	}

	bool remote_storage::UpdatePublishedFileTitle(unsigned long long hUpdateRequest, const char* pchTitle)
	{
		return false;
	}

	bool remote_storage::UpdatePublishedFileDescription(unsigned long long hUpdateRequest, const char* pchDescription)
	{
		return false;
	}

	bool remote_storage::UpdatePublishedFileVisibility(unsigned long long hUpdateRequest, unsigned int eVisibility)
	{
		return false;
	}

	bool remote_storage::UpdatePublishedFileTags(unsigned long long hUpdateRequest, int* pTags)
	{
		return false;
	}

	unsigned long long remote_storage::CommitPublishedFileUpdate(unsigned long long hUpdateRequest)
	{
		return 0;
	}

	unsigned long long remote_storage::GetPublishedFileDetails(unsigned long long unPublishedFileId)
	{
		return 0;
	}

	unsigned long long remote_storage::DeletePublishedFile(unsigned long long unPublishedFileId)
	{
		return 0;
	}

	unsigned long long remote_storage::EnumerateUserPublishedFiles(unsigned int uStartIndex)
	{
		return 0;
	}

	unsigned long long remote_storage::SubscribePublishedFile(unsigned long long unPublishedFileId)
	{
		return 0;
	}

	unsigned long long remote_storage::EnumerateUserSubscribedFiles(unsigned int uStartIndex)
	{
		return 0;
	}

	unsigned long long remote_storage::UnsubscribePublishedFile(unsigned long long unPublishedFileId)
	{
		return 0;
	}

	bool remote_storage::UpdatePublishedFileSetChangeDescription(unsigned long long hUpdateRequest,
	                                                             const char* cszDescription)
	{
		return false;
	}

	unsigned long long remote_storage::GetPublishedItemVoteDetails(unsigned long long unPublishedFileId)
	{
		return 0;
	}

	unsigned long long remote_storage::UpdateUserPublishedItemVote(unsigned long long unPublishedFileId, bool bVoteUp)
	{
		return 0;
	}

	unsigned long long remote_storage::GetUserPublishedItemVoteDetails(unsigned long long unPublishedFileId)
	{
		return 0;
	}

	unsigned long long remote_storage::EnumerateUserSharedWorkshopFiles(unsigned int nAppId, steam_id creatorSteamID,
	                                                                    unsigned int uStartIndex, int* pRequiredTags,
	                                                                    int* pExcludedTags)
	{
		return 0;
	}

	unsigned long long remote_storage::PublishVideo(unsigned int eVideoProvider, const char* cszVideoAccountName,
	                                                const char* cszVideoIdentifier, const char* cszFileName,
	                                                unsigned int nConsumerAppId, const char* cszTitle,
	                                                const char* cszDescription, unsigned int eVisibility, int* pTags)
	{
		return 0;
	}

	unsigned long long remote_storage::SetUserPublishedFileAction(unsigned long long unPublishedFileId,
	                                                              unsigned int eAction)
	{
		return 0;
	}

	unsigned long long remote_storage::EnumeratePublishedFilesByUserAction(
		unsigned int eAction, unsigned int uStartIndex)
	{
		return 0;
	}

	unsigned long long remote_storage::EnumeratePublishedWorkshopFiles(unsigned int eType, unsigned int uStartIndex,
	                                                                   unsigned int cDays, unsigned int cCount,
	                                                                   int* pTags, int* pUserTags)
	{
		return 0;
	}

	unsigned long long remote_storage::UGCDownloadToLocation(unsigned long long hContent, const char* cszLocation,
	                                                         unsigned int uUnk)
	{
		return 0;
	}
}
