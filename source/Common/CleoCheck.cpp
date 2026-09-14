#include "CleoCheck.h"
#include <windows.h>

namespace
{
	using GetVersionFn = int(__stdcall*)();

	constexpr int MINIMUM_CLEO_VERSION = 5 << 24;

	bool detectOutdatedCleo()
	{
		HMODULE cleo = GetModuleHandleA("CLEO.asi");
		if (!cleo) return false;

		auto cleoVersion = reinterpret_cast<GetVersionFn>(GetProcAddress(cleo, "_CLEO_GetVersion@0"));
		if (!cleoVersion) return true;

		return cleoVersion() < MINIMUM_CLEO_VERSION;
	}
}

bool CleoCheck::isOutdatedCleoLoaded()
{
	static const bool outdated = detectOutdatedCleo();
	return outdated;
}
