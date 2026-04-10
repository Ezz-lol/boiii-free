#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// min and max is required by gdi, therefore NOMINMAX won't work
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#include <string>
#include <functional>
#include <filesystem>

namespace utils::nt {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-attributes"
template <typename T, typename... Args>
using stdcall_t = T(__stdcall *)(Args...);
#pragma clang diagnostic pop

template <typename T, typename... Args> using cdecl_t = T(__cdecl *)(Args...);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-attributes"
template <typename T, typename This = void, typename... Args>
using thiscall_t = T(__thiscall *)(This *, Args...);
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-attributes"
template <typename T, typename... Args>
using stdcall_t = T(__fastcall *)(Args...);
#pragma clang diagnostic pop

class library final {
public:
  static library load(const char *name);
  static library load(const std::string &name);
  static library load(const std::filesystem::path &path);
  static library get_by_address(const void *address);

  library();
  explicit library(const std::string &name);
  explicit library(HMODULE handle);

  library &operator=(const library &) = default;
  library(const library &a) : module_(a.module_) {}

  bool operator!=(const library &obj) const { return !(*this == obj); };
  bool operator==(const library &obj) const;

  operator bool() const;
  operator HMODULE() const;

  void unprotect() const;
  [[nodiscard]] void *get_entry_point() const;
  [[nodiscard]] size_t get_relative_entry_point() const;

  [[nodiscard]] bool is_valid() const;
  [[nodiscard]] std::string get_name() const;
  [[nodiscard]] std::filesystem::path get_path() const;
  [[nodiscard]] std::filesystem::path get_folder() const;
  [[nodiscard]] std::uint8_t *get_ptr() const;
  void free();

  [[nodiscard]] HMODULE get_handle() const;

private:
  template <typename Fn> static Fn cast_proc(FARPROC proc) {
    static_assert(std::is_pointer_v<Fn>, "Fn must be function pointer");
    return reinterpret_cast<Fn>(proc);
  }

public:
  template <typename Fn> Fn get_proc(const char *name) const {
    if (!module_)
      return nullptr;

    FARPROC p = GetProcAddress(module_, name);
    if (!p)
      return nullptr;

    return cast_proc<Fn>(p);
  }

  template <typename Fn> Fn get_proc(const std::string &name) const {
    return get_proc<Fn>(name.c_str());
  }

  template <typename Fn> std::function<Fn> get(const std::string &name) const {
    Fn *fp = get_proc<Fn *>(name);
    if (!fp)
      return {};
    return std::function<Fn>(fp);
  }

  template <typename T, typename... Args>
  T invoke(const std::string &name, Args... args) const {
    auto f = get_proc<cdecl_t<T, Args...>>(name);
    if (!f)
      return T();

    return f(args...);
  }

  template <typename T, typename... Args>
  T invoke_pascal(const std::string &name, Args... args) const {
    auto f = get_proc<stdcall_t<T, Args...>>(name);
    if (!f)
      return T();

    return f(args...);
  }

  template <typename T, typename... Args>
  T invoke_this(const std::string &name, void *this_ptr, Args... args) const {
    auto f = get_proc<thiscall_t<T, void, Args...>>(name);
    if (!f)
      return T();

    return f(this_ptr, args...);
  }

  [[nodiscard]] std::vector<PIMAGE_SECTION_HEADER> get_section_headers() const;

  [[nodiscard]] PIMAGE_NT_HEADERS get_nt_headers() const;
  [[nodiscard]] PIMAGE_DOS_HEADER get_dos_header() const;
  [[nodiscard]] PIMAGE_OPTIONAL_HEADER get_optional_header() const;

  [[nodiscard]] void **get_iat_entry(const std::string &module_name,
                                     std::string proc_name) const;
  [[nodiscard]] void **get_iat_entry(const std::string &module_name,
                                     const char *proc_name) const;

private:
  HMODULE module_;
};

template <auto InvalidHandle = nullptr> class handle {
public:
  handle() = default;

  handle(const HANDLE h) : handle_(h) {}

  ~handle() {
    if (*this) {
      CloseHandle(this->handle_);
      this->handle_ = (HANDLE)InvalidHandle;
    }
  }

  handle(const handle &) = delete;
  handle &operator=(const handle &) = delete;

  handle(handle &&obj) noexcept : handle() { this->operator=(std::move(obj)); }

  handle &operator=(handle &&obj) noexcept {
    if (this != &obj) {
      this->~handle();
      this->handle_ = obj.handle_;
      obj.handle_ = (HANDLE)InvalidHandle;
    }

    return *this;
  }

  handle &operator=(HANDLE h) noexcept {
    this->~handle();
    this->handle_ = h;

    return *this;
  }

  operator bool() const { return this->handle_ != (HANDLE)InvalidHandle; }

  operator HANDLE() const { return this->handle_; }

private:
  HANDLE handle_{(HANDLE)InvalidHandle};
};

class registry_key {
public:
  registry_key() = default;

  registry_key(HKEY key) : key_(key) {}

  registry_key(const registry_key &) = delete;
  registry_key &operator=(const registry_key &) = delete;

  registry_key(registry_key &&obj) noexcept : registry_key() {
    this->operator=(std::move(obj));
  }

  registry_key &operator=(registry_key &&obj) noexcept {
    if (this != obj.GetRef()) {
      this->~registry_key();
      this->key_ = obj.key_;
      obj.key_ = nullptr;
    }

    return *this;
  }

  ~registry_key() {
    if (this->key_) {
      RegCloseKey(this->key_);
    }
  }

  operator HKEY() const { return this->key_; }

  operator bool() const { return this->key_ != nullptr; }

  HKEY *operator&() { return &this->key_; }

  registry_key *GetRef() { return this; }

  const registry_key *GetRef() const { return this; }

private:
  HKEY key_{};
};

registry_key open_or_create_registry_key(HKEY base, const std::string &input);

bool is_wine();
bool is_shutdown_in_progress();

__declspec(noreturn) void raise_hard_exception();
std::string load_resource(int id);

void relaunch_self();
__declspec(noreturn) void terminate(uint32_t code = 0);

std::string get_user_name();
} // namespace utils::nt