#include <std_include.hpp>
#include "../steam.hpp"

#include <utils/nt.hpp>

#include "component/steam_proxy.hpp"

namespace steam
{
	uint64_t ugc::CreateQueryUserUGCRequest(uint_least32_t unAccountID, uint32_t eListType,
	                                        uint32_t eMatchingUGCType,
	                                        uint32_t eSortOrder, uint32_t nCreatorAppID,
	                                        uint32_t nConsumerAppID, uint32_t unPage)
	{
		return 0;
	}

	uint64_t ugc::CreateQueryAllUGCRequest(uint32_t eQueryType,
	                                       uint32_t eMatchingeMatchingUGCTypeFileType,
	                                       uint32_t nCreatorAppID, uint32_t nConsumerAppID,
	                                       uint32_t unPage)
	{
		return 0;
	}

	uint64_t ugc::CreateQueryUGCDetailsRequest(uint64_t* pvecPublishedFileID,
	                                           uint32_t unNumPublishedFileIDs)
	{
		return 0;
	}

	uint64_t ugc::SendQueryUGCRequest(uint64_t handle)
	{
		return 0;
	}

	bool ugc::GetQueryUGCResult(uint64_t handle, uint32_t index, void* pDetails)
	{
		return false;
	}

	bool ugc::GetQueryUGCPreviewURL(uint64_t handle, uint32_t index, char* pchURL, uint32_t cchURLSize)
	{
		return false;
	}

	bool ugc::GetQueryUGCMetadata(uint64_t handle, uint32_t index, char* pchMetadata,
	                              uint32_t cchMetadatasize)
	{
		return false;
	}

	bool ugc::GetQueryUGCChildren(uint64_t handle, uint32_t index, uint64_t* pvecPublishedFileID,
	                              uint32_t cMaxEntries)
	{
		return false;
	}

	bool ugc::GetQueryUGCStatistic(uint64_t handle, uint32_t index, uint32_t eStatType,
	                               uint32_t* pStatValue)
	{
		return false;
	}

	uint32_t ugc::GetQueryUGCNumAdditionalPreviews(uint64_t handle, uint32_t index)
	{
		return 0;
	}

	bool ugc::GetQueryUGCAdditionalPreview(uint64_t handle, uint32_t index, uint32_t previewIndex,
	                                       char* pchURLOrVideoID, uint32_t cchURLSize, char* pchOriginalFileName,
	                                       uint32_t cchOriginalFileNameSize, void* pPreviewType)
	{
		return false;
	}

	uint32_t ugc::GetQueryUGCNumKeyValueTags(uint64_t handle, uint32_t index)
	{
		return 0;
	}

	bool ugc::GetQueryUGCKeyValueTag(uint64_t handle, uint32_t index, uint32_t keyValueTagIndex,
	                                 char* pchKey, uint32_t cchKeySize, char* pchValue, uint32_t cchValueSize)
	{
		return false;
	}

	bool ugc::ReleaseQueryUGCRequest(uint64_t handle)
	{
		return false;
	}

	bool ugc::AddRequiredTag(uint64_t handle, const char* pTagName)
	{
		return false;
	}

	bool ugc::AddExcludedTag(uint64_t handle, const char* pTagName)
	{
		return false;
	}

	bool ugc::SetReturnKeyValueTags(uint64_t handle, bool bReturnKeyValueTags)
	{
		return false;
	}

	bool ugc::SetReturnLongDescription(uint64_t handle, bool bReturnLongDescription)
	{
		return false;
	}

	bool ugc::SetReturnMetadata(uint64_t handle, bool bReturnMetadata)
	{
		return false;
	}

	bool ugc::SetReturnChildren(uint64_t handle, bool bReturnChildren)
	{
		return false;
	}

	bool ugc::SetReturnAdditionalPreviews(uint64_t handle, bool bReturnAdditionalPreviews)
	{
		return false;
	}

	bool ugc::SetReturnTotalOnly(uint64_t handle, bool bReturnTotalOnly)
	{
		return false;
	}

	bool ugc::SetLanguage(uint64_t handle, const char* pchLanguage)
	{
		return false;
	}

	bool ugc::SetAllowCachedResponse(uint64_t handle, uint32_t unMaxAgeSeconds)
	{
		return false;
	}

	bool ugc::SetCloudFileNameFilter(uint64_t handle, const char* pMatchCloudFileName)
	{
		return false;
	}

	bool ugc::SetMatchAnyTag(uint64_t handle, bool bMatchAnyTag)
	{
		return false;
	}

	bool ugc::SetSearchText(uint64_t handle, const char* pSearchText)
	{
		return false;
	}

	bool ugc::SetRankedByTrendDays(uint64_t handle, uint32_t unDays)
	{
		return false;
	}

	bool ugc::AddRequiredKeyValueTag(uint64_t handle, const char* pKey, const char* pValue)
	{
		return false;
	}

	uint64_t ugc::RequestUGCDetails(uint64_t nPublishedFileID, uint32_t unMaxAgeSeconds)
	{
		return 0;
	}

	uint64_t ugc::CreateItem(uint32_t nConsumerAppId, uint32_t eFileType)
	{
		return 0;
	}

	uint64_t ugc::StartItemUpdate(uint32_t nConsumerAppId, uint64_t nPublishedFileID)
	{
		return 0;
	}

	bool ugc::SetItemTitle(uint64_t handle, const char* pchTitle)
	{
		return false;
	}

	bool ugc::SetItemDescription(uint64_t handle, const char* pchDescription)
	{
		return false;
	}

	bool ugc::SetItemUpdateLanguage(uint64_t handle, const char* pchUpdateLanguage)
	{
		return false;
	}

