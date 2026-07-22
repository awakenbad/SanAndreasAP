#pragma once
#include <cstdlib>
#include <string>

// std::stoi throws on malformed input, which would take the whole game down with it. Every
// number this mod parses comes from outside - socket messages from the client, values read back
// out of the companion save file - so none of it can be trusted to actually be numeric.
inline int parseIntOr(const std::string& t_text, int t_fallback)
{
	char* end = nullptr;
	long value = strtol(t_text.c_str(), &end, 10);
	if (end == t_text.c_str()) return t_fallback;
	return static_cast<int>(value);
}

inline float parseFloatOr(const std::string& t_text, float t_fallback)
{
	char* end = nullptr;
	float value = strtof(t_text.c_str(), &end);
	if (end == t_text.c_str()) return t_fallback;
	return value;
}
