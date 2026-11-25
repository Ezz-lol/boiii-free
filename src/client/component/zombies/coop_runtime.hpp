#pragma once
namespace zombies::coop {
void validate_startup();
void check_startup();
void shutdown_map() noexcept;
bool admission_ready() noexcept;
bool expanded_host() noexcept;
} // namespace zombies::coop
