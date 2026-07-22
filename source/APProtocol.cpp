#include "APProtocol.h"
#include "ParseUtils.h"
#include <cstring>

namespace
{
	// Matches a prefix and hands back what follows it in one step. The point is that the prefix
	// and its length can no longer drift apart: the old parser carried a hand-counted substr()
	// offset next to each prefix string, so editing one meant remembering to edit the other.
	bool stripPrefix(const std::string& t_line, const char* t_prefix, std::string& t_rest)
	{
		size_t length = std::strlen(t_prefix);
		if (t_line.compare(0, length, t_prefix) != 0) return false;

		t_rest = t_line.substr(length);
		return true;
	}
}

APProtocol::Message APProtocol::parse(const std::string& t_line)
{
	Message message;
	std::string rest;

	if (stripPrefix(t_line, "STATUS:", rest))
	{
		message.kind = MessageKind::Status;
		message.text = rest;
		return message;
	}

	if (stripPrefix(t_line, "SENT:", rest))
	{
		message.kind = MessageKind::ItemSent;
		message.text = rest;
		return message;
	}

	if (stripPrefix(t_line, "LOCATE:TAG:", rest))
	{
		message.kind = MessageKind::LocateTag;
		message.index = parseIntOr(rest, -1);
		return message;
	}

	if (stripPrefix(t_line, "SHOPITEM:", rest))
	{
		size_t colon = rest.find(':');
		if (colon == std::string::npos) return message; // malformed - stays Unknown

		message.kind = MessageKind::ShopItem;
		message.index = parseIntOr(rest.substr(0, colon), -1);
		message.text = rest.substr(colon + 1);
		return message;
	}

	if (stripPrefix(t_line, "GIVE:", rest))
	{
		// GIVE:<index>:<effect>[:<value>] - the index is mandatory, since without it the item
		// cannot be deduplicated and would be re-granted on every reconnect.
		size_t afterIndex = rest.find(':');
		if (afterIndex == std::string::npos) return message; // malformed - stays Unknown

		message.index = parseIntOr(rest.substr(0, afterIndex), -1);
		if (message.index < 0) return message;

		std::string body = rest.substr(afterIndex + 1);
		size_t colon = body.find(':');
		message.kind = MessageKind::Give;
		message.effect = (colon == std::string::npos) ? body : body.substr(0, colon);
		message.text = (colon == std::string::npos) ? "" : body.substr(colon + 1);
		return message;
	}

	if (stripPrefix(t_line, "CTRL:", rest))
	{
		// The value is optional: "CTRL:deathlink_kill" carries no colon.
		size_t colon = rest.find(':');
		message.kind = MessageKind::Control;
		message.effect = (colon == std::string::npos) ? rest : rest.substr(0, colon);
		message.text = (colon == std::string::npos) ? "" : rest.substr(colon + 1);
		return message;
	}

	return message;
}

std::string APProtocol::missionCheck(const std::string& t_missionId)
{
	return "CHECK:MISSION:" + t_missionId + "\n";
}

// Submissions travel as mission checks too - they share the mission ID space.
std::string APProtocol::missionCheck(int t_missionId)
{
	return missionCheck(std::to_string(t_missionId));
}

std::string APProtocol::pickUpCheck()
{
	return "CHECK:PICKUP:0\n";
}

std::string APProtocol::tagCheck(int t_tagIndex)
{
	return "CHECK:TAG:" + std::to_string(t_tagIndex) + "\n";
}

std::string APProtocol::submissionLevelCheck(int t_slot)
{
	return "CHECK:SUBLEVEL:" + std::to_string(t_slot) + "\n";
}

std::string APProtocol::shopCheck(int t_slot)
{
	return "CHECK:SHOP:" + std::to_string(t_slot) + "\n";
}

std::string APProtocol::playerDied()
{
	return "PLAYER_DIED\n";
}
