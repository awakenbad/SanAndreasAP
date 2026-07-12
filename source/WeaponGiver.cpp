#include "../header/WeaponGiver.h"

void WeaponGiver::giveSprayCan()
{
    CPed* player = FindPlayerPed();
    if (player && plugin::KeyPressed(VK_TAB)) {
        CStreaming::RequestModel(MODEL_SPRAYCAN, 2);
        CStreaming::LoadAllRequestedModels(false);
        player->GiveWeapon(WEAPONTYPE_SPRAYCAN, 100, true);
        player->SetCurrentWeapon(WEAPONTYPE_SPRAYCAN);
        CStreaming::SetModelIsDeletable(MODEL_SPRAYCAN);
    }
}
