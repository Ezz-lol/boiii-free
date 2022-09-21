#include <std_include.hpp>

#include "auth3_server.hpp"
#include "../keys.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

namespace demonware
{
	namespace
	{
#pragma pack(push, 1)
		struct auth_ticket
		{
			unsigned int m_magicNumber;
			char m_type;
			unsigned int m_titleID;
			unsigned int m_timeIssued;
			unsigned int m_timeExpires;
			unsigned __int64 m_licenseID;
			unsigned __int64 m_userID;
			char m_username[64];
			char m_sessionKey[24];
			char m_usingHashMagicNumber[3];
			char m_hash[4];
		};
#pragma pack(pop)
	}

	void auth3_server::send_reply(reply* data)
	{
		if (!data) return;
		this->send(data->data());
	}

	void auth3_server::handle(const std::string& packet)
	{
		if (packet.starts_with("POST /auth/"))
		{
#ifndef NDEBUG
			printf("[DW]: [auth]: user requested authentication.\n");
#endif
			return;
		}

		unsigned int title_id = 0;
		unsigned int iv_seed = 0;
		std::string identity{};
		std::string token{};

		rapidjson::Document j;
		j.Parse(packet.data(), packet.size());

		if (j.HasMember("title_id") && j["title_id"].IsString())
		{
			title_id = std::stoul(j["title_id"].GetString());
		}

		if (j.HasMember("iv_seed") && j["iv_seed"].IsString())
		{
			iv_seed = std::stoul(j["iv_seed"].GetString());
		}

		if (j.HasMember("identity") && j["identity"].IsString())
		{
			identity = j["identity"].GetString();
		}

		if (j.HasMember("extra_data") && j["extra_data"].IsString())
		{
			rapidjson::Document extra_data;
			auto& ed = j["extra_data"];
			extra_data.Parse(ed.GetString(), ed.GetStringLength());

			if (extra_data.HasMember("token") && extra_data["token"].IsString())
			{
				auto& token_field = extra_data["token"];
				std::string token_b64(token_field.GetString(), token_field.GetStringLength());
				token = utils::cryptography::base64::decode(token_b64);
			}
		}

#ifndef NDEBUG
		printf("[DW]: [auth]: authenticating user %s\n", token.data() + 64);
#endif

		std::string auth_key(reinterpret_cast<char*>(token.data() + 32), 24);
		std::string session_key(
			"\x13\x37\x13\x37\x13\x37\x13\x37\x13\x37\x13\x37\x13\x37\x13\x37\x13\x37\x13\x37\x13\x37\x13\x37", 24);

		// client_ticket
		auth_ticket ticket{};
		std::memset(&ticket, 0x0, sizeof ticket);
		ticket.m_magicNumber = 0x0EFBDADDE;
		ticket.m_type = 0;
		ticket.m_titleID = title_id;
		ticket.m_timeIssued = static_cast<uint32_t>(time(nullptr));
		ticket.m_timeExpires = ticket.m_timeIssued + 30000;
		ticket.m_licenseID = 0;
		ticket.m_userID = reinterpret_cast<uint64_t>(token.data() + 56);
		strncpy_s(ticket.m_username, sizeof(ticket.m_username), reinterpret_cast<char*>(token.data() + 64), 64);
		std::memcpy(ticket.m_sessionKey, session_key.data(), 24);

		const auto iv = utils::cryptography::tiger::compute(std::string(reinterpret_cast<char*>(&iv_seed), 4));
		const auto ticket_enc = utils::cryptography::des3::encrypt(
			std::string(reinterpret_cast<char*>(&ticket), sizeof(ticket)), iv, auth_key);
		const auto ticket_b64 = utils::cryptography::base64::encode(
			reinterpret_cast<const unsigned char*>(ticket_enc.data()), 128);

		// server_ticket
		uint8_t auth_data[128];
		std::memset(&auth_data, 0, sizeof auth_data);
		std::memcpy(auth_data, session_key.data(), 24);
		const auto auth_data_b64 = utils::cryptography::base64::encode(auth_data, 128);

		demonware::set_session_key(session_key);

		// header time
		char date[64];
		const auto now = time(nullptr);
		tm gmtm{};
		gmtime_s(&gmtm, &now);
		strftime(date, 64, "%a, %d %b %G %T", &gmtm);

		rapidjson::Document extra;
		extra.SetObject();

		std::string username = std::string(ticket.m_username, sizeof(ticket.m_username)).data();
		extra.AddMember("username", username, extra.GetAllocator());
		extra.AddMember("time_to_live", 9999, extra.GetAllocator());

		const auto lul = utils::cryptography::base64::encode("lul");
		extra.AddMember("extended_data", lul, extra.GetAllocator());

		rapidjson::StringBuffer extra_buffer{};
		rapidjson::Writer<rapidjson::StringBuffer, rapidjson::Document::EncodingType, rapidjson::ASCII<>>
			extra_writer(extra_buffer);
		extra.Accept(extra_writer);

		std::string extra_data{};
		extra_data.append(extra_buffer.GetString(), extra_buffer.GetLength());
		// json content
		rapidjson::Document doc;
		doc.SetObject();

		doc.AddMember("auth_task", "29", doc.GetAllocator());
		doc.AddMember("code", "700", doc.GetAllocator());

		auto seed = std::to_string(iv_seed);
		doc.AddMember("iv_seed", seed, doc.GetAllocator());
		doc.AddMember("client_ticket", ticket_b64, doc.GetAllocator());
		doc.AddMember("server_ticket", auth_data_b64, doc.GetAllocator());
		doc.AddMember("client_id", "", doc.GetAllocator());
		doc.AddMember("account_type", "steam", doc.GetAllocator());
		doc.AddMember("crossplay_enabled", false, doc.GetAllocator());
		doc.AddMember("loginqueue_eanbled", false, doc.GetAllocator());
		doc.AddMember("identity", identity, doc.GetAllocator());
		doc.AddMember("extra_data", extra_data, doc.GetAllocator());

		rapidjson::Value value{};
		doc.AddMember("lsg_endpoint", value, doc.GetAllocator());

		rapidjson::StringBuffer buffer{};
		rapidjson::Writer<rapidjson::StringBuffer, rapidjson::Document::EncodingType, rapidjson::ASCII<>>
			writer(buffer);
		doc.Accept(writer);

		// http stuff
		std::string result;
		result.append("HTTP/1.1 200 OK\r\n");
		result.append("Server: TornadoServer/4.5.3\r\n");
		result.append("Content-Type: application/json\r\n");
		result.append(utils::string::va("Date: %s GMT\r\n", date));
		result.append(utils::string::va("Content-Length: %d\r\n\r\n", buffer.GetLength()));
		result.append(buffer.GetString(), buffer.GetLength());

		raw_reply reply(result);

		this->send_reply(&reply);

#ifndef NDEBUG
		printf("[DW]: [auth]: user successfully authenticated.\n");
#endif
	}
}
