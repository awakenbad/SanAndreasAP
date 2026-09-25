#pragma once
#include "BranchController.h"

// Anything that works differently from the rest, inherits this
class EdgeCase : public BranchController
{
public:
	using BranchController::BranchController;

	EdgeCase* asEdgeCase() override { return this; }

	virtual void update() = 0;
};
