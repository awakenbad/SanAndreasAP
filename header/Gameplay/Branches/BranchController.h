#pragma once
#include <CVector.h>
#include "BranchRow.h"
#include "Marker.h"

class BranchProgress;
class EdgeCase;

class BranchController
{
public:
	static constexpr int NO_PREREQUISITE = 0;

	explicit BranchController(const BranchRow& t_row) : m_row(t_row) {}
	virtual ~BranchController() = default;

	virtual EdgeCase* asEdgeCase() { return nullptr; }

	const char* scriptName() const { return m_row.scriptName; }
	int address() const { return m_row.address; }

	int counter() const;
	bool finished() const;
	bool gateOpen(const BranchProgress& t_progress) const;

	bool running() const;
	bool positionsInitialised() const;

	Marker defaultMarker() const;

protected:
	static CVector positionAt(int t_byteOffset, int t_strideBytes = 4);

	const BranchRow m_row;
};


