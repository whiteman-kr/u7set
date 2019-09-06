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
		virtual bool SaveData(Proto::Envelope* message) const final;
		virtual bool LoadData(const Proto::Envelope& message) final;

		// Draw Functions
		//
	public:
		virtual void Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const final;

	protected:
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const final;

		// Public Methods
		//
	public:
		virtual QString buildName() const final;

		// Properties
		//
	public:

		// Data
		//
	private:

	};

}
