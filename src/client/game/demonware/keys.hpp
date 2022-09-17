#pragma once

namespace demonware
{
	void derive_keys_s1();
	void queue_packet_to_hash(const std::string& packet);
	void set_session_key(const std::string& key);
	std::string get_decrypt_key();
	std::string get_encrypt_key();
	std::string get_hmac_key();
	std::string get_response_id();
}
