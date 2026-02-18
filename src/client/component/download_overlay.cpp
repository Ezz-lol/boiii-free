#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"
#include "scheduler.hpp"

#include "download_overlay.hpp"

#include <utils/hook.hpp>
#include <utils/concurrency.hpp>

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

// Declared in imgui_impl_win32.cpp but guarded by #if 0 in the header
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace download_overlay
{
	namespace
	{
		utils::concurrency::container<download_state> state_{};
		utils::hook::detour present_hook{};
		bool imgui_initialized{false};
		HWND game_hwnd{nullptr};
		WNDPROC original_wndproc{nullptr};

		// Forward declaration
		LRESULT CALLBACK wndproc_stub(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

		void render()
		{
			const auto s = state_.copy();
			if (!s.active) return;

			ImGui::SetNextWindowPos(ImVec2(10.f, 10.f), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(440.f, 0.f), ImGuiCond_Always);

			constexpr ImGuiWindowFlags flags =
				ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize;

			if (!ImGui::Begin("Downloading Content", nullptr, flags))
			{
				ImGui::End();
				return;
			}

			// Title line
			ImGui::TextUnformatted(s.item_name.c_str());

			// Secondary info (file name / file count)
			if (!s.status_line.empty())
			{
				ImGui::TextDisabled("%s", s.status_line.c_str());
			}

			ImGui::Spacing();

			// Progress bar
			if (s.total_bytes > 0)
			{
				const float pct = static_cast<float>(s.downloaded_bytes) / static_cast<float>(s.total_bytes);
				char overlay[32];
				snprintf(overlay, sizeof(overlay), "%.1f%%", pct * 100.f);
				ImGui::ProgressBar(pct, ImVec2(-1.f, 0.f), overlay);

				ImGui::Text("%.2f MB / %.2f MB",
					s.downloaded_bytes / 1048576.f,
					s.total_bytes / 1048576.f);
			}
			else
			{
				// Indeterminate — no total size known
				const float t = static_cast<float>(ImGui::GetTime());
				ImGui::ProgressBar(-1.f * (t - floorf(t)), ImVec2(-1.f, 0.f), "Downloading...");
				if (s.downloaded_bytes > 0)
					ImGui::Text("%.2f MB downloaded", s.downloaded_bytes / 1048576.f);
			}

			ImGui::Spacing();

			// Speed and ETA on same row when both available
			const bool has_speed = s.speed_bps > 1024.f;
			const bool has_eta   = s.eta_seconds >= 0;

			if (has_speed)
			{
				if (s.speed_bps >= 1048576.f)
					ImGui::Text("Speed: %.2f MB/s", s.speed_bps / 1048576.f);
				else
					ImGui::Text("Speed: %.1f KB/s", s.speed_bps / 1024.f);

				if (has_eta) ImGui::SameLine(0.f, 24.f);
			}
			if (has_eta)
			{
				if (s.eta_seconds >= 3600)
					ImGui::Text("ETA: %dh %dm", s.eta_seconds / 3600, (s.eta_seconds % 3600) / 60);
				else if (s.eta_seconds >= 60)
					ImGui::Text("ETA: %dm %ds", s.eta_seconds / 60, s.eta_seconds % 60);
				else
					ImGui::Text("ETA: %ds", s.eta_seconds);
			}

			// Stop button
			if (s.on_cancel)
			{
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
				ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.65f, 0.15f, 0.15f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.20f, 0.20f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.50f, 0.10f, 0.10f, 1.f));
				if (ImGui::Button("Stop Download", ImVec2(-1.f, 0.f)))
				{
					// Fire callback and clear — copy it out first to avoid holding the lock
					const auto cb = s.on_cancel;
					clear();
					if (cb) cb();
				}
				ImGui::PopStyleColor(3);
			}

			ImGui::End();
		}

		LRESULT CALLBACK wndproc_stub(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
		{
			if (imgui_initialized)
				ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
			return CallWindowProcA(original_wndproc, hwnd, msg, wparam, lparam);
		}

		HRESULT __stdcall present_stub(IDXGISwapChain* swap_chain, UINT sync_interval, UINT flags)
		{
			if (!imgui_initialized)
			{
				ID3D11Device* device = nullptr;
				if (SUCCEEDED(swap_chain->GetDevice(IID_PPV_ARGS(&device))))
				{
					ID3D11DeviceContext* context = nullptr;
					device->GetImmediateContext(&context);

					DXGI_SWAP_CHAIN_DESC desc{};
					swap_chain->GetDesc(&desc);

					ImGui::CreateContext();
					ImGui::GetIO().IniFilename = nullptr;
					ImGui::GetIO().LogFilename = nullptr;

					ImGui_ImplWin32_Init(desc.OutputWindow);
					ImGui_ImplDX11_Init(device, context);

					// Hook WndProc so mouse/keyboard reach ImGui (needed for the Stop button)
					game_hwnd = desc.OutputWindow;
					original_wndproc = reinterpret_cast<WNDPROC>(
						SetWindowLongPtrA(game_hwnd, GWLP_WNDPROC,
							reinterpret_cast<LONG_PTR>(wndproc_stub)));

					device->Release();
					context->Release();
					imgui_initialized = true;
				}
			}

			if (imgui_initialized)
			{
				ImGui_ImplDX11_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();
				render();
				ImGui::Render();
				ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			}

			return present_hook.invoke<HRESULT>(swap_chain, sync_interval, flags);
		}

		void setup_present_hook()
		{
			// Create a tiny invisible window just for vtable theft —
			// GetDesktopWindow() is not a valid swap chain output.
			WNDCLASSEXA wc{};
			wc.cbSize        = sizeof(wc);
			wc.lpfnWndProc   = DefWindowProcA;
			wc.hInstance     = GetModuleHandleA(nullptr);
			wc.lpszClassName = "DX11Dummy_DO";
			RegisterClassExA(&wc);

			HWND dummy_hwnd = CreateWindowExA(0, wc.lpszClassName, nullptr,
				WS_OVERLAPPED, 0, 0, 1, 1, nullptr, nullptr, wc.hInstance, nullptr);
			if (!dummy_hwnd)
			{
				UnregisterClassA(wc.lpszClassName, wc.hInstance);
				return;
			}

			DXGI_SWAP_CHAIN_DESC sd{};
			sd.BufferCount       = 1;
			sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sd.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.OutputWindow      = dummy_hwnd;
			sd.SampleDesc.Count  = 1;
			sd.Windowed          = TRUE;
			sd.SwapEffect        = DXGI_SWAP_EFFECT_DISCARD;

			ID3D11Device*        dummy_device  = nullptr;
			ID3D11DeviceContext* dummy_context = nullptr;
			IDXGISwapChain*      dummy_chain   = nullptr;
			D3D_FEATURE_LEVEL    fl{};

			const HRESULT hr = D3D11CreateDeviceAndSwapChain(
				nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
				nullptr, 0, D3D11_SDK_VERSION,
				&sd, &dummy_chain, &dummy_device, &fl, &dummy_context);

			DestroyWindow(dummy_hwnd);
			UnregisterClassA(wc.lpszClassName, wc.hInstance);

			if (FAILED(hr)) return;

			// vtable index 8 = IDXGISwapChain::Present
			void** vtable = *reinterpret_cast<void***>(dummy_chain);
			present_hook.create(vtable[8], present_stub);

			dummy_chain->Release();
			dummy_device->Release();
			dummy_context->Release();
		}
	}

	void update(const download_state& s)
	{
		state_.access([&](download_state& st) { st = s; });
	}

	void clear()
	{
		state_.access([](download_state& st) { st = {}; });
	}

	struct component final : client_component
	{
		void post_unpack() override
		{
			scheduler::once(setup_present_hook, scheduler::async);
		}

		void pre_destroy() override
		{
			if (imgui_initialized)
			{
				// Restore original WndProc before shutting down
				if (game_hwnd && original_wndproc)
					SetWindowLongPtrA(game_hwnd, GWLP_WNDPROC,
						reinterpret_cast<LONG_PTR>(original_wndproc));

				ImGui_ImplDX11_Shutdown();
				ImGui_ImplWin32_Shutdown();
				ImGui::DestroyContext();
			}
		}
	};
}

REGISTER_COMPONENT(download_overlay::component)
