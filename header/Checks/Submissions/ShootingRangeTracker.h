#pragma once
#include "TieredSubmissionTracker.h"

#include <string>

int shootingRangeSubmissionForWeapon(const std::string& t_weaponName);

class ShootingRangeTracker : public TieredSubmissionTracker
{
public:
	ShootingRangeTracker(int t_submissionID, const SubmissionTierSpec& t_spec, int t_weaponIndex);

	void enforceSubmissionReward() override;

protected:
	float getProgress() const override;

private:
	const int WEAPON_INDEX;
};
