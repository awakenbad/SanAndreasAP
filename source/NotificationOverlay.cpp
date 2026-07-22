#include "NotificationOverlay.h"
#include "ScreenScale.h"
#include <algorithm>
#include <CFont.h>
#include <CRGBA.h>
#include <CAudioEngine.h>
#include <enums/eAudioEvents.h>
#include <CRadar.h>
#include <CSprite2d.h>

// All measured at 1920x1080 and converted to the player's resolution via ScreenScale::of().
namespace
{
	const float RIGHT_MARGIN = 30.0f;
	const float BOTTOM_MARGIN = 60.0f;
	const float BOX_PADDING_H = 12.0f;
	const float BOX_PADDING_V = 8.0f;
	const float TEXT_HEIGHT = 36.0f;
	const float ICON_SIZE = 32.0f;
	const float ICON_TEXT_GAP = 10.0f;
	const float SLOT_GAP = 8.0f;
	const float SLOT_HEIGHT = TEXT_HEIGHT + BOX_PADDING_V * 2.0f + SLOT_GAP;

	// Bottom-left, clear of the radar's top edge.
	const float RADAR_MESSAGE_LEFT = 80.0f;
	const float RADAR_MESSAGE_FROM_BOTTOM = 290.0f;
}

void NotificationOverlay::show(const std::string& text, NotificationIcon icon)
{
	m_notifications.push_back({ text, icon, {}, false });
}

void NotificationOverlay::showAboveRadar(const std::string& text)
{
	m_radarMessage = text;
	m_radarMessageExpiresAt = std::chrono::steady_clock::now() + DISPLAY_DURATION;

	AudioEngine.ReportFrontendAudioEvent(AE_FRONTEND_PICKUP_INFO, 1.0f, 1.0f);
}

void NotificationOverlay::drawAboveRadar(std::chrono::steady_clock::time_point now) const
{
	if (m_radarMessage.empty() || now >= m_radarMessageExpiresAt) return;

	unsigned char alpha = 255;
	auto remaining = m_radarMessageExpiresAt - now;
	if (remaining < FADE_DURATION)
	{
		float t = std::chrono::duration<float>(remaining).count() / std::chrono::duration<float>(FADE_DURATION).count();
		if (t < 0.0f) t = 0.0f;
		alpha = static_cast<unsigned char>(255.0f * t);
	}
	if (alpha == 0) return;

	CFont::SetFontStyle(FONT_SUBTITLES);
	CFont::SetScale(ScreenScale::of(0.7f), ScreenScale::of(1.4f));
	CFont::SetColor(CRGBA(255, 255, 255, alpha));
	CFont::SetProportional(true);
	CFont::SetOrientation(ALIGN_LEFT);
	CFont::SetDropShadowPosition(1);
	CFont::SetBackground(false, false);
	CFont::SetWrapx(static_cast<float>(RsGlobal.maximumWidth));

	float x = ScreenScale::of(RADAR_MESSAGE_LEFT);
	float y = static_cast<float>(RsGlobal.maximumHeight) - ScreenScale::of(RADAR_MESSAGE_FROM_BOTTOM);

	float textWidth = CFont::GetStringWidth(m_radarMessage.c_str(), true);
	CRect box(x - ScreenScale::of(6.0f), y,
		x + textWidth + ScreenScale::of(6.0f), y + ScreenScale::of(30.0f));
	CSprite2d::DrawRect(box, CRGBA(0, 0, 0, static_cast<unsigned char>(150.0f * alpha / 255.0f)));

	CFont::PrintString(x, y, m_radarMessage.c_str());
}

void NotificationOverlay::draw()
{
	auto now = std::chrono::steady_clock::now();
	drawAboveRadar(now);

	while (!m_notifications.empty() && m_notifications.front().started && now >= m_notifications.front().expiresAt)
	{
		m_notifications.pop_front();
	}

	if (m_notifications.empty()) return;

	size_t visibleCount = 0;
	while (visibleCount < m_notifications.size() && m_notifications[visibleCount].started)
	{
		visibleCount++;
	}

	bool backlogged = m_notifications.size() > MAX_VISIBLE;

	if (backlogged)
	{
		for (size_t i = 0; i < visibleCount; ++i)
		{
			Notification& notification = m_notifications[i];
			if (!notification.fades) continue; // already on the fast schedule

			notification.fades = false;
			auto conveyorExpiry = now + BACKLOG_ADMIT_INTERVAL * static_cast<long long>(i + 1);
			if (conveyorExpiry < notification.expiresAt)
			{
				notification.expiresAt = conveyorExpiry;
			}
		}
	}

	if (visibleCount < MAX_VISIBLE && visibleCount < m_notifications.size()
		&& (!backlogged || now >= m_nextAdmitAt))
	{
		Notification& admitted = m_notifications[visibleCount];
		admitted.started = true;
		admitted.fades = !backlogged;
		admitted.expiresAt = now + (backlogged
			? BACKLOG_DISPLAY_DURATION
			: std::chrono::duration_cast<std::chrono::milliseconds>(DISPLAY_DURATION));

		AudioEngine.ReportFrontendAudioEvent(AE_FRONTEND_PICKUP_INFO, 1.0f, 1.0f);

		m_nextAdmitAt = now + BACKLOG_ADMIT_INTERVAL;
		visibleCount++;
	}

	// Newest of the visible batch takes the bottom slot, older ones stack upward.
	for (size_t i = 0; i < visibleCount; ++i)
	{
		drawOne(m_notifications[visibleCount - 1 - i], static_cast<int>(i), now);
	}
}

