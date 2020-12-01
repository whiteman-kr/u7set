#pragma once

#include "VFrame30Lib_global.h"
#include "SchemaLayer.h"
#include "../lib/PropertyObject.h"
#include "../lib/TypesAndEnums.h"
#include <QColor>


namespace Afb
{
	class AfbElement;
}


namespace VFrame30
{
	class SchemaLayer;
	class CDrawParam;
	class VideoFrameWidgetAgent;
	class SchemaItem;
	class LogicSchema;
	class UfbSchema;
	class Bus;
	class SchemaDetails;

	class Schema;

	// Proxy class for using in scripts
	//
	/*! \class ScriptSchema
		\ingroup view
		\brief Represents a class that contains schema. Can be get from global <b>view.Schema</b>.
	*/
	class ScriptSchema : public QObject
	{
		Q_OBJECT

		/// \brief Schema unique identifier (SchemaID).
		Q_PROPERTY(QString SchemaID READ schemaId)

		/// \brief Schema caption.
		Q_PROPERTY(QString Caption READ caption)

	public:
		explicit ScriptSchema(std::shared_ptr<Schema> schema);
		~ScriptSchema();

	public slots:
		/// \brief Return true if is ApplicationLogic schema.
		bool isLogicSchema() const;

		/// \brief Return true if is UserFunctionalBlock schema.
		bool isUfbSchema() const;

		/// \brief Return true if is Monitor schema.
		bool isMonitorSchema() const;

		/// \brief Return true if is TuningClient schema.
		bool isTuningSchema() const;

		/// \brief Return true if is Diag schema.
		bool isDiagSchema() const;

	private:
		QString schemaId() const;
		QString caption() const;

	private:
		std::shared_ptr<Schema> m_schema;
	};


