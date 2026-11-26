#pragma once
// Exact stock-object identities; no game binaries or captured scripts are embedded.
#include "map_import_binding.hpp"
#include "map_runtime_bridge.hpp"
// Exact stock-object import contract.
// does not establish linked helper ownership, runtime behavior, or activation.
namespace zombies::coop::map_import_binding::fixed_temple {
inline contract captured_contract() {
  contract c{};
  c.independently_verified = true; // bounded raw metadata + ACTS, not runtime proof
  c.logical_name = "scripts/zm/zm_temple_pack_a_punch.gsc";
  c.sha256 = {0x17, 0xcd, 0x95, 0x78, 0xff, 0xfd, 0xca, 0x9e, 0xe1, 0x55, 0x22, 0xa9, 0x46, 0x9b, 0x71, 0xa2, 0x78, 0x7f, 0x47, 0x76, 0x11, 0x3e, 0xf9, 0x9a, 0x3c, 0xe7, 0xfb, 0xe7, 0x75, 0x75, 0xf2, 0x1d};
  c.object_size = 22672;
  c.source_crc = 0x917da0da; c.code = 0x496; c.code_size = 0x3c26;
  c.companion_include = "custom_scripts/zm/eroded_coop_temple_readiness_entry";
  call_site site{};
  site.owner = {0xf5b5ebc1, 0x618, 0xb78, 0x10e95862, 0xe4a74e4b, 0x0, 0x0};
  site.reference = 0x66e; site.opcode = 0x18be; site.parameter_byte = 0;
  site.instruction = {0xbe, 0x18, 0x0, 0x70, 0x64, 0x4f, 0x61, 0x42, 0x4a, 0x72, 0xed, 0x91, 0x8d, 0x7b, 0x0, 0x0, 0x0, 0x0};
  c.bindings.push_back({{0x7b8d91ed, 0xe4a74e4b, 0, 0x25},
                        {0x7b8d91ed, 0x12caf4ee, 0, 0x05}, {std::move(site)}});
  c.repeated_import_keys.emplace(import_key{0x96c6fb2, 0xe4a74e4b, 0x0, 0x21}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xc6a563a, 0xe4a74e4b, 0x0, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x14c24d9d, 0xe4a74e4b, 0x0, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x1c75a89a, 0xe4a74e4b, 0x1, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x1d972325, 0xe4a74e4b, 0x0, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x1e1dd74f, 0xe4a74e4b, 0x0, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x2e46ac53, 0xe4a74e4b, 0x3, 0x25}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x30493584, 0xe4a74e4b, 0x0, 0x21}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x32fa5072, 0xe4a74e4b, 0x1, 0x24}, std::uint16_t{9});
  c.repeated_import_keys.emplace(import_key{0x3f10449f, 0xe4a74e4b, 0x0, 0x22}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0x4083a98e, 0xe4a74e4b, 0x0, 0x24}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0x422037f5, 0xe4a74e4b, 0x0, 0x24}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x47d28a09, 0xe4a74e4b, 0x1, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x4bd0142f, 0xe4a74e4b, 0x1, 0x32}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x4fb3fde4, 0xe4a74e4b, 0x2, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x5613042d, 0xe4a74e4b, 0x2, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x5e3bbe0e, 0xe4a74e4b, 0x0, 0x24}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x654eecd4, 0xe4a74e4b, 0x1, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x69c2f683, 0xe4a74e4b, 0x1, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x6ada35ba, 0xe4a74e4b, 0x2, 0x22}, std::uint16_t{17});
  c.repeated_import_keys.emplace(import_key{0x72a94f05, 0xe4a74e4b, 0x2, 0x22}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x74d6b22f, 0xad23e503, 0x1, 0x4}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x76f13293, 0x82b91a51, 0x0, 0x2}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x7922262b, 0x14b42b8a, 0x2, 0x2}, std::uint16_t{6});
  c.repeated_import_keys.emplace(import_key{0x7d15e2f8, 0xe4a74e4b, 0x2, 0x22}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x7db62cab, 0xe4a74e4b, 0x0, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x7fd23fc0, 0xe4a74e4b, 0x2, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x80bf3fc3, 0xe4a74e4b, 0x0, 0x21}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x87897a6e, 0xe4a74e4b, 0x0, 0x21}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x8bdea13c, 0xe4a74e4b, 0x4, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x907c2f2c, 0xe4a74e4b, 0x0, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x921a1574, 0xe4a74e4b, 0x1, 0x24}, std::uint16_t{6});
  c.repeated_import_keys.emplace(import_key{0x966ecb29, 0x84970cc4, 0x2, 0x2}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x99201f25, 0xe4a74e4b, 0x2, 0x22}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0xa1b06555, 0xe4a74e4b, 0x2, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xa96a2721, 0xe4a74e4b, 0x2, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xa96a2721, 0xe4a74e4b, 0x4, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xade37183, 0xe4a74e4b, 0x1, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xba049ca0, 0xe4a74e4b, 0x0, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xc2931a36, 0xe4a74e4b, 0x1, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xc35e6aab, 0xad23e503, 0x1, 0x4}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0xcb3d1c9b, 0xe4a74e4b, 0x2, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xcc4d8661, 0xe4a74e4b, 0x0, 0x21}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xce7aeb18, 0xe4a74e4b, 0x0, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xd7a0a9d8, 0xe4a74e4b, 0x0, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xdc8c8404, 0xe4a74e4b, 0x0, 0x24}, std::uint16_t{9});
  c.repeated_import_keys.emplace(import_key{0xde8377bf, 0xe4a74e4b, 0x0, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xeaa69754, 0xe4a74e4b, 0x0, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xf5ba63f2, 0xe4a74e4b, 0x1, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xfe0cfd2e, 0xe4a74e4b, 0x2, 0x22}, std::uint16_t{3});
  return c;
}
} // namespace zombies::coop::map_import_binding::fixed_temple

