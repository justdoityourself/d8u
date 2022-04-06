/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include "../catch.hpp"
#include "crypto.hpp"
#include "hash.hpp"
#include "init.hpp"

using namespace d8u::transform;

TEST_CASE("RNG", "[d8u::crypto]")
{
    d8u_init();

    std::array<uint8_t, 64> a;
    d8u::random_bytes_secure(a);
}

TEST_CASE("AES256 encrypt/decrypt", "[d8u::transform]")
{
    d8u::sse_vector v = { 0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0 };
    d8u::sse_vector t = { 0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0 };

    Password pw(std::string_view("PASSWORD"));

    encrypt(v, pw);
    decrypt(v, pw);

    CHECK(std::equal(v.begin(), v.end(), t.begin()));


    d8u::sse_vector u = { 0x1B,0x54,0x59,0xC3,0xAB,0x6E,0x44,0xBF,0xA2,0xF3,0xB9,0x10,0x75,0x33,0x16,0x5D,0x23,0x97,0x21,0xB7,0x01,0xA7,0x49,0x38,0x93,0xA8,0x21,0xFB,0x23,0xC6,0xF0,0x5A };
    d8u::sse_vector u2 = { 0x1B,0x54,0x59,0xC3,0xAB,0x6E,0x44,0xBF,0xA2,0xF3,0xB9,0x10,0x75,0x33,0x16,0x5D,0x23,0x97,0x21,0xB7,0x01,0xA7,0x49,0x38,0x93,0xA8,0x21,0xFB,0x23,0xC6,0xF0,0x5A };

    encrypt(u, pw);
    decrypt(u, pw);

    CHECK(std::equal(u.begin(), u.end(), u2.begin()));
}

TEST_CASE("SHA", "[d8u::hash]")
{
    std::array<uint8_t, 32> sha256;
    d8u::hash::sha256(std::string_view("HASHTHIS"), sha256);

    std::array<uint8_t, 64> sha512;
    d8u::hash::sha512(std::string_view("HASHTHIS"), sha512);
}

TEST_CASE("RSA", "[d8u::crypto]")
{
    auto [pub, pri] = d8u::crypto::KeyPair::Create();

    auto src = std::string_view("MESSAGE!");
    auto emsg = pub.Encrypt(src);

    auto msg = pri.Decrypt(emsg);

    CHECK(std::equal(msg.begin(), msg.end(), src.begin()));

    auto sig = pri.Sign(src);

    auto res = pub.Verify(src, sig);

    CHECK(res == true);
}