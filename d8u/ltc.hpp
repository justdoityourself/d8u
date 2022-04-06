/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <array>
#include <vector>

#include "tomcrypt.h"


#include "hash.hpp"
#include "memory.hpp"
#include "util.hpp"

bool ltc_test_all() {
	crypt_mp_init("l");

	register_cipher(&aes_desc);

	int status;

	symmetric_CBC cbc;

	unsigned char pt[64], /*ct[64],*/ tmp[64], akey[32], iv[16], iv2[16];
	unsigned long taglen;
	unsigned char tag[16];
	memset(pt, 7, 64);

	int err = gcm_memory(/*find_cipher("aes")*/0, (const unsigned char*)akey, 32, (const unsigned char*)iv, 16, NULL, NULL, pt, 64, pt, tag, &taglen, GCM_ENCRYPT);
	if (err) return false;
	err = gcm_memory(/*find_cipher("aes")*/0, (const unsigned char*)akey, 32, (const unsigned char*)iv, 16, NULL, NULL, pt, 64, pt, tag, &taglen, GCM_DECRYPT);
	if (err) return false;

	auto cipher_idx = find_cipher("aes");
	if (cipher_idx == -1) {
		fprintf(stderr, "test requires AES");
		return false;
	}

	unsigned long l;
	status = cbc_start(cipher_idx, iv, akey, 32, 0, &cbc);
	if (status) return false;
	status = cbc_getiv(iv2, &l, &cbc);
	if (status) return false;
	status = cbc_encrypt(pt, /*ct*/pt, 64, &cbc);
	if (status) return false;
	status = cbc_setiv(iv2, l, &cbc);
	if (status) return false;
	status = cbc_decrypt(/*ct*/pt, pt, 64, &cbc);
	if (status) return false;


	unsigned char text[] = "hello world!";
	unsigned char enc_out[80];
	unsigned char dec_out[80];
	symmetric_key symkey;
	int keysize = 32;

	status = aes_keysize(&keysize);
	if (status) return false;

	static const unsigned char aeskey[] = {
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
	};

	
	status = aes_setup(aeskey, 32, 0, &symkey);
	if (status) return false;

	status = aes_ecb_encrypt(text, enc_out, &symkey);
	if (status) return false;

	status = aes_ecb_decrypt(enc_out, dec_out, &symkey);
	if (status) return false;



	const int prng_idx = register_prng(&sprng_desc);


	rsa_key key;
	const int bitsize = 2048;
	err = rsa_make_key(NULL, prng_idx, bitsize / 8, 65537, &key);
	if (err) return false;

	unsigned char outpv[bitsize * 5 / 8]; // guesstimate
	unsigned long pvlen = sizeof(outpv);
	err = rsa_export(outpv, &pvlen, PK_PRIVATE, &key);
	if (err) return false;

	unsigned char outpb[bitsize * 5 / 8];
	unsigned long pblen = sizeof(outpb);
	err = rsa_export(outpb, &pblen, PK_PUBLIC, &key);
	if (err) return false;

	rsa_free(&key);



	rsa_key skey;
	err = rsa_import(outpv, pvlen, &skey);
	if (err) return false;

	const ltc_hash_descriptor& hash_desc = sha512_desc;
	const int hash_idx = register_hash(&hash_desc);

	char msg[] = "THIS IS A MESSAGE";
	unsigned char hash[64];
	hash_state md;
	hash_desc.init(&md);
	hash_desc.process(&md, (const unsigned char*)msg, (unsigned long)strlen(msg));
	hash_desc.done(&md, hash);


	const int padding = LTC_PKCS_1_V1_5;
	const unsigned long saltlen = 0;

	// Register PRNG algorithm (PSS only).
	//const int prng_idx = padding == LTC_PKCS_1_PSS ? register_prng(&sprng_desc) : 0;

	// Sign hash.
#define MAX_RSA_SIZE 4096 // bits
	unsigned char sig[MAX_RSA_SIZE / 8];
	unsigned long siglen = sizeof(sig);
	err = rsa_sign_hash_ex(hash, hash_desc.hashsize, sig, &siglen, padding, NULL, prng_idx, hash_idx, saltlen, &skey);
	if (err) return false;

	rsa_key pubkey;
	err = rsa_import(outpb, pblen, &pubkey);
	if (err) return false;


	int stat = 0;
	err = rsa_verify_hash_ex(sig, siglen, hash, hash_desc.hashsize, padding, hash_idx, saltlen, &stat, &pubkey);
	if (err) return false;
	if (!stat) return false;

	//if (err != CRYPT_OK) return error(err);
	//if (!stat) return error("Invalid signature");

	unsigned char enc[MAX_RSA_SIZE / 8];
	unsigned long enclen = sizeof(enc);
	err = rsa_encrypt_key((const unsigned char*)msg, /* data we wish to encrypt */
		(unsigned long)strlen(msg), /* data is 16 bytes long */
		enc, /* where to store ciphertext */
		&enclen, /* length of ciphertext */
		nullptr, /* our lparam for this program */
		0, /* lparam is 7 bytes long */
		NULL, /* PRNG state */
		prng_idx, /* prng idx */
		hash_idx, /* hash idx */
		&pubkey); /* our RSA key */
	if (err) return false;

	/* now let’s decrypt the encrypted key */
	unsigned char dec[MAX_RSA_SIZE / 8];
	unsigned long declen = sizeof(enc);
	int res = 0;
	err = rsa_decrypt_key(enc, /* encrypted data */
		enclen, /* length of ciphertext */
		dec, /* where to put plaintext */
		&declen, /* plaintext length */
		nullptr, /* lparam for this program */
		0, /* lparam is 7 bytes long */
		hash_idx, /* hash idx */
		&res, /* validity of data */
		&skey); /* our RSA key */
	if (err) return false;
	if (res!=1) return false;

	/* if all went well pt == pt2, l2 == 16, res == 1 */


	rsa_free(&pubkey);
	rsa_free(&skey);

	return true;
}


