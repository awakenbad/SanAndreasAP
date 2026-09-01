#pragma once
#include <string>
#include <deque>
#include <chrono>

enum class NotificationIcon
{
	None,
	Money,
	ProgressiveMission,
	HealthUpgrade,
	Hospital,
	Spray,
	Oyster,
	Snapshot,
	Horseshoe,
	ArmorUpgrade,
	Taxi,
	Stamina,
	FireImmunity,
	Boxing,
	Weapon,
	Trap,
	ItemSent,
	StreetRaces,
	WangCars
};

class NotificationOverlay
{
public:
	void show(const std::string& text, NotificationIcon icon = NotificationIcon::None, int radarSpriteOverride = -1);

	void showAboveRadar(const std::string& text);

	void draw();

private:
	static std::chrono::milliseconds displayDuration();

	static constexpr std::chrono::milliseconds FADE_DURATION{ 600 };
	static constexpr size_t MAX_VISIBLE = 5;

	static constexpr std::chrono::milliseconds BACKLOG_ADMIT_INTERVAL{ 100 };
	static constexpr std::chrono::milliseconds BACKLOG_DISPLAY_DURATION{
		BACKLOG_ADMIT_INTERVAL * static_cast<long long>(MAX_VISIBLE) };

	struct Notification
	{
		std::string text;
		NotificationIcon icon;
		std::chrono::steady_clock::time_point expiresAt;
		bool started = false;
		bool fades = true;
		int radarSprite = -1;
	};
	std::deque<Notification> m_notifications;

	std::chrono::steady_clock::time_point m_nextAdmitAt{};

	std::string m_radarMessage;
	std::chrono::steady_clock::time_point m_radarMessageExpiresAt{};

	void drawOne(const Notification& notification, int slot, std::chrono::steady_clock::time_point now) const;
	void drawAboveRadar(std::chrono::steady_clock::time_point now) const;
};
