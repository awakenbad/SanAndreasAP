#pragma once
#include "Collectible.h"

class SnapshotTracker : public Collectible<50>
{
public:
	SnapshotTracker();
	bool update() override;

protected:
	float readCount() const override;
	int identifyCollected() const override;
};
