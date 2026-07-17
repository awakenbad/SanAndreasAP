#pragma once
#include <array>
#include <string>

struct ShopItemInfo
{
	const char* gxtKey;      // tMenuPanel row-cell GXT key, from data/shopping.dat nametags
	const char* displayName; // for notifications / AP location naming
};

// The 16 items every Ammu-Nation tier sells (data/shopping.dat sections ammun1-5 are all
// identical). The index in this array is the CHECK:SHOP:<index> slot id sent to the client.
inline const std::array<ShopItemInfo, 16> shopItems = { {
	{ "PISTOL",  "Pistol" },
	{ "PISTSL",  "Silenced Pistol" },
	{ "DEAGLE",  "Desert Eagle" },
	{ "GRENADE", "Grenade" },
	{ "SHOTGN1", "Shotgun" },
	{ "SHOTGN2", "Combat Shotgun" },
	{ "SHOTGN3", "Sawn-off Shotgun" },
	{ "UZI",     "Micro Uzi" },
	{ "TEC9",    "Tec-9" },
	{ "MP5",     "MP5" },
	{ "AK",      "AK-47" },
	{ "M4",      "M4" },
	{ "RIFLE",   "Country Rifle" },
	{ "SNIPE",   "Sniper Rifle" },
	{ "DETONA",  "Satchel Charge" },
	{ "ARMOUR",  "Armor" },
} };

// Ammu-Nation shop randomization. Purchases are detected via the shop's own confirm panel:
// while the "amunat" script runs, the 2-column confirm menu shows the selected item's GXT key
// until a completed purchase flips that cell to "BOUGHT" (observed live). On that edge the
// vanilla reward (weapon ammo / armor) is reverted from a pre-purchase snapshot, and the slot
// is queued so Mod can send the AP check - money stays spent, making it a shop of checks.
class AmmuNationShop
{
public:
	// Call once per tick.
	void update();

	// Returns the shopItems index of a just-intercepted purchase (once), or -1.
	int pollPurchasedSlot();

	// Scouted AP item name for a slot, pushed by the client via SHOPITEM messages.
	void setSlotContents(int t_slot, const std::string& t_text);

	// Draws what each visible shop row's check actually contains, next to the native menu.
	// Call from the HUD draw event.
	void drawShopContents();

private:
	bool isShopScriptActive() const;
	void snapshotPlayerState();
	void restorePlayerState();
	int slotForKey(const char* t_gxtKey) const;
	void drawContentsLine(float t_x, float t_y, const std::string& t_text) const;

	std::array<std::string, 16> m_slotContents;

	int m_pendingPurchasedSlot = -1;
	int m_confirmPanelItem = -1;
	bool m_boughtVisible = false;

	struct WeaponSlotSnapshot
	{
		int type = 0;
		unsigned int clip = 0;
		unsigned int total = 0;
	};
	std::array<WeaponSlotSnapshot, 13> m_weaponSnapshot{};
	float m_armourSnapshot = 0.0f;
	bool m_snapshotValid = false;
};
