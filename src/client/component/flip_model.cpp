#include <std_include.hpp>

#include "imgui_overlay.hpp"

#include <game/utils.hpp>
#include <loader/component_loader.hpp>

#include <utils/concurrency.hpp>
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
struct swap_chain_methods {
  present_t present;
  resize_buffers_t resize_buffers;
};

// keep the original methods per vtable
// swap chains can share a table or use different ones
utils::concurrency::container<std::map<void **, swap_chain_methods>> methods;
// draw on the game chain in either mode
// only apply flip-model rules to the chain we converted
std::atomic<IDXGISwapChain *> game_swap_chain{nullptr};
std::atomic<IDXGISwapChain *> converted_swap_chain{nullptr};

/**
 * @brief Retrieve the original methods for the vtable used by this chain.
 * @param swap_chain The object passed into one of our patched vtable entries.
 * @return A copy of the methods saved before that vtable was patched.
 */
swap_chain_methods get_original_methods(IDXGISwapChain *swap_chain) {
  auto **vtable = *reinterpret_cast<void ***>(swap_chain);
  return methods.access<swap_chain_methods>(
      [vtable](const auto &tables) { return tables.at(vtable); });
}

utils::hook::detour create_swap_chain_hook;

HRESULT STDMETHODCALLTYPE present_stub(IDXGISwapChain *swap_chain,
                                       const UINT sync_interval, UINT flags) {
  const auto original = get_original_methods(swap_chain);
  // test presents never draw a frame
  if ((flags & DXGI_PRESENT_TEST) != 0) {
    return original.present(swap_chain, sync_interval, flags);
  }
  // draw before Present sends the back buffer to the screen
  if (swap_chain == game_swap_chain.load()) {
    imgui_overlay::render_frame(swap_chain);
  }

  // only allow tearing on a converted windowed chain created with that flag
  if (swap_chain == converted_swap_chain.load() && sync_interval == 0) {
    DXGI_SWAP_CHAIN_DESC desc{};
    BOOL fullscreen = FALSE;
    if (SUCCEEDED(swap_chain->GetDesc(&desc)) &&
        (desc.Flags & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING) &&
        SUCCEEDED(swap_chain->GetFullscreenState(&fullscreen, nullptr)) &&
        !fullscreen) {
      flags |= DXGI_PRESENT_ALLOW_TEARING;
    }
  }
  return original.present(swap_chain, sync_interval, flags);
}

HRESULT STDMETHODCALLTYPE resize_buffers_stub(IDXGISwapChain *swap_chain,
                                              UINT count, const UINT width,
                                              const UINT height,
                                              const DXGI_FORMAT format,
                                              UINT flags) {
  DXGI_SWAP_CHAIN_DESC desc{};
  // leave stock resize args alone
  if (swap_chain == converted_swap_chain.load() &&
      SUCCEEDED(swap_chain->GetDesc(&desc))) {
    // Resizing has to keep the flags the swap chain was created with
    flags |= desc.Flags & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    if (desc.SwapEffect == DXGI_SWAP_EFFECT_FLIP_DISCARD && count == 1) {
      count = 2;
    }
  }
  return get_original_methods(swap_chain)
      .resize_buffers(swap_chain, count, width, height, format, flags);
}

/**
 * @brief Intercept frame presentation on the swap chain the game created.
 * @param swap_chain The chain returned by the game's DXGI factory.
 * @param converted Whether BOIII substituted a flip-model descriptor.
 */
void hook_swap_chain(IDXGISwapChain *swap_chain, bool converted) {
  auto **vtable = *reinterpret_cast<void ***>(swap_chain);
  // patch each shared vtable once
  methods.access([vtable](auto &tables) {
    if (tables.contains(vtable)) {
      return;
    }
    // save the originals before another thread can call our hooks
    tables.emplace(vtable,
                   swap_chain_methods{
                       reinterpret_cast<present_t>(vtable[SWAP_CHAIN_PRESENT]),
                       reinterpret_cast<resize_buffers_t>(
                           vtable[SWAP_CHAIN_RESIZE_BUFFERS])});
    utils::hook::set(&vtable[SWAP_CHAIN_PRESENT],
                     reinterpret_cast<void *>(present_stub));
    utils::hook::set(&vtable[SWAP_CHAIN_RESIZE_BUFFERS],
                     reinterpret_cast<void *>(resize_buffers_stub));
  });
  // track the new chain even if it shares a vtable we already patched
  converted_swap_chain = converted ? swap_chain : nullptr;
  game_swap_chain = swap_chain;
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
  if (desc && can_flip(*desc)) {
    DXGI_SWAP_CHAIN_DESC flip = *desc;
    flip.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    flip.BufferCount = std::max(desc->BufferCount, 2u);
    if (tearing_supported) {
      flip.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    }

    const HRESULT result = create_swap_chain_hook.invoke<HRESULT>(
        factory, device, &flip, swap_chain);
    if (SUCCEEDED(result)) {
      hook_swap_chain(*swap_chain, true);
      return result;
    }
  }

  // hook stock chains too, including fullscreen, -noflipmodel and failed flip creation
  const HRESULT result =
      create_swap_chain_hook.invoke<HRESULT>(factory, device, desc, swap_chain);
  if (SUCCEEDED(result)) {
    hook_swap_chain(*swap_chain, false);
  }
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
