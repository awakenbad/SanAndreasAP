#pragma once
#include "Collectible.h"

class TagTracker : public Collectible<100>
{
public:
	TagTracker();

	bool update() override;

protected:
	float readCount() const override;
	int identifyCollected() const override;
};
