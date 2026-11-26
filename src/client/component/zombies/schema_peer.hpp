#pragma once
#include "companion.hpp"
#include "companion_payloads.hpp"
#include <cstdint>
#include <string>
#include <string_view>

namespace zombies::coop {
static_assert(companion_revision >= 1 && companion_revision <= 9, "Update bounded peer token framing for multi-digit revisions.");
inline std::string schema_token() {
  return std::to_string(companion_revision) + ":" + std::string(companion_contract) + ":" + payloads::bundle_sha256;
}
// Domain separated, length framed and bound to the existing challenge AND the
// complete native connect command. This declares compatible code; it is not
// remote attestation. The engine's clientfield checksum is still authoritative.
inline std::string schema_proof_message(std::string_view challenge,
                                        std::string_view connect,
                                        std::string_view token) {
  std::string out("BOIII coop schema v" + std::to_string(companion_revision) + "\n");
  const auto append = [&out](std::string_view value) {
    const auto length = static_cast<std::uint32_t>(value.size());
    for (int shift = 0; shift != 32; shift += 8)
      out.push_back(static_cast<char>((length >> shift) & 255));
    out.append(value);
  };
  append(challenge); append(connect); append(token);
  return out;
}
constexpr int expanded_descriptor_capacity(std::string_view value) noexcept {
  return value.size() == 1 && value[0] >= '5' && value[0] <= '8'
             ? value[0] - '0' : 0;
}
inline bool schema_descriptor_matches(std::string_view revision,
                                      std::string_view contract,
                                      std::string_view bundle) noexcept {
  return revision.size() == 1 && revision[0] == '0' + companion_revision && contract == companion_contract &&
         bundle == payloads::bundle_sha256;
}
// The optional connect tail is bounded before its two length-prefixed strings
// are decoded. No capability strings enter the native 1024-byte userinfo.
inline bool decode_schema_proof(std::string_view tail, std::string& token,
                                std::string& signature) {
  if (tail.size() < 8 || tail.size() > 512) return false;
  std::size_t offset = 0;
  const auto read = [&tail, &offset](std::string& value, std::size_t maximum) {
    if (tail.size() - offset < 4) return false;
    std::uint32_t length = 0;
    for (int shift = 0; shift != 32; shift += 8)
      length |= static_cast<std::uint32_t>(static_cast<unsigned char>(tail[offset++])) << shift;
    if (length > maximum || length > tail.size() - offset) return false;
    value.assign(tail.substr(offset, length)); offset += length; return true;
  };
  return read(token, 131) && token == schema_token() &&
         read(signature, 256) && !signature.empty() && offset == tail.size();
}
} // namespace zombies::coop
