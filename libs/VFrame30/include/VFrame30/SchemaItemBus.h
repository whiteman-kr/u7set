#pragma once

#include <VFrame30/FblItemRect.h>
#include "../../AppSignalLib/Bus.h"

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
		virtual void draw(CDrawParam* drawParam) const override;

		// Methods
		//
	public:
		virtual QString buildName() const override;

	protected:
		virtual void setBusPins(const AppSignalLib::Bus& bus);

		// Properties
		//
	public:
		QString busTypeId() const;

		const AppSignalLib::Bus& busType() const;
		void setBusType(const AppSignalLib::Bus& bus);

		Hash busTypeHash() const;

	protected:
		const AppSignalLib::Bus & bus() const;

		// Data
		//
	private:
		Hash m_busTypeHash = 0xFFFFFFFFFFFFFFFF;
		AppSignalLib::Bus m_bus;
	};


	//
	//
	//		SchemaItemBusComposer
	//
	//
	class SchemaItemBusComposer final : public SchemaItemBus
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
		virtual void draw(CDrawParam* drawParam) const override;

		// Public Methods
		//
	public:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const override;

		virtual QString toolTipText(double dpiX, double dpiY, double devicePixelRatio) const override;
		virtual QString buildName() const override;

	protected:
		virtual void setBusPins(const AppSignalLib::Bus& bus) override;

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
	class SchemaItemBusExtractor final : public SchemaItemBus
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
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:
		virtual void draw(CDrawParam* drawParam) const override;

		// Public Methods
		//
	public:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const override;

		virtual QString toolTipText(double dpiX, double dpiY, double devicePixelRatio) const override;
		virtual QString buildName() const override;

		void specificPropertyCouldBeChanged(QString propertyName, const QVariant& value);

	protected:
		virtual void setBusPins(const AppSignalLib::Bus& bus) override;

		// Properties
		//
	public:

		// Data
		//
	private:
	};

}
