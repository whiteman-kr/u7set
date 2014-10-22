#include "Stable.h"
#include "DiagScheme.h"

namespace VFrame30
{

	DiagScheme::DiagScheme(void)
	{
		setUnit(SchemeUnit::Display);

		setDocWidth(1000);
		setDocHeight(750);

		Layers.push_back(std::make_shared<SchemeLayer>("Drawing", true));
		Layers.push_back(std::make_shared<SchemeLayer>("Notes", false));

		return;
	}

	DiagScheme::~DiagScheme(void)
	{
	}

}
