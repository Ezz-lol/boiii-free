#include "../src/client/component/zombies/schema_peer.hpp"
#include <cassert>
#include <string>
using namespace zombies::coop;
int main() {
  const auto token = schema_token(); assert(token.size() == 131);
  assert(expanded_descriptor_capacity("8") == 8);
  for (auto bad : {"", "4", "9", "08", "8x", "-8", "2147483648"})
    assert(expanded_descriptor_capacity(bad) == 0);
  assert(schema_descriptor_matches("1", companion_contract, payloads::bundle_sha256));
  assert(!schema_descriptor_matches("2", companion_contract, payloads::bundle_sha256));
  assert(!schema_descriptor_matches("1", "wrong", payloads::bundle_sha256));
  assert(schema_proof_message("ab", "c", token) != schema_proof_message("a", "bc", token));
  assert(schema_proof_message("challenge", "connect one", token) !=
         schema_proof_message("challenge", "connect two", token));
  std::string tail;
  const auto put = [&tail](std::string_view value) {
    for (int shift=0; shift!=32; shift+=8)
      tail.push_back(static_cast<char>((value.size() >> shift)&255));
    tail.append(value);
  };
  put(token); put("signature");
  std::string parsed, signature;
  assert(decode_schema_proof(tail, parsed, signature));
  assert(parsed == token && signature == "signature");
  for (std::size_t i=0; i<tail.size(); ++i)
    assert(!decode_schema_proof(std::string_view(tail).substr(0,i), parsed, signature));
  assert(!decode_schema_proof(tail+"x", parsed, signature));
  auto changed=tail; changed[4]='2';
  assert(!decode_schema_proof(changed, parsed, signature));
  assert(!decode_schema_proof(std::string(513,'x'), parsed, signature));
  for (int i=0; i<4; ++i) {
    changed=tail; changed[i]=static_cast<char>(255);
    assert(!decode_schema_proof(changed, parsed, signature));
  }
}
