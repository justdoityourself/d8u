/* Copyright (C) 2022 D8DATAWORKS - All Rights Reserved */

#pragma once

#ifdef D8U_CRYPTOPP
bool d8u_init() {
	return true;
}
#elif D8U_LTC
extern "C" {
#include "tomcrypt.h"
}


bool d8u_init() {
	crypt_mp_init("l");

	if (0 != register_cipher(&aes_desc))
		return false;

	if (0 != register_hash(&sha512_desc))
		return false;

	if (1 != register_hash(&sha256_desc))
		return false;

	if (2 != register_hash(&sha1_desc))
		return false;

	if (register_prng(&sprng_desc))
		return false;

	return true;
}
#elif D8U_RESEARCH
bool d8u_init() {
	return true;
}
#elif D8U_NAIVE
bool d8u_init() {
	return true;
}
#else // This will build by default and the cause trouble runtime if not supported.
bool d8u_init() {
	return true;
}
#endif