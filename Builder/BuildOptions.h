#pragma once

namespace Builder
{
	struct BuildOptions
	{
		E::BuildOptionValue generateAppLogicDrawings{E::BuildOptionValue::Inherit};
		E::BuildOptionValue generateAppSignalsXml{E::BuildOptionValue::Inherit};
		E::BuildOptionValue generateAppSignalsExtXml{E::BuildOptionValue::Inherit};
		E::BuildOptionValue generateExtraDebugInfo{E::BuildOptionValue::Inherit};
		E::BuildOptionValue runSimTestsOnBuild{E::BuildOptionValue::Inherit};

		static bool makeDecision(bool settingValue, E::BuildOptionValue value);
	};
} // namespace Builder