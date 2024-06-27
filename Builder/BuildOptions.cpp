#include "BuildOptions.h"

namespace Builder
{
	bool BuildOptions::makeDecision(bool settingValue, E::BuildOptionValue value)
	{
		return value == E::BuildOptionValue::True || (value == E::BuildOptionValue::Inherit && settingValue == true);
	}
} // namespace Builder