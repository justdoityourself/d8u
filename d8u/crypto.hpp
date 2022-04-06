/* Copyright (C) 2022 D8DATAWORKS - All Rights Reserved */

#pragma once

#ifdef D8U_CRYPTOPP
#include "cryptopp_crypto.hpp"
#elif D8U_LTC
#include "ltc_crypto.hpp"
#elif D8U_RESEARCH
#include "research_crypto.hpp"
#elif D8U_NAIVE
#include "naive_crypto.hpp"
#else // This will build by default and the cause trouble runtime if not supported.
#include "naive_crypto.hpp"
#endif