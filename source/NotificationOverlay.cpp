#include "NotificationOverlay.h"
#include <CFont.h>
#include <CRGBA.h>
#include <CAudioEngine.h>
#include <enums/eAudioEvents.h>
#include <CRadar.h>
#include <CSprite2d.h>

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
}

void NotificationOverlay::show(const std::string& text, NotificationIcon icon)
{
	m_notifications.push_back({ text, icon, std::chrono::steady_clock::now() + DISPLAY_DURATION });

	AudioEngine.ReportFrontendAudioEvent(AE_FRONTEND_PICKUP_INFO, 1.0f, 1.0f);
}

void NotificationOverlay::draw()
{
	auto now = std::chrono::steady_clock::now();
	while (!m_notifications.empty() && now >= m_notifications.front().expiresAt)
	{
		m_notifications.pop_front();
	}

	if (m_notifications.empty()) return;

	int slot = 0;
	for (auto it = m_notifications.rbegin(); it != m_notifications.rend() && slot < static_cast<int>(MAX_VISIBLE); ++it, ++slot)
	{
		drawOne(*it, slot, now);
	}
}

void NotificationOverlay::drawOne(const Notification& notification, int slot, std::chrono::steady_clock::time_point now) const
{
	unsigned char alpha = 255;
	auto remaining = notification.expiresAt - now;
	if (remaining < FADE_DURATION)
	{
		float t = std::chrono::duration<float>(remaining).count() / std::chrono::duration<float>(FADE_DURATION).count();
		if (t < 0.0f) t = 0.0f;
		alpha = static_cast<unsigned char>(255.0f * t);
	}
	if (alpha == 0) return;

	CFont::SetFontStyle(FONT_SUBTITLES);
	CFont::SetScale(0.9f, 1.8f);
	CFont::SetColor(CRGBA(255, 255, 255, alpha));
	CFont::SetProportional(true);
	CFont::SetOrientation(ALIGN_RIGHT);
	CFont::SetDropShadowPosition(1);
	CFont::SetBackground(false, false);

	float x = static_cast<float>(RsGlobal.maximumWidth) - RIGHT_MARGIN;
	float y = static_cast<float>(RsGlobal.maximumHeight) - BOTTOM_MARGIN - static_cast<float>(slot) * SLOT_HEIGHT;

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
	default: break;
	}

	float boxLeft = x - textWidth - BOX_PADDING_H;
	if (iconSprite)
	{
		boxLeft -= ICON_SIZE + ICON_TEXT_GAP;
	}

	CRect box(boxLeft, y - BOX_PADDING_V, x + BOX_PADDING_H, y + TEXT_HEIGHT + BOX_PADDING_V);
	CSprite2d::DrawRect(box, CRGBA(0, 0, 0, static_cast<unsigned char>(150.0f * alpha / 255.0f)));

	if (iconSprite)
	{
		float iconY = y + (TEXT_HEIGHT - ICON_SIZE) / 2.0f;
		iconSprite->Draw(boxLeft + BOX_PADDING_H, iconY, ICON_SIZE, ICON_SIZE, CRGBA(255, 255, 255, alpha));
	}

	CFont::PrintString(x, y, notification.text.c_str());
}
