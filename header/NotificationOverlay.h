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
};

class NotificationOverlay
{
public:
	void show(const std::string& text, NotificationIcon icon = NotificationIcon::None);

	// Call every frame from Events::drawHudEvent (after the native HUD draw, so this renders on
	// top of anything vanilla drew that frame).
	void draw();

private:
	static constexpr std::chrono::seconds DISPLAY_DURATION{ 8 };
	// The last stretch of a notification's life fades its alpha from full to zero instead of it
	// just vanishing on the tick its timer runs out.
	static constexpr std::chrono::milliseconds FADE_DURATION{ 600 };
	// A burst of many items arriving at once (e.g. reconnecting with a long backlog) shouldn't
	// stack an unbounded tower up the screen - only the most recent few are ever drawn; the rest
	// still show once older ones expire and free up a slot.
	static constexpr size_t MAX_VISIBLE = 5;

	struct Notification
	{
		std::string text;
		NotificationIcon icon;
		// Set once, at creation - each notification counts down independently regardless of how
		// many others are stacked above or below it.
		std::chrono::steady_clock::time_point expiresAt;
	};
	// Oldest at the front, newest at the back. The newest occupies the anchor slot at the bottom
	// of the stack; each older one still active is drawn one slot further up.
	std::deque<Notification> m_notifications;

	void drawOne(const Notification& notification, int slot, std::chrono::steady_clock::time_point now) const;
};
