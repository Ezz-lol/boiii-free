#include "string.hpp"
#include "cryptography.hpp"
#include "nt.hpp"
#include "finally.hpp"

#undef max
using namespace std::string_literals;

/// http://www.opensource.apple.com/source/CommonCrypto/CommonCrypto-55010/Source/libtomcrypt/doc/libTomCryptDoc.pdf

namespace utils::cryptography
{
	namespace
	{
		struct __
		{
			__()
			{
				ltc_mp = ltm_desc;

				register_cipher(&aes_desc);
				register_cipher(&des3_desc);

				register_prng(&sprng_desc);
				register_prng(&fortuna_desc);
				register_prng(&yarrow_desc);

				register_hash(&sha1_desc);
				register_hash(&sha256_desc);
				register_hash(&sha512_desc);
			}
		} ___;

		[[maybe_unused]] const char* cs(const uint8_t* data)
		{
			return reinterpret_cast<const char*>(data);
		}

		[[maybe_unused]] char* cs(uint8_t* data)
		{
			return reinterpret_cast<char*>(data);
		}

		[[maybe_unused]] const uint8_t* cs(const char* data)
		{
			return reinterpret_cast<const uint8_t*>(data);
		}

		[[maybe_unused]] uint8_t* cs(char* data)
		{
			return reinterpret_cast<uint8_t*>(data);
		}

		[[maybe_unused]] unsigned long ul(const size_t value)
		{
			return static_cast<unsigned long>(value);
		}

		class prng
		{
		public:
			prng(const ltc_prng_descriptor& descriptor, const bool autoseed = true)
				: state_(std::make_unique<prng_state>())
				  , descriptor_(descriptor)
			{
				this->id_ = register_prng(&descriptor);
				if (this->id_ == -1)
				{
					throw std::runtime_error("PRNG "s + this->descriptor_.name + " could not be registered!");
				}

				if (autoseed)
				{
					this->auto_seed();
				}
				else
				{
					this->descriptor_.start(this->state_.get());
				}
			}

			~prng()
			{
				this->descriptor_.done(this->state_.get());
			}

			prng_state* get_state() const
			{
				this->descriptor_.ready(this->state_.get());
				return this->state_.get();
			}

			int get_id() const
			{
				return this->id_;
			}

			void add_entropy(const void* data, const size_t length) const
			{
				this->descriptor_.add_entropy(static_cast<const uint8_t*>(data), ul(length), this->state_.get());
			}

			void read(void* data, const size_t length) const
			{
				this->descriptor_.read(static_cast<unsigned char*>(data), ul(length), this->get_state());
			}

		private:
			int id_;
			std::unique_ptr<prng_state> state_;
			const ltc_prng_descriptor& descriptor_;

			void auto_seed() const
			{
				rng_make_prng(128, this->id_, this->state_.get(), nullptr);

				int i[4]; // uninitialized data
				auto* i_ptr = &i;
				this->add_entropy(reinterpret_cast<uint8_t*>(&i), sizeof(i));
				this->add_entropy(reinterpret_cast<uint8_t*>(&i_ptr), sizeof(i_ptr));

				auto t = time(nullptr);
				this->add_entropy(reinterpret_cast<uint8_t*>(&t), sizeof(t));
			}
		};

		const prng prng_(fortuna_desc);
	}

	ecc::key::key()
	{
		ZeroMemory(&this->key_storage_, sizeof(this->key_storage_));
	}

	ecc::key::~key()
	{
		this->free();
	}

	ecc::key::key(key&& obj) noexcept
		: key()
	{
		this->operator=(std::move(obj));
	}

	ecc::key::key(const key& obj)
		: key()
	{
		this->operator=(obj);
	}

	ecc::key& ecc::key::operator=(key&& obj) noexcept
	{
		if (this != &obj)
		{
			std::memmove(&this->key_storage_, &obj.key_storage_, sizeof(this->key_storage_));
			ZeroMemory(&obj.key_storage_, sizeof(obj.key_storage_));
		}

		return *this;
	}

	ecc::key& ecc::key::operator=(const key& obj)
	{
		if (this != &obj && obj.is_valid())
		{
			this->deserialize(obj.serialize(obj.key_storage_.type));
		}

		return *this;
	}

