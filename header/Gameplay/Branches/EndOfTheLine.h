#pragma once

class BranchProgress;
struct CVector;

namespace EndOfTheLine
{
	void install(const BranchProgress& t_progress);
	void setMissionsRequired(int t_count);
	void update();

	bool markerAt(const CVector& t_position);
	int missionsCompleted();
	// Enough missions done for the marker to start End of the Line
	bool unlocked();
}
