#include <std_include.hpp>
#include "keys.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

namespace demonware
{
	struct data_t
	{
		char m_session_key[24];
		char m_response[8];
		char m_hmac_key[20];
		char m_enc_key[16];
		char m_dec_key[16];
	} data{};

	std::string packet_buffer;

	void calculate_hmacs_s1(const char* data_, const unsigned int data_size, const char* key,
	                        const unsigned int key_size,
	                        char* dst, const unsigned int dst_size)
	{
		char buffer[64];
		unsigned int pos = 0;
		unsigned int out_offset = 0;
		char count = 1;
		std::string result;

		// buffer add key
		std::memcpy(&buffer[pos], key, key_size);
		pos += key_size;

		// buffer add count
		buffer[pos] = count;
		pos++;

		// calculate hmac
		result = utils::cryptography::hmac_sha1::compute(std::string(buffer, pos), std::string(data_, data_size));

		// save output
		std::memcpy(dst, result.data(), std::min(20u, (dst_size - out_offset)));
		out_offset = 20;

		// second loop
		while (true)
		{
			// if we filled the output buffer, exit
			if (out_offset >= dst_size)
				break;

			// buffer add last result
			pos = 0;
			std::memcpy(&buffer[pos], result.data(), 20);
			pos += 20;

			// buffer add key
			std::memcpy(&buffer[pos], key, key_size);
			pos += key_size;

			// buffer add count
			count++;
			buffer[pos] = count;
			pos++;

			// calculate hmac
			result = utils::cryptography::hmac_sha1::compute(std::string(buffer, pos), std::string(data_, data_size));

			// save output
			std::memcpy(dst + out_offset, result.data(), std::min(20u, (dst_size - out_offset)));
			out_offset += 20;
		}
	}

	void derive_keys_s1()
	{
		const auto out_1 = utils::cryptography::sha1::compute(packet_buffer); // out_1 size 20

		auto data_3 = utils::cryptography::hmac_sha1::compute(data.m_session_key, out_1);

		char out_2[16];
		calculate_hmacs_s1(data_3.data(), 20, "CLIENTCHAL", 10, out_2, 16);

		char out_3[72];
		calculate_hmacs_s1(data_3.data(), 20, "BDDATA", 6, out_3, 72);

		std::memcpy(data.m_response, &out_2[8], 8);
		std::memcpy(data.m_hmac_key, &out_3[20], 20);
		std::memcpy(data.m_dec_key, &out_3[40], 16);
		std::memcpy(data.m_enc_key, &out_3[56], 16);

#ifndef NDEBUG
		printf("[DW] Response id: %s\n", utils::string::dump_hex(std::string(&out_2[8], 8)).data());
		printf("[DW] Hash verify: %s\n", utils::string::dump_hex(std::string(&out_3[20], 20)).data());
		printf("[DW] AES dec key: %s\n", utils::string::dump_hex(std::string(&out_3[40], 16)).data());
		printf("[DW] AES enc key: %s\n", utils::string::dump_hex(std::string(&out_3[56], 16)).data());
		printf("[DW] Bravo 6, going dark.\n");
#endif
	}

	void queue_packet_to_hash(const std::string& packet)
	{
		packet_buffer.append(packet);
	}

	void set_session_key(const std::string& key)
	{
		std::memcpy(data.m_session_key, key.data(), 24);
	}

	std::string get_decrypt_key()
	{
		return std::string(data.m_dec_key, 16);
	}

	std::string get_encrypt_key()
	{
		return std::string(data.m_enc_key, 16);
	}

	std::string get_hmac_key()
	{
		return std::string(data.m_hmac_key, 20);
	}

	std::string get_response_id()
	{
		return std::string(data.m_response, 8);
	}
}