	class VFRAME30LIBSHARED_EXPORT Schema :
		public PropertyObject,
		public Proto::ObjectSerialization<Schema>,
		public DebugInstCounter<Schema>
	{
		Q_OBJECT

	protected:
		Schema(void);

	public:
		virtual ~Schema(void);

		void Init(void);

		// Serialization
		//
		friend Proto::ObjectSerialization<Schema>;

	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

	private:
		static std::shared_ptr<Schema> CreateObject(const Proto::Envelope& message);

		// Methods
		//
	public:
		virtual void Draw(CDrawParam* drawParam, const QRectF& clipRect) const;
		void Print();

		virtual void MouseClick(const QPointF& docPoint, VideoFrameWidgetAgent* pVideoFrameWidgetAgent) const;
		void RunClickScript(const std::shared_ptr<SchemaItem>& schemaItem) const;

		int GetDocumentWidth(double DpiX, double zoom) const;
		int GetDocumentHeight(double DpiY, double zoom) const;

		int GetLayerCount() const;

		void BuildFblConnectionMap() const;

		bool updateAllSchemaItemFbs(const std::vector<std::shared_ptr<Afb::AfbElement>>& afbs, int* updatedItemCount, QString* errorMessage);
		bool updateAllSchemaItemUfb(const std::vector<std::shared_ptr<UfbSchema>>& ufbs, int* updatedItemCount, QString* errorMessage);
		bool updateAllSchemaItemBusses(const std::vector<Bus>& busses, int* updatedItemCount, QString* errorMessage);

		virtual QStringList getSignalList() const;
		virtual QStringList getLabels() const;
		virtual std::vector<QUuid> getGuids() const;

		virtual QString details(const QString& path) const;			// form details JSON object (signal list)
		static SchemaDetails parseDetails(const QString& detailsString);// parse details section (from DB), result is signal list

		std::shared_ptr<SchemaItem> getItemById(const QUuid& id) const;

		template<typename SchemaItemType>
		bool hasSchemaItemType() const;

		// Properties and Datas
		//
	public:
		[[nodiscard]] QUuid guid() const noexcept;
		void setGuid(const QUuid& guid);

		[[nodiscard]] QString schemaId() const noexcept;
		void setSchemaId(const QString& id);

		[[nodiscard]] QString caption() const noexcept;
		void setCaption(const QString& caption);

		[[nodiscard]] QString tagsAsString() const noexcept;
		[[nodiscard]] QStringList tagsAsList() const noexcept;

		void setTags(QString tags);
		void setTagsList(const QStringList& tags);

		[[nodiscard]] bool joinHorzPriority() const;
		void setJoinHorzPriority(bool value);

		[[nodiscard]] QString joinLeftSchemaId() const;
		void setJoinLeftSchemaId(const QString& value);

		[[nodiscard]] QString joinTopSchemaId() const;
		void setJoinTopSchemaId(const QString& value);

		[[nodiscard]] QString joinRightSchemaId() const;
		void setJoinRightSchemaId(const QString& value);

		[[nodiscard]] QString joinBottomSchemaId() const;
		void setJoinBottomSchemaId(const QString& value);

		[[nodiscard]] double docWidth() const;
		void setDocWidth(double width);

		[[nodiscard]] double docWidthRegional() const;
		void setDocWidthRegional(double width);

		[[nodiscard]] double docHeight() const;
		void setDocHeight(double height);

		[[nodiscard]] double docHeightRegional() const;
		void setDocHeightRegional(double height);

		[[nodiscard]] SchemaUnit unit() const noexcept;
		void setUnit(SchemaUnit value) noexcept;

		[[nodiscard]] int activeLayerIndex() const;
		[[nodiscard]] QUuid activeLayerGuid() const;
		[[nodiscard]] std::shared_ptr<VFrame30::SchemaLayer> activeLayer() const;
		void setActiveLayer(std::shared_ptr<VFrame30::SchemaLayer> layer);

		[[nodiscard]] double gridSize() const noexcept;
		void setGridSize(double value);

		[[nodiscard]] int pinGridStep() const noexcept;
		void setPinGridStep(int value);

		[[nodiscard]] bool excludeFromBuild() const noexcept;
		void setExcludeFromBuild(bool value);

		[[nodiscard]] QColor backgroundColor() const noexcept;
		void setBackgroundColor(const QColor& value);

		[[nodiscard]] bool isLogicSchema() const noexcept;
		[[nodiscard]] bool isUfbSchema() const noexcept;
		[[nodiscard]] bool isMonitorSchema() const noexcept;
		[[nodiscard]] bool isTuningSchema() const noexcept;
		[[nodiscard]] bool isDiagSchema() const noexcept;

		[[nodiscard]] LogicSchema* toLogicSchema() noexcept;
		[[nodiscard]] const LogicSchema* toLogicSchema() const noexcept;

		[[nodiscard]] UfbSchema* toUfbSchema() noexcept;
		[[nodiscard]] const UfbSchema* toUfbSchema() const noexcept;

		[[nodiscard]] int changeset() const noexcept;
		void setChangeset(int value);

	public:
		std::vector<std::shared_ptr<SchemaLayer>> Layers;

	private:
		QUuid m_guid;
		QString m_schemaID;
		QString m_caption;

		QStringList m_tags;

		bool m_joinHorzPriority = false;
		QString m_joinLeftSchemaId;
		QString m_joinTopSchemaId;
		QString m_joinRightSchemaId;
		QString m_joinBottomSchemaId;

		double m_width = 0.0;					// pixels or inches, depends on m_unit
		double m_height = 0.0;					// pixels or inches, depends on m_unit

		SchemaUnit m_unit = SchemaUnit::Inch;

		int m_activeLayer = 0;
		double m_gridSize = 1.0;				// Grid size for this schema, depends on SchemaUnit
		int m_pinGridStep = 2;					// Grid multiplier to determine vertical distance between pins

		bool m_excludeFromBuild = false;		// Exclude Schema from build or any other processing

		QColor m_backgroundColor = {qRgb(0xFF, 0xFF, 0xFF)};

		int m_changeset = -1;					// Changeset, this field is not stored in file
	};


