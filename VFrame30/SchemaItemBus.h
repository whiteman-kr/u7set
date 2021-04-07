#pragma once

#include "FblItemRect.h"
#include "Bus.h"

namespace VFrame30
{
	//
	//
	//		SchemaItemBus
	//
	//
	class SchemaItemBus : public FblItemRect
	{
		Q_OBJECT

		friend ::Factory<SchemaItem>::DerivedType<SchemaItemBus>;

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
		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const override;

		// Methods
		//
	public:
		virtual QString buildName() const override;

	protected:
		virtual void setBusPins(const VFrame30::Bus& bus);

		// Properties
		//
	public:
		QString busTypeId() const;

		const VFrame30::Bus& busType() const;
		void setBusType(const VFrame30::Bus& bus);

		Hash busTypeHash() const;

	protected:
		const Bus& bus() const;

		// Data
		//
	private:
		Hash m_busTypeHash = 0xFFFFFFFFFFFFFFFF;
		Bus m_bus;
	};


	//
	//
	//		SchemaItemBusComposer
	//
	//
	class SchemaItemBusComposer : public SchemaItemBus
	{
		Q_OBJECT

		friend ::Factory<SchemaItem>::DerivedType<SchemaItemBusComposer>;

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
		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const override;

		// Public Methods
		//
	public:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const override;

		virtual QString toolTipText(int dpiX, int dpiY) const final;
		virtual QString buildName() const final;

	protected:
		virtual void setBusPins(const VFrame30::Bus& bus) final;

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
	class SchemaItemBusExtractor : public SchemaItemBus
	{
		Q_OBJECT

		friend ::Factory<SchemaItem>::DerivedType<SchemaItemBusExtractor>;

	public:
		SchemaItemBusExtractor();
		SchemaItemBusExtractor(SchemaUnit unit);
		virtual ~SchemaItemBusExtractor();

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

		virtual QString toolTipText(int dpiX, int dpiY) const final;
		virtual QString buildName() const final;

		void specificPropertyCouldBeChanged(QString propertyName, const QVariant& value);

	protected:
		virtual void setBusPins(const VFrame30::Bus& bus) final;

		// Properties
		//
	public:

		// Data
		//
	private:
	};

}
