#include "crypt/crypt.h"

#include <string>

#include "third_party/mbedtls/include/mbedtls/cipher.h"
#include "third_party/mbedtls/include/mbedtls/error.h"
#include "third_party/mbedtls/include/mbedtls/md.h"
#include "third_party/mbedtls/include/mbedtls/pkcs5.h"

namespace buried {
	std::string AESCrypt::GetKey(const std::string& salt, const std::string password) {
		int32_t keylen = 32;
		uint32_t iterations = 1000;
		unsigned char key[32] = { 0 };
		mbedtls_md_context_t md_ctx;
		mbedtls_md_init(&md_ctx);
		const mbedtls_md_info_t* md_info =
			mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
		mbedtls_md_setup(&md_ctx, md_info, 1);
		mbedtls_md_starts(&md_ctx);
		int ret = mbedtls_pkcs5_pbkdf2_hmac(
			&md_ctx, 
			reinterpret_cast<const unsigned char*>(password.data()),password.size(), 
			reinterpret_cast<const unsigned char*>(salt.data()),salt.size(), 
			iterations, 
			keylen, 
			key);
		mbedtls_md_free(&md_ctx);
		if (ret != 0) {
			return "";
		}
		return std::string((char*)key, keylen);
	}

	class AESImpl {
	private:
		mbedtls_cipher_context_t m_encrypt_ctx;
		mbedtls_cipher_context_t m_decrypt_ctx;

		uint32_t m_encrypt_block_size = 0;
		uint32_t m_decrypt_block_size = 0;

		unsigned char m_iv[16] = { 0 };
	public:
		explicit AESImpl(const std::string& key) { Init(key.data(), key.size()); }
		~AESImpl() { UnInit(); }

		AESImpl(const AESImpl& other) = delete;
		AESImpl& operator=(const AESImpl& other) = delete;

		void Init(const char* key, size_t key_size);

		void UnInit();

		std::string Encrypt(const void* input, size_t input_size);

		std::string Decrypt(const void* input, size_t input_size);

	};

	void AESImpl::Init(const char* key, size_t key_size) {
		mbedtls_cipher_init(&m_encrypt_ctx);
		mbedtls_cipher_setup(
			&m_encrypt_ctx, mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_256_CBC));
		mbedtls_cipher_set_padding_mode(&m_encrypt_ctx, MBEDTLS_PADDING_PKCS7);
		mbedtls_cipher_setkey(&m_encrypt_ctx,
			reinterpret_cast<const unsigned char*>(key),
			key_size * 8, MBEDTLS_ENCRYPT);

		m_encrypt_block_size = mbedtls_cipher_get_block_size(&m_encrypt_ctx);

		mbedtls_cipher_init(&m_decrypt_ctx);
		mbedtls_cipher_setup(
			&m_decrypt_ctx, mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_256_CBC));
		mbedtls_cipher_set_padding_mode(&m_decrypt_ctx, MBEDTLS_PADDING_PKCS7);
		mbedtls_cipher_setkey(&m_decrypt_ctx,
			reinterpret_cast<const unsigned char*>(key),
			key_size * 8, MBEDTLS_DECRYPT);

		m_decrypt_block_size = mbedtls_cipher_get_block_size(&m_decrypt_ctx);
	}

	void AESImpl::UnInit() {
		mbedtls_cipher_free(&m_encrypt_ctx);
		mbedtls_cipher_free(&m_decrypt_ctx);
	}

	std::string AESImpl::Encrypt(const void* input, size_t input_size) {
		mbedtls_cipher_set_iv(&m_encrypt_ctx, m_iv, sizeof(m_iv));
		mbedtls_cipher_reset(&m_encrypt_ctx);

		std::string output(input_size + m_encrypt_block_size, 0);
		size_t olen = 0;
		int ret = mbedtls_cipher_update(
			&m_encrypt_ctx, reinterpret_cast<const unsigned char*>(input), input_size,
			reinterpret_cast<unsigned char*>(output.data()), &olen);
		if (ret != 0) {
			return "";
		}
		size_t olen2 = 0;
		ret = mbedtls_cipher_finish(
			&m_encrypt_ctx, reinterpret_cast<unsigned char*>(output.data()) + olen,
			&olen2);
		if (ret != 0) {
			return "";
		}
		output.resize(olen + olen2);
		return output;
	}

	std::string AESImpl::Decrypt(const void* input, size_t input_size) {
		mbedtls_cipher_set_iv(&m_decrypt_ctx, m_iv, sizeof(m_iv));
		mbedtls_cipher_reset(&m_decrypt_ctx);

		std::string output(input_size + m_decrypt_block_size, 0);
		size_t olen = 0;
		int ret = mbedtls_cipher_update(
			&m_decrypt_ctx, reinterpret_cast<const unsigned char*>(input), input_size,
			reinterpret_cast<unsigned char*>(output.data()), &olen);
		if (ret != 0) {
			return "";
		}
		size_t olen2 = 0;
		ret = mbedtls_cipher_finish(
			&m_decrypt_ctx, reinterpret_cast<unsigned char*>(output.data()) + olen,
			&olen2);
		if (ret != 0) {
			return "";
		}
		output.resize(olen + olen2);
		return output;
	}

	AESCrypt::AESCrypt(const std::string& key)
		: m_pimpl(std::make_unique<AESImpl>(key)) {}

	AESCrypt::~AESCrypt() {}

	std::string AESCrypt::Encrypt(const std::string& input) {
		return m_pimpl->Encrypt(input.data(), input.size());
	}

	std::string AESCrypt::Decrypt(const std::string& input) {
		return m_pimpl->Decrypt(input.data(), input.size());
	}

	std::string AESCrypt::Encrypt(const void* input, size_t input_size) {
		return m_pimpl->Encrypt(input, input_size);
	}

	std::string AESCrypt::Decrypt(const void* input, size_t input_size) {
		return m_pimpl->Decrypt(input, input_size);
	}


}