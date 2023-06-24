#pragma once

#include "ui/gui.h"

struct BoardProperties
{
	float4 primaryColour{ 1.f, 1.f, 1.f, 1.f };
	float4 secondaryColour{ 0.f, 0.f, 0.f, 1.f };
	int2 size{ 8, 8 };

	bool onRender(Gui* gui)
	{
		auto window = Gui::Window(gui, "Board Properties");
		auto updated = false;
		updated |= window.rgbaColour("Primary Colour", primaryColour);
		updated |= window.rgbaColour("Secondary Colour", secondaryColour);
		updated |= window.slider("Board Size", size, 2, 20);

		return updated;
	}
};
