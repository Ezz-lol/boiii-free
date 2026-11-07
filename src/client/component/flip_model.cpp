#include <std_include.hpp>

#include <game/utils.hpp>
#include <loader/component_loader.hpp>

#include <utils/flags.hpp>
#include <utils/hook.hpp>

namespace flip_model {
namespace {
constexpr size_t FACTORY_CREATE_SWAP_CHAIN = 10;
constexpr size_t SWAP_CHAIN_PRESENT = 8;
constexpr size_t SWAP_CHAIN_RESIZE_BUFFERS = 13;

bool tearing_supported = false;

using present_t = HRESULT(STDMETHODCALLTYPE *)(IDXGISwapChain *, UINT, UINT);
using resize_buffers_t = HRESULT(STDMETHODCALLTYPE *)(IDXGISwapChain *, UINT,
                                                      UINT, UINT, DXGI_FORMAT,
                                                      UINT);
present_t original_present = nullptr;
resize_buffers_t original_resize_buffers = nullptr;

utils::hook::detour create_swap_chain_hook;

// Every swap chain shares one vtable, so the stubs also run for swap chains
// that were never converted and have to check which kind they're handed
HRESULT STDMETHODCALLTYPE present_stub(IDXGISwapChain *swap_chain,
                                       const UINT sync_interval, UINT flags) {
  // Tearing isn't allowed in exclusive fullscreen or on test presents
  if (sync_interval == 0 && (flags & DXGI_PRESENT_TEST) == 0) {
    DXGI_SWAP_CHAIN_DESC desc{};
    BOOL fullscreen = FALSE;
    if (SUCCEEDED(swap_chain->GetDesc(&desc)) &&
        (desc.Flags & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING) &&
        SUCCEEDED(swap_chain->GetFullscreenState(&fullscreen, nullptr)) &&
        !fullscreen) {
      flags |= DXGI_PRESENT_ALLOW_TEARING;
    }
  }
  return original_present(swap_chain, sync_interval, flags);
}

HRESULT STDMETHODCALLTYPE resize_buffers_stub(IDXGISwapChain *swap_chain,
                                              UINT count, const UINT width,
                                              const UINT height,
                                              const DXGI_FORMAT format,
                                              UINT flags) {
  DXGI_SWAP_CHAIN_DESC desc{};
  if (SUCCEEDED(swap_chain->GetDesc(&desc))) {
    // Resizing has to keep the flags the swap chain was created with
    flags |= desc.Flags & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    if (desc.SwapEffect == DXGI_SWAP_EFFECT_FLIP_DISCARD && count == 1) {
      count = 2;
    }
  }
  return original_resize_buffers(swap_chain, count, width, height, format,
                                 flags);
}

// download_overlay already detours Present, and a second detour on it fails
void hook_swap_chain(IDXGISwapChain *swap_chain) {
  void **vtable = *reinterpret_cast<void ***>(swap_chain);
  if (vtable[SWAP_CHAIN_PRESENT] == reinterpret_cast<void *>(present_stub)) {
    return;
  }
  original_present = reinterpret_cast<present_t>(vtable[SWAP_CHAIN_PRESENT]);
  original_resize_buffers =
      reinterpret_cast<resize_buffers_t>(vtable[SWAP_CHAIN_RESIZE_BUFFERS]);
  utils::hook::set(&vtable[SWAP_CHAIN_PRESENT],
                   reinterpret_cast<void *>(present_stub));
  utils::hook::set(&vtable[SWAP_CHAIN_RESIZE_BUFFERS],
                   reinterpret_cast<void *>(resize_buffers_stub));
}

bool can_flip(const DXGI_SWAP_CHAIN_DESC &desc) {
  // The swap chain is created before the player config is loaded, so a dvar
  // can't turn this off
  static const bool disabled = utils::flags::has_flag("noflipmodel");
  // Exclusive fullscreen already flips directly
  if (disabled || (*game::r_fullscreen && game::r_fullscreen->get_int() == 1)) {
    return false;
  }

  switch (desc.BufferDesc.Format) {
  case DXGI_FORMAT_R8G8B8A8_UNORM:
  case DXGI_FORMAT_B8G8R8A8_UNORM:
  case DXGI_FORMAT_R10G10B10A2_UNORM:
  case DXGI_FORMAT_R16G16B16A16_FLOAT:
    return desc.SampleDesc.Count == 1 &&
           desc.SwapEffect == DXGI_SWAP_EFFECT_DISCARD;
  default:
    return false;
  }
}

// The game presents through the desktop compositor in windowed and borderless,
// which delays every frame to the next refresh and repeats or drops frames
HRESULT STDMETHODCALLTYPE create_swap_chain_stub(IDXGIFactory *factory,
                                                 IUnknown *device,
                                                 DXGI_SWAP_CHAIN_DESC *desc,
                                                 IDXGISwapChain **swap_chain) {
  if (desc == nullptr || !can_flip(*desc)) {
    return create_swap_chain_hook.invoke<HRESULT>(factory, device, desc,
                                                  swap_chain);
  }

  DXGI_SWAP_CHAIN_DESC flip = *desc;
  flip.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  flip.BufferCount = std::max(desc->BufferCount, 2u);
  if (tearing_supported) {
    flip.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
  }

  const HRESULT result = create_swap_chain_hook.invoke<HRESULT>(
      factory, device, &flip, swap_chain);
  if (FAILED(result)) {
    return create_swap_chain_hook.invoke<HRESULT>(factory, device, desc,
                                                  swap_chain);
  }
  hook_swap_chain(*swap_chain);
  return result;
}

void hook_create_swap_chain() {
  IDXGIFactory5 *factory = nullptr;
  if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
    return;
  }

  BOOL allow_tearing = FALSE;
  tearing_supported = SUCCEEDED(factory->CheckFeatureSupport(
                          DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allow_tearing,
                          sizeof(allow_tearing))) &&
                      allow_tearing;

  void **vtable = *reinterpret_cast<void ***>(factory);
  create_swap_chain_hook.create(vtable[FACTORY_CREATE_SWAP_CHAIN],
                                create_swap_chain_stub);
  factory->Release();
}
} // namespace

struct component final : client_component {
  DEFINE_COMPONENT_NAME("flip_model");

  void post_unpack() override { hook_create_swap_chain(); }
};
} // namespace flip_model

REGISTER_COMPONENT(flip_model::component)
