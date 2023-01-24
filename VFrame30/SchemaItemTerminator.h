#pragma once

#include "FblItemRect.h"

namespace VFrame30
{

	class SchemaItemTerminator final : public FblItemRect
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
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:
		virtual void draw(CDrawParam* drawParam) const override;

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