// Exact stock-object import contract.
// does not establish linked helper ownership, runtime behavior, or activation.
namespace zombies::coop::map_import_binding::fixed_castle {
inline contract captured_contract() {
  contract c{};
  c.independently_verified = true; // bounded raw metadata + ACTS, not runtime proof
  c.logical_name = "scripts/zm/zm_castle_ee_bossfight.gsc";
  c.sha256 = {0xf2, 0x39, 0xb8, 0x65, 0xa4, 0xc4, 0x8f, 0x6f, 0xe, 0x7, 0xda, 0xd, 0x76, 0xf, 0x97, 0x1b, 0xe4, 0x6b, 0xf7, 0xf0, 0xe3, 0x8d, 0xfa, 0x1c, 0x34, 0x99, 0x6e, 0x61, 0xfd, 0x58, 0x4, 0x7c};
  c.object_size = 62582;
  c.source_crc = 0x31d10ad; c.code = 0x13e5; c.code_size = 0xa551;
  c.companion_include = "custom_scripts/zm/eroded_coop_castle_readiness_entry";
  call_site site{};
  site.owner = {0x2adef490, 0x13f0, 0x1c00, 0xc35e6aab, 0xb1bc995c, 0x0, 0x0};
  site.reference = 0x19d6; site.opcode = 0x90e; site.parameter_byte = 0;
  site.instruction = {0xe, 0x9, 0x0, 0x36, 0x59, 0x4e, 0x73, 0x62, 0x2f, 0x41, 0x91, 0x1a, 0x83, 0x6f, 0x0, 0x0, 0x0, 0x0};
  c.bindings.push_back({{0x6f831a91, 0xb1bc995c, 0, 0x25},
                        {0x6f831a91, 0x4f329c55, 0, 0x05}, {std::move(site)}});
  c.repeated_import_keys.emplace(import_key{0x8a46476, 0xb1bc995c, 0x0, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xa30814d, 0xb1bc995c, 0x1, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xb6b79a0, 0xb1bc995c, 0x1, 0x22}, std::uint16_t{12});
  c.repeated_import_keys.emplace(import_key{0xb87262e, 0x84970cc4, 0x2, 0x2}, std::uint16_t{7});
  c.repeated_import_keys.emplace(import_key{0x1ab5ebec, 0xad23e503, 0x1, 0x4}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x1b1fe540, 0xb1bc995c, 0x1, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x1b20ff86, 0xb1bc995c, 0x1, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x1b8c5958, 0x82b91a51, 0x3, 0x2}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x1c7585d1, 0x52bfb19e, 0x1, 0x4}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x20559b9e, 0xb1bc995c, 0x2, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x26299103, 0xb1bc995c, 0x1, 0x22}, std::uint16_t{7});
  c.repeated_import_keys.emplace(import_key{0x26beb37e, 0x48131a3f, 0x3, 0x2}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0x28bb5727, 0xb1bc995c, 0x1, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x2992720d, 0xb1bc995c, 0x0, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x32fa5072, 0xb1bc995c, 0x1, 0x24}, std::uint16_t{7});
  c.repeated_import_keys.emplace(import_key{0x331efedc, 0x71e9cb84, 0x2, 0x4}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x36927923, 0xb1bc995c, 0x4, 0x25}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x369525ff, 0xb1bc995c, 0x0, 0x21}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x379868a6, 0xb1bc995c, 0x1, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x37f7858a, 0xb1bc995c, 0x1, 0x24}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x37f7858a, 0xb1bc995c, 0x3, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x38a9aff8, 0x564407fb, 0x0, 0x4}, std::uint16_t{11});
  c.repeated_import_keys.emplace(import_key{0x4083a98e, 0xb1bc995c, 0x0, 0x24}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0x422037f5, 0xb1bc995c, 0x0, 0x24}, std::uint16_t{6});
  c.repeated_import_keys.emplace(import_key{0x434db4ff, 0xb1bc995c, 0x0, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x43718187, 0xcc27597, 0x2, 0x5}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x43718187, 0x5753664b, 0x7, 0x4}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x47d18840, 0x84970cc4, 0x1, 0x2}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x48f26766, 0xb1bc995c, 0x0, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x4e3a8fa9, 0xb1bc995c, 0x1, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x501eb072, 0xb1bc995c, 0x0, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x507c2f7b, 0x172ba133, 0x3, 0x4}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x50f16166, 0x71e9cb84, 0x5, 0x2}, std::uint16_t{18});
  c.repeated_import_keys.emplace(import_key{0x56bce50a, 0x172ba133, 0x1, 0x2}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x5b49d38c, 0xb1bc995c, 0x1, 0x22}, std::uint16_t{6});
  c.repeated_import_keys.emplace(import_key{0x5c13fae0, 0x84970cc4, 0x2, 0x2}, std::uint16_t{10});
  c.repeated_import_keys.emplace(import_key{0x5d64f4b4, 0xb1bc995c, 0x4, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x5db6ba34, 0xb1bc995c, 0x1, 0x25}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x624b877f, 0xb1bc995c, 0x2, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x6271b605, 0x84970cc4, 0x2, 0x2}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x67e7a937, 0x80983c42, 0x1, 0x2}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x688ed188, 0x71e9cb84, 0x1, 0x4}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x69554b3e, 0x84970cc4, 0x3, 0x2}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0x6ada35ba, 0xb1bc995c, 0x2, 0x22}, std::uint16_t{9});
  c.repeated_import_keys.emplace(import_key{0x6e2770d8, 0xb1bc995c, 0x1, 0x22}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x704217d1, 0xb1bc995c, 0x0, 0x24}, std::uint16_t{17});
  c.repeated_import_keys.emplace(import_key{0x724abc52, 0xce7c3ed5, 0x5, 0x5}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x72a94f05, 0xb1bc995c, 0x2, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x735d5e85, 0xb1bc995c, 0x0, 0x25}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x74d4ae55, 0xb1bc995c, 0x0, 0x31}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x74d6b22f, 0x71e9cb84, 0x2, 0x4}, std::uint16_t{22});
  c.repeated_import_keys.emplace(import_key{0x74d6b22f, 0x71e9cb84, 0x2, 0x5}, std::uint16_t{6});
  c.repeated_import_keys.emplace(import_key{0x74d6b22f, 0xad23e503, 0x1, 0x4}, std::uint16_t{7});
  c.repeated_import_keys.emplace(import_key{0x74d6b22f, 0xad23e503, 0x1, 0x14}, std::uint16_t{19});
  c.repeated_import_keys.emplace(import_key{0x78d09fbd, 0xb1bc995c, 0x1, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x7922262b, 0x14b42b8a, 0x1, 0x2}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x7922262b, 0x14b42b8a, 0x2, 0x2}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x7922262b, 0xad23e503, 0x1, 0x4}, std::uint16_t{11});
  c.repeated_import_keys.emplace(import_key{0x7922262b, 0xad23e503, 0x1, 0x14}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0x7faf4c39, 0x14b42b8a, 0x2, 0x2}, std::uint16_t{8});
  c.repeated_import_keys.emplace(import_key{0x80983c42, 0x80983c42, 0x1, 0x2}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x84970cc4, 0xb1bc995c, 0x1, 0x22}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0x8c8e79fe, 0xb1bc995c, 0x0, 0x24}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x8f279593, 0xb1bc995c, 0x0, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x921a1574, 0xb1bc995c, 0x1, 0x24}, std::uint16_t{10});
  c.repeated_import_keys.emplace(import_key{0x93570bbc, 0xb1bc995c, 0x0, 0x31}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x96db213f, 0xb1bc995c, 0x0, 0x25}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x99201f25, 0xb1bc995c, 0x2, 0x22}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x9b998934, 0x22646992, 0x2, 0x5}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x9d134160, 0xad23e503, 0x1, 0x4}, std::uint16_t{6});
  c.repeated_import_keys.emplace(import_key{0x9d134160, 0xad23e503, 0x1, 0x14}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xa0938376, 0x172ba133, 0x0, 0x4}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xa0b29c50, 0xb1bc995c, 0x2, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xa1658f19, 0xb1bc995c, 0x0, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xa1658f19, 0xb1bc995c, 0x1, 0x24}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0xa1658f19, 0xb1bc995c, 0x1, 0x25}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xa1658f19, 0xb1bc995c, 0x2, 0x24}, std::uint16_t{8});
  c.repeated_import_keys.emplace(import_key{0xa96a2721, 0xb1bc995c, 0x2, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xa96a2721, 0xb1bc995c, 0x4, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xad721cc0, 0xb1bc995c, 0x0, 0x25}, std::uint16_t{7});
  c.repeated_import_keys.emplace(import_key{0xb3c0448e, 0xb1bc995c, 0x1, 0x32}, std::uint16_t{7});
  c.repeated_import_keys.emplace(import_key{0xb9fd52a4, 0x82b91a51, 0x3, 0x2}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xbcd31c2c, 0xdabbe128, 0x1, 0x2}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xbdcdc423, 0xb1bc995c, 0x1, 0x22}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0xc20e4083, 0xb1bc995c, 0x0, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xc2931a36, 0xb1bc995c, 0x2, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xc35e6aab, 0xad23e503, 0x1, 0x4}, std::uint16_t{6});
  c.repeated_import_keys.emplace(import_key{0xc35e6aab, 0xad23e503, 0x1, 0x14}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xc3945cd5, 0xb1bc995c, 0x2, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xc3945cd5, 0xb1bc995c, 0x3, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xca8445e1, 0xb1bc995c, 0x0, 0x24}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0xcb3d1c9b, 0xb1bc995c, 0x2, 0x22}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0xcd1d99bd, 0xb1bc995c, 0x1, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xd48f2ab3, 0xb1bc995c, 0x1, 0x24}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0xd974570f, 0x52bfb19e, 0x4, 0x2}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0xdb20c8d6, 0xb1bc995c, 0x1, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xdbca4c5d, 0xad23e503, 0x1, 0x4}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xdc8c8404, 0xb1bc995c, 0x0, 0x24}, std::uint16_t{9});
  c.repeated_import_keys.emplace(import_key{0xdc99997a, 0xb1bc995c, 0x2, 0x22}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0xe1c7a0c8, 0xb1bc995c, 0x1, 0x22}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0xe20da5e0, 0xb1bc995c, 0x2, 0x25}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xe2af603e, 0xb1bc995c, 0x0, 0x21}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xe2af603e, 0xb1bc995c, 0x1, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xe2f41bf2, 0xb1bc995c, 0x0, 0x25}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xe3ea9055, 0xb1bc995c, 0x0, 0x21}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xe3ea9055, 0xb1bc995c, 0x0, 0x31}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0xe5dafbe3, 0xb1bc995c, 0x0, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xeaab05dc, 0x84970cc4, 0x3, 0x2}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xeb99da89, 0xdabbe128, 0x1, 0x2}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xeda2be50, 0xb1bc995c, 0x1, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xeed6dab7, 0xb1bc995c, 0x2, 0x22}, std::uint16_t{28});
  c.repeated_import_keys.emplace(import_key{0xf6b53f16, 0xb1bc995c, 0x0, 0x31}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xfa7da172, 0x744abc1c, 0x0, 0x1}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0xfa7da172, 0x744abc1c, 0x0, 0x5}, std::uint16_t{2});
  return c;
}
} // namespace zombies::coop::map_import_binding::fixed_castle

