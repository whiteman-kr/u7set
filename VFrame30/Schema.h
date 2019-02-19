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
		// ������������ ������� ������ ��� ������������, �.�. ��� �������� ������� �� ��������� �� ����������������,
		// � ������ �����������
		//
		static std::shared_ptr<Schema> CreateObject(const Proto::Envelope& message);

		// Methods
		//
	public:
		virtual void Draw(CDrawParam* drawParam, const QRectF& clipRect) const;
		void Print();

		virtual void MouseClick(const QPointF& docPoint, VideoFrameWidgetAgent* pVideoFrameWidgetAgent) const;
		void RunClickScript(const std::shared_ptr<SchemaItem>& schemaItem/*, VideoFrameWidgetAgent* pVideoFrameWidgetAgent*/) const;

		// �������� ������ ��������� � �����
		//
		int GetDocumentWidth(double DpiX, double zoom) const;
		int GetDocumentHeight(double DpiY, double zoom) const;

		int GetLayerCount() const;
		
		// ��������� connectionMap ������� �������� � ������ ����.
		// std::map<SchemaPoint, int> connectionMap		���� - ���������� ����, �������� - ���������� ����������� � ����
		//
		void BuildFblConnectionMap() const;

		bool updateAllSchemaItemFbs(const std::vector<std::shared_ptr<Afb::AfbElement>>& afbs, int* updatedItemCount, QString* errorMessage);
		bool updateAllSchemaItemUfb(const std::vector<std::shared_ptr<UfbSchema>>& ufbs, int* updatedItemCount, QString* errorMessage);
		bool updateAllSchemaItemBusses(const std::vector<Bus>& busses, int* updatedItemCount, QString* errorMessage);

		virtual QStringList getSignalList() const;
		virtual QStringList getLabels() const;
		virtual std::vector<QUuid> getGuids() const;

		virtual QString details() const;				// form details JSON object (signal list)
		static SchemaDetails parseDetails(const QString& details);	// parse details section (from DB), result is signal list

		std::shared_ptr<SchemaItem> getItemById(const QUuid& id) const;

		// Properties and Datas
		//
	public:
		QUuid guid() const;
		void setGuid(const QUuid& guid);

		QString schemaId() const;
		void setSchemaId(const QString& id);

		QString caption() const;
		void setCaption(const QString& caption);

		QString tagsAsString() const;
		QStringList tagsAsList() const;

		void setTags(QString tags);
		void setTagsList(const QStringList& tags);

		double docWidth() const;
		void setDocWidth(double width);
		double docWidthRegional() const;
		void setDocWidthRegional(double width);

		double docHeight() const;
		void setDocHeight(double height);
		double docHeightRegional() const;
		void setDocHeightRegional(double height);

		SchemaUnit unit() const;
		void setUnit(SchemaUnit value);

		int activeLayerIndex() const;
		QUuid activeLayerGuid() const;
		std::shared_ptr<VFrame30::SchemaLayer> activeLayer() const;
		void setActiveLayer(std::shared_ptr<VFrame30::SchemaLayer> layer);

		double gridSize() const;
		void setGridSize(double value);

		int pinGridStep() const;
		void setPinGridStep(int value);

		bool excludeFromBuild() const;
		void setExcludeFromBuild(bool value);

		QColor backgroundColor() const;
		void setBackgroundColor(const QColor& value);

		bool isLogicSchema() const;
		bool isUfbSchema() const;
		bool isMonitorSchema() const;
		bool isTuningSchema() const;
		bool isDiagSchema() const;

		LogicSchema* toLogicSchema();
		const LogicSchema* toLogicSchema() const;

		UfbSchema* toUfbSchema();
		const UfbSchema* toUfbSchema() const;

		int changeset() const;
		void setChangeset(int value);

	public:
		std::vector<std::shared_ptr<SchemaLayer>> Layers;

	private:
		QUuid m_guid;
		QString m_schemaID;
		QString m_caption;

		QStringList m_tags;

		double m_width = 0.0;					// pixels or inches, depends on m_unit
		double m_height = 0.0;					// pixels or inches, depends on m_unit

		SchemaUnit m_unit = SchemaUnit::Inch;	// ������� ���������, � ������� �������� ���������� (����� ���� ������ ����� ��� �����)

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
		SchemaDetails& operator=(SchemaDetails&&) = default;

		bool operator<(const SchemaDetails& b) const noexcept;

	public:
		static QString getDetailsString(const Schema* schema);
		bool parseDetails(const QString& details);

		bool saveData(Proto::SchemaDetails* message) const;
		bool loadData(const Proto::SchemaDetails& message);

		bool searchForString(const QString& searchText) const;

		bool hasTag(const QString& tag) const;
		bool hasTag(const QStringList& tags) const;
		const std::set<QString>& tags() const;

		bool hasEquipmentId(const QString& equipmentId) const;

	public:
		int m_version = 0;
		QString m_schemaId;
		QString m_caption;
		bool m_excludedFromBuild = false;
		QString m_equipmentId;			// Valid for LogicSchemas
		QString m_lmDescriptionFile;	// Valid for LogicSchemas and UfbSchemas
		std::set<QString> m_signals;
		std::set<QString> m_labels;
		std::set<QString> m_connections;
		std::set<QString> m_tags;		// All tags are kept in lowercase
		std::set<QUuid> m_guids;
	};

	class VFRAME30LIBSHARED_EXPORT SchemaDetailsSet : public Proto::ObjectSerialization<SchemaDetailsSet>
	{
	public:
		SchemaDetailsSet();

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
		void add(std::shared_ptr<SchemaDetails> details);

		std::vector<SchemaDetails> schemasDetails() const;
		std::vector<SchemaDetails> schemasDetails(QString equipmentId) const;

		std::shared_ptr<SchemaDetails> schemaDetails(QString schemaId) const;

	private:
		std::map<QString, std::shared_ptr<SchemaDetails>> m_details;		// Key is schemaId
	};


#ifdef VFRAME30LIB_LIBRARY
	extern Factory<VFrame30::Schema> SchemaFactory;
#endif
}


