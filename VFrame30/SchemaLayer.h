#pragma once

#include "../CommonLib/Factory.h"
#include "../CommonLib/DebugInstCounter.h"
#include "../Proto/ProtoSerialization.h"
#include "SchemaItem.h"
#include "SchemaPoint.h"

namespace VFrame30
{
	class SchemaLayer;
	using SchemaLayerPtr = std::shared_ptr<VFrame30::SchemaLayer>;
	class SchemaLayer;
	class Schema;

	extern ::Factory<VFrame30::SchemaLayer> VideoLayerFactory;

	// Proxy class for using in scripts
	//
	/*! \class ScriptSchemaLayer
		\ingroup view
		\brief Represents a class that contains schema layer. Can be get from schema by index or by name.
	*/
	class ScriptSchemaLayer : public QObject
	{
		Q_OBJECT

		/// \brief SchemaLayer caption.
		Q_PROPERTY(QString caption READ caption)

		/// \brief SchemaLayer visibility.
		Q_PROPERTY(bool visible READ visible WRITE setVisible)

	public:
		explicit ScriptSchemaLayer(SchemaLayerPtr schemaLayer);
		virtual ~ScriptSchemaLayer() = default;

	public slots:

	private:
		QString caption() const;

		bool visible() const;
		void setVisible(bool value);

	private:
		SchemaLayerPtr m_schemaLayer;
	};


	class SchemaLayer :
		public QObject,
		public Proto::ObjectSerialization<SchemaLayer>,
		public std::enable_shared_from_this<SchemaLayer>,
		public DebugInstCounter<SchemaLayer>
	{
		Q_OBJECT

	public:
		SchemaLayer(void);
		SchemaLayer(Schema* parentSchema, const QString& name, bool compile);
		virtual ~SchemaLayer(void);

	private:
		void Init(const QString& name, bool compile);

		// Serialization
		//
		friend Proto::ObjectSerialization<SchemaLayer>;

	private:
		// Use this func only while serialization, cause during object creation it is not fully initialized
		//
		static std::shared_ptr<SchemaLayer> CreateObject(const Proto::Envelope& message);

	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Methods
		//
	public:
		std::shared_ptr<SchemaItem> getItemById(const QUuid id) const;

		// If in the connectionMap there is a pinPos, then increment value, if not then add new record with value 1
		//
		void ConnectionMapPosInc(SchemaPoint pinPos);
		int GetPinPosConnectinCount(SchemaPoint pinPos) const;

		template<typename SchemaItemType>
		std::shared_ptr<SchemaItemType> getItemUnderPointByType(QPointF point) const;		// This will work only inside VFrame30 :(

		std::shared_ptr<SchemaItem> getItemUnderPoint(QPointF point, const QString& className = QString{}) const;
		std::list<std::shared_ptr<SchemaItem>> getItemListUnderPoint(QPointF point, const QString& className = QString{}) const;
		std::list<std::shared_ptr<SchemaItem>> getItemListInRectangle(const QRectF& rect) const;

		std::shared_ptr<SchemaItem> findPinUnderPoint(QPointF point, double gridSize, int pinGridStep) const;

		// Items access and manipulation
		//
		void clearItems();
		bool removeItem(const SchemaItemPtr& item);

		void pushBackItem(SchemaItemPtr item);

		template<typename It>
		void pushBackItems(It begin, It end)
		{
			while (begin != end)
			{
				pushBackItem(*begin);
				++begin;
			}
		}

		template<typename It>
		void setItems(It begin, It end)
		{
			m_items.clear();
			pushBackItems(begin, end);
		}

		// DO NOT modify it to the value semantic, ide relies on getting an iterator for m_items,
		// for example for searching next.
		//
		const std::vector<SchemaItemPtr>& items() const;

		// Properties
		//
	public:
		Schema* parentSchema();
		const Schema* parentSchema() const;
		void setParentSchema(Schema* parentSchema);

		QUuid guid() const;
		void setGuid(QUuid value);

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
		std::map<SchemaPoint, int> connectionMap;			// Key is pin position, value is count of pins on the point

	private:
		Schema* m_parentSchema = nullptr;

		std::vector<std::shared_ptr<SchemaItem>> m_items;		// Layer items

		QUuid m_guid;
		QString m_name;
		bool m_compile = false;
		bool m_show = true;
		bool m_print = true;
	};

}
