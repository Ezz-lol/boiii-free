#pragma once

#include "structs/array.hpp"
#include <fstream>
#include <string>
#include <tomcrypt.h>
#include <vector>

namespace utils::cryptography {
namespace ecc {
class key final {
public:
  key();
  ~key();

  key(key &&obj) noexcept;
  key(const key &obj);
  key &operator=(key &&obj) noexcept;
  key &operator=(const key &obj);

  bool is_valid() const;

  ecc_key &get();
  const ecc_key &get() const;

  std::string get_public_key() const;

  void set(const std::string &pub_key_buffer);

  void deserialize(const std::string &key);
  void deserialize(const std::string *key);

  std::string serialize(int type = PK_PRIVATE) const;

  std::string get_openssl() const;
  void set_openssl(const std::string &key);

  void free();

  bool operator==(key &key) const;

  uint64_t get_hash() const;

private:
  ecc_key key_storage_{};
};

key generate_key(int bits);
key generate_key(int bits, const std::string &entropy);
std::string sign_message(const key &key, const std::string &message);
bool verify_message(const key &key, const uint8_t *message, size_t message_len,
                    const uint8_t *signature, size_t signature_size);
inline bool verify_message(const key &key, const std::string_view &message,
                           const std::string_view &signature) {
  return verify_message(
      key, reinterpret_cast<const uint8_t *>(message.data()), message.size(),
      reinterpret_cast<const uint8_t *>(signature.data()), signature.size());
}
template <const size_t N>
inline bool verify_message(const key &key, const std::string_view &message,
                           const array<uint8_t, N> &signature) {
  return verify_message(key, message, signature, N);
}

template <const size_t N>
inline bool verify_message(const key &key, const std::string_view &message,
                           const std::array<uint8_t, N> &signature) {
  return verify_message(key, message, signature.data(), N);
}

inline bool verify_message(const key &key, const std::string_view &message,
                           const std::vector<uint8_t> &signature) {
  return verify_message(key, reinterpret_cast<const uint8_t *>(message.data()),
                        message.size(), signature.data(), signature.size());
}

inline bool verify_message(const key &key, const std::string_view &message,
                           const char *signature, size_t signature_size) {
  return verify_message(
      key, reinterpret_cast<const uint8_t *>(message.data()), message.size(),
      reinterpret_cast<const uint8_t *>(signature), signature_size);
}

template <const size_t N>
inline bool verify_message(const key &key, const std::string_view &message,
                           const array<char, N> &signature) {
  return verify_message(key, message,
                        reinterpret_cast<const uint8_t *>(signature), N);
}

template <const size_t N>
inline bool verify_message(const key &key, const std::string_view &message,
                           const std::array<char, N> &signature) {
  return verify_message(key, message,
                        reinterpret_cast<const uint8_t *>(signature.data()), N);
}

inline bool verify_message(const key &key, const std::string_view &message,
                           const std::vector<char> &signature) {
  return verify_message(
      key, reinterpret_cast<const uint8_t *>(message.data()), message.size(),
      reinterpret_cast<const uint8_t *>(signature.data()), signature.size());
}

inline bool verify_message(const key &key, const uint8_t *message,
                           size_t message_size,
                           const std::string_view &signature) {
  return verify_message(key, message, message_size,
                        reinterpret_cast<const uint8_t *>(signature.data()),
                        signature.size());
}
template <const size_t N>
inline bool verify_message(const key &key, const uint8_t *message,
                           size_t message_size,
                           const array<uint8_t, N> &signature) {
  return verify_message(key, message, message_size, signature, N);
}

template <const size_t N>
inline bool verify_message(const key &key, const uint8_t *message,
                           size_t message_size,
                           const std::array<uint8_t, N> &signature) {
  return verify_message(key, message, message_size, signature.data(), N);
}

inline bool verify_message(const key &key, const uint8_t *message,
                           size_t message_size,
                           const std::vector<uint8_t> &signature) {
  return verify_message(key, message, message_size, signature.data(),
                        signature.size());
}

inline bool verify_message(const key &key, const uint8_t *message,
                           size_t message_size, const char *signature,
                           size_t signature_size) {
  return verify_message(key, message, message_size,
                        reinterpret_cast<const uint8_t *>(signature),
                        signature_size);
}

template <const size_t N>
inline bool verify_message(const key &key, const uint8_t *message,
                           size_t message_size,
                           const array<char, N> &signature) {
  return verify_message(key, message, message_size,
                        reinterpret_cast<const uint8_t *>(signature), N);
}

template <const size_t N>
inline bool verify_message(const key &key, const uint8_t *message,
                           size_t message_size,
                           const std::array<char, N> &signature) {
  return verify_message(key, message, message_size,
                        reinterpret_cast<const uint8_t *>(signature.data()), N);
}

inline bool verify_message(const key &key, const uint8_t *message,
                           size_t message_size,
                           const std::vector<char> &signature) {
  return verify_message(key, message, message_size,
                        reinterpret_cast<const uint8_t *>(signature.data()),
                        signature.size());
}

template <const size_t C>
inline bool verify_message(const key &key, const array<uint8_t, C> &message,
                           const std::string_view &signature) {
  return verify_message(key, message, C,
                        reinterpret_cast<const uint8_t *>(signature.data()),
                        signature.size());
}
template <const size_t C, const size_t N>
inline bool verify_message(const key &key, const array<uint8_t, C> &message,
                           const array<uint8_t, N> &signature) {
  return verify_message(key, message, C, signature, N);
}

template <const size_t C, const size_t N>
inline bool verify_message(const key &key, const array<uint8_t, C> &message,
                           const std::array<uint8_t, N> &signature) {
  return verify_message(key, message, C, signature.data(), N);
}

template <const size_t C>
inline bool verify_message(const key &key, const array<uint8_t, C> &message,
                           const std::vector<uint8_t> &signature) {
  return verify_message(key, message, C, signature.data(), signature.size());
}

template <const size_t C>
inline bool verify_message(const key &key, const array<uint8_t, C> &message,
                           const char *signature, size_t signature_size) {
  return verify_message(key, message, C,
                        reinterpret_cast<const uint8_t *>(signature),
                        signature_size);
}

template <const size_t C, const size_t N>
inline bool verify_message(const key &key, const array<uint8_t, C> &message,
                           const array<char, N> &signature) {
  return verify_message(key, message, C,
                        reinterpret_cast<const uint8_t *>(signature), N);
}

template <const size_t C, const size_t N>
inline bool verify_message(const key &key, const array<uint8_t, C> &message,
                           const std::array<char, N> &signature) {
  return verify_message(key, message, C,
                        reinterpret_cast<const uint8_t *>(signature.data()), N);
}

template <const size_t C>
inline bool verify_message(const key &key, const array<uint8_t, C> &message,
                           const std::vector<char> &signature) {
  return verify_message(key, message, C,
                        reinterpret_cast<const uint8_t *>(signature.data()),
                        signature.size());
}

template <const size_t C>
inline bool verify_message(const key &key,
                           const std::array<uint8_t, C> &message,
                           const std::string_view &signature) {
  return verify_message(key, message, C,
                        reinterpret_cast<const uint8_t *>(signature.data()),
                        signature.size());
}
template <const size_t C, const size_t N>
inline bool verify_message(const key &key,
                           const std::array<uint8_t, C> &message,
                           const array<uint8_t, N> &signature) {
  return verify_message(key, message, C, signature, N);
}

template <const size_t C, const size_t N>
inline bool verify_message(const key &key,
                           const std::array<uint8_t, C> &message,
                           const std::array<uint8_t, N> &signature) {
  return verify_message(key, message, C, signature.data(), N);
}

template <const size_t C>
inline bool verify_message(const key &key,
                           const std::array<uint8_t, C> &message,
                           const std::vector<uint8_t> &signature) {
  return verify_message(key, message, C, signature.data(), signature.size());
}

template <const size_t C>
inline bool verify_message(const key &key,
                           const std::array<uint8_t, C> &message,
                           const char *signature, size_t signature_size) {
  return verify_message(key, message, C,
                        reinterpret_cast<const uint8_t *>(signature),
                        signature_size);
}

template <const size_t C, const size_t N>
inline bool verify_message(const key &key,
                           const std::array<uint8_t, C> &message,
                           const array<char, N> &signature) {
  return verify_message(key, message, C,
                        reinterpret_cast<const uint8_t *>(signature), N);
}

template <const size_t C, const size_t N>
inline bool verify_message(const key &key,
                           const std::array<uint8_t, C> &message,
                           const std::array<char, N> &signature) {
  return verify_message(key, message, C,
                        reinterpret_cast<const uint8_t *>(signature.data()), N);
}

template <const size_t C>
inline bool verify_message(const key &key,
                           const std::array<uint8_t, C> &message,
                           const std::vector<char> &signature) {
  return verify_message(key, message, C,
                        reinterpret_cast<const uint8_t *>(signature.data()),
                        signature.size());
}

template <const size_t C>
inline bool verify_message(const key &key, const std::vector<uint8_t> &message,
                           const std::string_view &signature) {
  return verify_message(key, message.data(), message.size(),
                        reinterpret_cast<const uint8_t *>(signature.data()),
                        signature.size());
}
template <const size_t N>
inline bool verify_message(const key &key, const std::vector<uint8_t> &message,
                           const array<uint8_t, N> &signature) {
  return verify_message(key, message.data(), message.size(), signature, N);
}

template <const size_t N>
inline bool verify_message(const key &key, const std::vector<uint8_t> &message,
                           const std::array<uint8_t, N> &signature) {
  return verify_message(key, message.data(), message.size(), signature.data(),
                        N);
}

inline bool verify_message(const key &key, const std::vector<uint8_t> &message,
                           const std::vector<uint8_t> &signature) {
  return verify_message(key, message.data(), message.size(), signature.data(),
                        signature.size());
}

inline bool verify_message(const key &key, const std::vector<uint8_t> &message,
                           const char *signature, size_t signature_size) {
  return verify_message(key, message.data(), message.size(),
                        reinterpret_cast<const uint8_t *>(signature),
                        signature_size);
}

template <const size_t N>
inline bool verify_message(const key &key, const std::vector<uint8_t> &message,
                           const array<char, N> &signature) {
  return verify_message(key, message.data(), message.size(),
                        reinterpret_cast<const uint8_t *>(signature), N);
}

template <const size_t N>
inline bool verify_message(const key &key, const std::vector<uint8_t> &message,
                           const std::array<char, N> &signature) {
  return verify_message(key, message.data(), message.size(),
                        reinterpret_cast<const uint8_t *>(signature.data()), N);
}

inline bool verify_message(const key &key, const std::vector<uint8_t> &message,
                           const std::vector<char> &signature) {
  return verify_message(key, message.data(), message.size(),
                        reinterpret_cast<const uint8_t *>(signature.data()),
                        signature.size());
}

bool encrypt(const key &key, std::string &data);
bool decrypt(const key &key, std::string &data);
} // namespace ecc

namespace rsa {
std::string encrypt(const std::string &data, const std::string &hash,
                    const std::string &key);
}

namespace des3 {
std::string encrypt(const std::string &data, const std::string &iv,
                    const std::string &key);
std::string decrypt(const std::string &data, const std::string &iv,
                    const std::string &key);
} // namespace des3

namespace tiger {
std::string compute(const std::string &data, bool hex = false);
std::string compute(const uint8_t *data, size_t length, bool hex = false);
} // namespace tiger

namespace aes {
std::string encrypt(const std::string &data, const std::string &iv,
                    const std::string &key);
std::string decrypt(const std::string &data, const std::string &iv,
                    const std::string &key);
} // namespace aes

namespace hmac_sha1 {
std::string compute(const std::string &data, const std::string &key);
}

namespace sha1 {
std::string compute(const std::string &data, bool hex = false);
std::string compute(const uint8_t *data, size_t length, bool hex = false);

std::string compute(std::ifstream &data, bool hex = false);

} // namespace sha1

namespace sha256 {
std::string compute(const std::string &data, bool hex = false);
std::string compute(const uint8_t *data, size_t length, bool hex = false);
} // namespace sha256

namespace sha512 {
std::string compute(const std::string &data, bool hex = false);
std::string compute(const uint8_t *data, size_t length, bool hex = false);
} // namespace sha512

namespace base64 {
std::string encode(const uint8_t *data, size_t len);
std::string encode(const std::string &data);
std::string decode(const std::string &data);
} // namespace base64

namespace jenkins_one_at_a_time {
unsigned int compute(const std::string &data);
unsigned int compute(const char *key, size_t len);
}; // namespace jenkins_one_at_a_time

namespace random {
uint32_t get_integer();
std::string get_challenge();
void get_data(void *data, size_t size);
} // namespace random
} // namespace utils::cryptography