// Exact stock-object contract; runtime behavior requires in-game validation.
namespace zombies::coop::map_import_binding::fixed_factory {
inline contract captured_contract() {
  contract c{}; c.independently_verified = true;
  c.logical_name = "scripts/zm/zm_factory_teleporter.gsc";
  c.sha256 = {0xc9, 0xa4, 0x58, 0xb4, 0xfb, 0x2b, 0xb3, 0x60, 0xb2, 0xd9, 0xb0, 0x8f, 0x22, 0x64, 0x4a, 0x84, 0xde, 0x60, 0x11, 0x3f, 0xfe, 0x66, 0x3b, 0x41, 0x66, 0x9b, 0xe5, 0x9a, 0xe8, 0x13, 0xa2, 0x78};
  c.object_size = 19164;
  c.source_crc = 0x8f8ec24e; c.code = 0x733; c.code_size = 0x2e19;
  c.companion_include = "custom_scripts/zm/eroded_coop_factory_teleport_entry";
  {
    binding rule{}; rule.source = {0x552e05fb, 0x4c372a0, 0x0, 0x24};
    rule.destination = {0x552e05fb, 0x22f68969, 0x0, 0x4};
    { call_site site{};
      site.owner = {0xa74f6ab9, 0x1898, 0x1ad0, 0xf3eef534, 0x4c372a0, 0x1, 0x0};
      site.reference = 0x19e6; site.opcode = 0x476; site.parameter_byte = 0;
      site.instruction = {0x76, 0x4, 0x0, 0x56, 0x79, 0x46, 0x4d, 0x65, 0x72, 0x61, 0xfb, 0x5, 0x2e, 0x55, 0x0, 0x0, 0x0, 0x0};
      rule.sites.push_back(std::move(site)); }
    c.bindings.push_back(std::move(rule));
  }
  c.repeated_import_keys.emplace(import_key{0x895b00, 0x4c372a0, 0x1, 0x32}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x87ea89a, 0x4c372a0, 0x2, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x1ab5ebec, 0xad23e503, 0x1, 0x4}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0x3ab4ff7d, 0x80983c42, 0x2, 0x2}, std::uint16_t{6});
  c.repeated_import_keys.emplace(import_key{0x3f10449f, 0x4c372a0, 0x0, 0x22}, std::uint16_t{7});
  c.repeated_import_keys.emplace(import_key{0x40b54710, 0x4c372a0, 0x2, 0x25}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x49539bfb, 0x4c372a0, 0x1, 0x25}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x4bd0142f, 0x4c372a0, 0x1, 0x32}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x50ccee8d, 0x4c372a0, 0x0, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x5369a434, 0x1437a8b5, 0x1, 0x5}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x5f0f1e6d, 0x4c372a0, 0x1, 0x24}, std::uint16_t{6});
  c.repeated_import_keys.emplace(import_key{0x5f4a2230, 0x4c372a0, 0x1, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x653ba713, 0x4c372a0, 0x0, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x67520c6a, 0x82b91a51, 0x4, 0x4}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x6ada35ba, 0x4c372a0, 0x2, 0x22}, std::uint16_t{10});
  c.repeated_import_keys.emplace(import_key{0x6f1ebe57, 0x4c372a0, 0x1, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x76f13293, 0x82b91a51, 0x0, 0x2}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x7922262b, 0x14b42b8a, 0x2, 0x2}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x7922262b, 0xad23e503, 0x1, 0x4}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x80983c42, 0x80983c42, 0x1, 0x2}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x8bdea13c, 0x4c372a0, 0x3, 0x24}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0x9113fdce, 0x4c372a0, 0x1, 0x25}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x9113fdce, 0x4c372a0, 0x2, 0x25}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x921a1574, 0x4c372a0, 0x1, 0x24}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0x9b7fda5e, 0x4c372a0, 0x2, 0x22}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0xa5218027, 0xe216c11c, 0x3, 0x2}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xc1021075, 0x4c372a0, 0x2, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xc35e6aab, 0xad23e503, 0x1, 0x4}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xc9ea7a64, 0x82b91a51, 0x3, 0x2}, std::uint16_t{8});
  c.repeated_import_keys.emplace(import_key{0xd5eb00e3, 0x4c372a0, 0x1, 0x24}, std::uint16_t{9});
  c.repeated_import_keys.emplace(import_key{0xdc8c8404, 0x4c372a0, 0x0, 0x24}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0xdc99997a, 0x4c372a0, 0x2, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xe48f905e, 0x4c372a0, 0x1, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xe68ee88a, 0x4c372a0, 0x1, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xef3f75eb, 0x82b91a51, 0x1, 0x4}, std::uint16_t{6});
  c.repeated_import_keys.emplace(import_key{0xf1aa5a21, 0x4c372a0, 0x2, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xf3a68d87, 0x4c372a0, 0x0, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xfaf8f736, 0x4c372a0, 0x1, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xfb40eb40, 0xaab55cf5, 0x0, 0x1}, std::uint16_t{2});
  return c;
}
} // namespace fixed_factory

