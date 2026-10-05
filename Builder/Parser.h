#pragma once

#include "UuidGenerator.h"

#include <VFrame30/ActuatorHeader.h>
#include <VFrame30/FblItem.h>
#include <VFrame30/HorzVertLinks.h>


class AppLogicGraph;
class LmDescription;

namespace Afb
{
	class AfbComponent;
	class AfbElement;
} // namespace Afb

namespace AppSignalLib
{
	class BusSet;
} // namespace AppSignalLib

namespace Hardware
{
	class EquipmentSet;
	class OptoModuleStorage;
	class DeviceModule;
} // namespace Hardware

namespace VFrame30
{
	class UfbSchema;
	class LogicSchema;
	class SchemaItemAfb;
	class FblItemRect;
	class Schema;

	class SchemaItemTerminator;
	class SchemaItemSignal;
	class SchemaItemConst;
	class SchemaItemTransmitter;
	class SchemaItemReceiver;
	class SchemaItemLoopback;
	class SchemaItemBus;
	class SchemaLayer;

	class ActuatorHeader;
	class ActuatorSchema;
} // namespace VFrame30

namespace Builder
{
	class Context;
	class BuildResultWriter;
	class IssueLogger;
	class SignalSet;
	class LmDescriptionSet;

	class Link
	{
	public:
		Link() = default;

		template<typename Container>
		Link(const Container& points)
		{
			setPoints(points);
		}

		template<typename Container>
		void setPoints(const Container& points)
		{
			m_points.clear();
			m_horzVertLinks.clear();

			Q_ASSERT(points.size() >= 2);
			m_points.reserve(points.size());
			m_points = {points.begin(), points.end()};

			constexpr const QUuid uuid{0x66555511, 0x1122, 0x4444, 0x88, 0x86, 0x32, 0x29, 0x22, 0x33, 0x33, 0x11};
			m_horzVertLinks.AddLinks(m_points.begin(), m_points.end(), uuid, 4);
		}


		VFrame30::SchemaPoint ptBegin() const;
		VFrame30::SchemaPoint ptEnd() const;

		bool isPinOnLink(VFrame30::SchemaPoint pt) const;

	private:
		std::vector<VFrame30::SchemaPoint> m_points;
		VFrame30::CHorzVertLinks m_horzVertLinks; // Used for IsPointOnLink
	};

	struct Bush
	{
		QUuid outputPin;                          // Output pin for this branch, can be the only
		std::set<QUuid> inputPins;                // Input pins for this branch
		std::map<QUuid, Link> links;              // Links for this branch
		std::map<QUuid, std::shared_ptr<VFrame30::FblItemRect>> fblItems;

		VFrame30::FblItemRect* itemByPinGuid(QUuid pinId) const;
		VFrame30::FblItemRect* itemByGuid(QUuid uuid) const;
		VFrame30::AfbPin pinByGuid(QUuid pinId);

		std::vector<QUuid> getAllUuid() const;    // Used for IssueLogger
		std::vector<QUuid> getLinksUuids() const; // Used for IssueLogger

		std::vector<VFrame30::AfbPin> getInputPinsForItem(QUuid fblItemUuid) const;

		bool hasCommonFbls(const Bush& bush) const;

		// bool hasInputOrOutput(const QUuid& uuid) const;
		// bool hasJoinedInOuts(Bush& bush) const;

		void debugInfo() const;
	};

	struct BushContainer
	{
		std::vector<Bush> bushes;

		int getBranchByPinPos(VFrame30::SchemaPoint pt) const;
		int getBranchByPinGuid(const QUuid& guid) const;

		void removeEmptyBushes();

		void debugInfo();
	};


	// ------------------------------------------------------------------------
	//
	//		AppLogicItem
	//
	// ------------------------------------------------------------------------

	struct AppLogicItem
	{
		// Data
		//
		std::shared_ptr<VFrame30::FblItemRect> m_fblItem;
		std::shared_ptr<VFrame30::Schema> m_schema;

		QUuid m_groupId; // SchemaItemUfb is expanded to the group of items, all these expanded items have the same m_groupId
						 // This id is empty if item is not in group

		// Methods
		//
		AppLogicItem() = default;
		AppLogicItem(const AppLogicItem&) = default;
		AppLogicItem(const std::shared_ptr<VFrame30::FblItemRect>& fblItem, const std::shared_ptr<VFrame30::Schema>& schema);

		const Afb::AfbElement& afbElement() const;
		Afb::AfbElement& afbElement();

		std::shared_ptr<Afb::AfbComponent> afbComponent();
		std::shared_ptr<Afb::AfbComponent> afbComponent() const;

