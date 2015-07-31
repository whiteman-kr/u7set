#pragma once

#include "SchemeLayer.h"
#include "Afb.h"
#include "../include/TypesAndEnums.h"

namespace VFrame30
{
	class CDrawParam;
	class VideoFrameWidgetAgent;
	class SchemeLayer;
	class SchemeItem;

	
	class VFRAME30LIBSHARED_EXPORT Scheme :
		public QObject,
		public Proto::ObjectSerialization<Scheme>,
		public DebugInstCounter<Scheme>
	{
		Q_OBJECT

		Q_PROPERTY(bool ExcludeFromBuild READ excludeFromBuild WRITE setExcludeFromBuild)
	
	protected:
		Scheme(void);
	
	public:
		virtual ~Scheme(void);

		void Init(void);

		// Serialization
		//
		friend Proto::ObjectSerialization<Scheme>;

	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

	private:
		// Использовать функцию только при сериализации, т.к. при создании объекта он полностью не инициализируется,
		// и должен прочитаться
		//
		static Scheme* CreateObject(const Proto::Envelope& message);

		// Methods
		//
	public:
		virtual void Draw(CDrawParam* drawParam, const QRectF& clipRect) const;
		void Print();

		virtual void MouseClick(const QPointF& docPoint, VideoFrameWidgetAgent* pVideoFrameWidgetAgent) const;
		void RunClickScript(const std::shared_ptr<SchemeItem>& schemeItem/*, VideoFrameWidgetAgent* pVideoFrameWidgetAgent*/) const;

		// Получить размер документа в точка
		//
		int GetDocumentWidth(double DpiX, double zoom) const;
		int GetDocumentHeight(double DpiY, double zoom) const;

		int GetLayerCount() const;
		
		// Заполнить connectionMap который хранится в каждом слое.
		// std::map<SchemePoint, int> connectionMap		Ключ - координата пина, значение - количество подключений к пину
		//
		void BuildFblConnectionMap() const;

		// Properties and Datas
		//
	public:
		QUuid guid() const;
		void setGuid(const QUuid& guid);

		QString strID() const;
		void setStrID(const QString& strID);

		QString caption() const;
		void setCaption(const QString& caption);

		double docWidth() const;
		void setDocWidth(double width);

		double docHeight() const;
		void setDocHeight(double height);

		SchemeUnit unit() const;
		void setUnit(SchemeUnit value);

		double gridSize() const;
		void setGridSize(double value);

		int pinGridStep() const;
		void setPinGridStep(int value);

		bool excludeFromBuild() const;
		void setExcludeFromBuild(bool value);

		const Afb::AfbElementCollection& afbCollection() const;
		void setAfbCollection(const std::vector<std::shared_ptr<Afb::AfbElement>>& elements);
		
	public:
		std::vector<std::shared_ptr<SchemeLayer>> Layers;

	private:
		QUuid m_guid;
		QString m_strID;
		QString m_caption;

		double m_width;				// pixels or inches, depends on m_unit
		double m_height;			// pixels or inches, depends on m_unit

		SchemeUnit m_unit;			// Единицы измерения, в которых хранятся координаты (может быть только дюймы или точки)

		double m_gridSize = 1.0;	// Grid size for this scheme, depends on SchemeUnit
		int m_pinGridStep = 2;		// Grid multiplier to determine vertical distance between pins

		bool m_excludeFromBuild = false;	// Exclude Scheme from build or any other processing

		Afb::AfbElementCollection m_afbCollection;
	};


#ifdef VFRAME30LIB_LIBRARY
	extern Factory<VFrame30::Scheme> SchemeFactory;
#endif
}


