#pragma once

namespace steam
{
	class ugc
	{
	public:
		~ugc() = default;

		virtual uint64_t CreateQueryUserUGCRequest(uint_least32_t unAccountID, uint32_t eListType,
		                                           uint32_t eMatchingUGCType,
		                                           uint32_t eSortOrder, uint32_t nCreatorAppID,
		                                           uint32_t nConsumerAppID, uint32_t unPage);
		virtual uint64_t CreateQueryAllUGCRequest(uint32_t eQueryType,
		                                          uint32_t eMatchingeMatchingUGCTypeFileType,
		                                          uint32_t nCreatorAppID, uint32_t nConsumerAppID,
		                                          uint32_t unPage);

		virtual uint64_t CreateQueryUGCDetailsRequest(uint64_t* pvecPublishedFileID,
		                                              uint32_t unNumPublishedFileIDs);
		virtual uint64_t SendQueryUGCRequest(uint64_t handle);

		virtual bool GetQueryUGCResult(uint64_t handle, uint32_t index, void* pDetails);

		virtual bool GetQueryUGCPreviewURL(uint64_t handle, uint32_t index, char* pchURL, uint32_t cchURLSize);
		virtual bool GetQueryUGCMetadata(uint64_t handle, uint32_t index, char* pchMetadata,
		                                 uint32_t cchMetadatasize);
		virtual bool GetQueryUGCChildren(uint64_t handle, uint32_t index, uint64_t* pvecPublishedFileID,
		                                 uint32_t cMaxEntries);
		virtual bool GetQueryUGCStatistic(uint64_t handle, uint32_t index, uint32_t eStatType,
		                                  uint32_t* pStatValue);

		virtual uint32_t GetQueryUGCNumAdditionalPreviews(uint64_t handle, uint32_t index);
		virtual bool GetQueryUGCAdditionalPreview(uint64_t handle, uint32_t index, uint32_t previewIndex,
		                                          char* pchURLOrVideoID, uint32_t cchURLSize, char* pchOriginalFileName,
		                                          uint32_t cchOriginalFileNameSize, void* pPreviewType);
		virtual uint32_t GetQueryUGCNumKeyValueTags(uint64_t handle, uint32_t index);
		virtual bool GetQueryUGCKeyValueTag(uint64_t handle, uint32_t index, uint32_t keyValueTagIndex,
		                                    char* pchKey, uint32_t cchKeySize, char* pchValue, uint32_t cchValueSize);

		virtual bool ReleaseQueryUGCRequest(uint64_t handle);

		virtual bool AddRequiredTag(uint64_t handle, const char* pTagName);
		virtual bool AddExcludedTag(uint64_t handle, const char* pTagName);
		virtual bool SetReturnKeyValueTags(uint64_t handle, bool bReturnKeyValueTags);
		virtual bool SetReturnLongDescription(uint64_t handle, bool bReturnLongDescription);
		virtual bool SetReturnMetadata(uint64_t handle, bool bReturnMetadata);
		virtual bool SetReturnChildren(uint64_t handle, bool bReturnChildren);
		virtual bool SetReturnAdditionalPreviews(uint64_t handle, bool bReturnAdditionalPreviews);
		virtual bool SetReturnTotalOnly(uint64_t handle, bool bReturnTotalOnly);
		virtual bool SetLanguage(uint64_t handle, const char* pchLanguage);
		virtual bool SetAllowCachedResponse(uint64_t handle, uint32_t unMaxAgeSeconds);

		virtual bool SetCloudFileNameFilter(uint64_t handle, const char* pMatchCloudFileName);

		virtual bool SetMatchAnyTag(uint64_t handle, bool bMatchAnyTag);
		virtual bool SetSearchText(uint64_t handle, const char* pSearchText);
		virtual bool SetRankedByTrendDays(uint64_t handle, uint32_t unDays);

		virtual bool AddRequiredKeyValueTag(uint64_t handle, const char* pKey, const char* pValue);

