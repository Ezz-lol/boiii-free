#pragma once

#include <std_include.hpp>

#include <steam/common.hpp>

#ifdef interface
#undef interface
#endif

namespace steam {
typedef std::unordered_map<std::string, void *> methodTable_t;

class interface final {
public:
  interface();
  interface(void *interface_ptr);

  operator bool() const;

  template <typename T, typename... Args>
  T invoke(const std::string &method_name, Args... args) {
    if (!this->interface_ptr_) {
      throw std::runtime_error("Invalid interface pointer");
    }

    const void *method = this->find_method(method_name);
    if (!method) {
      throw std::runtime_error("Unable to find method: " + method_name);
    }

    return static_cast<T(__thiscall *)(void *, Args...)>(method)(
        this->interface_ptr_, args...);
  }

  template <typename T, typename... Args>
  T invoke(const size_t table_entry, Args... args) {
    if (!this->interface_ptr_) {
      throw std::runtime_error("Invalid interface pointer");
    }

    return static_cast<T(__thiscall *)(void *, Args...)>(
        (*this->interface_ptr_)[table_entry])(this->interface_ptr_, args...);
  }

private:
  void ***interface_ptr_;
  methodTable_t methods_;

  void *find_method(const std::string &name);
  void *search_method(const std::string &name);

  std::string analyze_method(const void *method_ptr);
};
} // namespace steam