// Exact stock-object contract; runtime behavior requires in-game validation.
namespace zombies::coop::map_import_binding::fixed_theater {
inline contract captured_contract() {
  contract c{}; c.independently_verified = true;
  c.logical_name = "scripts/zm/zm_theater_teleporter.gsc";
  c.sha256 = {0xd5, 0x6, 0x52, 0x5f, 0xa0, 0x4b, 0x30, 0xa0, 0x8c, 0xe1, 0x6a, 0x35, 0x3a, 0x29, 0xe8, 0x8c, 0x24, 0xdd, 0xd3, 0x73, 0x9b, 0x70, 0x5c, 0x9a, 0x8f, 0x32, 0x8f, 0xaa, 0x73, 0x46, 0xe, 0xfb};
  c.object_size = 23308;
  c.source_crc = 0x9f220e2d; c.code = 0x80a; c.code_size = 0x3892;
  c.companion_include = "custom_scripts/zm/eroded_coop_theater_teleport_entry";
  {
    binding rule{}; rule.source = {0x552e05fb, 0x8847920b, 0x2, 0x24};
    rule.destination = {0x552e05fb, 0x2ba81d54, 0x2, 0x4};
    { call_site site{};
      site.owner = {0xec8a4772, 0x15e0, 0x1a50, 0xf3eef534, 0x8847920b, 0x1, 0x0};
      site.reference = 0x16a6; site.opcode = 0x931; site.parameter_byte = 0;
      site.instruction = {0x31, 0x9, 0x0, 0x65, 0x62, 0x70, 0x31, 0x4e, 0x61, 0x49, 0xfb, 0x5, 0x2e, 0x55, 0x0, 0x0, 0x0, 0x0};
      rule.sites.push_back(std::move(site)); }
    { call_site site{};
      site.owner = {0xec8a4772, 0x15e0, 0x1a50, 0xf3eef534, 0x8847920b, 0x1, 0x0};
      site.reference = 0x190e; site.opcode = 0x931; site.parameter_byte = 0;
      site.instruction = {0x31, 0x9, 0x0, 0x35, 0x75, 0x6a, 0x2f, 0x66, 0x5a, 0x2f, 0xfb, 0x5, 0x2e, 0x55, 0x0, 0x0, 0x0, 0x0};
      rule.sites.push_back(std::move(site)); }
    { call_site site{};
      site.owner = {0xec8a4772, 0x15e0, 0x1a50, 0xf3eef534, 0x8847920b, 0x1, 0x0};
      site.reference = 0x1a2e; site.opcode = 0x92e; site.parameter_byte = 0;
      site.instruction = {0x2e, 0x9, 0x0, 0x72, 0x6a, 0x31, 0x36, 0x6c, 0x55, 0x70, 0xfb, 0x5, 0x2e, 0x55, 0x0, 0x0, 0x0, 0x0};
      rule.sites.push_back(std::move(site)); }
    c.bindings.push_back(std::move(rule));
  }
  {
    binding rule{}; rule.source = {0x784ee767, 0x8847920b, 0x2, 0x22};
    rule.destination = {0x784ee767, 0x2ba81d54, 0x2, 0x2};
    { call_site site{};
      site.owner = {0x6ffef777, 0x1c80, 0x2890, 0x552e05fb, 0x8847920b, 0x2, 0x0};
      site.reference = 0x1d68; site.opcode = 0xb30; site.parameter_byte = 0;
      site.instruction = {0x30, 0xb, 0x0, 0x4b, 0x55, 0x39, 0x66, 0x4f, 0x67, 0xe7, 0x4e, 0x78, 0x0, 0x0, 0x0, 0x0};
      rule.sites.push_back(std::move(site)); }
    { call_site site{};
      site.owner = {0x6ffef777, 0x1c80, 0x2890, 0x552e05fb, 0x8847920b, 0x2, 0x0};
      site.reference = 0x2314; site.opcode = 0xbcf; site.parameter_byte = 0;
      site.instruction = {0xcf, 0xb, 0x0, 0x43, 0x67, 0xe7, 0x4e, 0x78, 0x0, 0x0, 0x0, 0x0};
      rule.sites.push_back(std::move(site)); }
    { call_site site{};
      site.owner = {0x6ffef777, 0x1c80, 0x2890, 0x552e05fb, 0x8847920b, 0x2, 0x0};
      site.reference = 0x2354; site.opcode = 0xeec; site.parameter_byte = 0;
      site.instruction = {0xec, 0xe, 0x0, 0x30, 0x67, 0xe7, 0x4e, 0x78, 0x0, 0x0, 0x0, 0x0};
      rule.sites.push_back(std::move(site)); }
    { call_site site{};
      site.owner = {0x6ffef777, 0x1c80, 0x2890, 0x552e05fb, 0x8847920b, 0x2, 0x0};
      site.reference = 0x23d8; site.opcode = 0x1215; site.parameter_byte = 0;
      site.instruction = {0x15, 0x12, 0x0, 0x7a, 0x4d, 0x31, 0x6d, 0x71, 0x67, 0xe7, 0x4e, 0x78, 0x0, 0x0, 0x0, 0x0};
      rule.sites.push_back(std::move(site)); }
    c.bindings.push_back(std::move(rule));
  }
  {
    binding rule{}; rule.source = {0x7e0ed731, 0x8847920b, 0x2, 0x25};
    rule.destination = {0x7e0ed731, 0x2ba81d54, 0x2, 0x5};
    { call_site site{};
      site.owner = {0x6ffef777, 0x1c80, 0x2890, 0x552e05fb, 0x8847920b, 0x2, 0x0};
      site.reference = 0x224e; site.opcode = 0x15c4; site.parameter_byte = 0;
      site.instruction = {0xc4, 0x15, 0x0, 0x46, 0x4d, 0x65, 0x72, 0x61, 0x6c, 0x59, 0x31, 0xd7, 0xe, 0x7e, 0x0, 0x0, 0x0, 0x0};
      rule.sites.push_back(std::move(site)); }
    c.bindings.push_back(std::move(rule));
  }
  c.repeated_import_keys.emplace(import_key{0x895b00, 0x8847920b, 0x1, 0x32}, std::uint16_t{7});
  c.repeated_import_keys.emplace(import_key{0x2f58a11, 0x172ba133, 0x0, 0x4}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xa30814d, 0x8847920b, 0x1, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x1488cf91, 0x8847920b, 0x0, 0x21}, std::uint16_t{6});
  c.repeated_import_keys.emplace(import_key{0x1ab5ebec, 0xad23e503, 0x1, 0x4}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x25294cfe, 0xe216c11c, 0x3, 0x2}, std::uint16_t{6});
  c.repeated_import_keys.emplace(import_key{0x26299103, 0x8847920b, 0x1, 0x22}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x32fa5072, 0x8847920b, 0x1, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x331efedc, 0x71e9cb84, 0x1, 0x4}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x3d5de9eb, 0x84970cc4, 0x3, 0x2}, std::uint16_t{7});
  c.repeated_import_keys.emplace(import_key{0x3f10449f, 0x8847920b, 0x0, 0x22}, std::uint16_t{6});
  c.repeated_import_keys.emplace(import_key{0x40b54710, 0x8847920b, 0x2, 0x25}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x492a85ce, 0x8847920b, 0x1, 0x25}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x50f16166, 0x71e9cb84, 0x5, 0x2}, std::uint16_t{6});
  c.repeated_import_keys.emplace(import_key{0x552e05fb, 0x8847920b, 0x2, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x5e48550f, 0x8847920b, 0x1, 0x25}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x5f4a2230, 0x8847920b, 0x1, 0x24}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x653ba713, 0x8847920b, 0x0, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x668410f0, 0x8847920b, 0x1, 0x25}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x6ada35ba, 0x8847920b, 0x2, 0x22}, std::uint16_t{16});
  c.repeated_import_keys.emplace(import_key{0x72a94f05, 0x8847920b, 0x2, 0x22}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x74d6b22f, 0x71e9cb84, 0x2, 0x4}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0x74d6b22f, 0xad23e503, 0x1, 0x4}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x7661b8fe, 0x172ba133, 0x1, 0x4}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x76f13293, 0x82b91a51, 0x0, 0x2}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x784ee767, 0x8847920b, 0x2, 0x22}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x788ec1d6, 0x8847920b, 0x2, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x7922262b, 0xad23e503, 0x1, 0x4}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x80983c42, 0x80983c42, 0x1, 0x2}, std::uint16_t{9});
  c.repeated_import_keys.emplace(import_key{0x8332f7f6, 0x84970cc4, 0x1, 0x2}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x921a1574, 0x8847920b, 0x1, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x9d134160, 0xad23e503, 0x1, 0x4}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xa5218027, 0xe216c11c, 0x3, 0x2}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0xb6f106c8, 0x172ba133, 0x4, 0x4}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0xc35e6aab, 0xad23e503, 0x1, 0x4}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xc4cf74dc, 0x8847920b, 0x3, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xc9ea7a64, 0x82b91a51, 0x3, 0x2}, std::uint16_t{9});
  c.repeated_import_keys.emplace(import_key{0xd5eb00e3, 0x8847920b, 0x1, 0x24}, std::uint16_t{9});
  c.repeated_import_keys.emplace(import_key{0xdc8c8404, 0x8847920b, 0x0, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xdcb14347, 0x8847920b, 0x1, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xdfc351df, 0x80983c42, 0x1, 0x2}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0xe46c8b14, 0x172ba133, 0x2, 0x5}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xe68ee88a, 0x8847920b, 0x1, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xe9c3870b, 0x71e9cb84, 0x2, 0x4}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xeda2be50, 0x8847920b, 0x1, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xf0b11cbc, 0x52bfb19e, 0x1, 0x2}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xf1aa5a21, 0x8847920b, 0x2, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xfaf8f736, 0x8847920b, 0x1, 0x24}, std::uint16_t{3});
  return c;
}
} // namespace fixed_theater

