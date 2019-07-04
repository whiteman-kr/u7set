#pragma once

#include "PosRectImpl.h"


namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT SchemaItemFrame : public PosRectImpl
	{
		Q_OBJECT

	public:
		SchemaItemFrame(void);
		explicit SchemaItemFrame(SchemaUnit unit);
		virtual ~SchemaItemFrame(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:
		virtual void Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const override;

		enum [[nodiscard]] ErrorCode
		{
			Ok,
			ParamError,
			InternalError,
			SourceSchemaHasSameId,
			SchemasHasDiffrenetUnitsWithoutAutoscale
		};

		ErrorCode setSchemaToFrame(VFrame30::Schema* destSchema, const VFrame30::Schema* sourceSchema);

		// Properties and Data
		//
	public:
		QString schemaId() const;
		void setSchemaId(QString value);

		bool allowScale() const;
		void setAllowScale(bool value);

		bool keepAspectRatio() const;
		void setKeepAspectRatio(bool value);

	private:
		QString m_schemaId{"SCHEMAID"};

		bool m_allowScale = true;
		bool m_keepAspectRatio = true;
	};
}