	bool ecc::key::is_valid() const
	{
		return (!memory::is_set(&this->key_storage_, 0, sizeof(this->key_storage_)));
	}

	ecc_key& ecc::key::get()
	{
		return this->key_storage_;
	}

	const ecc_key& ecc::key::get() const
	{
		return this->key_storage_;
	}

	std::string ecc::key::get_public_key() const
	{
		uint8_t buffer[512] = {0};
		unsigned long length = sizeof(buffer);

		if (ecc_ansi_x963_export(&this->key_storage_, buffer, &length) == CRYPT_OK)
		{
			return std::string(cs(buffer), length);
		}

		return {};
	}

	void ecc::key::set(const std::string& pub_key_buffer)
	{
		this->free();

		if (ecc_ansi_x963_import(cs(pub_key_buffer.data()),
		                         ul(pub_key_buffer.size()),
		                         &this->key_storage_) != CRYPT_OK)
		{
			ZeroMemory(&this->key_storage_, sizeof(this->key_storage_));
		}
	}

	void ecc::key::deserialize(const std::string& key)
	{
		this->free();

		if (ecc_import(cs(key.data()), ul(key.size()),
		               &this->key_storage_) != CRYPT_OK
		)
		{
			ZeroMemory(&this->key_storage_, sizeof(this->key_storage_));
		}
	}

	std::string ecc::key::serialize(const int type) const
	{
		uint8_t buffer[4096] = {0};
		unsigned long length = sizeof(buffer);

		if (ecc_export(buffer, &length, type, &this->key_storage_) == CRYPT_OK)
		{
			return std::string(cs(buffer), length);
		}

		return "";
	}

	void ecc::key::free()
	{
		if (this->is_valid())
		{
			ecc_free(&this->key_storage_);
		}

		ZeroMemory(&this->key_storage_, sizeof(this->key_storage_));
	}

	bool ecc::key::operator==(key& key) const
	{
		return (this->is_valid() && key.is_valid() && this->serialize(PK_PUBLIC) == key.serialize(PK_PUBLIC));
	}

	uint64_t ecc::key::get_hash() const
	{
		const auto hash = sha1::compute(this->get_public_key());
		if (hash.size() >= 8)
		{
			return *reinterpret_cast<const uint64_t*>(hash.data());
		}

		return 0;
	}

	ecc::key ecc::generate_key(const int bits)
	{
		key key;
		ecc_make_key(prng_.get_state(), prng_.get_id(), bits / 8, &key.get());

		return key;
	}

	ecc::key ecc::generate_key(const int bits, const std::string& entropy)
	{
		key key{};
		const prng yarrow(yarrow_desc, false);
		yarrow.add_entropy(entropy.data(), entropy.size());

		ecc_make_key(yarrow.get_state(), yarrow.get_id(), bits / 8, &key.get());

		return key;
	}

	std::string ecc::sign_message(const key& key, const std::string& message)
	{
		if (!key.is_valid()) return "";

		uint8_t buffer[512];
		unsigned long length = sizeof(buffer);

		ecc_sign_hash(cs(message.data()), ul(message.size()), buffer, &length, prng_.get_state(), prng_.get_id(),
		              &key.get());

		return std::string(cs(buffer), length);
	}

	bool ecc::verify_message(const key& key, const std::string& message, const std::string& signature)
	{
		if (!key.is_valid()) return false;

		auto result = 0;
		return (ecc_verify_hash(cs(signature.data()),
		                        ul(signature.size()),
		                        cs(message.data()),
		                        ul(message.size()), &result,
		                        &key.get()) == CRYPT_OK && result != 0);
	}

	bool ecc::encrypt(const key& key, std::string& data)
	{
		std::string out_data{};
		out_data.resize(std::max(ul(data.size() * 3), ul(0x100)));

		auto out_len = ul(out_data.size());
		auto crypt = [&]()
		{
			return ecc_encrypt_key(cs(data.data()), ul(data.size()), cs(out_data.data()), &out_len,
			                       prng_.get_state(), prng_.get_id(), find_hash("sha512"), &key.get());
		};

		auto res = crypt();

		if (res == CRYPT_BUFFER_OVERFLOW)
		{
			out_data.resize(out_len);
			res = crypt();
		}

		if (res != CRYPT_OK)
		{
			return false;
		}

		out_data.resize(out_len);
		data = std::move(out_data);
		return true;
	}