// Exact stock-object contract; runtime behavior requires in-game validation.
namespace zombies::coop::map_import_binding::fixed_dragon {
inline contract captured_contract() {
  contract c{}; c.independently_verified = true;
  c.logical_name = "scripts/zm/zm_stalingrad_dragon.gsc";
  c.sha256 = {0x86, 0x2b, 0x86, 0x58, 0x9f, 0x8e, 0x13, 0x55, 0x2f, 0x4e, 0x80, 0xa, 0x83, 0x70, 0x1d, 0xfa, 0xc8, 0xe6, 0x84, 0x8f, 0x84, 0xe0, 0x3d, 0xa5, 0xff, 0x69, 0x6a, 0xd0, 0x49, 0xb8, 0xa6, 0xf3};
  c.object_size = 92270;
  c.source_crc = 0x3eb9339; c.code = 0x252d; c.code_size = 0xded5;
  c.companion_include = "custom_scripts/zm/eroded_coop_stalingrad_transport_entry";
  {
    binding rule{}; rule.source = {0x280af5e8, 0xe81d2518, 0x0, 0x25};
    rule.destination = {0x6835d2f9, 0x76a0c748, 0x0, 0x5};
    { call_site site{};
      site.owner = {0x1a158beb, 0x8810, 0x8888, 0x31efd8da, 0xe81d2518, 0x1, 0x0};
      site.reference = 0x886c; site.opcode = 0x1c79; site.parameter_byte = 0;
      site.instruction = {0x79, 0x1c, 0x0, 0x73, 0xe8, 0xf5, 0xa, 0x28, 0x0, 0x0, 0x0, 0x0};
      site.function_reference = false;
      rule.sites.push_back(std::move(site)); }
    c.bindings.push_back(std::move(rule));
  }
  {
    binding rule{}; rule.source = {0x3f0f5f61, 0xe81d2518, 0x0, 0x25};
    rule.destination = {0xe7c3b93d, 0x76a0c748, 0x0, 0x5};
    { call_site site{};
      site.owner = {0xe9256f09, 0x3cc0, 0x3cf8, 0xd2059ba7, 0xe81d2518, 0x1, 0x0};
      site.reference = 0x3cda; site.opcode = 0x1482; site.parameter_byte = 0;
      site.instruction = {0x82, 0x14, 0x0, 0x67, 0x47, 0x32, 0x61, 0x5f, 0xf, 0x3f, 0x0, 0x0, 0x0, 0x0};
      site.function_reference = false;
      rule.sites.push_back(std::move(site)); }
    c.bindings.push_back(std::move(rule));
  }
  {
    binding rule{}; rule.source = {0x83dd194d, 0xe81d2518, 0x0, 0x24};
    rule.destination = {0x276fcdf9, 0x76a0c748, 0x0, 0x4};
    { call_site site{};
      site.owner = {0x610a3cbb, 0x5928, 0x5988, 0x70d68a42, 0xe81d2518, 0x0, 0x0};
      site.reference = 0x5956; site.opcode = 0x1da9; site.parameter_byte = 0;
      site.instruction = {0xa9, 0x1d, 0x0, 0x51, 0x30, 0x6c, 0x31, 0x6d, 0x49, 0x4f, 0x4d, 0x19, 0xdd, 0x83, 0x0, 0x0, 0x0, 0x0};
      site.function_reference = false;
      rule.sites.push_back(std::move(site)); }
    c.bindings.push_back(std::move(rule));
  }
  c.repeated_import_keys.emplace(import_key{0x4da6e8, 0x48c05c81, 0x1, 0x12}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x82f90c, 0xe81d2518, 0x0, 0x22}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x17b7a6a, 0xe81d2518, 0x0, 0x32}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x21146aa, 0xe81d2518, 0x2, 0x25}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xc9aa1ff, 0x82b91a51, 0x2, 0x4}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x123d36d9, 0xe81d2518, 0x1, 0x24}, std::uint16_t{7});
  c.repeated_import_keys.emplace(import_key{0x1439c75a, 0xe81d2518, 0x0, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x1ab5ebec, 0xad23e503, 0x1, 0x4}, std::uint16_t{8});
  c.repeated_import_keys.emplace(import_key{0x1d9fdf1d, 0x172ba133, 0x1, 0x5}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x20b34915, 0xe81d2518, 0x0, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x2948b557, 0xe81d2518, 0x1, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x307b88f2, 0x84970cc4, 0x2, 0x2}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0x316422d1, 0xe81d2518, 0x2, 0x22}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0x32fa5072, 0xe81d2518, 0x1, 0x24}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x331efedc, 0x71e9cb84, 0x2, 0x4}, std::uint16_t{10});
  c.repeated_import_keys.emplace(import_key{0x33594820, 0xe81d2518, 0x0, 0x21}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x367f8966, 0xcc27597, 0x1, 0x4}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x38a9aff8, 0x564407fb, 0x0, 0x4}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x3b09a8e3, 0xe81d2518, 0x0, 0x21}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x3bc22c00, 0xe81d2518, 0x0, 0x21}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x422037f5, 0xe81d2518, 0x0, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x43718187, 0xcc27597, 0x1, 0x4}, std::uint16_t{27});
  c.repeated_import_keys.emplace(import_key{0x43718187, 0xcc27597, 0x1, 0x5}, std::uint16_t{11});
  c.repeated_import_keys.emplace(import_key{0x43718187, 0xcc27597, 0x2, 0x5}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x43ac000b, 0xad23e503, 0x1, 0x4}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x47d18840, 0x84970cc4, 0x1, 0x2}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x48c6a3a2, 0xe81d2518, 0x0, 0x24}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x496d5723, 0xe81d2518, 0x1, 0x22}, std::uint16_t{14});
  c.repeated_import_keys.emplace(import_key{0x49df1607, 0x40f1a093, 0x2, 0x2}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x4fd8b482, 0xe81d2518, 0x0, 0x21}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x500dc2f7, 0xe81d2518, 0x1, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x501eb072, 0xe81d2518, 0x0, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x50f16166, 0x71e9cb84, 0x5, 0x2}, std::uint16_t{11});
  c.repeated_import_keys.emplace(import_key{0x52fddbd0, 0xe81d2518, 0x0, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x593c2af4, 0xce7c3ed5, 0x2, 0x5}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x593e8a39, 0x80983c42, 0x1, 0x2}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x5b49d38c, 0xe81d2518, 0x1, 0x22}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x5bafaddd, 0xe81d2518, 0x2, 0x25}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x5cbfc55e, 0x72ee5da4, 0x2, 0x4}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x62b5e7aa, 0x40f1a093, 0x0, 0x5}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x62e4226e, 0xe81d2518, 0x1, 0x22}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x65f192a6, 0xe81d2518, 0x1, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x6ada35ba, 0xe81d2518, 0x2, 0x22}, std::uint16_t{11});
  c.repeated_import_keys.emplace(import_key{0x6e2770d8, 0xe81d2518, 0x1, 0x22}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x72a98d9c, 0xe81d2518, 0x1, 0x24}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x74d6b22f, 0x71e9cb84, 0x2, 0x4}, std::uint16_t{19});
  c.repeated_import_keys.emplace(import_key{0x74d6b22f, 0xad23e503, 0x1, 0x4}, std::uint16_t{22});
  c.repeated_import_keys.emplace(import_key{0x76f13293, 0x82b91a51, 0x0, 0x2}, std::uint16_t{11});
  c.repeated_import_keys.emplace(import_key{0x76f13293, 0x82b91a51, 0x0, 0x12}, std::uint16_t{8});
  c.repeated_import_keys.emplace(import_key{0x7922262b, 0x14b42b8a, 0x2, 0x2}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0x7922262b, 0xad23e503, 0x1, 0x2}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0x7922262b, 0xad23e503, 0x1, 0x4}, std::uint16_t{23});
  c.repeated_import_keys.emplace(import_key{0x7922262b, 0xad23e503, 0x1, 0x14}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0x7ee20edb, 0xe81d2518, 0x1, 0x25}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x7faf4c39, 0x14b42b8a, 0x1, 0x2}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x7faf4c39, 0x14b42b8a, 0x2, 0x2}, std::uint16_t{7});
  c.repeated_import_keys.emplace(import_key{0x80983c42, 0x80983c42, 0x1, 0x2}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x80c3dfb0, 0xe81d2518, 0x1, 0x25}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x8226b1a6, 0xe81d2518, 0x0, 0x21}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x825bb926, 0xe81d2518, 0x0, 0x25}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x829c1ad9, 0xe81d2518, 0x0, 0x21}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x84970cc4, 0xe81d2518, 0x1, 0x22}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x84970cc4, 0xe81d2518, 0x2, 0x22}, std::uint16_t{14});
  c.repeated_import_keys.emplace(import_key{0x84970cc4, 0xe81d2518, 0x4, 0x22}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x8642a7fe, 0xe81d2518, 0x0, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x8d33a9b5, 0xe81d2518, 0x0, 0x22}, std::uint16_t{7});
  c.repeated_import_keys.emplace(import_key{0x8d48f9f7, 0xe81d2518, 0x0, 0x21}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x8f9f34e0, 0xcc27597, 0x3, 0x2}, std::uint16_t{32});
  c.repeated_import_keys.emplace(import_key{0x8f9f34e0, 0xcc27597, 0x3, 0x12}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x8f9f34e0, 0xcc27597, 0x4, 0x2}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x8ff2374e, 0xe81d2518, 0x1, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x921a1574, 0xe81d2518, 0x1, 0x24}, std::uint16_t{7});
  c.repeated_import_keys.emplace(import_key{0x96eb8639, 0xe81d2518, 0x0, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x9b7fda5e, 0xe81d2518, 0x2, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x9c3abbf4, 0x724a4794, 0x0, 0x4}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x9d134160, 0xad23e503, 0x1, 0x4}, std::uint16_t{20});
  c.repeated_import_keys.emplace(import_key{0x9fa4f2a3, 0xe81d2518, 0x1, 0x22}, std::uint16_t{8});
  c.repeated_import_keys.emplace(import_key{0xa08562dc, 0xe81d2518, 0x3, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xa1be712d, 0xe81d2518, 0x0, 0x21}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0xa5195379, 0xcc27597, 0x3, 0x2}, std::uint16_t{15});
  c.repeated_import_keys.emplace(import_key{0xa8fb77bd, 0xe81d2518, 0x0, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xacdda91d, 0xe81d2518, 0x2, 0x25}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xada64136, 0xe81d2518, 0x0, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xb138e200, 0xe81d2518, 0x0, 0x21}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xb1a4952d, 0xe81d2518, 0x0, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xb499c281, 0xe81d2518, 0x0, 0x21}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xb8862210, 0xe81d2518, 0x2, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xb9f719ec, 0xad23e503, 0x1, 0x4}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xbba545e9, 0xe81d2518, 0x0, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xbc7ce905, 0xe81d2518, 0x1, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xbe798a34, 0x72ee5da4, 0x0, 0x4}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xbf4afe33, 0xe81d2518, 0x0, 0x22}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xc014d7a0, 0xe81d2518, 0x1, 0x24}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0xc08b162c, 0xe81d2518, 0x0, 0x21}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xc0e035d6, 0xe81d2518, 0x1, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xc1cdedfb, 0xccb8d056, 0x1, 0x2}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xc24d6636, 0xe81d2518, 0x0, 0x21}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0xc35e6aab, 0xcc27597, 0x1, 0x5}, std::uint16_t{7});
  c.repeated_import_keys.emplace(import_key{0xc35e6aab, 0xad23e503, 0x1, 0x4}, std::uint16_t{18});
  c.repeated_import_keys.emplace(import_key{0xc40e4649, 0xe81d2518, 0x1, 0x25}, std::uint16_t{16});
  c.repeated_import_keys.emplace(import_key{0xc4d5ec1f, 0xe81d2518, 0x1, 0x22}, std::uint16_t{9});
  c.repeated_import_keys.emplace(import_key{0xc5bbc2a7, 0xe81d2518, 0x1, 0x35}, std::uint16_t{7});
  c.repeated_import_keys.emplace(import_key{0xc8ca818a, 0xad23e503, 0x1, 0x4}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0xc985552d, 0xe81d2518, 0x3, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xca758bf2, 0x40f1a093, 0x1, 0x4}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xcab64a16, 0xe81d2518, 0x0, 0x21}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xcad59534, 0xe81d2518, 0x1, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xcbded78b, 0xe81d2518, 0x0, 0x21}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xcd1d99bd, 0xe81d2518, 0x1, 0x24}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0xcea50a94, 0xe81d2518, 0x1, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xcf6274b2, 0xe81d2518, 0x0, 0x22}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0xcf8fccfe, 0xdcf9c464, 0x1, 0x5}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xd013f7fa, 0xe81d2518, 0x2, 0x24}, std::uint16_t{11});
  c.repeated_import_keys.emplace(import_key{0xd2c92ee7, 0xd73b9283, 0x0, 0x2}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xd3de6822, 0xad23e503, 0x1, 0x4}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xd4556285, 0xe81d2518, 0x2, 0x25}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xd48f2ab3, 0xe81d2518, 0x1, 0x24}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0xd5eb00e3, 0xe81d2518, 0x1, 0x24}, std::uint16_t{12});
  c.repeated_import_keys.emplace(import_key{0xd8f45568, 0x40f1a093, 0x2, 0x2}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xda6acfd2, 0xe81d2518, 0x3, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xda9435cb, 0xe81d2518, 0x0, 0x21}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0xdb20c8d6, 0xe81d2518, 0x1, 0x24}, std::uint16_t{19});
  c.repeated_import_keys.emplace(import_key{0xdc8c8404, 0xe81d2518, 0x0, 0x24}, std::uint16_t{8});
  c.repeated_import_keys.emplace(import_key{0xdc99997a, 0xe81d2518, 0x2, 0x22}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0xdf42c640, 0xe81d2518, 0x1, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xe0710a11, 0x172ba133, 0x0, 0x4}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xe48f905e, 0xe81d2518, 0x1, 0x24}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0xe9c3870b, 0x71e9cb84, 0x2, 0x4}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xeb1965f1, 0xe81d2518, 0x1, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xed594d21, 0xe81d2518, 0x9, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xeda2be50, 0xe81d2518, 0x1, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xef3f75eb, 0x82b91a51, 0x1, 0x5}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xf0b11cbc, 0x172ba133, 0x1, 0x2}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xf679a325, 0xe81d2518, 0x1, 0x22}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0xf70dde0b, 0x48c05c81, 0x8, 0x5}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xfaf8f736, 0xe81d2518, 0x1, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xfce6cca8, 0xe81d2518, 0x2, 0x25}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xfcf56ab5, 0xcc27597, 0x0, 0x4}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xfcf56ab5, 0xcc27597, 0x1, 0x5}, std::uint16_t{2});
  return c;
}
} // namespace fixed_dragon

