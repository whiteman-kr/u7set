#pragma once

namespace VFrame30
{
	// This interface provide a way to get linked items like app signal, connection, loopback for the SchemaItem.
	//
	class IMatsSchemaItemAssociations
	{
	public:
		virtual ~IMatsSchemaItemAssociations() = default;

		virtual QStringList associatedDiagObjectIds() const = 0;
		virtual QStringList associatedAppSignalIds() const = 0;
		virtual QStringList associatedImpactAppSignalIds() const = 0;
		virtual QStringList associatedConnectionIds() const = 0;
		virtual QStringList associatedLoopbackIds() const = 0;
		virtual QStringList associatedSchemaItemLabels() const = 0;
	};
} // namespace VFrame30