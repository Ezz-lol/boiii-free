#pragma once

#include "game/structs/dw/core.hpp"

namespace game {
namespace dw {
namespace commerce {

class bdCommerce {
public:
  task::manage::bdRemoteTaskManager *m_remoteTaskManager;
};

class bdCommerceItemQuantity : public bdTaskResult, public bdSerializable {
public:
  bdUInt64 m_inventoryItemId;
  bdUInt32 m_itemQuantity;
};
typedef bdUInt32 bdGiftID;

class bdCommerceGiftItem : public bdTaskResult, public bdSerializable {
public:
  bdGiftID m_giftId;
  bdUInt64 m_recipientId;
  bdUInt64 m_ownerId;
  bdUInt32 m_itemId;
  bdUInt32 m_itemQuantity;
  bdUInt32 m_expiry;
  bdUByte8 m_metaData[255];
};
class bdCommerceItem : public bdTaskResult, public bdSerializable {
public:
  bdUInt64 m_inventoryItemId;
  bdUInt64 m_ownerId;
  bdUInt32 m_itemId;
  bdUInt32 m_itemQuantity;
  bdUInt32 m_expiry;
  bdUByte8 m_metaData[255];
  bdUInt32 m_metaDataSize;
};
class bdCommerceReceiptStatus : public bdTaskResult, public bdSerializable {
public:
  bdBool m_isValid;
  bdUByte8 m_receiptType;
};
class bdCommerceWriter : public bdTaskResult, public bdSerializable {
public:
  bdUInt64 m_writerID;
  bdUInt32 m_modDateTime;
  bdNChar8 m_writerName[256];
};
class bdCommerceCurrency : public bdTaskResult, public bdSerializable {
public:
  bdInt32 m_currencyID;
  bdInt32 m_value;
};

} // namespace commerce
} // namespace dw
} // namespace game
