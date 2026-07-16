#pragma once
#include <string>
#include <unordered_map>

struct WeaponInfo
{
	eWeaponType type;
	eModelID model;
	unsigned int ammo;
};

inline const std::unordered_map<std::string, WeaponInfo> weaponDataByName = {
	{ "Pistol",             { WEAPONTYPE_PISTOL,          MODEL_COLT45,       100 } },
	{ "Silenced Pistol",    { WEAPONTYPE_PISTOL_SILENCED, MODEL_SILENCED,     100 } },
	{ "Desert Eagle",       { WEAPONTYPE_DESERT_EAGLE,    MODEL_DESERT_EAGLE,  40 } },
	{ "Shotgun",            { WEAPONTYPE_SHOTGUN,         MODEL_CHROMEGUN,     50 } },
	{ "Sawn-off Shotgun",   { WEAPONTYPE_SAWNOFF,         MODEL_SAWNOFF,       50 } },
	{ "Combat Shotgun",     { WEAPONTYPE_SPAS12,          MODEL_SHOTGSPA,      50 } },
	{ "Micro Uzi",          { WEAPONTYPE_MICRO_UZI,       MODEL_MICRO_UZI,    200 } },
	{ "MP5",                { WEAPONTYPE_MP5,             MODEL_MP5LNG,       200 } },
	{ "AK-47",              { WEAPONTYPE_AK47,            MODEL_AK47,         200 } },
	{ "M4",                 { WEAPONTYPE_M4,              MODEL_M4,           200 } },
	{ "Tec-9",              { WEAPONTYPE_TEC9,            MODEL_TEC9,         200 } },
	{ "Country Rifle",      { WEAPONTYPE_COUNTRYRIFLE,    MODEL_CUNTGUN,       50 } },
	{ "Sniper Rifle",       { WEAPONTYPE_SNIPERRIFLE,     MODEL_SNIPER,        25 } },
	{ "Rocket Launcher",    { WEAPONTYPE_RLAUNCHER,       MODEL_ROCKETLA,      10 } },
	{ "Rocket Launcher HS", { WEAPONTYPE_RLAUNCHER_HS,    MODEL_HEATSEEK,      10 } },
	{ "Flamethrower",       { WEAPONTYPE_FTHROWER,        MODEL_FLAME,        300 } },
	{ "Minigun",            { WEAPONTYPE_MINIGUN,         MODEL_MINIGUN,      500 } },
	{ "Grenade",            { WEAPONTYPE_GRENADE,         MODEL_GRENADE,        5 } },
	{ "Molotov Cocktail",   { WEAPONTYPE_MOLOTOV,         MODEL_MOLOTOV,        5 } },
	{ "Tear Gas",           { WEAPONTYPE_TEARGAS,         MODEL_TEARGAS,        5 } },
	{ "Satchel Charge",     { WEAPONTYPE_SATCHEL_CHARGE,  MODEL_SATCHEL,        5 } },
};