	bool ecc::decrypt(const key& key, std::string& data)
	{
		std::string out_data{};
		out_data.resize(std::max(ul(data.size() * 3), ul(0x100)));

		auto out_len = ul(out_data.size());
		auto crypt = [&]()
		{
			return ecc_decrypt_key(cs(data.data()), ul(data.size()), cs(out_data.data()), &out_len, &key.get());
		};

		auto res = crypt();

		if (res == CRYPT_BUFFER_OVERFLOW)
		{
			out_data.resize(out_len);
			res = crypt();
		}

		if (res != CRYPT_OK)
		{
			return false;
		}

		out_data.resize(out_len);
		data = std::move(out_data);
		return true;
	}

	std::string rsa::encrypt(const std::string& data, const std::string& hash, const std::string& key)
	{
		rsa_key new_key;
		rsa_import(cs(key.data()), ul(key.size()), &new_key);
		const auto _ = finally([&]()
		{
			rsa_free(&new_key);
		});


		std::string out_data{};
		out_data.resize(std::max(ul(data.size() * 3), ul(0x100)));

		auto out_len = ul(out_data.size());
		auto crypt = [&]()
		{
			return rsa_encrypt_key(cs(data.data()), ul(data.size()), cs(out_data.data()), &out_len, cs(hash.data()),
			                       ul(hash.size()), prng_.get_state(), prng_.get_id(), find_hash("sha512"), &new_key);
		};

		auto res = crypt();

		if (res == CRYPT_BUFFER_OVERFLOW)
		{
			out_data.resize(out_len);
			res = crypt();
		}

		if (res == CRYPT_OK)
		{
			out_data.resize(out_len);
			return out_data;
		}

		return {};
	}

	std::string des3::encrypt(const std::string& data, const std::string& iv, const std::string& key)
	{
		std::string enc_data;
		enc_data.resize(data.size());

		symmetric_CBC cbc;
		const auto des3 = find_cipher("3des");

		cbc_start(des3, cs(iv.data()), cs(key.data()), static_cast<int>(key.size()), 0, &cbc);
		cbc_encrypt(cs(data.data()), cs(enc_data.data()), ul(data.size()), &cbc);
		cbc_done(&cbc);

		return enc_data;
	}

	std::string des3::decrypt(const std::string& data, const std::string& iv, const std::string& key)
	{
		std::string dec_data;
		dec_data.resize(data.size());

		symmetric_CBC cbc;
		const auto des3 = find_cipher("3des");

		cbc_start(des3, cs(iv.data()), cs(key.data()), static_cast<int>(key.size()), 0, &cbc);
		cbc_decrypt(cs(data.data()), cs(dec_data.data()), ul(data.size()), &cbc);
		cbc_done(&cbc);

		return dec_data;
	}

	std::string tiger::compute(const std::string& data, const bool hex)
	{
		return compute(cs(data.data()), data.size(), hex);
	}

	std::string tiger::compute(const uint8_t* data, const size_t length, const bool hex)
	{
		uint8_t buffer[24] = {0};

		hash_state state;
		tiger_init(&state);
		tiger_process(&state, data, ul(length));
		tiger_done(&state, buffer);

		std::string hash(cs(buffer), sizeof(buffer));
		if (!hex) return hash;

		return string::dump_hex(hash, "");
	}

	std::string aes::encrypt(const std::string& data, const std::string& iv, const std::string& key)
	{
		std::string enc_data;
		enc_data.resize(data.size());

		symmetric_CBC cbc;
		const auto aes = find_cipher("aes");

		cbc_start(aes, cs(iv.data()), cs(key.data()),
		          static_cast<int>(key.size()), 0, &cbc);
		cbc_encrypt(cs(data.data()),
		            cs(enc_data.data()),
		            ul(data.size()), &cbc);
		cbc_done(&cbc);

		return enc_data;
	}

