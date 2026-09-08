#include "SnapshotLock.h"

#include <CPickups.h>
#include <Patch.h>

namespace
{
	constexpr uintptr_t PICTURE_TAKEN_CALL_SITE = 0x73C243;
	constexpr uintptr_t PICTURE_TAKEN = 0x456A70;

	bool g_locked = false;

	void __cdecl onPictureTaken()
	{
		if (g_locked) return;

		CPickups::PictureTaken();
	}
}

void SnapshotLock::install()
{
	const unsigned char* site = reinterpret_cast<const unsigned char*>(PICTURE_TAKEN_CALL_SITE);
	if (site[0] != 0xE8) return;

	uintptr_t target = PICTURE_TAKEN_CALL_SITE + 5 + *reinterpret_cast<const int*>(site + 1);
	if (target != PICTURE_TAKEN) return;

	plugin::patch::RedirectCall(PICTURE_TAKEN_CALL_SITE, &onPictureTaken);
}

void SnapshotLock::setLocked(bool t_locked)
{
	g_locked = t_locked;
}
