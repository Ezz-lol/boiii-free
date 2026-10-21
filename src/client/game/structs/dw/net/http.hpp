#pragma once

#include <game/structs/dw/core.hpp>
#include <game/structs/dw/thread.hpp>

namespace game {
namespace dw {
namespace net {

namespace intercept {
class bdDownloadInterceptor;
class bdUploadInterceptor;
} // namespace intercept

namespace http {

class bdCSHTTPWrapper : bdReferencable {
public:
  enum class bdStatus : uint32_t {
    UNINITIALIZED = 0x0,
    EMPTY = 0x1,
    CREATING = 0x2,
    PENDING = 0x3,
    DONE = 0x4,
    ABORTED = 0x5,
    FAILED = 0x6,
    TIMED_OUT = 0x7,
    FAILED_TO_START = 0x8,
    MAX_STATUS = 0x9,
  };

  struct bdHTTPHeader {
    const bdNChar8 *key;
    const bdNChar8 *value;
  };
};

class bdHTTPWrapperBase : public bdCSHTTPWrapper, public thread::bdRunnable {
public:
  enum class bdOperation : uint32_t {
    BD_IDLE = 0x0,
    BD_UPLOAD = 0x1,
    BD_DOWNLOAD = 0x2,
    BD_DELETE = 0x3,
    BD_COPY = 0x4,
    BD_AUTH = 0x5,
    BD_POST = 0x6,
    BD_PUT = 0x7,
    BD_GET = 0x8,
  };

  bdBool m_isInitialized;
  bdBool m_threadExiting;
  bdMutex m_threadMutex;
  bdSemaphore *m_httpSemaphore;
  thread::bdThread *m_thread;
  bdCSHTTPWrapper::bdStatus m_status;
  bdHTTPWrapperBase::bdOperation m_operation;
  bdInt m_httpErrorCode;
};

class bdHTTPWrapper : public bdHTTPWrapperBase {
public:
  bdInt m_templateID;
  bdInt m_connectionID;
  bdInt m_requestID;
  void *m_URIPool;
  bdBool m_abortOperation;
  bdMutex m_transactionLock;
  bdUByte8 *m_operationBuffer;
  bdUInt m_operationBufferSize;
  bdUInt32 m_transferedUpload;
  bdUInt32 m_transferedDownload;
  bdFloat32 m_transferSpeed;
  bdUByte8 m_httpCommonBuffer[8192];
  intercept::bdDownloadInterceptor *m_downloadHandler;
  intercept::bdUploadInterceptor *m_uploadHandler;
  bdBool m_chunkedUpload;
  bdUByte8 *m_replyBuffer;
  bdUInt32 m_replyBufferSize;
  bdNChar8 m_errorBuffer[1024];
  bdInt m_internalErrorCode;
  bdBool m_verboseOutput;
  bdBool m_progressMeter;
};

typedef bdHTTPWrapper http_t;

} // namespace http
} // namespace net
} // namespace dw
} // namespace game
