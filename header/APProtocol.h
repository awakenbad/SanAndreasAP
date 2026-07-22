#pragma once
#include <string>

// The wire protocol spoken with the Python AP client, both directions in one place. Every literal
// here is also hard-coded on the client side, so this file is the contract between the two halves
// of the project - and nothing in it touches the game, so it can be read (and corrected) without
// launching GTA.
namespace APProtocol
{
	// ---- Inbound -----------------------------------------------------------------------------

	enum class MessageKind
	{
		Unknown,   // unrecognised or malformed - callers ignore it rather than guess
		Status,    // STATUS:<text>                  plain client status line
		ItemSent,  // SENT:<text>                    an item we found for some other player's world
		LocateTag, // LOCATE:TAG:<index>             highlight one spray tag on the map
		ShopItem,  // SHOPITEM:<slot>:<text>         what an Ammu-Nation slot now contains
		Give,      // GIVE:<index>:<effect>[:<value>]  an item granted to us, at its list position
		Control,   // CTRL:<name>[:<value>]          not an item - see below
	};

	// Control messages are kept separate from items on purpose. The server replays every item on
	// every connect, so items must be deduplicated by index; a control message is an event with
	// no position in that list, and deduplicating one would silently drop it.
	//
	// A parsed inbound line. Which fields carry meaning depends on kind; the rest stay empty,
	// so reading the wrong one is harmless rather than undefined.
	class Message
	{
	public:
		MessageKind kind = MessageKind::Unknown;
		// Status/ItemSent: the text to display. ShopItem: the slot's contents.
		// Give/Control: the value.
		std::string text;
		// Give: the effect name ("money", "trap_fat", ...). Control: the control name.
		std::string effect;
		// LocateTag/ShopItem: the index. Give: position in the client's items_received list.
		// -1 when absent or not a number.
		int index = -1;
	};

	Message parse(const std::string& t_line);

	// ---- Outbound ----------------------------------------------------------------------------
	// Each returns a complete newline-terminated line ready for APSocket::sendToServer.

	std::string missionCheck(const std::string& t_missionId);
	std::string missionCheck(int t_missionId);
	std::string pickUpCheck();
	std::string tagCheck(int t_tagIndex);
	std::string submissionLevelCheck(int t_slot);
	std::string shopCheck(int t_slot);
	std::string playerDied();
}
