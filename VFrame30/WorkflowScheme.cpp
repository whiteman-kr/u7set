#include "Stable.h"
#include "WorkflowScheme.h"

namespace VFrame30
{

	WorkflowScheme::WorkflowScheme(void)
	{
		setUnit(SchemeUnit::Display);

		setDocWidth(1000);
		setDocHeight(750);

		Layers.push_back(std::make_shared<SchemeLayer>("Drawing", true));
		Layers.push_back(std::make_shared<SchemeLayer>("Notes", false));

		return;
	}
	
	WorkflowScheme::~WorkflowScheme(void)
	{
	}
}