		// Items can be kept in set, it is just comparing m_fblItem pointers
		//
		bool operator<(const AppLogicItem& li) const;
		bool operator==(const AppLogicItem& li) const;

		// Write fully parsed AppLogicItem to the output for further analysis by third-party tools.
		//
		static void writeXml(QXmlStreamWriter& writer, const VFrame30::FblItemRect& fblItem);

		static void writeXml(QXmlStreamWriter& writer, const VFrame30::SchemaItemAfb& item);
		static void writeXml(QXmlStreamWriter& writer, const VFrame30::SchemaItemTerminator& item);
		static void writeXml(QXmlStreamWriter& writer, const VFrame30::SchemaItemSignal& item);
		static void writeXml(QXmlStreamWriter& writer, const VFrame30::SchemaItemConst& item);
		static void writeXml(QXmlStreamWriter& writer, const VFrame30::SchemaItemTransmitter& item);
		static void writeXml(QXmlStreamWriter& writer, const VFrame30::SchemaItemReceiver& item);
		static void writeXml(QXmlStreamWriter& writer, const VFrame30::SchemaItemLoopback& item);
		static void writeXml(QXmlStreamWriter& writer, const VFrame30::SchemaItemBus& item);

		static void writeXml(QXmlStreamWriter& writer, const Afb::AfbElement& afbElement);
	};


	// ------------------------------------------------------------------------
	//
	//		ApplicationLogicModule
	//
	// ------------------------------------------------------------------------
	class AppLogicModule : public QObject
	{
		Q_OBJECT

	public:
		AppLogicModule(QString moduleId, QString lmDescriptionFile, const ::LmDescription& lmDescription, IssueLogger& log);
		virtual ~AppLogicModule();

	public:
		bool addBranch(std::shared_ptr<VFrame30::Schema> schema, const BushContainer& bushes);

		bool createGraph();
		bool orderItems();

		//	Make deep copy with new guid for everything, items, pins, associated, etc...
		//
		std::shared_ptr<AppLogicModule> deepCopy(QUuid groupId, const QString& label) const;

		bool checkItemsRelationsConsistency() const;

		static bool checkItemsRelationsConsistency(const QString& equipmentId, const std::list<AppLogicItem>& items, IssueLogger& log);

		bool removeInOutItemKeepAssoc(const QUuid& itemGuid);

		void dump() const;
		[[nodiscard]] QByteArray writeParsedXml() const;


	public:
		// Set connection between SchemaItemInput/SchemaItemOutput by StrIds
		//
		bool setInputOutputsElementsConnection();

	public:
		QString equipmentId() const;
		QString lmDescriptionFile() const;

		const std::list<AppLogicItem>& items() const;
		std::list<AppLogicItem>& items();

		const std::map<QUuid, AppLogicItem>& fblItemsAcc() const;
		std::map<QUuid, AppLogicItem>& fblItemsAcc();
		void setFblItemsAcc(std::map<QUuid, AppLogicItem> v);

	private:
		IssueLogger& m_log;
		QString m_equipmentId;                       // EquipmentId or UFB SchemaID
		QString m_lmDescriptionFile;                 // LogicModule description filename
		const LmDescription& m_lmDescription;        // LogicModule description

		mutable UuidGenerator m_uuidGeneratorUfbDeepCopy;
		mutable UuidGenerator m_uuidGeneratorInOut;

		std::vector<AppLogicItem> m_graphItems;      // Items for graph
		std::unique_ptr<::AppLogicGraph> m_graph;

		std::list<AppLogicItem> m_items;             // Ordered items
		std::map<QUuid, AppLogicItem> m_fblItemsAcc; // Temporary buffer, filled in addBranch, cleared in orderItems


		//
		QHash<QString, bool> m_signaledItems;
	};

	// Here we store actuator type and all its logic schemas
	//
	struct BuildActuatorType
	{
		VFrame30::ActuatorHeader actuatorHeader;                                  // ActuatorHeader for this actuator type
		std::shared_ptr<Hardware::DeviceModule> acmPreset;                        // ACM Preset
		std::map<QString, std::shared_ptr<Hardware::DeviceAppSignal>> acmInputs;  // ACM inputs
		std::map<QString, std::shared_ptr<Hardware::DeviceAppSignal>> acmOutputs; // ACM outputs
		std::vector<std::shared_ptr<VFrame30::ActuatorSchema>> schemas;           // Actuator logic schemas for this actuator type
		std::shared_ptr<AppLogicModule> parseResult;                              // Actuator module for this actuator type,
																				  // created after parsing all schemas

