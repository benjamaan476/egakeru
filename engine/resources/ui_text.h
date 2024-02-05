#pragma once

#include <pch.h>

namespace egkr
{
	namespace text
	{
		struct font_data;

		enum class type
		{
			bitmap,
			system
		};

		struct ui_text
		{
			type type;
			font_data::shared_ptr data;
//TODO
			//renderbuffer
		};
	}
}