void NotificationOverlay::drawOne(const Notification& notification, int slot, std::chrono::steady_clock::time_point now) const
{
	unsigned char alpha = 255;
	if (notification.fades)
	{
		auto remaining = notification.expiresAt - now;
		if (remaining < FADE_DURATION)
		{
			float t = std::chrono::duration<float>(remaining).count() / std::chrono::duration<float>(FADE_DURATION).count();
			if (t < 0.0f) t = 0.0f;
			alpha = static_cast<unsigned char>(255.0f * t);
		}
	}
	if (alpha == 0) return;

	CFont::SetFontStyle(FONT_SUBTITLES);
	CFont::SetScale(ScreenScale::of(0.9f), ScreenScale::of(1.8f));
	CFont::SetColor(CRGBA(255, 255, 255, alpha));
	CFont::SetProportional(true);
	CFont::SetOrientation(ALIGN_RIGHT);
	CFont::SetDropShadowPosition(1);
	CFont::SetBackground(false, false);

	float x = static_cast<float>(RsGlobal.maximumWidth) - ScreenScale::of(RIGHT_MARGIN);
	float y = static_cast<float>(RsGlobal.maximumHeight) - ScreenScale::of(BOTTOM_MARGIN)
		- static_cast<float>(slot) * ScreenScale::of(SLOT_HEIGHT);

	CFont::SetRightJustifyWrap(0.0f);

	float textWidth = CFont::GetStringWidth(notification.text.c_str(), true);

	CSprite2d* iconSprite = nullptr;
	switch (notification.icon)
	{
	case NotificationIcon::Money: iconSprite = &CRadar::RadarBlipSprites[RADAR_SPRITE_CASH]; break;
	case NotificationIcon::ProgressiveMission: iconSprite = &CRadar::RadarBlipSprites[RADAR_SPRITE_CJ]; break;
	case NotificationIcon::HealthUpgrade: iconSprite = &CRadar::RadarBlipSprites[RADAR_SPRITE_GIRLFRIEND]; break;
	case NotificationIcon::ArmorUpgrade: iconSprite = &CRadar::RadarBlipSprites[RADAR_SPRITE_POLICE]; break;
	case NotificationIcon::Taxi: iconSprite = &CRadar::RadarBlipSprites[RADAR_SPRITE_SPRAY]; break;
	case NotificationIcon::Stamina: iconSprite = &CRadar::RadarBlipSprites[RADAR_SPRITE_TRUCK]; break;
	case NotificationIcon::FireImmunity: iconSprite = &CRadar::RadarBlipSprites[RADAR_SPRITE_FIRE]; break;
	case NotificationIcon::Boxing: iconSprite = &CRadar::RadarBlipSprites[RADAR_SPRITE_GYM]; break;
	case NotificationIcon::Weapon: iconSprite = &CRadar::RadarBlipSprites[RADAR_SPRITE_AMMUGUN]; break;
	case NotificationIcon::Trap: iconSprite = &CRadar::RadarBlipSprites[RADAR_SPRITE_ENEMYATTACK]; break;
	// The airyard (plane) icon reads as "shipped off somewhere else".
	case NotificationIcon::ItemSent: iconSprite = &CRadar::RadarBlipSprites[RADAR_SPRITE_AIRYARD]; break;
	default: break;
	}

	float boxLeft = x - textWidth - ScreenScale::of(BOX_PADDING_H);
	if (iconSprite)
	{
		boxLeft -= ScreenScale::of(ICON_SIZE + ICON_TEXT_GAP);
	}

	CRect box(boxLeft, y - ScreenScale::of(BOX_PADDING_V),
		x + ScreenScale::of(BOX_PADDING_H), y + ScreenScale::of(TEXT_HEIGHT + BOX_PADDING_V));
	CSprite2d::DrawRect(box, CRGBA(0, 0, 0, static_cast<unsigned char>(150.0f * alpha / 255.0f)));

	if (iconSprite)
	{
		float iconY = y + ScreenScale::of((TEXT_HEIGHT - ICON_SIZE) / 2.0f);
		iconSprite->Draw(boxLeft + ScreenScale::of(BOX_PADDING_H), iconY,
			ScreenScale::of(ICON_SIZE), ScreenScale::of(ICON_SIZE), CRGBA(255, 255, 255, alpha));
	}

	CFont::PrintString(x, y, notification.text.c_str());
}
