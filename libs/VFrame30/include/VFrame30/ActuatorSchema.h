//#pragma once
//
//#include <VFrame30/Schema.h>
//
//
//namespace VFrame30
//{
//	class SchemaItemSignal;
//	class SchemaItemLoopback;
//	class SchemaItemTransmitter;
//	class SchemaItemReceiver;
//
//	class LogicSchema : public Schema
//	{
//		Q_OBJECT
//
//	public:
//		LogicSchema(void);
//		virtual ~LogicSchema(void);
//
//		virtual const SchemaTraits& traits() const override;
//
//		// Serialization
//		//
//	protected:
//		virtual bool SaveData(Proto::Envelope* message) const override;
//		virtual bool LoadData(const Proto::Envelope& message) override;
//
//		// Methods
//		//
//	public:
//		virtual void Draw(CDrawParam* drawParam, const QRectF& clipRect) override;
//
//	public:
//		std::map<QString, SchemaItemSignal*> getSignalItemsMap() const;       // Get the map of SchemaItemSignal* items,
//																			  // key is signal ID
//		std::map<QString, SchemaItemReceiver*> getSignalReceiversMap() const; // Get the map of SchemaItemReceiver* items,
//																			  // key is signal ID
//		std::map<QString, SchemaItemLoopback*> getLoopbacksMap() const;       // Get the map of SchemaItemLoopback* items,
//																			  // key is loopback ID
//		std::map<QString, SchemaItemTransmitter*> getTransmittersMap() const; // Get the map of SchemaItemTransmitter* items,
//																			  // key is Connection ID
//		std::map<QString, SchemaItemReceiver*> getReceiversMap() const;       // Get the map of SchemaItemReceiver* items,
//																			  // key is Connection ID
//	public:
//		QString equipmentIds() const;
//		const QStringList& equipmentIdList() const;
//		void setEquipmentIds(const QString& s);
//		void setEquipmentIdList(const QStringList& s);
//
//		QStringList* mutable_equipmentIds();
//
//		bool isMultichannelSchema() const;
//		int channelCount() const;
//
//		int nextCounterValue();
//
//		QString lmDescriptionFile() const;
//		void setLmDescriptionFile(QString value);
//
//		// Data
//		//
//	private:
//		QStringList m_equipmentIds;
//		int m_counter = 0;                           // Count is used to generate new uniques StrIDs for App Signals
//		QString m_lmDescriptionFile = "LM_SF41.xml"; // LogicModule Description
//	};
//} // namespace VFrame30
