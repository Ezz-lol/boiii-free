#pragma once

#include <game/structs/dw/core.hpp>

namespace game {
namespace dw {
namespace social {
namespace external {
namespace facebook {
class bdFacebook {
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

class bdFacebookProfilePicture : public bdTaskResult {
public:
  bdUInt64 m_accountID;
  bdUInt32 m_pictureDataLength;
  bdUByte8 m_pictureData[16384];
};
class bdFacebookFriend : public bdTaskResult {
public:
  bdUInt64 m_accountID;
  bdNChar8 m_accountName[128];
  bdOnlineUserID m_userID;
  bdNChar8 m_userName[64];
  bdUByte8 m_platformState[64];
  bdUInt32 m_platformStateLength;
};
class bdFacebookUserAttribute : public bdTaskResult, public bdSerializable {
public:
  bdUInt32 m_name;
  bdNChar8 m_value[512];
};
class bdFacebookTag : public bdTaskResult, public bdSerializable {
public:
  bdUInt64 m_accountID;
  bdUInt16 m_x;
  bdUInt16 m_y;
};
class bdFacebookAttachmentProperty : public bdTaskResult,
                                     public bdSerializable {
public:
  bdNChar8 m_name[50];
  bdNChar8 m_value[70];
  bdNChar8 m_href[1024];
};

class bdFacebookAccount : public bdTaskResult {
public:
  bdUInt64 m_accountID;
  bdNChar8 m_accountName[128];
};

} // namespace facebook
} // namespace external
} // namespace social
} // namespace dw
} // namespace game
