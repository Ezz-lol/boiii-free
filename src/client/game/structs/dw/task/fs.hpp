#pragma once

#include <game/structs/dw/core.hpp>

namespace game {
namespace dw {
namespace task {
namespace fs {

class bdFileData : bdTaskResult {
public:
  void *m_fileData;
  bdUInt m_fileSize;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"m_fileData\": {:p}, \"m_fileSize\": {} }}",
                       reinterpret_cast<const void *>(m_fileData), m_fileSize);
  }
};

class bdFileInfo : bdTaskResult {
public:
  enum class bdVisibility : uint32_t {
    PUBLIC = 0x0,
    PRIVATE = 0x1,
    COUNT = 0x2,
  };

  static inline constexpr const char *
  serialize(const bdFileInfo::bdVisibility visibility) {
    switch (visibility) {
    case bdFileInfo::bdVisibility::PUBLIC:
      return "VISIBILITY_PUBLIC";
    case bdFileInfo::bdVisibility::PRIVATE:
      return "VISIBILITY_PRIVATE";
    case bdFileInfo::bdVisibility::COUNT:
      return "VISIBILITY_COUNT";
    default:
      return "VISIBILITY_INVALID";
    }
  }

  bdUInt64 m_fileID;
  bdUInt32 m_createTime;
  bdUInt32 m_modifedTime;
  bdFileInfo::bdVisibility m_visibility;
  bdOnlineUserID m_ownerID;
  bdNChar8 m_fileName[128];
  bdUInt32 m_fileSize;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{  \"m_fileID\": {}, \"m_createTime\": {}, \"m_modifedTime\": {}, "
        "\"m_visibility\": {}, \"m_ownerID\": {:016X}, \"m_fileName\": {}, "
        "\"m_fileSize\": {} }}",
        m_fileID, m_createTime, m_modifedTime, serialize(m_visibility),
        m_ownerID, ::serialize(m_fileName), m_fileSize);
  }
};

class bdContextUserStorageFileInfo : bdTaskResult {
public:
  bdUInt32 m_createTime;
  bdUInt32 m_modifedTime;
  bdFileInfo::bdVisibility m_visibility;
  bdUserID m_ownerID;
  bdNChar8 m_fileName[128];
  bdUInt32 m_fileSize;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"m_createTime\": {}, \"m_modifedTime\": {}, "
        "\"m_visibility\": {}, "
        "\"m_ownerID\": {}, \"m_fileName\": {}, \"m_fileSize\": {} }}",
        m_createTime, m_modifedTime, bdFileInfo::serialize(m_visibility),
        ::serialize(m_ownerID), ::serialize(m_fileName), m_fileSize);
  }
};

typedef bdContextUserStorageFileInfo bdUserStorageFileInfo;

class __declspec(align(8)) dwFileTask {
public:
  std::string m_filename;
  bdFileData m_fileData;
  bdLobbyErrorCode m_error;
  bdUInt64 m_fileID;
  bdUInt m_fileSize;
  bdUInt m_bufferSize;
  bdUserStorageFileInfo m_fileInfo;
  void *m_buffer;
  bool m_optional;
  bool m_lsgOnly;
  bool m_lpcFile;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"m_filename\": \"{}\", \"m_fileData\": {}, \"m_error\": {}, "
        "\"m_fileID\": {}, "
        "\"m_fileSize\": {}, \"m_bufferSize\": {}, \"m_fileInfo\": {}, "
        "\"m_buffer\": {:p}, "
        "\"m_optional\": {}, \"m_lsgOnly\": {}, \"m_lpcFile\": {} }}",
        m_filename, ::serialize(m_fileData), dw::serialize(m_error), m_fileID,
        m_fileSize, m_bufferSize, m_fileInfo.serialize(),
        reinterpret_cast<const void *>(m_buffer), ::serialize(m_optional),
        ::serialize(m_lsgOnly), ::serialize(m_lpcFile));
  }
};

struct fileRetryInfo {
  int32_t lastAttemptTime;
  int32_t lastAttemptInterval;
  int32_t retryCount;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"lastAttemptTime\": {}, \"lastAttemptInterval\": "
                       "{}, \"retryCount\": {} }}",
                       lastAttemptTime, lastAttemptInterval, retryCount);
  }
};

enum class dwFileOpResult : int32_t {
  SUCCESS = 0x0,
  NOT_FOUND = 0x1,
  FAILURE = 0x2,
};

inline constexpr const char *serialize(const dwFileOpResult result) {
  switch (result) {
  case dwFileOpResult::SUCCESS:
    return "FILE_OP_SUCCESS";
  case dwFileOpResult::NOT_FOUND:
    return "FILE_OP_NOT_FOUND";
  case dwFileOpResult::FAILURE:
    return "FILE_OP_FAILURE";
  default:
    return "FILE_OP_INVALID";
  }
}

struct dwFileOperationInfo {
  dwFileTask fileTask;
  fileRetryInfo retryInfo;
  int32_t taskType;
  bool fetchCompleted;
  bool writeCompleted;
  bool isUserFile;
  bool isCompressedFile;
  uint8_t *fileBuffer;
  int32_t bufferSize;
  int32_t decompressedSize;
  dwFileOpResult result;
  fastcallPtr_t<void(const ControllerIndex_t controllerIndex, void *)>
      fnCallbackSuccess;
  fastcallPtr_t<taskCompleteResults(const ControllerIndex_t controllerIndex,
                                    void *)>
      fnCallbackNotFound;
  fastcallPtr_t<taskCompleteResults(const ControllerIndex_t controllerIndex,
                                    void *)>
      fnCallbackFailure;
  bool *alreadyUploaded;
  uint8_t *parityBits;
  void *userData;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"fileTask\": {}, \"retryInfo\": {}, \"taskType\": "
                       "{:08X}, \"fetchCompleted\": {}, "
                       "\"writeCompleted\": {}, \"isUserFile\": {}, "
                       "\"isCompressedFile\": {}, \"fileBuffer\": {:p}, "
                       "\"bufferSize\": {}, \"decompressedSize\": {}, "
                       "\"result\": {}, \"fnCallbackSuccess\": {:p}, "
                       "\"fnCallbackNotFound\": {:p}, \"fnCallbackFailure\": "
                       "{:p}, \"alreadyUploaded\": {:p}, "
                       "\"parityBits\": {:p}, \"userData\": {:p} }}",
                       fileTask.serialize(), retryInfo.serialize(), taskType,
                       ::serialize(fetchCompleted), ::serialize(writeCompleted),
                       ::serialize(isUserFile), ::serialize(isCompressedFile),
                       reinterpret_cast<const void *>(fileBuffer), bufferSize,
                       decompressedSize, dw::task::fs::serialize(result),
                       reinterpret_cast<const void *>(fnCallbackSuccess),
                       reinterpret_cast<const void *>(fnCallbackNotFound),
                       reinterpret_cast<const void *>(fnCallbackFailure),
                       reinterpret_cast<const void *>(alreadyUploaded),
                       reinterpret_cast<const void *>(parityBits),
                       reinterpret_cast<const void *>(userData));
  }
};

} // namespace fs

} // namespace task
} // namespace dw
} // namespace game