// Exact stock-object contract; runtime behavior requires in-game validation.
namespace zombies::coop::map_import_binding::fixed_stalingrad_wearables {
inline contract captured_contract() {
  contract c{}; c.independently_verified = true;
  c.logical_name = "scripts/zm/zm_stalingrad_wearables.gsc";
  c.sha256 = {0xfb, 0x9, 0x8f, 0x58, 0x8e, 0x4d, 0x83, 0xa1, 0x7d, 0xa5, 0xfc, 0x74, 0xd0, 0x7a, 0xc4, 0xa1, 0x4b, 0x19, 0x5, 0x9f, 0xaf, 0x22, 0xaf, 0x18, 0xcd, 0xcb, 0xe2, 0xa7, 0x70, 0x63, 0x5e, 0xc};
  c.object_size = 20696;
  c.source_crc = 0x4e139222; c.code = 0xa73; c.code_size = 0x2e25;
  c.companion_include = "custom_scripts/zm/eroded_coop_stalingrad_transport_entry";
  {
    binding rule{}; rule.source = {0xcc32e7df, 0x23c72813, 0x0, 0x24};
    rule.destination = {0xb33c5038, 0x76a0c748, 0x0, 0x4};
    { call_site site{};
      site.owner = {0xd59d7be6, 0x1940, 0x1a10, 0x7e873fe6, 0x23c72813, 0x0, 0x0};
      site.reference = 0x19f0; site.opcode = 0xd6f; site.parameter_byte = 0;
      site.instruction = {0x6f, 0xd, 0x0, 0x62, 0x55, 0x50, 0x59, 0x4c, 0xdf, 0xe7, 0x32, 0xcc, 0x0, 0x0, 0x0, 0x0};
      site.function_reference = false;
      rule.sites.push_back(std::move(site)); }
    c.bindings.push_back(std::move(rule));
  }
  c.repeated_import_keys.emplace(import_key{0xb6b79a0, 0x23c72813, 0x1, 0x22}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x1ab5ebec, 0x4dbf3ae3, 0x0, 0x4}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x1ab5ebec, 0xad23e503, 0x1, 0x4}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0x1fc9779e, 0x23c72813, 0x0, 0x21}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x20f2df00, 0x23c72813, 0x0, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x2436f867, 0x23c72813, 0x0, 0x25}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x37cbcf1a, 0x23c72813, 0x2, 0x22}, std::uint16_t{9});
  c.repeated_import_keys.emplace(import_key{0x37cbcf1a, 0x5132b4d6, 0x2, 0x22}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0x49df1607, 0x40f1a093, 0x2, 0x2}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x588ad36a, 0x23c72813, 0x0, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x63a43fb8, 0xac8e5d72, 0x1, 0x2}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x688ed188, 0x71e9cb84, 0x1, 0x4}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x69554b3e, 0x84970cc4, 0x2, 0x2}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x69554b3e, 0x84970cc4, 0x3, 0x2}, std::uint16_t{6});
  c.repeated_import_keys.emplace(import_key{0x70aa26aa, 0x5132b4d6, 0x0, 0x21}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x74d6b22f, 0x71e9cb84, 0x2, 0x4}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x74d6b22f, 0xad23e503, 0x1, 0x4}, std::uint16_t{10});
  c.repeated_import_keys.emplace(import_key{0x7922262b, 0x14b42b8a, 0x2, 0x2}, std::uint16_t{6});
  c.repeated_import_keys.emplace(import_key{0x7922262b, 0xad23e503, 0x1, 0x4}, std::uint16_t{12});
  c.repeated_import_keys.emplace(import_key{0x793f10ed, 0x23c72813, 0x1, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x7faf4c39, 0x14b42b8a, 0x2, 0x2}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x8870cfa8, 0xad23e503, 0x1, 0x4}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x8bdea13c, 0x5132b4d6, 0x2, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x921a1574, 0x23c72813, 0x1, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x9b7fda5e, 0x14b42b8a, 0x0, 0x2}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x9b7fda5e, 0x23c72813, 0x2, 0x22}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x9b7fda5e, 0x5132b4d6, 0x2, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x9c197fbf, 0x23c72813, 0x0, 0x21}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xa1b986ba, 0x40f1a093, 0x4, 0x4}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0xa8fb77bd, 0x23c72813, 0x0, 0x22}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xb3c0448e, 0x23c72813, 0x1, 0x32}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xbcd31c2c, 0xdabbe128, 0x1, 0x2}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xc35e6aab, 0xad23e503, 0x1, 0x4}, std::uint16_t{11});
  c.repeated_import_keys.emplace(import_key{0xc4d5ec1f, 0x5132b4d6, 0x1, 0x22}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xcaffcf07, 0x23c72813, 0x0, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xcc826737, 0x3b5de3f, 0x1, 0x4}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xd1d91f24, 0x3b5de3f, 0x0, 0x4}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xd5eb00e3, 0x23c72813, 0x1, 0x24}, std::uint16_t{9});
  c.repeated_import_keys.emplace(import_key{0xd5eb00e3, 0x5132b4d6, 0x1, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xdc8c8404, 0x5132b4d6, 0x0, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xe48f905e, 0x23c72813, 0x1, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xe48f905e, 0x5132b4d6, 0x1, 0x24}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0xeaab05dc, 0x84970cc4, 0x3, 0x2}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xeb99da89, 0xdabbe128, 0x1, 0x2}, std::uint16_t{3});
  return c;
}
} // namespace fixed_stalingrad_wearables

