#pragma once
#include <string>
#include "NotificationOverlay.h"

// What an incoming GIVE: effect actually does. Mod switches on this, so adding an item means
// adding an enumerator and a table row - and the compiler can point at a switch that hasn't
// handled the new one yet.
enum class ItemEffect
{
	Money,
	Weapon,
	ProgressiveMission,
	ProgressiveMap,
	SubmissionCheck, // unlocks one submission's checks; which one is in submissionId
	ArmorRefill,
	CarRepair,
	Trap,
};

// One row of the effect table: the wire name, what it does, and how it is announced. Keeping all
// three together is the whole point - each name used to appear once in a dispatch if-chain and
// again in a separate presentation map, so an item could be granted silently (or announced but
// never granted) and nothing would catch the mismatch.
class ItemEffectSpec
{
public:
	// The name in GIVE:<name> - must match the client's ITEM_ID_TO_EFFECT values exactly.
	const char* name;
	ItemEffect effect;
	// ItemEffect::SubmissionCheck only; -1 otherwise.
	int submissionId;
	// ItemEffect::Trap only - the type TrapHandler expects; nullptr otherwise.
	const char* trapName;
	// On-screen text, where "{}" is replaced with the value that arrived alongside the item.
	const char* message;
	NotificationIcon icon;
};

// nullptr when the name isn't one this build knows - a client newer than the mod can name items
// we have no handler for, and ignoring them beats guessing.
const ItemEffectSpec* findItemEffect(const std::string& t_name);

// Empty when the effect is a silent control message.
std::string formatItemMessage(const ItemEffectSpec& t_spec, const std::string& t_value);
