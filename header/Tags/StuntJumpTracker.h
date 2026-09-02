#pragma once
#include "Collectible.h"

class StuntJumpTracker : public Collectible<70>
{
public:
	StuntJumpTracker();

	bool update() override;

protected:
	float readCount() const override;
	int identifyCollected() const override;
};
