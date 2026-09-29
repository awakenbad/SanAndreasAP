#pragma once

class BranchProgress;

namespace BranchControllers
{
	void setEnabled(bool t_enabled);
	bool enabled();

	void update(const BranchProgress& t_progress);
}
