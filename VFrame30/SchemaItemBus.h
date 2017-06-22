#pragma once

#include "FblItemRect.h"

namespace VFrame30
{

	//
	//
	//		SchemaItemBus
	//
	//
	class VFRAME30LIBSHARED_EXPORT SchemaItemBus : public FblItemRect
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<SchemaItem>::DerivedType<SchemaItemBus>;
#endif

	public:
		SchemaItemBus();
		SchemaItemBus(SchemaUnit unit);
		virtual ~SchemaItemBus();

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
		QString busTypeId() const;
		void setBusTypeId(const QString& value);

		// Data
		//
	private:
		QString m_busTypeId = {"BUSTYPEID"};
	};


	//
	//
	//		SchemaItemBusComposer
	//
	//
	class VFRAME30LIBSHARED_EXPORT SchemaItemBusComposer : public SchemaItemBus
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<SchemaItem>::DerivedType<SchemaItemBusComposer>;
#endif

	public:
		SchemaItemBusComposer();
		SchemaItemBusComposer(SchemaUnit unit);
		virtual ~SchemaItemBusComposer();

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

		// Data
		//
	private:
	};


	//
	//
	//		SchemaItemBusExtractor
	//
	//
	class VFRAME30LIBSHARED_EXPORT SchemaItemBusExtractor : public SchemaItemBus
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<SchemaItem>::DerivedType<SchemaItemBusExtractor>;
#endif

	public:
		SchemaItemBusExtractor();
		SchemaItemBusExtractor(SchemaUnit unit);
		virtual ~SchemaItemBusExtractor();

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
		//virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const override;
		virtual QString buildName() const override;

		// Properties
		//
	public:
//		const QString& appSignalId() const;
//		void setAppSignalId(const QString& value);

//		bool showValidity() const;
//		void setShowValidity(bool value);

//		bool isValidityPin(const QUuid& pin) const;
//		bool isOutputPin(const QUuid& pinGuid) const;

//		E::ColumnData data() const;
//		void setData(E::ColumnData value);

//		int precision() const;
//		void setPrecision(int value);

//		E::AnalogFormat analogFormat() const;
//		void setAnalogFormat(E::AnalogFormat value);

		// Data
		//
	private:
//		QString m_appSignalId = "#APPSIGNALID";
//		bool m_showValidity = true;

//		E::ColumnData m_dataType = E::ColumnData::AppSignalID;	// Data for displaying

//		int m_precision = 2;
//		E::AnalogFormat m_analogFormat = E::AnalogFormat::f_9;
	};

}