		bool isValid() const { return (acmPreset != nullptr && parseResult != nullptr); }
	};


	// ------------------------------------------------------------------------
	//
	//		ApplicationLogicData
	//
	// ------------------------------------------------------------------------
	class AppLogicData
	{
	public:
		explicit AppLogicData(SignalSet& signalSet, LmDescriptionSet& lmDescriptions, IssueLogger& log);

		// Public methods
		//
	public:
		bool addLogicModuleData(QString equipmentId, const BushContainer& bushContainer, std::shared_ptr<VFrame30::LogicSchema> schema);
		bool addUfbData(const BushContainer& bushContainer, std::shared_ptr<VFrame30::UfbSchema> schema);
		bool addActuatorData(const BushContainer& bushContainer,
							 std::shared_ptr<VFrame30::ActuatorSchema> schema,
							 const BuildActuatorType& bat);

		bool orderLogicModuleItems();
		bool orderUfbItems();
		bool orderActuatorSchemaItems();

		bool expandUfbsForAppLogic();
		bool expandUfbsForActuatorLogic();
		bool expandUfbs(std::vector<std::shared_ptr<AppLogicModule>> modules);

		static bool bindTwoPins(VFrame30::AfbPin& outPin, VFrame30::AfbPin& inputPin);

		bool setAfbComponentsLms(const LmDescriptionSet* lmDescriptionSet);
		bool setAfbComponentsActuators(const LmDescriptionSet* lmDescriptionSet);

		bool resolvePackedLogicAfbs();

		bool setInputOutputsElementsConnectionAppLogic();
		bool setInputOutputsElementsConnectionActuatorLogic();

		bool createGraphs();

		/// @brief Write fully parsed AppLogicData to the output for further analysis by third-party tools.
		bool writeToOutput(QString buildPath, BuildResultWriter& buildResultWriter, const std::vector<Hardware::DeviceModule*>& fscModules);

		// Properties
		//
	public:
		const std::list<std::shared_ptr<AppLogicModule>>& modules() const;
		std::list<std::shared_ptr<AppLogicModule>>& modules();
		std::shared_ptr<AppLogicModule> module(const QString& moduleStrID);

		const std::map<QString, std::shared_ptr<AppLogicModule>>& ufbs() const;
		std::shared_ptr<AppLogicModule> ufb(const QString& ufbId) const;

		const std::map<QString, BuildActuatorType>& actuators() const;
		BuildActuatorType actuator(const QString& actuatorTypeId) const;

	private:
		std::list<std::shared_ptr<AppLogicModule>> m_modules;
		std::map<QString, std::shared_ptr<AppLogicModule>> m_ufbs;

		std::map<QString, BuildActuatorType> m_actuators; // Key is ActuatorHeader::actuatorTypeId()

		SignalSet& m_signalSet;
		LmDescriptionSet& m_lmDescriptions;
		IssueLogger& m_log;
	};


	// Container to store schemas parse result, in multithreading
	// After parsing all schemas call setToAppData to set result to modules
	//
	class ReadyParseDataContainer
	{
	public:
		void add(QString equipmentId, std::shared_ptr<BushContainer> bushContainer, std::shared_ptr<VFrame30::LogicSchema> schema);

		void setToAppData(AppLogicData* appData);

	private:
		struct AppData
		{
			QString equipmentId;
			std::shared_ptr<BushContainer> bushContainer;
			std::shared_ptr<VFrame30::LogicSchema> schema;

			bool operator<(const AppData& other) const;
		};

		mutable QMutex m_mutex;
		std::set<AppData> m_appData;
	};

	// ------------------------------------------------------------------------
	//
	//		ApplicationLogicBuilder
	//
	// ------------------------------------------------------------------------
	class Parser : public QObject
	{
		Q_OBJECT

	public:
		Parser() = delete;
		explicit Parser(Builder::Context* context, QStringList buildSchemaTags);

	public:
		bool parse();

	private:
		bool parseUfbs();             // UFB Schemas
		bool parseActuatorLogic();    // Actuator Schemas
		bool parseApplicationLogic(); // Application Logic Schemas

	protected:
		bool loadUfbFiles(std::vector<std::shared_ptr<VFrame30::UfbSchema>>& out);
		bool loadAppLogicFiles(std::vector<std::shared_ptr<VFrame30::LogicSchema>>& out);

		std::optional<std::vector<BuildActuatorType>> loadActuators();

		template<typename Type>
		bool loadFiles(std::vector<std::pair<DbFileInfo, std::shared_ptr<Type>>>& out, int parentFileId, QString filter);