		virtual uint64_t RequestUGCDetails(uint64_t nPublishedFileID, uint32_t unMaxAgeSeconds);

		virtual uint64_t CreateItem(uint32_t nConsumerAppId, uint32_t eFileType);

		virtual uint64_t StartItemUpdate(uint32_t nConsumerAppId, uint64_t nPublishedFileID);

		virtual bool SetItemTitle(uint64_t handle, const char* pchTitle);
		virtual bool SetItemDescription(uint64_t handle, const char* pchDescription);
		virtual bool SetItemUpdateLanguage(uint64_t handle, const char* pchUpdateLanguage);
		virtual bool SetItemMetadata(uint64_t handle, const char* pchMetadata);
		virtual bool SetItemVisibility(uint64_t handle, uint32_t eVisibility);
		virtual bool SetItemTags(uint64_t updateHandle, const void* pTags);
		virtual bool SetItemContent(uint64_t handle, const char* pszContentFolder);
		virtual bool SetItemPreview(uint64_t handle, const char* pszPreviewFile);
		virtual bool RemoveItemKeyValueTags(uint64_t handle, const char* pchKey);
		virtual bool AddItemKeyValueTag(uint64_t handle, const char* pchKey, const char* pchValue);

		virtual bool AddItemPreviewFile(uint64_t handle, const char* pszPreviewFile, uint32_t type);
		virtual bool AddItemPreviewVideo(uint64_t handle, const char* pszVideoID);
		virtual bool UpdateItemPreviewFile(uint64_t handle, uint32_t index, const char* pszPreviewFile);
		virtual bool UpdateItemPreviewVideo(uint64_t handle, uint32_t index, const char* pszVideoID);
		virtual bool RemoveItemPreview(uint64_t handle, uint32_t index);


		virtual uint64_t SubmitItemUpdate(uint64_t handle, const char* pchChangeNote);
		virtual uint32_t GetItemUpdateProgress(uint64_t handle, uint64_t* punBytesProcessed,
		                                       uint64_t* punBytesTotal);

		virtual uint64_t SetUserItemVote(uint64_t nPublishedFileID, bool bVoteUp);
		virtual uint64_t GetUserItemVote(uint64_t nPublishedFileID);

		virtual uint64_t AddItemToFavorites(uint32_t nAppId, uint64_t nPublishedFileID);
		virtual uint64_t RemoveItemFromFavorites(uint32_t nAppId, uint64_t nPublishedFileID);

		virtual uint64_t SubscribeItem(uint64_t nPublishedFileID);
		virtual uint64_t UnsubscribeItem(uint64_t nPublishedFileID);
		virtual uint32_t GetNumSubscribedItems();
		virtual uint32_t GetSubscribedItems(uint64_t* pvecPublishedFileID, uint32_t cMaxEntries);

		virtual uint32_t GetItemState(uint64_t nPublishedFileID);
		virtual bool GetItemInstallInfo(uint64_t nPublishedFileID, uint64_t* punSizeOnDisk, char* pchFolder,
		                                uint32_t cchFolderSize, uint32_t* punTimeStamp);
		virtual bool GetItemDownloadInfo(uint64_t nPublishedFileID, uint64_t* punBytesDownloaded,
		                                 uint64_t* punBytesTotal);
		virtual bool DownloadItem(uint64_t nPublishedFileID, bool bHighPriority);
		virtual bool BInitWorkshopForGameServer(uint64_t unWorkshopDepotID, const char* pszFolder);
		virtual void SuspendDownloads(bool bSuspend);

		virtual uint64_t StartPlaytimeTracking(uint64_t* pvecPublishedFileID,
		                                       uint32_t unNumPublishedFileIDs);
		virtual uint64_t StopPlaytimeTracking(uint64_t* pvecPublishedFileID,
		                                      uint32_t unNumPublishedFileIDs);
		virtual uint64_t StopPlaytimeTrackingForAllItems();
	};
}
