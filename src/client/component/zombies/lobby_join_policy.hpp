#pragma once

#include <charconv>
#include <cstdint>
#include <optional>
#include <string_view>

namespace zombies::coop
{
    // A transport descriptor, not an assertion that client script schemas match.
    enum class advertised_route { legacy, online, lan, invalid, opt_in_required };

    constexpr bool raw_match_fallback_allowed(advertised_route route) noexcept
    {
        // Native privacy, content and reservation refusals remain authoritative.
        return route == advertised_route::legacy;
    }

    constexpr bool local_ipv4_scope(std::uint8_t a, std::uint8_t b) noexcept
    {
        return a == 10 || a == 127 || (a == 172 && b >= 16 && b <= 31)
            || (a == 192 && b == 168) || (a == 169 && b == 254)
            || (a == 100 && b >= 64 && b <= 127) || a == 25 || a == 26;
    }

    inline std::optional<int> parse_lobby_capacity(std::string_view text) noexcept
    {
        if (text.empty() || text.size() > 2 || text.front() < '1' || text.front() > '9')
            return std::nullopt;
        int value{};
        const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
        if (result.ec != std::errc{} || result.ptr != text.data() + text.size())
            return std::nullopt;
        return value;
    }

    inline advertised_route advertised_lobby_route(bool zombies, bool dedicated,
        std::string_view revision, std::string_view network, std::string_view capacity,
        bool enabled, bool local_scope) noexcept
    {
        if (revision.empty()) return advertised_route::legacy;
        const auto limit = parse_lobby_capacity(capacity);
        if (!zombies || dedicated || revision != "1" || !limit || *limit < 5 || *limit > 8)
            return advertised_route::invalid;
        if (network != "live" && network != "lan") return advertised_route::invalid;
        if (network == "lan" && !local_scope) return advertised_route::invalid;
        if (!enabled) return advertised_route::opt_in_required;
        return network == "lan" ? advertised_route::lan : advertised_route::online;
    }
}