	// SchemaDaetails is a class to parse to/from JSON doc
	// Format:
	//		Version : 1
	//		SchemaID : "SCHMEAID"
	//		Signals: ["id", "id", "id", ...]
	//		Labels: ["Label1", "Label2", "Label3", ...]
	//		ItemGuids: ["guid1", "guid2", "guid3", ...]
	//

	class VFRAME30LIBSHARED_EXPORT SchemaDetails
	{
	public:
		SchemaDetails() noexcept;
		SchemaDetails(const SchemaDetails&) = default;
		SchemaDetails(SchemaDetails&&) = default;
		SchemaDetails(const QString& details) noexcept;

		SchemaDetails& operator=(const SchemaDetails&) = default;
		SchemaDetails& operator=(SchemaDetails&&) noexcept = default;

		bool operator<(const SchemaDetails& b) const noexcept;

	public:
		static QString getDetailsString(const Schema* schema, const QString& path);
		bool parseDetails(const QString& details);

		bool saveData(Proto::SchemaDetails* message) const;
		bool loadData(const Proto::SchemaDetails& message);

		bool searchForString(const QString& searchText) const;

		bool hasTag(const QString& tag) const;
		bool hasTag(const QStringList& tags) const;
		const std::set<QString>& tags() const;

		bool hasEquipmentId(const QString& equipmentId) const;
		bool hasSignal(const QString& signalId) const;

	public:
		int m_version = 0;
		QString m_schemaId;
		QString m_caption;
		bool m_excludedFromBuild = false;
		QString m_equipmentId;			// Valid for LogicSchemas
		QString m_lmDescriptionFile;	// Valid for LogicSchemas and UfbSchemas
		QString m_path;					// Path in terms of SchemaEditor ("/ABC/DEF", "/")
		std::set<QString> m_signals;
		std::set<QString> m_labels;
		std::set<QString> m_connections;
		std::set<QString> m_loopbacks;
		std::set<QString> m_tags;		// All tags are kept in lowercase
		std::set<QUuid> m_guids;
	};

	class VFRAME30LIBSHARED_EXPORT SchemaDetailsSet : public Proto::ObjectSerialization<SchemaDetailsSet>
	{
	public:
		SchemaDetailsSet();
		SchemaDetailsSet(const SchemaDetailsSet&) = default;
		SchemaDetailsSet(SchemaDetailsSet&&) = default;
		SchemaDetailsSet& operator=(const SchemaDetailsSet&) = default;
		SchemaDetailsSet& operator=(SchemaDetailsSet&&) noexcept = default;

		// Serializatin implementation of Proto::ObjectSerialization<>
		//
		friend Proto::ObjectSerialization<SchemaDetailsSet>;

	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

	private:
		static std::shared_ptr<SchemaDetailsSet> CreateObject(const Proto::Envelope& message);

		// Properties and functions
		//
	public:
		void clear();

		void add(const QString& details);
		void add(const SchemaDetails& details);
		void add(SchemaDetails&& details);
		void add(std::shared_ptr<SchemaDetails> details);

		std::vector<SchemaDetails> schemasDetails() const;
		std::vector<SchemaDetails> schemasDetails(QString equipmentId) const;

		std::shared_ptr<SchemaDetails> schemaDetails(QString schemaId) const;
		std::shared_ptr<SchemaDetails> schemaDetails(int index) const;

		QStringList schemasByAppSignalId(const QString& appSignalId) const;

		int schemaCount() const;
		QString schemaCaptionById(const QString& schemaId) const;
		QString schemaCaptionByIndex(int schemaIndex) const;
		QString schemaIdByIndex(int schemaIndex) const;

	private:
		std::map<QString, std::shared_ptr<SchemaDetails>> m_details;	// Key is schemaId
	};


#ifdef VFRAME30LIB_LIBRARY
	extern Factory<VFrame30::Schema> SchemaFactory;
#endif
}


