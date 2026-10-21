#pragma once

#include <game/structs/core.hpp>

namespace game {
namespace dw {
namespace crypt {

typedef bytearray<0x80> aesIv_t;

class __declspec(align(8)) bdCypher {
public:
  // TODO
  struct VTbl {};
  VTbl *vtbl;
  const bdUInt32 m_blockSize;
};

struct des_key {
  ulong32 ek[32];
  ulong32 dk[32];
};

struct des3_key {
  ulong32 ek[3][32];
  ulong32 dk[3][32];
};

struct blowfish_key {
  ulong32 S[4][256];
  ulong32 K[18];
};

struct rijndael_key {
  ulong32 eK[60];
  ulong32 dK[60];
  int Nr;
};

union Symmetric_key {
  des_key des;
  des3_key des3;
  blowfish_key blowfish;
  rijndael_key rijndael;
  void *data;
};

typedef Symmetric_key symmetric_key;

struct symmetric_CBC {
  int32_t cipher;
  int32_t blocklen;
  aesIv_t IV[128];
  symmetric_key key;
};

class bdCypherAES : public bdCypher {
public:
  bdUInt32 m_keySize;
  uint8_t _padding14[4];
  symmetric_CBC m_cbc;
};

} // namespace crypt
} // namespace dw
} // namespace game
