#include <std_include.hpp>
#include <loader/component_loader.hpp>

#include "scheduler.hpp"
#include "upnp.hpp"

#include <natupnp.h>

namespace upnp {
namespace {
std::mutex state_mutex;
std::string mapped_endpoint;
uint16_t mapped_port{};
std::atomic_bool operation_running{};

std::string local_ipv4() {
  const auto handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (handle == INVALID_SOCKET)
    return {};

  sockaddr_in route{};
  route.sin_family = AF_INET;
  route.sin_port = htons(53);
  inet_pton(AF_INET, "1.1.1.1", &route.sin_addr);

  std::string result;
  if (connect(handle, reinterpret_cast<sockaddr *>(&route), sizeof(route)) == 0) {
    sockaddr_in local{};
    int size = sizeof(local);
    if (getsockname(handle, reinterpret_cast<sockaddr *>(&local), &size) == 0) {
      char text[INET_ADDRSTRLEN]{};
      if (inet_ntop(AF_INET, &local.sin_addr, text, sizeof(text)))
        result = text;
    }
  }
  closesocket(handle);
  return result;
}

std::wstring widen(const std::string &text) { return {text.begin(), text.end()}; }

bool create_nat(CComPtr<IUPnPNAT> &nat,
                CComPtr<IStaticPortMappingCollection> &mappings) {
  if (FAILED(CoCreateInstance(CLSID_UPnPNAT, nullptr, CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(&nat))))
    return false;
  return SUCCEEDED(nat->get_StaticPortMappingCollection(&mappings)) && mappings;
}

void map_port(const uint16_t port) {
  const auto initialized = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  const bool uninitialize = SUCCEEDED(initialized);
  const auto local = local_ipv4();
  CComPtr<IUPnPNAT> nat;
  CComPtr<IStaticPortMappingCollection> mappings;
  if (local.empty() || !create_nat(nat, mappings)) {
    operation_running = false;
    if (uninitialize)
      CoUninitialize();
    return;
  }

  const CComBSTR protocol(L"UDP");
  const CComBSTR client(widen(local).c_str());
  const CComBSTR description(L"BOIII friend match");
  CComPtr<IStaticPortMapping> mapping;
  const auto result = mappings->Add(port, protocol, port, client, VARIANT_TRUE,
                                    description, &mapping);
  if (SUCCEEDED(result) && mapping) {
    BSTR external_raw{};
    if (SUCCEEDED(mapping->get_ExternalIPAddress(&external_raw)) && external_raw) {
      char external[INET_ADDRSTRLEN]{};
      WideCharToMultiByte(CP_ACP, 0, external_raw, -1, external,
                          sizeof(external), nullptr, nullptr);
      SysFreeString(external_raw);
      IN_ADDR parsed{};
      if (inet_pton(AF_INET, external, &parsed) == 1) {
        std::lock_guard lock(state_mutex);
        mapped_port = port;
        mapped_endpoint = std::string(external) + ":" + std::to_string(port);
      }
    }
  }
  operation_running = false;
  if (uninitialize)
    CoUninitialize();
}

void unmap_port(const uint16_t port) {
  const auto initialized = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  const bool uninitialize = SUCCEEDED(initialized);
  CComPtr<IUPnPNAT> nat;
  CComPtr<IStaticPortMappingCollection> mappings;
  if (create_nat(nat, mappings)) {
    const CComBSTR protocol(L"UDP");
    mappings->Remove(port, protocol);
  }
  if (uninitialize)
    CoUninitialize();
}
} // namespace

void open_port(const uint16_t port) {
  if (port < 1024 || operation_running.exchange(true))
    return;
  scheduler::once([port] { map_port(port); }, scheduler::async);
}

void close_port() {
  uint16_t port{};
  {
    std::lock_guard lock(state_mutex);
    port = mapped_port;
    mapped_port = 0;
    mapped_endpoint.clear();
  }
  if (port)
    scheduler::once([port] { unmap_port(port); }, scheduler::async);
}

std::string external_endpoint() {
  std::lock_guard lock(state_mutex);
  return mapped_endpoint;
}

class component final : public client_component {
public:
  void pre_destroy() override { close_port(); }
};
} // namespace upnp

REGISTER_COMPONENT(upnp::component)
