#pragma once

#include <game/structs/core.hpp>
#include <game/structs/dw/core.hpp>

namespace game {
namespace dw {
namespace content {
class bdMultiPartURL : bdTaskResult {
public:
  bdNChar8 m_urls[50][385];
  bdUInt32 m_numUrls;
  bdUInt64 m_fileID;
};

class bdURL : bdTaskResult {
public:
  bdNChar8 m_url[384];
  bdUInt16 m_serverType;
  bdNChar8 m_serverIndex[128];
  bdUInt64 m_fileID;
};

class bdPreCopyResult : bdTaskResult {
public:
  bdURL m_source;
  bdNChar8 m_destination[384];
  bdUInt32 m_fileSize;
};

class bdTag : bdTaskResult, bdSerializable {
public:
  bdUInt64 m_priTag;
  bdUInt64 m_secTag;
};

class bdFileMetaData : bdTaskResult {
public:
  bdUInt64 m_fileID;
  bdUInt32 m_createTime;
  bdUInt32 m_modifedTime;
  bdUInt32 m_fileSize;
  bdUInt64 m_ownerID;
  bdNChar8 m_ownerName[64];
  bdUInt16 m_fileSlot;
  bdNChar8 m_fileName[128];
  bdNChar8 m_url[384];
  bdUInt16 m_category;
  bdUInt m_numTags;
  bdUByte8 m_metaData[512];
  bdUInt32 m_metaDataSize;
  bdUInt32 m_summaryFileSize;
  bdTag m_tags[60];
  bdUInt32 m_numCopiesMade;
  bdUInt64 m_originID;
};

class bdFileID : bdTaskResult, bdSerializable {
public:
  bdUInt64 m_fileID;
};
} // namespace content
} // namespace dw
} // namespace game
