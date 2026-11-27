#include "../src/client/component/zombies/stock_session.hpp"
#include <cassert>
#include <iostream>

using zombies::coop::stock_session_allowed;

int main() {
  assert(stock_session_allowed("", false, "", false));
  assert(stock_session_allowed("usermaps", false, "", false));
  assert(stock_session_allowed("usermaps", true, "usermaps", false));
  assert(stock_session_allowed("", true, "usermaps", false));
  // A stale publisher does not imply its unloaded mod is still active.
  assert(stock_session_allowed("", false, "123456789", false));
  assert(!stock_session_allowed("123456789", false, "", false));
  assert(!stock_session_allowed("custom_mod", false, "", false));
  assert(!stock_session_allowed("usermaps", true, "123456789", false));
  assert(!stock_session_allowed("", true, "custom_mod", false));
  assert(!stock_session_allowed("usermaps", true, "", false));
  assert(!stock_session_allowed("Usermaps", false, "", false));
  assert(!stock_session_allowed("usermaps/", false, "", false));
  assert(!stock_session_allowed(" usermaps", false, "", false));
  for (const auto selected : {"", "usermaps"}) {
    assert(!stock_session_allowed(selected, false, "", true));
    assert(!stock_session_allowed(selected, true, "usermaps", true));
  }
  std::cout << "PASS: builtin usermaps bootstrap, active Workshop mods, "
               "unknown mods and usermaps\n";
}
