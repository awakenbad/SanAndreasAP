#pragma once
#include <string>
#include <deque>
#include <chrono>

// Draws "received item" notifications as a fully custom on-screen overlay via CFont, entirely
// independent of CHud's shared help-text/message systems. Those get silently eaten by native
// GTA SA animations (the tag-spray camera zoom, the mission-passed stat screen) because both our
// notifications and the native ones go through the same "current message" slot. Drawing directly
// with CFont each frame, on our own timer, means nothing native can steal or cut short our text.
enum class NotificationIcon
{
	None,
	Money,
	ProgressiveMission,
	HealthUpgrade,
	ArmorUpgrade,
	Taxi,
	Stamina,
	FireImmunity,
	Boxing,
	Weapon,
	Trap,
	ItemSent,
};

class NotificationOverlay
{
public:
	void show(const std::string& text, NotificationIcon icon = NotificationIcon::None);

	// One-off status lines (autosave and the like) draw above the radar instead of joining the
	// bottom-right stack, so they don't push item notifications around. Only the latest is kept.
	void showAboveRadar(const std::string& text);

	// Call every frame from Events::drawHudEvent (after the native HUD draw, so this renders on
	// top of anything vanilla drew that frame).
	void draw();

private:
	static constexpr std::chrono::seconds DISPLAY_DURATION{ 8 };
	// The last stretch of a notification's life fades its alpha from full to zero instead of it
	// just vanishing on the tick its timer runs out.
	static constexpr std::chrono::milliseconds FADE_DURATION{ 600 };
	// A burst of many items arriving at once (e.g. reconnecting with a long backlog) shouldn't
	// stack an unbounded tower up the screen, so only this many are on screen at a time. The rest
	// wait their turn - see draw().
	static constexpr size_t MAX_VISIBLE = 5;

	// While a backlog exists the stack behaves like a conveyor: one new notification is let in
	// this often, and the oldest drops off to make room. The pause is the whole point - without
	// it a burst is one indistinguishable flash, and the player cannot tell that a pile of things
	// arrived at all.
	static constexpr std::chrono::milliseconds BACKLOG_ADMIT_INTERVAL{ 100 };
	// Exactly long enough for the fifth notification admitted after it to push it off the top, so
	// eviction is just the normal expiry running out rather than a separate mechanism.
	static constexpr std::chrono::milliseconds BACKLOG_DISPLAY_DURATION{
		BACKLOG_ADMIT_INTERVAL * static_cast<long long>(MAX_VISIBLE) };

	struct Notification
	{
		std::string text;
		NotificationIcon icon;
		// Only meaningful once started. Set when the notification first becomes visible rather
		// than when it is created: a queued one has not begun its life yet, and giving it a
		// deadline it spends entirely off screen is what used to make backlogs disappear unseen.
		std::chrono::steady_clock::time_point expiresAt;
		bool started = false;
		// Backlogged notifications cut out instead of fading: their whole life is shorter than
		// FADE_DURATION, so fading would mean never being drawn at full opacity at all.
		bool fades = true;
	};
	// A queue: oldest at the front. The front MAX_VISIBLE are on screen, newest of them in the
	// anchor slot at the bottom and older ones stacked upward; anything behind them is waiting.
	std::deque<Notification> m_notifications;

	// Earliest time the next queued notification may be let in. Only consulted while backlogged.
	std::chrono::steady_clock::time_point m_nextAdmitAt{};

	std::string m_radarMessage;
	std::chrono::steady_clock::time_point m_radarMessageExpiresAt{};

	void drawOne(const Notification& notification, int slot, std::chrono::steady_clock::time_point now) const;
	void drawAboveRadar(std::chrono::steady_clock::time_point now) const;
};
