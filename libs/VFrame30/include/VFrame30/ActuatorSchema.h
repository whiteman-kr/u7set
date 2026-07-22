#pragma once

#include <VFrame30/Schema.h>


namespace VFrame30
{
	class SchemaItemSignal;
	class SchemaItemLoopback;


	class ActuatorSchema : public Schema
	{
		Q_OBJECT

	public:
		ActuatorSchema(void);
		virtual ~ActuatorSchema(void);

		virtual const SchemaTraits& traits() const override;

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
		std::map<QString, SchemaItemLoopback*> getLoopbacksMap() const; // Get the map of SchemaItemLoopback* items,
																		// key is loopback ID
	public:
		QString actuatorTypeId() const;
		void setActuatorTypeId(const QString& value);

		int nextCounterValue();

		QString lmDescriptionFile() const;
		void setLmDescriptionFile(QString value);

		// Data
		//
	private:
		QString m_actuatorTypeId;
		int m_counter = 0;                             // Count is used to generate new uniques StrIDs for App Signals
		QString m_lmDescriptionFile = "ACM1_SR70.xml"; // LogicModule Description
	};
} // namespace VFrame30
