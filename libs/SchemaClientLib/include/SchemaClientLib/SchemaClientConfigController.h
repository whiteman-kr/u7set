#pragma once

#include <ClientLib/ConfigController.h>
#include "../UtilsLib/ILogFile.h"
#include "../VFrame30/SchemaDetails.h"
#include "../libs/AppSignalLists/include/AppSignalLists/SignalList.h"
#include <QReadWriteLock>

class HostAddressPort;

namespace SchemaClientLib
{

	// This is a base class for clients config controller with schemas, like Monitor, Diagnostics, ...
	// Class is thread-safe.
	//
	class SchemaClientConfigController : public ClientLib::ConfigController
	{
		Q_OBJECT

	public:
		SchemaClientConfigController() = delete;
		SchemaClientConfigController(const SoftwareInfo& softwareInfo, const HostAddressPort& address1, const HostAddressPort& address2, ILogFile* logFile);

	protected:
		// Call this function to read schemas details from the server.
		// It must be called in the derived class.
		//
		bool getSchemasDetails();

		// Call this function to read AppSignalLists from the server.
		// It must be called in the derived class.
		//
		bool getAppSignalLists(const std::vector<OnlineLib::BuildFileInfo>& files);

		// Public properties
		//
	public:
		VFrame30::SchemaDetailsSet schemasDetailsSet() const;
		std::vector<VFrame30::SchemaDetails> schemasDetails() const;
		std::set<QString> schemaAppSignals(const QString& schemaId);

		QStringList schemasByAppSignalId(const QString& appSignalId) const;
		QStringList schemasByLoopbackId(const QString& loopbackId) const;

		int schemaCount() const;
		QString schemaCaptionById(const QString& schemaId) const;
		QString schemaCaptionByIndex(int schemaIndex) const;
		QString schemaIdByIndex(int schemaIndex) const;

		bool schemaHasTags(const QString& schemaId, const QStringList& tags) const;

		std::vector<VFrame30::SchemaDetails::TrendIndicatorSchemaItems> trendSchemaItems() const;

		AppSignalLists::AppSignalListSet appSignalListSet() const;

		// Data section
		//
	private:
		mutable QReadWriteLock m_schemaDetailsLock;
		VFrame30::SchemaDetailsSet m_schemaDetailsSet;

		mutable QReadWriteLock m_appSignalListSetLock;
		AppSignalLists::AppSignalListSet m_appSignalListSet;

	};

} // namespace ClientLib
