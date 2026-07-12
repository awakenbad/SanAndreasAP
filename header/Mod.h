#pragma once
#include <APSocket.h>
#include "WeaponGiver.h"
#include "CheckListener.h"
#include "CheckGiver.h"
#include "CStats.h"

class Mod
{
public:
	Mod();

	/// <summary>
	/// Starts mod logic
	/// </summary>
	void start();

private:
	WeaponGiver m_weaponGiver;
	CheckListener m_checkListener;
	CheckGiver m_checkGiver;
	APSocket m_apSocket;
	CheckEvent m_currentEvent;

	void parseIncomingMessages();
	void receiveCurrentCheckEvent();
};

