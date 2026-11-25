#pragma once

#include <cstdint>

namespace zombies::coop {
// Verified frontend targets: online CUSTOM402, LAN CUSTOM410. Screen400 also
// stages Solo (playlist150) and must never inherit CUSTOM's temporary cap8.
constexpr bool schema_host_eligible(bool frontend, int mode, int screen) noexcept {
  return mode == 0 || (mode == 1 && (!frontend || screen == 402 || screen == 410));
}
template <class SetZero, class PublishReset, class Fail>
void reset_schema_allocation(SetZero set_zero, PublishReset publish, Fail fail) noexcept {
  try { set_zero(); publish(); }
  catch (...) { try { fail(); } catch (...) {} }
}
class schema_context_policy {
public:
  enum class origin { none, host, peer_pending, peer_active };
  static constexpr std::int64_t peer_timeout_ms = 900000;
  void host() noexcept { origin_ = origin::host; deadline_ = 0; }
  void peer(std::int64_t now) noexcept { origin_ = origin::peer_pending; deadline_ = now + peer_timeout_ms; }
  void reset() noexcept { origin_ = origin::none; deadline_ = 0; }
  void active_peer() noexcept { origin_ = origin::peer_active; deadline_ = 0; }
  origin source() const noexcept { return origin_; }
  bool peer_owned() const noexcept { return origin_ == origin::peer_pending || origin_ == origin::peer_active; }
  // The verified descriptor may precede the engine's ZM mode transition. Do not
  // reset that allocation merely because the old frontend is still MP/CP.
  bool retain(std::int64_t now, bool frontend, bool zombies,
      bool host_matches, bool client_matches) noexcept {
    if (origin_ == origin::peer_pending) {
      if ((client_matches || !frontend) && zombies) { origin_ = origin::peer_active; return true; }
      return now >= deadline_ - peer_timeout_ms && now < deadline_;
    }
    if (origin_ == origin::peer_active && client_matches && zombies) return true;
    if (origin_ == origin::host && host_matches && zombies) return true;
    // Active map travel can briefly lack a native session. Only an actual
    // frontend/non-ZM transition clears a committed session, not shutdown_map.
    return origin_ != origin::none && !frontend && zombies;
  }
private:
  origin origin_ = origin::none;
  std::int64_t deadline_{};
};
} // namespace zombies::coop
