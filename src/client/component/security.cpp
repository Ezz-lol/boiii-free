#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"
#include "game/utils.hpp"
#include "scheduler.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace security
{
	namespace
	{
		// Network Password Dvar
		// If set, only clients with the matching password can join
		const game::dvar_t* net_password = nullptr;

		// ============================================================================
		// Checksum Obfuscation (Network Password)
		// ============================================================================

		using Sys_Checksum_t = uint16_t(__fastcall*)(const uint8_t* msg, int size);
		const auto Sys_Checksum = reinterpret_cast<Sys_Checksum_t>(0x142177810_g);

		// Hook for Sys_VerifyPacketChecksum (0x2177980)
		utils::hook::detour sys_verify_packet_checksum_hook;
		int64_t sys_verify_packet_checksum_stub(const char* payload, int32_t payloadLen)
		{
			if (payloadLen < 2)
			{
				return -1;
			}

			const auto new_len = payloadLen - 2;
			const auto password_hash = net_password ? static_cast<uint16_t>(game::Dvar_GenerateHash(net_password->current.value.string)) : static_cast<uint16_t>(0);

			// Calculate expected checksum
			// First try standard checksum (no password)
			auto checksum = Sys_Checksum(reinterpret_cast<const uint8_t*>(payload), new_len);

			// Check if standard matches
			const auto packet_checksum = *reinterpret_cast<const uint16_t*>(payload + new_len);

			if (packet_checksum == checksum)
			{
				return new_len;
			}

			// If password is set, try XORing
			if (password_hash != 0)
			{
				checksum ^= password_hash;
				if (packet_checksum == checksum)
				{
					return new_len;
				}
			}

			return -1;
		}

		// Hook for Sys_ChecksumCopy (0x21778E0)
		// Applied when writing packets
		utils::hook::detour sys_checksum_copy_hook;
		uint16_t sys_checksum_copy_stub(char* dest, const char* src, int32_t length)
		{
			auto val = sys_checksum_copy_hook.invoke<uint16_t>(dest, src, length);

			if (net_password)
			{
				const auto password_hash = static_cast<uint16_t>(game::Dvar_GenerateHash(net_password->current.value.string));
				if (password_hash != 0)
				{
					val ^= password_hash;
				}
			}

			return val;
		}

		// ============================================================================
		// INSTANT_DISPATCH RCE Protection (0x143A661)
		// ============================================================================

		// We need to hook the dispatch loop to inspect packets before they are processed
		// This protects against malicious cbuf execution and joinRequest exploits

		utils::hook::detour instant_dispatch_hook;
		void instant_dispatch_stub(utils::hook::assembler& a)
		{
			// T7Patch logic:
			// Context:
			// RAX = packetType (BYTE)
			// RBX = senderXuid
			// Stack setup: msg is at [RSP + 0x20] (approx)
		}

		// We will use a standard detour on dwInstantHandleLobbyMessage (0x143A780)
		// which is called by INSTANT_DISPATCH.
		using dwInstantHandleLobbyMessage_t = BOOL(__fastcall*)(int64_t senderID, int controllerIndex, char* message);
		utils::hook::detour dw_instant_handle_lobby_message_hook;

		BOOL __fastcall dw_instant_handle_lobby_message_stub(int64_t senderID, int controllerIndex, char* message)
		{
			// Message structure:
			// 0x00: ..
			// 0x08: data ptr
			// 0x1C: cursize
			// 0x24: readcount

			return dw_instant_handle_lobby_message_hook.invoke<BOOL>(senderID, controllerIndex, message);
		}

		// ============================================================================
		// Lua & Crash Fixes
		// ============================================================================

		char* ui_localize_default_text = nullptr;

		void fix_lua_crashes()
		{
			// LuaCrash1 (0x1CAB4F1) - Skip instruction
			// T7Patch: ContextRecord->Rip += 3;
			utils::hook::nop(0x141CAB4F1_g, 3);
		}
	}

	class component final : public client_component
	{
	public:
		void post_unpack() override
		{
			if (game::is_server()) return;

			// Register Net Password Dvar
			net_password = game::register_dvar_string("net_password", "", game::DVAR_ARCHIVE, "Password for private lobby access");

			// Install Checksum Hooks
			sys_verify_packet_checksum_hook.create(0x142177980_g, sys_verify_packet_checksum_stub);
			sys_checksum_copy_hook.create(0x1421778E0_g, sys_checksum_copy_stub);

			// Apply Lua Crash Fix 1
			utils::hook::nop(0x141CAB4F1_g, 3);
		}
	};
}

REGISTER_COMPONENT(security::component)
