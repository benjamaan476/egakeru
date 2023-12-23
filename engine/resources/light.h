#pragma once

#include <pch.h>

namespace egkr::light
{
	struct directional_light
	{
		float4 direction{};
		float4 colour{ 1.F };
	};

	struct point_light {
		float4 position{};
		float4 colour{ 1.F };
		// Usually 1, make sure denominator never gets smaller than 1
		float constant{};
		// Reduces light intensity linearly
		float linear{};
		// Makes the light fall off slower at longer distances.
		float quadratic{};

		float pad{};
	};
}