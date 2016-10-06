#pragma once

#include "FblItemRect.h"

namespace VFrame30
{

	class VFRAME30LIBSHARED_EXPORT SchemaItemTerminator : public FblItemRect
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<SchemaItem>::DerivedType<SchemaItemTerminator>;
#endif

	public:
		SchemaItemTerminator();
		SchemaItemTerminator(SchemaUnit unit);
		virtual ~SchemaItemTerminator();

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:
		virtual void Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const override;

	protected:
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const override;

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

}