	std::string aes::decrypt(const std::string& data, const std::string& iv, const std::string& key)
	{
		std::string dec_data;
		dec_data.resize(data.size());

		symmetric_CBC cbc;
		const auto aes = find_cipher("aes");

		cbc_start(aes, cs(iv.data()), cs(key.data()),
		          static_cast<int>(key.size()), 0, &cbc);
		cbc_decrypt(cs(data.data()),
		            cs(dec_data.data()),
		            ul(data.size()), &cbc);
		cbc_done(&cbc);

		return dec_data;
	}

	std::string hmac_sha1::compute(const std::string& data, const std::string& key)
	{
		std::string buffer;
		buffer.resize(20);

		hmac_state state;
		hmac_init(&state, find_hash("sha1"), cs(key.data()), ul(key.size()));
		hmac_process(&state, cs(data.data()), static_cast<int>(data.size()));

		auto out_len = ul(buffer.size());
		hmac_done(&state, cs(buffer.data()), &out_len);

		buffer.resize(out_len);
		return buffer;
	}

	std::string sha1::compute(const std::string& data, const bool hex)
	{
		return compute(cs(data.data()), data.size(), hex);
	}

	std::string sha1::compute(const uint8_t* data, const size_t length, const bool hex)
	{
		uint8_t buffer[20] = {0};

		hash_state state;
		sha1_init(&state);
		sha1_process(&state, data, ul(length));
		sha1_done(&state, buffer);

		std::string hash(cs(buffer), sizeof(buffer));
		if (!hex) return hash;

		return string::dump_hex(hash, "");
	}

	std::string sha256::compute(const std::string& data, const bool hex)
	{
		return compute(cs(data.data()), data.size(), hex);
	}

	std::string sha256::compute(const uint8_t* data, const size_t length, const bool hex)
	{
		uint8_t buffer[32] = {0};

		hash_state state;
		sha256_init(&state);
		sha256_process(&state, data, ul(length));
		sha256_done(&state, buffer);

		std::string hash(cs(buffer), sizeof(buffer));
		if (!hex) return hash;

		return string::dump_hex(hash, "");
	}

	std::string sha512::compute(const std::string& data, const bool hex)
	{
		return compute(cs(data.data()), data.size(), hex);
	}

	std::string sha512::compute(const uint8_t* data, const size_t length, const bool hex)
	{
		uint8_t buffer[64] = {0};

		hash_state state;
		sha512_init(&state);
		sha512_process(&state, data, ul(length));
		sha512_done(&state, buffer);

		std::string hash(cs(buffer), sizeof(buffer));
		if (!hex) return hash;

		return string::dump_hex(hash, "");
	}

	std::string base64::encode(const uint8_t* data, const size_t len)
	{
		std::string result;
		result.resize((len + 2) * 2);

		auto out_len = ul(result.size());
		if (base64_encode(data, ul(len), result.data(), &out_len) != CRYPT_OK)
		{
			return {};
		}

		result.resize(out_len);
		return result;
	}

	std::string base64::encode(const std::string& data)
	{
		return base64::encode(cs(data.data()), static_cast<unsigned>(data.size()));
	}

	std::string base64::decode(const std::string& data)
	{
		std::string result;
		result.resize((data.size() + 2) * 2);

		auto out_len = ul(result.size());
		if (base64_decode(data.data(), ul(data.size()), cs(result.data()), &out_len) != CRYPT_OK)
		{
			return {};
		}

		result.resize(out_len);
		return result;
	}

	unsigned int jenkins_one_at_a_time::compute(const std::string& data)
	{
		return compute(data.data(), data.size());
	}

	unsigned int jenkins_one_at_a_time::compute(const char* key, const size_t len)
	{
		unsigned int hash, i;
		for (hash = i = 0; i < len; ++i)
		{
			hash += key[i];
			hash += (hash << 10);
			hash ^= (hash >> 6);
		}
		hash += (hash << 3);
		hash ^= (hash >> 11);
		hash += (hash << 15);
		return hash;
	}

	uint32_t random::get_integer()
	{
		uint32_t result;
		random::get_data(&result, sizeof(result));
		return result;
	}

	std::string random::get_challenge()
	{
		std::string result;
		result.resize(sizeof(uint64_t));
		random::get_data(result.data(), result.size());
		return string::dump_hex(result, "");
	}

	void random::get_data(void* data, const size_t size)
	{
		prng_.read(data, size);
	}
}