/*namespace d8u
{
	namespace transform
	{
		template <typename T> void encrypt(T& buffer_to_encrypt, const Password& password)
		{
			CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption cfbEncryption(password.Key(), 32, password.IV());
			cfbEncryption.ProcessData((CryptoPP::byte*)buffer_to_encrypt.data(), (CryptoPP::byte*)buffer_to_encrypt.data(), buffer_to_encrypt.size());
		}

		template <typename T> d8u::sse_vector encrypt_copy(T& buffer_to_encrypt, const Password& password)
		{
			d8u::sse_vector output;
			output.resize(buffer_to_encrypt.size());

			CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption cfbEncryption(password.Key(), 32, password.IV());
			cfbEncryption.ProcessData(output.data(), buffer_to_encrypt.data(), buffer_to_encrypt.size());

			return output;
		}

		template <typename T> void decrypt(T& buffer_to_decrypt, const Password& password)
		{
			CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption cfbDecryption(password.Key(), 32, password.IV());
			cfbDecryption.ProcessData((CryptoPP::byte*)buffer_to_decrypt.data(), (CryptoPP::byte*)buffer_to_decrypt.data(), buffer_to_decrypt.size());
		}
	}

	template < typename D > void random_bytes_secure(D& d)
	{
		CryptoPP::AutoSeededRandomPool rnd;
		rnd.GenerateBlock((uint8_t*)d.data(), d.size());
	}

	namespace crypto
	{
		class PrivatePassword : public CryptoPP::RSA::PrivateKey
		{
		public:
			PrivatePassword() {}
			PrivatePassword(const CryptoPP::InvertibleRSAFunction& p) : CryptoPP::RSA::PrivateKey(p) {}

			PrivatePassword(const std::string& _s)
			{
				Import(_s);
			}

			void Import(const std::string& _s)
			{
				std::stringstream s(_s);

				std::string n; std::getline(s, n);
				std::string e; std::getline(s, e);
				std::string d; std::getline(s, d);

				Initialize(CryptoPP::Integer(n.data()), CryptoPP::Integer(e.data()), CryptoPP::Integer(d.data()));
			}

			std::string Export() const
			{
				std::stringstream s;
				s << std::hex << "0x" << m_n << "\n" << "0x" << m_e << "\n" << "0x" << m_d;

				return s.str();
			}

			template <typename T> d8u::sse_vector Sign(const T& m) const
			{
				CryptoPP::AutoSeededRandomPool rng;

				CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA256>::Signer signer(*this);

				size_t length = signer.MaxSignatureLength();
				CryptoPP::SecByteBlock signature(length);

				length = signer.SignMessage(rng, (const CryptoPP::byte*)m.data(), m.size(), signature);

				signature.resize(length);

				return d8u::sse_vector(signature.begin(), signature.end());
			}

			template < typename T > d8u::sse_vector Decrypt(const T& m) const
			{
				CryptoPP::AutoSeededRandomPool rng;
				CryptoPP::RSAES<CryptoPP::OAEP<CryptoPP::SHA256> >::Decryptor decryptor(*this);

				if (!decryptor.FixedCiphertextLength())
					throw std::runtime_error("Failed RSA Decryption (1)");

				if (m.size() > decryptor.FixedCiphertextLength())
					throw std::runtime_error("Failed RSA Decryption (2)");

				size_t dpl = decryptor.MaxPlaintextLength(m.size());

				if (!dpl)
					throw std::runtime_error("Failed RSA Decryption (3)");

				CryptoPP::SecByteBlock recovered(dpl);

				CryptoPP::DecodingResult result = decryptor.Decrypt(rng, m.data(), m.size(), recovered);

				if (!result.isValidCoding)
					throw std::runtime_error("Failed RSA Decryption (4)");

				if (result.messageLength > decryptor.MaxPlaintextLength(m.size()))
					throw std::runtime_error("Failed RSA Decryption (5)");

				recovered.resize(result.messageLength);

				return d8u::sse_vector(recovered.begin(), recovered.end());
			}
		};

		class PublicPassword : public CryptoPP::RSA::PublicKey
		{
		public:
			PublicPassword() {}
			PublicPassword(const CryptoPP::InvertibleRSAFunction& p) : CryptoPP::RSA::PublicKey(p) {}

			PublicPassword(const std::string& _s)
			{
				Import(_s);
			}

			void Import(const std::string& _s)
			{
				std::stringstream s(_s);
				std::string n; std::getline(s, n);
				std::string e; std::getline(s, e);
				Initialize(CryptoPP::Integer(n.data()), CryptoPP::Integer(e.data()));
			}

			void ImportSelfSigned(const std::string& _s)
			{
				auto f = _s.find('|');
				auto imported = _s.substr(0, f);
				auto signature = d8u::util::to_bin_sse(_s.substr(f + 1, _s.size() - f - 2));

				Import(imported);

				if (!Verify(imported, signature))
					throw std::runtime_error("Self Signing Error");
			}

			std::string ExportSelfSigned(const PrivatePassword& private_key)
			{
				std::string exported = Export();
				auto signature = private_key.Sign(exported);

				std::stringstream s;
				s << exported << "|" << d8u::util::to_hex(signature) << "*";

				return s.str();
			}

			std::string Export()
			{
				std::stringstream s;
				s << std::hex << "0x" << m_n << "\n" << "0x" << m_e;

				return s.str();
			}

			template < typename T, typename V > bool Verify(const T& m, const V& h)
			{
				CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA256>::Verifier verifier(*this);

				return verifier.VerifyMessage((const CryptoPP::byte*)m.data(), m.size(), h.data(), h.size());
			}

			template < typename T >d8u::sse_vector Encrypt(const T& m)
			{
				CryptoPP::AutoSeededRandomPool rng;
				CryptoPP::RSAES<CryptoPP::OAEP<CryptoPP::SHA256> >::Encryptor encryptor(*this);

				if (!encryptor.FixedMaxPlaintextLength())
					throw std::runtime_error("Failed RSA Encryption (1)");

				if (m.size() > encryptor.FixedMaxPlaintextLength())
					throw std::runtime_error("Failed RSA Encryption (2)");

				size_t ecl = encryptor.CiphertextLength(m.size());

				if (ecl == 0)
					throw std::runtime_error("Failed RSA Encryption (3)");

				CryptoPP::SecByteBlock ciphertext(ecl);

				encryptor.Encrypt(rng, (const CryptoPP::byte*)m.data(), m.size(), ciphertext);

				return d8u::sse_vector(ciphertext.begin(), ciphertext.end());
			}
		};

		class KeyPair
		{
		public:
			PublicPassword public_key;
			PrivatePassword private_key;

			void Import(const std::string& _s)
			{
				auto f = _s.find('|');
				public_key.Import(_s.substr(0, f));
				private_key.Import(_s.substr(f + 1, _s.size() - f - 2));
			}

			std::string Export()
			{
				std::stringstream s;
				s << public_key.Export() << "|" << private_key.Export() << "*";

				return s.str();
			}

			static KeyPair Create(uint32_t bits = 2048)
			{
				CryptoPP::AutoSeededRandomPool rng;

				CryptoPP::InvertibleRSAFunction params;
				params.GenerateRandomWithKeySize(rng, bits);

				return { PublicPassword(params), PrivatePassword(params) };
			}
		};




	}
}*/