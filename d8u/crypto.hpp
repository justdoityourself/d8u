/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <array>
#include <vector>

#include "cryptopp/aes.h"
#include "cryptopp/modes.h"
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>
#include <cryptopp/pssr.h>


#include "hash.hpp"
#include "memory.hpp"
#include "util.hpp"


namespace d8u
{
	namespace transform
	{
		template <typename T> void encrypt(T& buffer_to_encrypt, const Password& password)
		{
			CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption cfbEncryption(password.Key(), 32, password.IV());
			cfbEncryption.ProcessData((CryptoPP::byte*)buffer_to_encrypt.data(), (CryptoPP::byte*) buffer_to_encrypt.data(), buffer_to_encrypt.size());
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

			template <typename T> d8u::sse_vector Sign(const T & m) const
			{
				CryptoPP::AutoSeededRandomPool rng;

				CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA256>::Signer signer(*this);

				size_t length = signer.MaxSignatureLength();
				CryptoPP::SecByteBlock signature(length);

				length = signer.SignMessage(rng, (const CryptoPP::byte*)m.data(), m.size(), signature);

				signature.resize(length);

				return d8u::sse_vector(signature.begin(), signature.end());
			}

			template < typename T > d8u::sse_vector Decrypt(const T & m) const
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
				auto imported = _s.substr(0,f);
				auto signature = d8u::util::to_bin_sse(_s.substr(f + 1,_s.size()-f - 2));

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
				public_key.Import( _s.substr(0, f) );
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
}