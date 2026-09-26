#include "PhoneCall.h"
#include "ScriptGlobals.h"

void PhoneCall::runIfDue() const
{
	if (ScriptGlobals::readAt(calledOffset) != 0) return;
	if (ScriptGlobals::readAt(counterOffset) < counterAtLeast) return;

	body.run();
}
