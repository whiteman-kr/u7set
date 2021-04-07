#pragma once

#include "PosRectImpl.h"


namespace VFrame30
{
	class SchemaItemFrame : public PosRectImpl
	{
		Q_OBJECT

	public:
		SchemaItemFrame(void);
		explicit SchemaItemFrame(SchemaUnit unit);
		virtual ~SchemaItemFrame(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const final;
		virtual bool LoadData(const Proto::Envelope& message) final;

		// Draw Functions
		//
	public:
		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const final;

		// --
		//
		enum [[nodiscard]] ErrorCode
		{
			Ok,
			ParamError,
			InternalError,
			SourceSchemaHasSameId,
			SchemasHasDiffrenetUnits
		};

		SchemaItemFrame::ErrorCode setSchemaToFrame(VFrame30::Schema* destSchema, const VFrame30::Schema* sourceSchema);

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