// Exact stock-object contract; runtime behavior requires in-game validation.
namespace zombies::coop::map_import_binding::fixed_stalingrad_main {
inline contract captured_contract() {
  contract c{}; c.independently_verified = true;
  c.logical_name = "scripts/zm/zm_stalingrad.gsc";
  c.sha256 = {0x85, 0x95, 0x80, 0x8, 0xa2, 0x4b, 0xa7, 0xe4, 0x42, 0xc9, 0x62, 0x67, 0x2f, 0xba, 0x6c, 0xe, 0x92, 0xad, 0x7c, 0x1e, 0xab, 0xf4, 0xca, 0x53, 0x6, 0x7a, 0xa9, 0xf8, 0xbc, 0x20, 0x47, 0x5};
  c.object_size = 46588;
  c.source_crc = 0x56a63255; c.code = 0x27d5; c.code_size = 0x5e2b;
  c.companion_include = "custom_scripts/zm/eroded_coop_stalingrad_transport_entry";
  c.verified_function_reference_shape = true;
  {
    binding rule{}; rule.source = {0x277575cc, 0x320a344e, 0x0, 0x21};
    rule.destination = {0x4e760fca, 0x76a0c748, 0x0, 0x1};
    { call_site site{};
      site.owner = {0xc3511ada, 0x28d0, 0x3930, 0xd290ebfa, 0x320a344e, 0x0, 0x0};
      site.reference = 0x3562; site.opcode = 0x1bde; site.parameter_byte = 0;
      site.instruction = {0xde, 0x1b, 0x69, 0x42, 0x6c, 0x7a, 0xcc, 0x75, 0x75, 0x27, 0x0, 0x0, 0x0, 0x0};
      site.function_reference = true;
      rule.sites.push_back(std::move(site)); }
    c.bindings.push_back(std::move(rule));
  }
  c.repeated_import_keys.emplace(import_key{0xe2c7174, 0xaab55cf5, 0x3, 0x2}, std::uint16_t{8});
  c.repeated_import_keys.emplace(import_key{0xe2c7174, 0xaab55cf5, 0x5, 0x2}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x123d36d9, 0x320a344e, 0x1, 0x24}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x19d0b055, 0x8bc21961, 0x1, 0x2}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x1a06a362, 0x172ba133, 0x2, 0x2}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x1ab5ebec, 0xad23e503, 0x1, 0x4}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x1bddf246, 0x172ba133, 0x1, 0x2}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x1c2beb2b, 0x320a344e, 0x3, 0x22}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0x1c7585d1, 0x52bfb19e, 0x1, 0x4}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x2103ff4b, 0x320a344e, 0x1, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x26299103, 0x320a344e, 0x1, 0x22}, std::uint16_t{7});
  c.repeated_import_keys.emplace(import_key{0x307b88f2, 0x84970cc4, 0x2, 0x2}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x33d0d4d7, 0x320a344e, 0x0, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x34478a90, 0xf09ec4f7, 0x1, 0x5}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x391512da, 0x320a344e, 0x1, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x3f10449f, 0x320a344e, 0x0, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x43718187, 0xcc27597, 0x1, 0x5}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x47d18840, 0x84970cc4, 0x1, 0x2}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0x4bd0142f, 0x320a344e, 0x1, 0x32}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x50f16166, 0x71e9cb84, 0x5, 0x2}, std::uint16_t{23});
  c.repeated_import_keys.emplace(import_key{0x52211d40, 0x320a344e, 0x1, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x5b49d38c, 0x320a344e, 0x1, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x5d210e97, 0x172ba133, 0x3, 0x4}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x6c1294b8, 0x320a344e, 0x2, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x74d6b22f, 0x71e9cb84, 0x2, 0x4}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0x74d6b22f, 0xad23e503, 0x1, 0x4}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x76f13293, 0x82b91a51, 0x0, 0x2}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x7922262b, 0xad23e503, 0x1, 0x4}, std::uint16_t{9});
  c.repeated_import_keys.emplace(import_key{0x7ed6c714, 0x1c31c03d, 0x1, 0x2}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x7faf4c39, 0x14b42b8a, 0x2, 0x2}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0x80983c42, 0x80983c42, 0x1, 0x2}, std::uint16_t{11});
  c.repeated_import_keys.emplace(import_key{0x8332f7f6, 0x84970cc4, 0x1, 0x2}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0x903f6b36, 0x48c05c81, 0x2, 0x4}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x965d1d83, 0x320a344e, 0x2, 0x25}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0x965d1d83, 0x320a344e, 0x3, 0x25}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0x99201f25, 0x320a344e, 0x2, 0x22}, std::uint16_t{7});
  c.repeated_import_keys.emplace(import_key{0xb4f5e3b9, 0xd84e54db, 0x1, 0x4}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xb8494651, 0x320a344e, 0x1, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xbf268fdb, 0x320a344e, 0x0, 0x21}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0xc2ad8318, 0x320a344e, 0x0, 0x21}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xc35e6aab, 0xad23e503, 0x1, 0x4}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xc4d5ec1f, 0x320a344e, 0x1, 0x22}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0xc7f3ce11, 0x320a344e, 0x0, 0x24}, std::uint16_t{5});
  c.repeated_import_keys.emplace(import_key{0xcb3d1c9b, 0x320a344e, 0x2, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xcd600a16, 0x52bfb19e, 0x0, 0x2}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xd849b9d0, 0x172ba133, 0x0, 0x4}, std::uint16_t{3});
  c.repeated_import_keys.emplace(import_key{0xdb20c8d6, 0x320a344e, 0x1, 0x24}, std::uint16_t{4});
  c.repeated_import_keys.emplace(import_key{0xeaf76fb7, 0x95f41f71, 0x1, 0x2}, std::uint16_t{8});
  c.repeated_import_keys.emplace(import_key{0xf0b11cbc, 0x172ba133, 0x1, 0x2}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xf270b41d, 0x320a344e, 0x2, 0x22}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xfb40eb40, 0xaab55cf5, 0x1, 0x5}, std::uint16_t{2});
  return c;
}
} // namespace fixed_stalingrad_main

