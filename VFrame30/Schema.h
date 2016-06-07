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
		static Schema* CreateObject(const Proto::Envelope& message);

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

		// Properties and Datas
		//
	public:
		QUuid guid() const;
		void setGuid(const QUuid& guid);

		QString schemaID() const;
		void setSchemaID(const QString& id);

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
		bool isMonitorSchema() const;
		bool isDiagSchema() const;
		
	public:
		std::vector<std::shared_ptr<SchemaLayer>> Layers;

	private:
		QUuid m_guid;
		QString m_schemaID;
		QString m_caption;

		double m_width;				// pixels or inches, depends on m_unit
		double m_height;			// pixels or inches, depends on m_unit

		SchemaUnit m_unit;			// ������� ���������, � ������� �������� ���������� (����� ���� ������ ����� ��� �����)

		double m_gridSize = 1.0;	// Grid size for this schema, depends on SchemaUnit
		int m_pinGridStep = 2;		// Grid multiplier to determine vertical distance between pins

		bool m_excludeFromBuild = false;	// Exclude Schema from build or any other processing
	};


#ifdef VFRAME30LIB_LIBRARY
	extern Factory<VFrame30::Schema> SchemaFactory;
#endif
}


