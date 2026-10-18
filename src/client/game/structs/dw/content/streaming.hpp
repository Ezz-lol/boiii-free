#pragma once

#include <game/structs/core.hpp>
#include <game/structs/dw/core.hpp>
#include <game/structs/dw/hash.hpp>
#include <game/structs/dw/net/intercept.hpp>

#include "core.hpp"

namespace game {
namespace dw {

namespace content {
namespace streaming {

class bdCSHTTPWrapper : bdReferencable {};

struct StreamData {
  bdStopwatch timeSinceStreamInit;
  bdUInt16 nextPage;
  bdInt16 partIndexPage0;
  bdInt16 partIndexCurrentPage;
  bdInt16 partIndex;
};

class bdLiveStreamingInterceptor
    : public net::intercept::bdUploadInterceptor,
      public net::intercept::bdDownloadInterceptor {
public:
  bdCSHTTPWrapper *m_http;
  StreamData *m_streamData;
  bdUInt16 m_numParts;
  bdUInt32 m_totalFileSize;
  net::intercept::bdUploadInterceptor *m_clientUploadInterceptor;
  net::intercept::bdDownloadInterceptor *m_clientDownloadInterceptor;
};

class bdLiveStreamingFileMetaData : bdTaskResult {
public:
  bdUInt64 m_fileID;
  bdUInt32 m_createTime;
  bdUInt32 m_modifedTime;
  bdUInt32 m_fileSize;
  bdUInt64 m_ownerID;
  bdNChar8 m_ownerName[64];
  bdNChar8 m_fileName[128];
  bdUInt16 m_category;
  bdUInt m_numTags;
  bdInt16 m_numParts;
  bdUByte8 m_metaData[512];
  bdUInt32 m_metaDataSize;
  bdUInt32 m_summaryFileSize;
  bdTag m_tags[60];
  bdUInt32 m_numCopiesMade;
  bdNChar8 m_urls[10][385];
  bdUInt32 m_numUrls;
  bdUInt64 m_originFileID;
  bdInt16 m_partOffset;
};

class bdSummaryMetaHandler : bdFileMetaData {};

struct ThrottleData {
  bdBool throttling;
  bdInt16 throttlingInitIndex;
  bdStopwatch timeSinceThrottleInit;
};

class bdContentStreamingBase {
public:
  enum class bdStatus : uint32_t {
    READY = 0x0,
    PRE_HTTP_OPERATION = 0x1,
    HTTP_OPERATION = 0x2,
    THROTTLED = 0x3,
    POST_HTTP_OPERATION = 0x4,
    DONE = 0x5,
    FAILED = 0x6,
  };

  // TODO
  struct VTbl {};
  VTbl *vtbl;
  bdMultiPartURL m_multiPartURLs;
  bdBool m_multiPartComplete;
  bdURL m_URLs[10];
  net::intercept::bdUploadInterceptor *m_uploadHandler;
  bdFileMetaData m_fileMetaData;
  const void *m_uploadData;
  bdFileID *m_uploadFileID;
  bdFileMetaData *m_uploadMetaData;
  const void *m_thumbData;
  bdUInt32 m_thumbDataSize;
  bdBool m_sendChecksum;
  bdUByte8 m_checksum[33];
  bdLiveStreamingInterceptor *m_interceptor;
  bdPreCopyResult m_preCopyResults[10];
  bdUInt64 m_copySourceFileID;
  bdBool m_wasCopyingPooledFile;
  net::intercept::bdDownloadInterceptor *m_downloadHandler;
  bdFileMetaData *m_downloadMetaData;
  bdLiveStreamingFileMetaData *m_downloadMetaDataMultiPart;
  void *m_downloadData;
  bdUInt32 m_downloadDataSize;
  bdUInt32 m_startByte;
  bdUInt32 m_endByte;
  bdBool m_isSummary;
  bdBool m_isMultiPart;
  bdSummaryMetaHandler m_summaryMeta;
  bdRemoteTaskRef m_overallTask;
  bdRemoteTaskRef m_remoteTask;
  bdCSHTTPWrapper *m_http;
  hash::bdHashMD5 m_md5Hash;
  bdBool m_finalizeOnComplete;
  task::manage::bdRemoteTaskManager *m_remoteTaskManager;
  bdInt16 m_urlIndex;
  bdInt16 m_partDuration;
  bdInt32 m_numUrls;
  bdUInt32 m_uploadedSize;
  bdUInt16 m_retries;
  StreamData m_streamData;
  ThrottleData m_throttleData;
  bdUInt m_operation;
  bdStatus m_state;
};

enum class bdBatchDownloadInfoStatus : uint32_t {
  BATCH_NOT_STARTED = 0x0,
  BATCH_FILE_COMPLETED = 0x1,
  BATCH_IN_PROGRESS = 0x2,
  BATCH_STOPPED = 0x3,
  BATCH_ERROR = 0x4,
};

struct bdBatchDownloadInfo {
  bdFileMetaData fileMetaData;
  bdBatchDownloadInfoStatus status;
  bdUInt32 bytesDownloaded;
  union {
    struct {
      net::intercept::bdDownloadInterceptor *downloadHandler;
    } file;
    struct __declspec(align(8)) {
      void *summaryData;
      bdUInt32 summaryDataSize;
    } summary;
  };
};

class bdContentStreaming : public bdContentStreamingBase {
public:
  enum class bdBatchDownloadType : uint32_t {
    NONE = 0x0,
    PUBLISHER_FILES = 0x1,
    USER_FILES = 0x2,
    SUMMARY_FILES = 0x3,
  };

  bdContentStreaming::bdBatchDownloadType m_batchDownload;
  bdUInt32 m_batchIndex;
  bdBatchDownloadInfo *m_batchDownloadInfo;
  bdUInt32 m_batchNumFiles;
  bdRemoteTaskRef m_batchRemoteTaskRef;
  bdRemoteTaskRef m_batchOverallTaskRef;
};

class bdPooledStorage : public bdContentStreamingBase {
public:
  bdUInt64 m_pooledStoragefileID;
  bdBool m_fileIDReady;
};

} // namespace streaming
} // namespace content
} // namespace dw
} // namespace game
