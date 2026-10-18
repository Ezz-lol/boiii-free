#pragma once

#include <game/structs/dw/net/http.hpp>

namespace game {
namespace dw {
namespace identity {

enum class bdEnvironment : uint32_t {
  BD_ENVIRONMENT_DEV = 0x0,
  BD_ENVIRONMENT_CERT = 0x1,
  BD_ENVIRONMENT_PROD = 0x2,
  BD_ENVIRONMENT_MAX = 0x3,
};

class bdSingleIdentityJSONDeserializable {
public:
  // TODO
  struct VTbl {};

  VTbl *vtbl;
};

struct bdSingleIdentityError {
  bdNChar8 field[51];
  bdUInt32 code;
};

class __declspec(align(4)) bdSingleIdentityBase {
public:
  // TODO
  struct VTbl {};

  VTbl *vtbl;
  net::http::http_t *m_httpInterface;
  bdNChar8 *m_httpRequestBuffer;
  bdNChar8 *m_httpResponseBuffer;
  bdUInt32 m_requestBufferSize;
  bdUInt32 m_responseBufferSize;
  bdSingleIdentityJSONDeserializable *m_outputObject;
  bdRemoteTaskRef m_remoteTask;
  bdEnvironment m_environment;
  bdNChar8 m_client[64];
  bdSingleIdentityError m_errors[5];
  bdUInt16 m_numErrors;
};

} // namespace identity
} // namespace dw
} // namespace game
