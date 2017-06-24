#pragma once

#include "FblItemRect.h"
#include "Bus.h"

namespace VFrame30
{

	static const double BusSideLineWidth = mm2in(0.4);

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
		void setBusType(const VFrame30::Bus&bus);

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
