#pragma once
#include "EdgeCase.h"

// Body Harvest puts King in Exile's market on the spot where badlands is, so it has to be deleted first
class BcrashController : public EdgeCase
{
public:
	BcrashController();

	void update() override;
};

class CatController : public EdgeCase
{
public:
	CatController();

	void update() override;

private:
	static constexpr int SPOT_STRIDE = 24;
	static constexpr int FIRST_DATE_OFFSET = 2044;
	static constexpr int CABIN_OFFSET = 2024;
	static constexpr int CABIN_SPRITE_OFFSET = 1948;
	static constexpr int BLIP_HANDLE_OFFSET = 1932;

	Marker firstDate() const;
	Marker cabin() const;
	void writeSkippedMissionState() const;
};

class TruController : public EdgeCase
{
public:
	TruController();

	void update() override;

private:
	static constexpr int SAN_FIERRO_OFFSET = 1988;
};

// Wu Zi Mu and Farewell, My Love. One mission script re-entered at five counter stages with two
// races between, and the second marker is a literal the script holds in no global.
class BcesarController : public EdgeCase
{
public:
	BcesarController();

	void update() override;

private:
	static constexpr int FAREWELL_FIRST = 5;
	static constexpr int FAREWELL_LAST = 7;
};

class GarageController : public EdgeCase
{
public:
	GarageController();

	void update() override;

private:
	static constexpr int SCRASH_COUNTER_OFFSET = 2184;
	static constexpr int SCRASH_FINISHED = 2;
	static constexpr int SYND_COUNTER_OFFSET = 2180;
	static constexpr int SYND_FINISHED = 10;

	Marker garage() const;
};

class WuziController : public EdgeCase
{
public:
	WuziController();

	void update() override;
};

class TorenoController : public EdgeCase
{
public:
	TorenoController();

	void update() override;
};

class CasinoController : public EdgeCase
{
public:
	CasinoController();

	void update() override;
};

class VegasCrashController : public EdgeCase
{
public:
	VegasCrashController();

	void update() override;
};

// The Meat Business re-adds this blip at its end even when Madd Dogg is already done
class MaddDoggController : public EdgeCase
{
public:
	MaddDoggController();

	void update() override;
};
