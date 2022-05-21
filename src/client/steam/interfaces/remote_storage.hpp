#pragma once

namespace steam
{
	class remote_storage
	{
	public:
		~remote_storage() = default;

		virtual bool FileWrite(const char* pchFile, const void* pvData, int cubData);
		virtual int FileRead(const char* pchFile, void* pvData, int cubDataToRead);
		virtual bool FileForget(const char* pchFile);
		virtual bool FileDelete(const char* pchFile);
		virtual unsigned long long FileShare(const char* pchFile);
		virtual bool SetSyncPlatforms(const char* pchFile, unsigned int eRemoteStoragePlatform);
		virtual unsigned long long FileWriteStreamOpen(const char* pchFile);
		virtual int FileWriteStreamWriteChunk(unsigned long long hStream, const void* pvData, int cubData);
		virtual int FileWriteStreamClose(unsigned long long hStream);
		virtual int FileWriteStreamCancel(unsigned long long hStream);
		virtual bool FileExists(const char* pchFile);
		virtual bool FilePersisted(const char* pchFile);
		virtual int GetFileSize(const char* pchFile);
		virtual long long GetFileTimestamp(const char* pchFile);
		virtual unsigned int GetSyncPlatforms(const char* pchFile);
		virtual int GetFileCount();
		virtual const char* GetFileNameAndSize(int iFile, int* pnFileSizeInBytes);
		virtual bool GetQuota(int* pnTotalBytes, int* puAvailableBytes);
		virtual bool IsCloudEnabledForAccount();
		virtual bool IsCloudEnabledForApp();
		virtual void SetCloudEnabledForApp(bool bEnabled);
		virtual unsigned long long UGCDownload(unsigned long long hContent, unsigned int uUnk);
		virtual bool GetUGCDownloadProgress(unsigned long long hContent, unsigned int* puDownloadedBytes,
		                                    unsigned int* puTotalBytes);
		virtual bool GetUGCDetails(unsigned long long hContent, unsigned int* pnAppID, char** ppchName,
		                           int* pnFileSizeInBytes, steam_id* pSteamIDOwner);
		virtual int UGCRead(unsigned long long hContent, void* pvData, int cubDataToRead, unsigned int uOffset);
		virtual int GetCachedUGCCount();
		virtual unsigned long long GetCachedUGCHandle(int iCachedContent);
		virtual unsigned long long PublishWorkshopFile(const char* pchFile, const char* pchPreviewFile,
		                                               unsigned int nConsumerAppId, const char* pchTitle,
		                                               const char* pchDescription, unsigned int eVisibility, int* pTags,
		                                               unsigned int eWorkshopFileType);
		virtual unsigned long long CreatePublishedFileUpdateRequest(unsigned long long unPublishedFileId);
		virtual bool UpdatePublishedFileFile(unsigned long long hUpdateRequest, const char* pchFile);
		virtual bool UpdatePublishedFilePreviewFile(unsigned long long hUpdateRequest, const char* pchPreviewFile);
		virtual bool UpdatePublishedFileTitle(unsigned long long hUpdateRequest, const char* pchTitle);
		virtual bool UpdatePublishedFileDescription(unsigned long long hUpdateRequest, const char* pchDescription);
		virtual bool UpdatePublishedFileVisibility(unsigned long long hUpdateRequest, unsigned int eVisibility);
		virtual bool UpdatePublishedFileTags(unsigned long long hUpdateRequest, int* pTags);
		virtual unsigned long long CommitPublishedFileUpdate(unsigned long long hUpdateRequest);
		virtual unsigned long long GetPublishedFileDetails(unsigned long long unPublishedFileId);
		virtual unsigned long long DeletePublishedFile(unsigned long long unPublishedFileId);
		virtual unsigned long long EnumerateUserPublishedFiles(unsigned int uStartIndex);
		virtual unsigned long long SubscribePublishedFile(unsigned long long unPublishedFileId);
		virtual unsigned long long EnumerateUserSubscribedFiles(unsigned int uStartIndex);
		virtual unsigned long long UnsubscribePublishedFile(unsigned long long unPublishedFileId);
		virtual bool UpdatePublishedFileSetChangeDescription(unsigned long long hUpdateRequest,
		                                                     const char* cszDescription);
		virtual unsigned long long GetPublishedItemVoteDetails(unsigned long long unPublishedFileId);
		virtual unsigned long long UpdateUserPublishedItemVote(unsigned long long unPublishedFileId, bool bVoteUp);
		virtual unsigned long long GetUserPublishedItemVoteDetails(unsigned long long unPublishedFileId);
		virtual unsigned long long EnumerateUserSharedWorkshopFiles(unsigned int nAppId, steam_id creatorSteamID,
		                                                            unsigned int uStartIndex, int* pRequiredTags,
		                                                            int* pExcludedTags);
		virtual unsigned long long PublishVideo(unsigned int eVideoProvider, const char* cszVideoAccountName,
		                                        const char* cszVideoIdentifier, const char* cszFileName,
		                                        unsigned int nConsumerAppId, const char* cszTitle,
		                                        const char* cszDescription, unsigned int eVisibility, int* pTags);
		virtual unsigned long long SetUserPublishedFileAction(unsigned long long unPublishedFileId,
		                                                      unsigned int eAction);
		virtual unsigned long long EnumeratePublishedFilesByUserAction(unsigned int eAction, unsigned int uStartIndex);
		virtual unsigned long long EnumeratePublishedWorkshopFiles(unsigned int eType, unsigned int uStartIndex,
		                                                           unsigned int cDays, unsigned int cCount, int* pTags,
		                                                           int* pUserTags);
		virtual unsigned long long UGCDownloadToLocation(unsigned long long hContent, const char* cszLocation,
		                                                 unsigned int uUnk);
	};
}