	bool ugc::SetItemMetadata(uint64_t handle, const char* pchMetadata)
	{
		return false;
	}

	bool ugc::SetItemVisibility(uint64_t handle, uint32_t eVisibility)
	{
		return false;
	}

	bool ugc::SetItemTags(uint64_t updateHandle, const void* pTags)
	{
		return false;
	}

	bool ugc::SetItemContent(uint64_t handle, const char* pszContentFolder)
	{
		return false;
	}

	bool ugc::SetItemPreview(uint64_t handle, const char* pszPreviewFile)
	{
		return false;
	}

	bool ugc::RemoveItemKeyValueTags(uint64_t handle, const char* pchKey)
	{
		return false;
	}

	bool ugc::AddItemKeyValueTag(uint64_t handle, const char* pchKey, const char* pchValue)
	{
		return false;
	}

	bool ugc::AddItemPreviewFile(uint64_t handle, const char* pszPreviewFile, uint32_t type)
	{
		return false;
	}

	bool ugc::AddItemPreviewVideo(uint64_t handle, const char* pszVideoID)
	{
		return false;
	}

	bool ugc::UpdateItemPreviewFile(uint64_t handle, uint32_t index, const char* pszPreviewFile)
	{
		return false;
	}

	bool ugc::UpdateItemPreviewVideo(uint64_t handle, uint32_t index, const char* pszVideoID)
	{
		return false;
	}

	bool ugc::RemoveItemPreview(uint64_t handle, uint32_t index)
	{
		return false;
	}


	uint64_t ugc::SubmitItemUpdate(uint64_t handle, const char* pchChangeNote)
	{
		return 0;
	}

	uint32_t ugc::GetItemUpdateProgress(uint64_t handle, uint64_t* punBytesProcessed,
	                                    uint64_t* punBytesTotal)
	{
		return 0;
	}

	uint64_t ugc::SetUserItemVote(uint64_t nPublishedFileID, bool bVoteUp)
	{
		return 0;
	}

	uint64_t ugc::GetUserItemVote(uint64_t nPublishedFileID)
	{
		return 0;
	}

	uint64_t ugc::AddItemToFavorites(uint32_t nAppId, uint64_t nPublishedFileID)
	{
		return 0;
	}

	uint64_t ugc::RemoveItemFromFavorites(uint32_t nAppId, uint64_t nPublishedFileID)
	{
		return 0;
	}

	uint64_t ugc::SubscribeItem(uint64_t nPublishedFileID)
	{
		return 0;
	}

	uint64_t ugc::UnsubscribeItem(uint64_t nPublishedFileID)
	{
		return 0;
	}

	uint32_t ugc::GetNumSubscribedItems()
	{
		::steam_proxy::update_subscribed_items();

		uint32_t count = 0;
		::steam_proxy::access_subscribed_items([&](const steam_proxy::subscribed_item_map& items)
		{
			count = static_cast<uint32_t>(items.size());
		});

		return count;
	}

	uint32_t ugc::GetSubscribedItems(uint64_t* pvecPublishedFileID, uint32_t cMaxEntries)
	{
		uint32_t count = 0;
		::steam_proxy::access_subscribed_items([&](const steam_proxy::subscribed_item_map& items)
		{
			for (const auto& item : items)
			{
				if (count < cMaxEntries)
				{
					pvecPublishedFileID[count] = item.first;
					++count;
				}
			}
		});

		return count;
	}

	uint32_t ugc::GetItemState(uint64_t nPublishedFileID)
	{
		uint32_t state = 0;
		::steam_proxy::access_subscribed_items([&](const steam_proxy::subscribed_item_map& items)
		{
			const auto entry = items.find(nPublishedFileID);
			if (entry != items.end())
			{
				state = entry->second.state;
			}
		});

		return state;
	}

	bool ugc::GetItemInstallInfo(uint64_t nPublishedFileID, uint64_t* punSizeOnDisk, char* pchFolder,
	                             uint32_t cchFolderSize, uint32_t* punTimeStamp)
	{
		bool found = false;
		::steam_proxy::access_subscribed_items([&](const steam_proxy::subscribed_item_map& items)
		{
			const auto entry = items.find(nPublishedFileID);
			if (entry != items.end())
			{
				const auto& item = entry->second;
				found = item.available;
				memcpy(pchFolder, item.path.data(),
				       std::min(item.path.size() + 1, static_cast<size_t>(cchFolderSize)));
				pchFolder[cchFolderSize - 1] = 0;

				*punSizeOnDisk = item.size_on_disk;
				*punTimeStamp = item.time_stamp;
			}
		});

		return found;
	}

	bool ugc::GetItemDownloadInfo(uint64_t nPublishedFileID, uint64_t* punBytesDownloaded,
	                              uint64_t* punBytesTotal)
	{
		return false;
	}

	bool ugc::DownloadItem(uint64_t nPublishedFileID, bool bHighPriority)
	{
		return false;
	}

	bool ugc::BInitWorkshopForGameServer(uint64_t unWorkshopDepotID, const char* pszFolder)
	{
		return false;
	}

	void ugc::SuspendDownloads(bool bSuspend)
	{
	}

	uint64_t ugc::StartPlaytimeTracking(uint64_t* pvecPublishedFileID,
	                                    uint32_t unNumPublishedFileIDs)
	{
		return 0;
	}

	uint64_t ugc::StopPlaytimeTracking(uint64_t* pvecPublishedFileID,
	                                   uint32_t unNumPublishedFileIDs)
	{
		return 0;
	}

	uint64_t ugc::StopPlaytimeTrackingForAllItems()
	{
		return 0;
	}
}