// Exact stock-object contract; runtime behavior requires in-game validation.
namespace zombies::coop::map_import_binding::fixed_bgb_global {
inline contract captured_contract() {
  contract c{}; c.independently_verified = true;
  c.logical_name = "scripts/zm/bgbs/_zm_bgb_anywhere_but_here.gsc";
  c.sha256 = {0x50, 0x4f, 0x98, 0xbf, 0x67, 0x94, 0xbf, 0x48, 0x1b, 0x18, 0x40, 0x7, 0x90, 0x76, 0x97, 0xc6, 0xfd, 0xd8, 0xd4, 0x9d, 0xb8, 0xe4, 0x44, 0x83, 0xd5, 0x2c, 0x35, 0xc7, 0x4e, 0xa9, 0xf0, 0xa7};
  c.object_size = 4180;
  c.source_crc = 0x1f245628; c.code = 0x303; c.code_size = 0x98d;
  c.companion_include = "custom_scripts/zm/eroded_coop_bgb_transport_entry";
  c.verified_function_reference_shape = true;
  {
    binding rule{}; rule.source = {0x728dfe3, 0x93b7f03, 0x0, 0x24};
    rule.destination = {0x351a70f5, 0x370b3a48, 0x0, 0x4};
    { call_site site{};
      site.owner = {0x68ee913a, 0x430, 0x970, 0x7afbf7cd, 0x93b7f03, 0x0, 0x0};
      site.reference = 0x4fa; site.opcode = 0x1602; site.parameter_byte = 0;
      site.instruction = {0x2, 0x16, 0x0, 0x4b, 0x70, 0x64, 0xe3, 0xdf, 0x28, 0x7, 0x0, 0x0, 0x0, 0x0};
      site.function_reference = false;
      rule.sites.push_back(std::move(site)); }
    c.bindings.push_back(std::move(rule));
  }
  {
    binding rule{}; rule.source = {0xe4776d0, 0x93b7f03, 0x0, 0x21};
    rule.destination = {0xe4776d0, 0x370b3a48, 0x0, 0x1};
    { call_site site{};
      site.owner = {0x872df9d7, 0x350, 0x430, 0x8c87d8eb, 0x93b7f03, 0x0, 0x0};
      site.reference = 0x3c8; site.opcode = 0x1619; site.parameter_byte = 0;
      site.instruction = {0x19, 0x16, 0x41, 0x54, 0x41, 0x38, 0x54, 0x67, 0xd0, 0x76, 0x47, 0xe, 0x0, 0x0, 0x0, 0x0};
      site.function_reference = true;
      rule.sites.push_back(std::move(site)); }
    c.bindings.push_back(std::move(rule));
  }
  {
    binding rule{}; rule.source = {0x7afbf7cd, 0x93b7f03, 0x0, 0x21};
    rule.destination = {0x7afbf7cd, 0x370b3a48, 0x0, 0x1};
    { call_site site{};
      site.owner = {0x872df9d7, 0x350, 0x430, 0x8c87d8eb, 0x93b7f03, 0x0, 0x0};
      site.reference = 0x3be; site.opcode = 0x628; site.parameter_byte = 0;
      site.instruction = {0x28, 0x6, 0xcd, 0xf7, 0xfb, 0x7a, 0x0, 0x0, 0x0, 0x0};
      site.function_reference = true;
      rule.sites.push_back(std::move(site)); }
    c.bindings.push_back(std::move(rule));
  }
  c.repeated_import_keys.emplace(import_key{0xfaf8f736, 0x93b7f03, 0x1, 0x24}, std::uint16_t{2});
  c.repeated_import_keys.emplace(import_key{0xfd4ba5e1, 0x93b7f03, 0x2, 0x22}, std::uint16_t{4});
  return c;
}
} // namespace fixed_bgb_global

namespace zombies::coop::map_mechanics {
inline constexpr std::array<map_runtime_bridge::descriptor, 8> descriptors{{
  {"zm_temple", "scripts/zm/zm_temple_pack_a_punch.gsc", &map_import_binding::fixed_temple::captured_contract, 3, false, true},
  {"zm_castle", "scripts/zm/zm_castle_ee_bossfight.gsc", &map_import_binding::fixed_castle::captured_contract, 5, false, true},
  {"zm_factory", "scripts/zm/zm_factory_teleporter.gsc", &map_import_binding::fixed_factory::captured_contract, 8, false, true},
  {"zm_theater", "scripts/zm/zm_theater_teleporter.gsc", &map_import_binding::fixed_theater::captured_contract, 16, false, true},
  {"zm_stalingrad", "scripts/zm/zm_stalingrad_dragon.gsc", &map_import_binding::fixed_dragon::captured_contract, 96, false, true},
  {"zm_stalingrad", "scripts/zm/zm_stalingrad_wearables.gsc", &map_import_binding::fixed_stalingrad_wearables::captured_contract, 96, false, true},
  {"zm_stalingrad", "scripts/zm/zm_stalingrad.gsc", &map_import_binding::fixed_stalingrad_main::captured_contract, 96, true, true},
  {"", "scripts/zm/bgbs/_zm_bgb_anywhere_but_here.gsc", &map_import_binding::fixed_bgb_global::captured_contract, 64, false, false},
}};
inline constexpr std::size_t global_bgb_descriptor = 7;
static_assert(descriptors[global_bgb_descriptor].map.empty());
inline constexpr bool official_map(std::string_view map) noexcept {
  return map == "zm_zod" || map == "zm_factory" || map == "zm_castle" ||
      map == "zm_island" || map == "zm_stalingrad" || map == "zm_genesis" ||
      map == "zm_prototype" || map == "zm_asylum" || map == "zm_sumpf" ||
      map == "zm_theater" || map == "zm_cosmodrome" || map == "zm_temple" ||
      map == "zm_moon" || map == "zm_tomb";
}
} // namespace zombies::coop::map_mechanics
