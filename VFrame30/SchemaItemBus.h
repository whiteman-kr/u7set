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
		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const override;

		// Public Methods
		//
	public:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const override;
		virtual QString buildName() const override;

	protected:
		virtual void setBusPins(const VFrame30::Bus& bus);

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
		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const override;

		// Public Methods
		//
	public:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const override;
		virtual QString buildName() const override;

		void specificPropertyCouldBeChanged(QString propertyName, const QVariant& value);

	protected:
		virtual void setBusPins(const VFrame30::Bus& bus);

		// Properties
		//
	public:

		// Data
		//
	private:
	};

}
