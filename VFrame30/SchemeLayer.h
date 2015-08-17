#pragma once

#include "SchemeItem.h"

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
		// Use this func only while serialization, cause during obejcet creation it isnotfully initialized
		static SchemeLayer* CreateObject(const Proto::Envelope& message);

	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Methods
		//
	public:

		std::shared_ptr<SchemeItem> getItemById(const QUuid& id) const;

		// If in the connectioMap there is a pinPos, then increment value, if not then add new record with value 1
		void ConnectionMapPosInc(SchemePoint pinPos);
		int GetPinPosConnectinCount(SchemePoint pinPos, SchemeUnit unit) const;

		std::shared_ptr<SchemeItem> getItemUnderPoint(QPointF point, QString className = "") const;
		std::list<std::shared_ptr<SchemeItem>> getItemListInRectangle(const QRectF& rect) const;

		std::shared_ptr<SchemeItem> findPinUnderPoint(QPointF point, double gridSize, int pinGridStep) const;

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

		// Layer items
		std::list<std::shared_ptr<SchemeItem>> Items;

		// Key is pin position, value is count of pins on the point
		std::map<SchemePoint, int> connectionMap;

	private:
		QUuid m_guid;
		QString m_name;
		bool m_compile = false;
		bool m_show = true;
		bool m_print = true;
	};

#ifdef VFRAME30LIB_LIBRARY
	extern Factory<VFrame30::SchemeLayer> VideoLayerFactory;
#endif

}
