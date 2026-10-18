#pragma once

#include <game/structs/dw/core.hpp>

namespace game {
namespace dw {
namespace linkedaccount {

class bdLinkedAccounts {
public:
  // TODO
  struct VTbl;

  VTbl *vtbl;
  task::manage::bdRemoteTaskManager *m_remoteTaskManager;
};

class bdLinkedAccountsResult : public bdTaskResult {
  bdUserAccountID m_userAccount;
  bdUserAccountID m_linkedAccountsResult[16];
  bdUInt32 m_numLinkedAccounts;
};

} // namespace linkedaccount
} // namespace dw
} // namespace game
