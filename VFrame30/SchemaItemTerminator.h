#pragma once

#include "FblItemRect.h"

namespace VFrame30
{

	class SchemaItemTerminator : public FblItemRect
	{
		Q_OBJECT

		friend ::Factory<SchemaItem>::DerivedType<SchemaItemTerminator>;

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
		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const final;

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
