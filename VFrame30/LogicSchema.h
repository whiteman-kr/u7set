#pragma once

#include "Schema.h"
#include "SchemaItemSignal.h"

namespace VFrame30
{

	class LogicSchema : public Schema
	{
		Q_OBJECT

	public:
		LogicSchema(void);
		virtual ~LogicSchema(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Methods
		//
	public:
		virtual void Draw(CDrawParam* drawParam, const QRectF& clipRect) override;

	public:
		std::map<QString, SchemaItemSignal*> getInputItemsMap() const;
		std::map<QString, SchemaItemSignal*> getInOutItemsMap() const;
		std::map<QString, SchemaItemSignal*> getOutputItemsMap() const;


		std::set<QString> getInputSignalsSet() const;
		std::set<QString> getInOutSignalsSet() const;
		std::set<QString> getOutputSignalsSet() const;


		std::set<QString> getSignalMap() const;
		virtual QStringList getSignalList() const override;

		// Properties
		//
	public:
		QString equipmentIds() const;
		const QStringList& equipmentIdList() const;
		void setEquipmentIds(const QString& s);
		void setEquipmentIdList(const QStringList& s);

		QStringList* mutable_equipmentIds();

		bool isMultichannelSchema() const;
		int channelCount() const;

		int nextCounterValue();

		QString lmDescriptionFile() const;
		void setLmDescriptionFile(QString value);

	private:
		template <typename SchemaItemSignalType>
		std::map<QString, SchemaItemSignal*> getSignalItemsMap() const;	// Key is signalID, value is ItemType SchemaItemSignal-derived element

		template <typename SchemaItemSignalType>
		std::set<QString> getSignalItemsSignals() const;

		// Data
		//
	private:
		QStringList m_equipmentIds;
		int m_counter = 0;										// Count is used to generate new uniques StrIDs for App Siagnals
		QString m_lmDescriptionFile = "LogicModule0000.xml";	// LogicModule Description
	};
}
