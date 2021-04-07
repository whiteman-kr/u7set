#pragma once

#include "FblItemRect.h"

namespace VFrame30
{

	//
	//
	//		SchemaItemConnection
	//
	//
	class SchemaItemConnection : public FblItemRect
	{
		Q_OBJECT

		friend ::Factory<SchemaItem>::DerivedType<SchemaItemConnection>;

	public:
		SchemaItemConnection();
		SchemaItemConnection(SchemaUnit unit);
		virtual ~SchemaItemConnection();

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:
		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const override;

		// Public Methods
		//
	public:
		virtual QString buildName() const override;

		// Properties
		//
	public:
		QString connectionIds() const;
		void setConnectionIds(const QString& value);

		const QStringList& connectionIdsAsList() const;
		void setConnectionIdsAsList(const QStringList& value);

		// Data
		//
	private:
		QStringList m_connectionIds = {"CONNECTIONID"};
	};


	//
	//
	//		SchemaItemTransmitter
	//
	//
	class SchemaItemTransmitter : public SchemaItemConnection
	{
		Q_OBJECT

		friend ::Factory<SchemaItem>::DerivedType<SchemaItemTransmitter>;

	public:
		SchemaItemTransmitter();
		SchemaItemTransmitter(SchemaUnit unit);
		virtual ~SchemaItemTransmitter();

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const final;
		virtual bool LoadData(const Proto::Envelope& message) final;

		// Draw Functions
		//
	public:
		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const final;

		// Public Methods
		//
	public:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const final;
		virtual QString buildName() const final;
		virtual QString toolTipText(int dpiX, int dpiY) const final;

		// Properties
		//
	public:
		int pinCount() const;
		void setPinCount(int value);

		// Data
		//
	private:
		int m_pinCount = 1;
	};


	//
	//
	//		SchemaItemReceiver
	//
	//
	class SchemaItemReceiver : public SchemaItemConnection
	{
		Q_OBJECT

		friend ::Factory<SchemaItem>::DerivedType<SchemaItemReceiver>;

	public:
		SchemaItemReceiver();
		SchemaItemReceiver(SchemaUnit unit);
		virtual ~SchemaItemReceiver();

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const final;
		virtual bool LoadData(const Proto::Envelope& message) final;

		// Draw Functions
		//
	public:
		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const final;

		// Public Methods
		//
	public:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const final;
		virtual QString buildName() const override;
		virtual QString toolTipText(int dpiX, int dpiY) const final;

		// Properties
		//
	public:
		QString appSignalIds() const;
		void setAppSignalIds(const QString& value);

		const QStringList& appSignalIdsAsList() const;
		void setAppSignalIdsAsList(const QStringList& value);

		bool showValidity() const;
		void setShowValidity(bool value);

//		bool multiline() const;
//		void setMultiline(bool value);

		bool isValidityPin(const QUuid& pin) const;
		bool isOutputPin(const QUuid& pinGuid) const;

		E::ColumnData data() const;
		void setData(E::ColumnData value);

		int precision() const;
		void setPrecision(int value);

		E::AnalogFormat analogFormat() const;
		void setAnalogFormat(E::AnalogFormat value);

		QString customText() const;
		void setCustomText(const QString& value);

		// Data
		//
	private:
		QStringList m_appSignalIds = {"#APPSIGNALID"};
		bool m_showValidity = true;
		//bool m_multiline = true;

		E::ColumnData m_dataType = E::ColumnData::AppSignalID;	// Data for displaying

		int m_precision = 2;
		E::AnalogFormat m_analogFormat = E::AnalogFormat::f_9;
		QString m_customText;
	};

}
