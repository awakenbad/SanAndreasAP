#pragma once
#include <CSprite2d.h> // pulls in RenderWare.h for RsGlobal

// Every piece of custom HUD drawing in this mod is authored against a 1920x1080 reference
// screen. CFont::SetScale values, box sizes, icon sizes and pixel offsets ALL have to be
// multiplied by this factor - otherwise text renders postage-stamp sized at 4K and spills off
// the screen at 800x600. plugin-sdk's own GPS example does the same thing against the game's
// native 640x448 base; the two are equivalent, this one just keeps our already-tuned numbers.
//
// Deliberately uniform (driven by height alone) rather than separate X/Y factors, so text keeps
// its aspect ratio on ultrawide setups instead of stretching horizontally.
namespace ScreenScale
{
	inline float factor()
	{
		return static_cast<float>(RsGlobal.maximumHeight) / 1080.0f;
	}

	// Converts a value measured at 1080p into one for the player's actual resolution.
	inline float of(float t_referenceValue)
	{
		return t_referenceValue * factor();
	}
}