		template<typename SchemaType>
		bool loadSchemaFiles(std::vector<std::shared_ptr<SchemaType>>& out, int parentFileId, QString filter);

		template<typename SchemaType>
		bool checkSameLabelsAndGuids(const std::vector<std::shared_ptr<SchemaType>>& schemas) const;

		bool checkSameInputsAndOutputs(const std::vector<std::shared_ptr<VFrame30::UfbSchema>>& schemas) const;
		bool checkParamsReferencesFormat(const std::vector<std::shared_ptr<VFrame30::UfbSchema>>& schemas) const;

		bool checkEquipmentIds(VFrame30::LogicSchema* logicSchema);

		bool checkLmDescription(VFrame30::LogicSchema* logicSchema);

		bool checkAfbItemsVersion(VFrame30::Schema* schema);
		bool checkBusItemsVersion(VFrame30::Schema* schema, const AppSignalLib::BusSet& busSet);
		bool checkUfbItemsVersion(VFrame30::Schema* schema, const std::vector<std::shared_ptr<VFrame30::UfbSchema>>& ufbs);
		bool checkActuatorItemsVersion(const VFrame30::Schema& schema);
		bool checkSupportedSchemaItems(const VFrame30::Schema& schema);

		bool checkForUniqueLoopbackId(VFrame30::Schema* schema);
		bool checkForUniqueLoopbackId(std::shared_ptr<AppLogicModule> module);

		bool checkForResolvedReferences(std::shared_ptr<AppLogicModule> module);

		bool checkActuatorLogicInputsOutputs();

		bool parsUfbSchema(std::shared_ptr<VFrame30::UfbSchema> ufbSchema);
		bool parseUfbLayer(std::shared_ptr<VFrame30::UfbSchema> ufbSchema, std::shared_ptr<VFrame30::SchemaLayer> layer);

		bool parseActuatorSchema(std::shared_ptr<VFrame30::ActuatorSchema> actuatorSchema, const BuildActuatorType& bat);
		bool parseActuatorLayer(std::shared_ptr<VFrame30::ActuatorSchema> actuatorSchema,
								std::shared_ptr<VFrame30::SchemaLayer> layer,
								const BuildActuatorType& bat);

		bool parseAppLogicSchema(std::shared_ptr<VFrame30::LogicSchema> logicSchema, ReadyParseDataContainer* readyParseDataContainer);
		bool parseAppLogicLayer(std::shared_ptr<VFrame30::LogicSchema> logicSchema,
								std::shared_ptr<VFrame30::SchemaLayer> layer,
								ReadyParseDataContainer* readyParseDataContainer);

		bool multichannelProcessing(std::shared_ptr<VFrame30::LogicSchema> schema,
									std::shared_ptr<VFrame30::SchemaLayer> layer,
									QString equipmentId);

		bool filterSingleChannelBranchesInMultiSchema(std::shared_ptr<VFrame30::LogicSchema> schema,
													  QString equipmnetId,
													  BushContainer* bushContainer);

		bool findBushes(std::shared_ptr<VFrame30::Schema> schema,
						std::shared_ptr<VFrame30::SchemaLayer> layer,
						BushContainer* bushContainer) const;

		bool setBranchConnectionToPin(std::shared_ptr<VFrame30::Schema> schema,
									  std::shared_ptr<VFrame30::SchemaLayer> layer,
									  BushContainer* bushContainer) const;

		bool setPinConnections(std::shared_ptr<VFrame30::Schema> schema,
							   std::shared_ptr<VFrame30::SchemaLayer> layer,
							   BushContainer* bushContainer);

	private:
		DbController* db();
		int changesetId() const;

		const AppLogicData* applicationData() const;
		AppLogicData* applicationData();

	private:
		Builder::Context* m_context = nullptr;
		DbController* m_db = nullptr;
		IssueLogger& m_log;
		int m_changesetId = 0;

		QStringList m_buildSchemaTags;              // If empty, then build all schemas, otherwise only schemas with these tags

		std::vector<std::shared_ptr<VFrame30::UfbSchema>> m_ufbs;
		std::vector<BuildActuatorType> m_actuators; // Key is ActuatorHeader::actuatorTypeId()

		std::shared_ptr<AppLogicData> m_applicationData;
		LmDescriptionSet* m_lmDescriptions = nullptr;
		Hardware::EquipmentSet* m_equipmentSet = nullptr;
		SignalSet* m_signalSet = nullptr;
		AppSignalLib::BusSet* m_busSet = nullptr;
		Hardware::OptoModuleStorage* m_opticModuleStorage = nullptr;
	};

} // namespace Builder
