#pragma once

#include "core.hpp"

namespace game {
namespace dw {
namespace identity {
namespace uno {

class bdUno : bdSingleIdentityBase {};

enum class bdUnoTermsOfServiceType : uint32_t {
  PLAIN = 0x0,
  HTML = 0x1,
  XML = 0x2,
};

enum class bdUnoSubscriptionChannelType : uint32_t {
  UNDEFINED = 0x0,
  EMAIL = 0x1,
  SMS = 0x2,
  FACEBOOK = 0x3,
  TWITTER = 0x4,
  PUSH = 0x5,
  INGAME = 0x6,
  WEBSITE = 0x7,
};

class bdUnoTermsOfService : bdSingleIdentityJSONDeserializable {
public:
  bdUInt32 m_version;
  bdNChar8 m_dateAccepted[21];
  bdNChar8 m_locale[6];
  bdNChar8 m_created[21];
  bdNChar8 m_updated[21];
  bdNChar8 m_content[102400];
  bdUnoTermsOfServiceType m_type;
  bdNChar8 m_client[64];
};

} // namespace uno
} // namespace identity
} // namespace dw
} // namespace game
