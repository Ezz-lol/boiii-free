#include <std_include.hpp>

#include <stack>

#include "tac.hpp"

namespace game {
namespace tac {
static void *callstack_proxy_addr{nullptr};
static thread_local const void *address_to_call{};

void set_address_to_call(const void *address) { address_to_call = address; }

thread_local std::stack<uintptr_t> address_stack{};

const void *get_address_to_call() { return address_to_call; }

void store_address(const uintptr_t address) { address_stack.push(address); }

uintptr_t get_stored_address() {
  const uintptr_t res = address_stack.top();
  address_stack.pop();

  return res;
}
} // namespace tac
} // namespace game
