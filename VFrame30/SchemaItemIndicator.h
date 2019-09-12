#pragma once

#include "PosRectImpl.h"


namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT SchemaItemIndicator : public PosRectImpl
	{
		Q_OBJECT

	public:
		SchemaItemIndicator(void);
		explicit SchemaItemIndicator(SchemaUnit unit);
		virtual ~SchemaItemIndicator(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const final;
		virtual bool LoadData(const Proto::Envelope& message) final;

		// Draw Functions
		//
	public:
		virtual void Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const final;

	private:
		// --
		//

		// Properties and Data
		//
	public:
		E::IndicatorType type() const;
		void setType(E::IndicatorType value);

	private:
		E::IndicatorType m_type = E::IndicatorType::HistogramVert;
	};
}
