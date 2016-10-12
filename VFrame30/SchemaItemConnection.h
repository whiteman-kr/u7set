#pragma once

#include "FblItemRect.h"

namespace VFrame30
{

	//
	//
	//		SchemaItemConnection
	//
	//
	class VFRAME30LIBSHARED_EXPORT SchemaItemConnection : public FblItemRect
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<SchemaItem>::DerivedType<SchemaItemConnection>;
#endif

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
		virtual void Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const override;

		// Public Methods
		//
	public:
		virtual QString buildName() const override;

		// Properties
		//
	public:
		QString connectionId() const;
		void setConnectionId(const QString& value);

		// Data
		//
	private:
		QString m_connectionId = {"CONNECTIONID"};
	};


	//
	//
	//		SchemaItemTransmitter
	//
	//
	class VFRAME30LIBSHARED_EXPORT SchemaItemTransmitter : public SchemaItemConnection
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<SchemaItem>::DerivedType<SchemaItemTransmitter>;
#endif

	public:
		SchemaItemTransmitter();
		SchemaItemTransmitter(SchemaUnit unit);
		virtual ~SchemaItemTransmitter();

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:
		virtual void Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const override;

		// Text search
		//
	public:
		virtual bool searchText(const QString& text) const override;

		// Public Methods
		//
	public:
		virtual QString buildName() const override;

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
	class VFRAME30LIBSHARED_EXPORT SchemaItemReceiver : public SchemaItemConnection
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<SchemaItem>::DerivedType<SchemaItemReceiver>;
#endif

	public:
		SchemaItemReceiver();
		SchemaItemReceiver(SchemaUnit unit);
		virtual ~SchemaItemReceiver();

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:
		virtual void Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const override;

		// Text search
		//
	public:
		virtual bool searchText(const QString& text) const override;

		// Public Methods
		//
	public:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const override;
		virtual QString buildName() const override;

		// Properties
		//
	public:
		const QString& appSignalId() const;
		void setAppSignalId(const QString& value);

		bool showValidity() const;
		void setShowValidity(bool value);

		bool isValidityPin(const QUuid& pin) const;
		bool isOutputPin(const QUuid& pinGuid) const;

		// Data
		//
	private:
		QString m_appSignalId = "#APPSIGNALID";
		bool m_showValidity = true;
	};

}
