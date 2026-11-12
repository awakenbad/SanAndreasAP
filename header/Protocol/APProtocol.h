#pragma once
#include <string>
#include <vector>

namespace APProtocol
{
	// ---- Endpoint ----------------------------------------------------------------------------
	constexpr char CLIENT_HOST[] = "127.0.0.1";
	constexpr int CLIENT_PORT = 41992;

	// ---- Inbound -----------------------------------------------------------------------------

	enum class MessageKind
	{
		Unknown,   // unrecognised or malformed - callers ignore it rather than guess
		Status,    // STATUS:<text>                  plain client status line
		ItemSent,  // SENT:<text>                    an item we found for some other player's world
		Locate,    // LOCATE:<TYPE>:<index>          highlight one collectible on the map
		ShopItem,  // SHOPITEM:<slot>:<text>         what an Ammu-Nation slot now contains
		ShopSold,  // SHOPSOLD:<slot>:<0|1>
		ShopFlags, // SHOPFLAGS:<slot>:<n>
		Give,      // GIVE:<index>:<effect>[:<value>]  an item granted to us, at its list position
		ItemInfo,  // ITEMINFO:<index>:<flags>:<item>\t<sender>\t<game>\t<location>  item log entry for that GIVE
		Control,   // CTRL:<name>[:<value>]          not an item - see below
	};

	class Message
	{
	public:
		MessageKind kind = MessageKind::Unknown;
		std::string text;
		std::string effect;
		int index = -1;
		int flags = 0;
	};

	Message parse(const std::string& t_line);
	std::vector<std::string> splitList(const std::string& t_value, char t_separator);

	// ---- Outbound ----------------------------------------------------------------------------

	std::string missionCheck(const std::string& t_missionId);
	std::string missionCheck(int t_missionId);
	std::string collectibleCheck(const char* t_type, int t_index);
	std::string submissionLevelCheck(int t_slot);
	std::string shopCheck(int t_slot);
	std::string playerDied();
}
