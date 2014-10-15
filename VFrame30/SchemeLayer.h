#pragma once

#include "VideoItem.h"

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT SchemeLayer :
		public QObject,
		public Proto::ObjectSerialization<SchemeLayer>,
		public DebugInstCounter<SchemeLayer>
	{
		Q_OBJECT

	public:
		SchemeLayer(void);
		SchemeLayer(const QString& name, bool compile);
		virtual ~SchemeLayer(void);

	private:
		void Init(const QString& name, bool compile);

		// Serialization
		//
		friend Proto::ObjectSerialization<SchemeLayer>;

	private:
		// Использовать функцию только при сериализации, т.к. при создании объекта он полностью не инициализируется,
		// и должне прочитаться
		static SchemeLayer* CreateObject(const Proto::Envelope& message);

	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Methods
		//
	public:

		// Если в connectionMap есть pinPos, то инкрементировать значение, иначе добавить новую запись со сзначением 1
		//
		void ConnectionMapPosInc(VideoItemPoint pinPos);
		int GetPinPosConnectinCount(VideoItemPoint pinPos) const;

		std::shared_ptr<VideoItem> getItemUnderPoint(QPointF point) const;
		std::list<std::shared_ptr<VideoItem>> getItemListInRectangle(const QRectF& rect) const;

		// Properties
		//
	public:
		QUuid guid() const;
		void setGuid(const QUuid& guid);

		QString name() const;
		void setName(const QString& value);

		bool compile() const;
		void setCompile(bool value);

		bool show() const;
		void setShow(bool value);

		bool print() const;
		void setPrint(bool value);

		// Data
		//
	public:
		// Элементы слоя 
		//
		std::list<std::shared_ptr<VideoItem>> Items;

		// Таблица координат всех пинов, значением является количество координат лежащих на точке
		//
		std::map<VideoItemPoint, int> connectionMap;		// Ключ - координата пина, значение - количество пинов в координте

	private:
		QUuid m_guid;
		QString m_name;
		bool m_compile;
		bool m_show;
		bool m_print;
	};

#ifdef VFRAME30LIB_LIBRARY
	extern Factory<VFrame30::SchemeLayer> VideoLayerFactory;
#endif

}
