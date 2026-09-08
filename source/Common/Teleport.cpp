#include "Teleport.h"

#include <CGame.h>
#include <CStreaming.h>
#include <CTimeCycle.h>
#include <CVehicle.h>
#include <CWorld.h>
#include <common.h>

namespace
{
	constexpr float DROP_HEIGHT = 1.0f;

	void placePlayer(CVector& t_destination, bool t_bringVehicle, int t_areaCode)
	{
		CPlayerPed* player = FindPlayerPed();
		if (!player) return;

		player->m_nAreaCode = static_cast<unsigned char>(t_areaCode);
		CGame::currArea = t_areaCode;

		if (t_areaCode == 0) CTimeCycle::StopExtraColour(false);

		CStreaming::LoadScene(&t_destination);

		CVehicle* vehicle = player->m_pVehicle;
		if (t_bringVehicle && vehicle && player->bInVehicle)
		{
			vehicle->Teleport(t_destination, false);
		}
		else
		{
			player->Teleport(t_destination, false);
		}
	}
}

void Teleport::toGround(const CVector& t_destination, bool t_bringVehicle)
{
	CVector destination(t_destination.x, t_destination.y, 0.0f);
	CStreaming::LoadScene(&destination);
	destination.z = CWorld::FindGroundZForCoord(destination.x, destination.y) + DROP_HEIGHT;

	placePlayer(destination, t_bringVehicle, 0);
}

void Teleport::toExact(const CVector& t_destination, bool t_bringVehicle, int t_areaCode)
{
	CVector destination = t_destination;
	placePlayer(destination, t_bringVehicle, t_areaCode);
}
