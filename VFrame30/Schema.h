#pragma once

#include "SchemaLayer.h"
#include "Afb.h"
#include "../lib/PropertyObject.h"
#include "../lib/TypesAndEnums.h"

namespace VFrame30
{
	class CDrawParam;
	class VideoFrameWidgetAgent;
	class SchemaLayer;
	class SchemaItem;
	class LogicSchema;
	class UfbSchema;
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

		double gridSize() const;
		void setGridSize(double value);

		int pinGridStep() const;
		void setPinGridStep(int value);

		bool excludeFromBuild() const;
		void setExcludeFromBuild(bool value);

		bool isLogicSchema() const;
		bool isUfbSchema() const;
		bool isMonitorSchema() const;
		bool isDiagSchema() const;

		LogicSchema* toLogicSchema();
		const LogicSchema* toLogicSchema() const;

		int changeset() const;
		void setChangeset(int value);
		
	public:
		std::vector<std::shared_ptr<SchemaLayer>> Layers;

	private:
		QUuid m_guid;
		QString m_schemaID;
		QString m_caption;

		double m_width = 0.0;					// pixels or inches, depends on m_unit
		double m_height = 0.0;					// pixels or inches, depends on m_unit

		SchemaUnit m_unit = SchemaUnit::Inch;	// ������� ���������, � ������� �������� ���������� (����� ���� ������ ����� ��� �����)

		double m_gridSize = 1.0;				// Grid size for this schema, depends on SchemaUnit
		int m_pinGridStep = 2;					// Grid multiplier to determine vertical distance between pins

		bool m_excludeFromBuild = false;		// Exclude Schema from build or any other processing

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
//		SchemaDetails();
//		SchemaDetails(const SchemaDetails& src) = default;
//		SchemaDetails(SchemaDetails&& r) = default;

//		SchemaDetails& operator= (const SchemaDetails& src) = default;

		static QString getDetailsString(const Schema* schema);
		bool parseDetails(const QString& details);

		bool searchForString(const QString& searchText) const;

	public:
		// WARNING !!!! If add any new members, add them to the move construnctor;
		//
		int m_version = 0;
		QString m_schemaId;
		QString m_caption;
		QString m_equipmentId;			// Valid for LogicSchemas
		std::set<QString> m_signals;
		std::set<QString> m_labels;
		std::set<QString> m_connections;
		std::set<QUuid> m_guids;
		// WARNING !!!! If add any new members, add them to the move construnctor;
		//
	};


#ifdef VFRAME30LIB_LIBRARY
	extern Factory<VFrame30::Schema> SchemaFactory;
#endif
}


