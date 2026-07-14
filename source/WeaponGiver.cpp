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

void WeaponGiver::giveMolotov()
{
    CPed* player = FindPlayerPed();
    if (player && plugin::KeyPressed(VK_TAB)) {
        CStreaming::RequestModel(MODEL_MOLOTOV, 2);
        CStreaming::LoadAllRequestedModels(false);
        player->GiveWeapon(WEAPONTYPE_MOLOTOV, 100, true);
        player->SetCurrentWeapon(WEAPONTYPE_MOLOTOV);
        CStreaming::SetModelIsDeletable(MODEL_MOLOTOV);
    }
}
