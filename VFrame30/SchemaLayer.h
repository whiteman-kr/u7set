#pragma once

#include "SchemaItem.h"

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT SchemaLayer :
		public QObject,
		public Proto::ObjectSerialization<SchemaLayer>,
		public DebugInstCounter<SchemaLayer>
	{
		Q_OBJECT

	public:
		SchemaLayer(void);
		SchemaLayer(const QString& name, bool compile);
		virtual ~SchemaLayer(void);

	private:
		void Init(const QString& name, bool compile);

		// Serialization
		//
		friend Proto::ObjectSerialization<SchemaLayer>;

	private:
		// Use this func only while serialization, cause during obejcet creation it isnotfully initialized
		static std::shared_ptr<SchemaLayer> CreateObject(const Proto::Envelope& message);

	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Methods
		//
	public:
		std::shared_ptr<SchemaItem> getItemById(const QUuid& id) const;

		// If in the connectioMap there is a pinPos, then increment value, if not then add new record with value 1
		void ConnectionMapPosInc(SchemaPoint pinPos);
		int GetPinPosConnectinCount(SchemaPoint pinPos, SchemaUnit unit) const;

		template<typename SchemaItemType>
		std::shared_ptr<SchemaItemType> getItemUnderPointByType(QPointF point) const;		// This will work only inside VFrame30 :(

		std::shared_ptr<SchemaItem> getItemUnderPoint(QPointF point, QString className = "") const;
		std::list<std::shared_ptr<SchemaItem>> getItemListUnderPoint(QPointF point, QString className = "") const;
		std::list<std::shared_ptr<SchemaItem>> getItemListInRectangle(const QRectF& rect) const;

		std::shared_ptr<SchemaItem> findPinUnderPoint(QPointF point, double gridSize, int pinGridStep) const;

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
		std::list<std::shared_ptr<SchemaItem>> Items;

		// Key is pin position, value is count of pins on the point
		std::map<SchemaPoint, int> connectionMap;

	private:
		QUuid m_guid;
		QString m_name;
		bool m_compile = false;
		bool m_show = true;
		bool m_print = true;
	};

#ifdef VFRAME30LIB_LIBRARY
	extern Factory<VFrame30::SchemaLayer> VideoLayerFactory;
#endif

}
