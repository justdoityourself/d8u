/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <chrono>

namespace d8u
{
	namespace time
	{
		uint32_t epoch_seconds()
		{
			using namespace std::chrono;

			system_clock::time_point tp = system_clock::now();
			system_clock::duration dtn = tp.time_since_epoch();

			return (uint32_t)(dtn.count() * system_clock::period::num / system_clock::period::den);
		}
	}
}

