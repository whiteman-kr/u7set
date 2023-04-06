#pragma once

#include "IssueLogger.h"

namespace Gateway
{
	enum class GatewayType
	{
		IVS_Impulse,
	};

	struct Gateway
	{
		GatewayType type;
		QString gatewayID;
		QString description;
	};

	class GatewayDescriptionParser
	{
	public:
		GatewayDescription();

		void setDescription(const QString& desc);
		bool parse(Builder::IssueLogger* log);

	private:
		QString m_description;
		Builder::IssueLogger* m_log = nullptr;
	};
}

