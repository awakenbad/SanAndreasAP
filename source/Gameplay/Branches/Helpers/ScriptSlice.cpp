#include "ScriptSlice.h"
#include "RunningScripts.h"

void ScriptSlice::run() const
{
	RunningScripts::runSlice(from, to);
}
