#include "powerup_events.hpp"
#ifdef NDEBUG
#error The regression checks require assertions enabled
#endif
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
#include <new>
#include <tuple>


namespace power = zombies::coop::powerup_events;
namespace t7 = zombies::coop::t7;
using bytes = std::vector<std::uint8_t>;
static long allocation_budget = -1;
void* operator new(std::size_t size) {
    if (allocation_budget == 0) throw std::bad_alloc{};
    if (allocation_budget > 0) --allocation_budget;
    if (auto* p = std::malloc(size ? size : 1)) return p;
    throw std::bad_alloc{};
}
void* operator new[](std::size_t size) { return ::operator new(size); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t) noexcept { std::free(p); }
static unsigned checks = 0;
static void check(bool value) { ++checks; assert(value); }
// Deliberately non-cryptographic deterministic fixture fingerprint. It tests
// callback invocation and exact object matching only. The production adapter
// supplies real SHA-256; no retail contract or ownership is established here.
static power::digest hash(std::span<const std::uint8_t> input) {
    power::digest result{};
    for (std::size_t lane = 0; lane < 4; ++lane) {
        std::uint64_t value = 0xcbf29ce484222325ull ^ (0x9e3779b97f4a7c15ull * (lane + 1));
        for (const auto byte : input) value = (value ^ byte) * 0x100000001b3ull;
        value = (value ^ input.size()) * 0x100000001b3ull;
        for (std::size_t index = 0; index < 8; ++index)
            result[8 * lane + index] = static_cast<std::uint8_t>(value >> (index * 8));
    }
    return result;
}
static void put16(bytes& value, std::size_t at, std::uint16_t v) { power::detail::write16(value, at, v); }
static void put32(bytes& value, std::size_t at, std::uint32_t v) { t7::write32(value, at, v); }
static void string_at(bytes& value, std::size_t at, std::string_view text) {
    std::copy(text.begin(), text.end(), value.begin() + at); value[at + text.size()] = 0;
}
struct fixture { bytes input; power::contract c; };
static fixture make(bool shared, int includes = 1, bool common = false, int rows = 2) {
    const bool large = rows > 2 || includes > 1;
    const std::uint32_t code = large ? 0x11000u : 0x180u;
    const std::uint32_t code_size = large ? 0x12000u : 0x100u;
    const std::uint32_t split = large ? code + 0x100u : code + 0x80u;
    fixture f{bytes(code + code_size + 0x80, 0), {}};
    std::copy(t7::object_magic.begin(), t7::object_magic.end(), f.input.begin());
    put32(f.input, 8, 0x12345678); put32(f.input, 0x14, code); put32(f.input, 0x30, code_size);
    put32(f.input, 0x20, 0x60); put16(f.input, 0x3a, 2); put32(f.input, 0x34, 0x90);
    string_at(f.input, 0x90, "scripts/zm/_zm_powerups.gsc");
    const auto space = power::detail::fnv("zm_powerups");
    const auto original = power::detail::fnv("time_remaining_on_powerup");
    put32(f.input, 0x60, 0x1111); put32(f.input, 0x64, code);
    put32(f.input, 0x68, power::detail::fnv("show_on_hud")); put32(f.input, 0x6c, space); f.input[0x70] = 2;
    put32(f.input, 0x74, 0x2222); put32(f.input, 0x78, split); put32(f.input, 0x7c, original); put32(f.input, 0x80, space); f.input[0x84] = 2;
    if (includes) {
        put32(f.input, 0xc, large ? 0x400u : 0xc0u); f.input[0x44] = static_cast<std::uint8_t>(includes);
        string_at(f.input, 0x100, common ? "CUSTOM_SCRIPTS\\ZM\\ERODED_COOP.GSC" : "scripts/shared/callbacks_shared");
        for (int i = 0; i < includes; ++i) put32(f.input, t7::read32(f.input, 0xc) + 4 * i, 0x100);
    }
    const std::uint32_t table = large ? 0x900u : 0x130u;
    put32(f.input, 0x24, table); put16(f.input, 0x3c, static_cast<std::uint16_t>(rows));
    for (std::uint32_t i = 0; i < code_size; ++i) f.input[code + i] = static_cast<std::uint8_t>((i * 17 + 1) % 251);
    auto at = table;
    const auto emit = [&](std::uint32_t name, std::uint32_t ns, std::uint8_t params,
                          std::uint8_t flags, const std::vector<std::uint32_t>& refs) {
        put32(f.input, at, name); put32(f.input, at + 4, ns); put16(f.input, at + 8, static_cast<std::uint16_t>(refs.size()));
        f.input[at + 10] = params; f.input[at + 11] = flags; at += 12;
        for (const auto ref : refs) {
            put32(f.input, at, ref); at += 4;
            put16(f.input, ref, 0x1906); f.input[ref + 2] = 0;
        }
    };
    const auto selected = code + 0x20;
    std::vector<std::uint32_t> selected_refs{selected}; if (shared) selected_refs.push_back(split + 0x20);
    emit(original, space, 2, 0x25, selected_refs); // SYNTHETIC thread/method shape, not a retail pin.
    for (int i = 1; i < rows; ++i) emit(power::detail::fnv("getplayers"), 0, 0, 2, {split + 0x40u + 16u * static_cast<unsigned>(i - 1)});
    check(at <= code);
    f.c.independently_verified = true; // Synthetic fixture authorization ONLY.
    f.c.logical_name = "scripts/zm/_zm_powerups.gsc";
    f.c.source_crc = 0x12345678; f.c.code = code; f.c.code_size = code_size;
    f.c.owner = {0x1111, code, split, power::detail::fnv("show_on_hud"), space, 2, 0};
    f.c.reference = selected; f.c.opcode = 0x1906; f.c.site_parameter_byte = 0;
    const auto end = ((selected + 3u + 7u) & ~7u) + 8u;
    f.c.instruction.assign(f.input.begin() + selected, f.input.begin() + end);
    f.c.original_name = original; f.c.original_space = space;
    f.c.destination_name = power::detail::fnv("guarded_powerup_timer"); f.c.destination_space = power::detail::fnv("eroded_coop");
    f.c.import_params = 2; f.c.original_flags = 0x25; f.c.destination_flags = 0x05; f.c.sha256 = hash(f.input);
    return f;
}
using row_semantics = std::tuple<std::uint32_t, std::uint32_t, std::uint8_t, std::uint8_t>;
static auto imports(const bytes& data) {
    std::map<std::uint32_t, row_semantics> refs;
    auto at = t7::read32(data, 0x24);
    for (unsigned i = 0; i < power::detail::read16(data, 0x3c); ++i) {
        const auto count = power::detail::read16(data, at + 8);
        const row_semantics semantic{t7::read32(data, at), t7::read32(data, at + 4), data[at + 10], data[at + 11]};
        at += 12;
        for (unsigned j = 0; j < count; ++j) { check(refs.emplace(t7::read32(data, at), semantic).second); at += 4; }
    }
    return refs;
}
static auto includes(const bytes& data) {
    std::vector<std::string> result;
    for (unsigned i = 0; i < data[0x44]; ++i) {
        const auto name = t7::bounded_name(data, t7::read32(data, t7::read32(data, 0xc) + 4u * i));
        check(name.has_value()); result.push_back(*t7::canonical_script(*name));
    }
    return result;
}
static void verify(const fixture& f, const power::result& result, bool shared) {
    check(result.changed && result.bytes.size() <= power::output_limit);
    auto expected = imports(f.input); expected[f.c.reference] = {f.c.destination_name, f.c.destination_space, f.c.import_params, f.c.destination_flags};
    check(imports(result.bytes) == expected);
    auto deps = includes(f.input);
    if (std::find(deps.begin(), deps.end(), "custom_scripts/zm/eroded_coop") == deps.end()) deps.push_back("custom_scripts/zm/eroded_coop");
    check(includes(result.bytes) == deps);
    const auto table = t7::read32(f.input, 0x24);
    for (std::size_t i = 0; i < f.input.size(); ++i) {
        const bool include_header = (i >= 0xc && i < 0x10) || i == 0x44;
        const bool import_header = shared && ((i >= 0x24 && i < 0x28) || (i >= 0x3c && i < 0x3e));
        const bool selected_names = !shared && ((i >= table && i < table + 8) || i == table + 11);
        if (!include_header && !import_header && !selected_names) check(f.input[i] == result.bytes[i]);
    }
    check(t7::read32(result.bytes, 8) == f.c.source_crc);
    check(std::equal(f.input.begin() + f.c.code, f.input.begin() + f.c.code + f.c.code_size, result.bytes.begin() + f.c.code));
    check(std::equal(f.input.begin() + 0x60, f.input.begin() + 0x88, result.bytes.begin() + 0x60));
}
int main() {
    unsigned allocation_cuts = 0;
    for (bool shared : {false, true}) {
        for (bool present : {false, true}) for (int count : {0, 1, 2}) {
            auto f = make(shared, count, present); const auto before = f.input;
            auto result = power::transform(f.input, f.c, hash); verify(f, result, shared); check(f.input == before);
            check(!power::transform(result.bytes, f.c, hash).changed);
            auto saved = result.bytes; f.input.clear(); f.input.shrink_to_fit(); check(result.bytes == saved);
        }
        auto f = make(shared);
        for (std::size_t at = 0; at < f.input.size(); ++at) {
            auto bad = f.input; bad[at] ^= 1; check(!power::transform(bad, f.c, hash).changed);
        }
        for (int change = 0; change < 19; ++change) {
            auto c = f.c;
            switch (change) {
            case 0: c.independently_verified = false; break; case 1: c.sha256 = {}; break;
            case 2: c.logical_name = "scripts/zm/wrong.gsc"; break; case 3: ++c.source_crc; break;
            case 4: ++c.code; break; case 5: ++c.code_size; break; case 6: ++c.owner.checksum; break;
            case 7: ++c.owner.name; break; case 8: ++c.owner.space; break; case 9: ++c.owner.flags; break;
            case 10: ++c.owner.end; break; case 11: ++c.reference; break; case 12: ++c.opcode; break;
            case 13: ++c.site_parameter_byte; break; case 14: c.instruction[4] ^= 1; break;
            case 15: c.destination_name = c.original_name; c.destination_space = c.original_space; break;
            case 16: c.destination_name = 0; break; case 17: ++c.import_params; break; default: ++c.original_flags;
            }
            check(!power::transform(f.input, c, hash).changed);
        }
        for (int change = 0; change < 16; ++change) {
            auto bad = f.input; const auto table = t7::read32(bad, 0x24);
            switch (change) {
            case 0: put32(bad, 0x20, 0xfffffff0); break; case 1: put16(bad, 0x3a, 0); break;
            case 2: put32(bad, 0x78, f.c.owner.start); break; case 3: put32(bad, 0x24, 0xfffffff0); break;
            case 4: put16(bad, table + 8, 0); break; case 5: put16(bad, table + 8, 65535); break;
            case 6: bad[table + 11] = 6; break; case 7: bad[table + 11] |= 0xc0; break;
            case 8: put32(bad, table + 12, f.c.code - 1); break; case 9: put32(bad, 0xc, f.c.code); break;
            case 10: put32(bad, table, f.c.destination_name); put32(bad, table + 4, f.c.destination_space); break;
            case 11: put32(bad, 0x34, f.c.code); break; case 12: put32(bad, 0xc, table); break;
            case 13: put32(bad, 0x10, 0xfffffff0); break; case 14: put32(bad, 0x60, 0); break;
            default: put32(bad, table + 12, f.c.owner.end - 2);
            }
            auto c = f.c; c.sha256 = hash(bad); check(!power::transform(bad, c, hash).changed);
        }
        check(!power::transform(f.input, f.c, nullptr).changed);
        auto max = make(shared); max.input.resize(power::input_limit, 0); max.c.sha256 = hash(max.input);
        check(power::transform(max.input, max.c, hash).changed);
        max.input.push_back(0); max.c.sha256 = hash(max.input); check(!power::transform(max.input, max.c, hash).changed);
        auto too_many = make(shared, 255, false); check(!power::transform(too_many.input, too_many.c, hash).changed);
        auto existing = make(shared, 255, true); verify(existing, power::transform(existing.input, existing.c, hash), shared);
        for (long cut = 0; cut < 512; ++cut) {
            bool success = false;
            allocation_budget = cut;
            try { auto result = power::transform(f.input, f.c, hash); success = result.changed; }
            catch (const std::bad_alloc&) { ++allocation_cuts; }
            allocation_budget = -1;
            check(f.c.sha256 == hash(f.input));
            if (success) break;
            check(cut != 511);
        }
    }
    for (bool shared : {false, true}) {
        for (unsigned original = 0; original < 256; ++original) {
            for (unsigned destination = 0; destination < 256; ++destination) {
                auto f = make(shared);
                f.c.original_flags = static_cast<std::uint8_t>(original);
                f.c.destination_flags = static_cast<std::uint8_t>(destination);
                f.input[t7::read32(f.input, 0x24) + 11] = static_cast<std::uint8_t>(original);
                f.c.sha256 = hash(f.input);
                auto result = power::transform(f.input, f.c, hash);
                const bool canonical = (original == 0x05 || original == 0x25) && destination == 0x05;
                check(result.changed == canonical);
                if (canonical) verify(f, result, shared);
                else check(result.reason == "unsupported_call_flag_contract");
            }
        }
    }
    for (int rows : {4095, 4096}) {
        auto f = make(true, 1, false, rows); auto result = power::transform(f.input, f.c, hash);
        if (rows == 4095) verify(f, result, true); else check(!result.changed && result.reason == "import_count_limit");
    }
    std::cout << "checks=" << checks << " allocation_cuts=" << allocation_cuts
        << " synthetic_only=true retail_contract=false production=false\n";
}
