#include <CMessages.h>
#include <EventList.h>
#include <Events.h>
#include <cstdio>

#include "GameStorageHook.h"
#include "Mod.h"
#include "SaveRedirect.h"
#include "SnapshotLock.h"

using namespace plugin;

struct Main
{
	size_t m_frame = 0; // render frame counter
	Mod mod;

	Main()
	{
		GameStorageHook::install();
		SaveRedirect::install();
		SnapshotLock::install();

		// register event callbacks
		Events::gameProcessEvent += [] { gInstance.OnGameProcess(); };
		Events::drawHudEvent += [] { gInstance.mod.drawOverlay(); };
		Events::drawMenuBackgroundEvent += [] { gInstance.mod.updateMenuState(); };
		Events::drawMenuBackgroundEvent += [] { gInstance.mod.drawMenuOverlay(); };
		Events::drawBlipsEvent += [] { gInstance.mod.drawMissionCountsOnRadar(); };
		Events::drawBlipsEvent += [] { gInstance.mod.drawMissionCountsOnMap(); };
		Events::drawBlipsEvent += [] { gInstance.mod.drawCollectiblesOnRadar(); };
		Events::drawBlipsEvent += [] { gInstance.mod.drawCollectiblesOnMap(); };
	}

	void OnGameProcess()
	{
		//onScreenText();
		mod.start();
	}

	void onScreenText()
	{
		m_frame++;

		static char msg[255];
		sprintf_s(msg, "Hello from '%s' plugin! Frame %d", TARGET_NAME, m_frame);

		CMessages::AddMessageJumpQ(msg, 500, 0);
	}
} gInstance;
