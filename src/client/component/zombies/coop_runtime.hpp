#pragma once
namespace zombies::coop {
void validate_startup();
void check_startup();
void shutdown_map() noexcept;
bool admission_ready() noexcept;
// Atomic only: no VM, dvar, or state_mutex access.
int active_startup_epoch() noexcept;
bool session_ending() noexcept;
bool mark_session_ending(int expected_epoch) noexcept;
bool expanded_host() noexcept;
} // namespace zombies::coop
