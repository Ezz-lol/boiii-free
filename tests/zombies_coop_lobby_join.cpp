#include "../src/client/component/zombies/lobby_join_policy.hpp"
#include <array>
#include <cassert>
#include <iostream>

using namespace zombies::coop;

int main()
{
    assert(advertised_lobby_route(false, true, "", "", "", false, false) == advertised_route::legacy);
    for (const auto limit : {"5", "6", "7", "8"})
    {
        assert(advertised_lobby_route(true, false, "1", "live", limit, true, false) == advertised_route::online);
        assert(advertised_lobby_route(true, false, "1", "lan", limit, true, true) == advertised_route::lan);
        assert(advertised_lobby_route(true, false, "1", "live", limit, false, true) == advertised_route::opt_in_required);
        assert(advertised_lobby_route(true, false, "1", "lan", limit, true, false) == advertised_route::invalid);
    }
    for (const auto invalid : {"", "0", "4", "9", "18", "05", " 8", "+8", "8x", "8 ", "-8", "999999999999999999999"})
        assert(advertised_lobby_route(true, false, "1", "live", invalid, true, true) == advertised_route::invalid);
    for (const auto revision : {"0", "2", "01", "1x", "1 "})
        assert(advertised_lobby_route(true, false, revision, "live", "8", true, true) == advertised_route::invalid);
    for (const auto mode : {"", "LIVE", "offline", "local", "systemlink", "lan "})
        assert(advertised_lobby_route(true, false, "1", mode, "8", true, true) == advertised_route::invalid);
    assert(advertised_lobby_route(false, false, "1", "live", "8", true, true) == advertised_route::invalid);
    assert(advertised_lobby_route(true, true, "1", "live", "8", true, true) == advertised_route::invalid);
    assert(local_ipv4_scope(10, 0) && local_ipv4_scope(172, 16) && local_ipv4_scope(172, 31));
    assert(local_ipv4_scope(192, 168) && local_ipv4_scope(169, 254) && local_ipv4_scope(127, 0));
    assert(local_ipv4_scope(100, 64) && local_ipv4_scope(100, 127) && local_ipv4_scope(25, 0) && local_ipv4_scope(26, 0));
    assert(!local_ipv4_scope(172, 15) && !local_ipv4_scope(172, 32) && !local_ipv4_scope(100, 63));
    assert(!local_ipv4_scope(100, 128) && !local_ipv4_scope(8, 8) && !local_ipv4_scope(0, 0));
    for (const auto route : {advertised_route::online, advertised_route::lan, advertised_route::invalid,
                            advertised_route::opt_in_required})
        assert(!raw_match_fallback_allowed(route));
    assert(raw_match_fallback_allowed(advertised_route::legacy));
    std::cout << "PASS: custom lobby transport descriptors, explicit opt-in, strict capacities and LAN scope\n";
}
