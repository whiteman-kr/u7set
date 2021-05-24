#pragma once

#include "FblItemRect.h"

namespace VFrame30
{
	class UfbSchema;
}

namespace VFrame30
{
	//
	// SchemaItemUfb
	//
	class SchemaItemUfb : public FblItemRect
	{
		Q_OBJECT

	public:
		SchemaItemUfb(void);
		explicit SchemaItemUfb(SchemaUnit unit);
		SchemaItemUfb(SchemaUnit unit, const UfbSchema* ufbSchema, QString* errorMsg);

		virtual ~SchemaItemUfb(void);

		// Draw Functions
		//
	public:
		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* pLayer) const final;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const final;
		virtual bool LoadData(const Proto::Envelope& message) final;

		// Methods
		//
	public:
		virtual QString buildName() const final;

		bool updateUfbElement(const UfbSchema* ufbSchema, QString* errorMsg);

		// Properties and Data
		//
	public:
		[[nodiscard]] QString ufbSchemaId() const;
		[[nodiscard]] QString ufbCaption() const;
		[[nodiscard]] int ufbSchemaVersion() const;

		[[nodiscard]] QString specificProperties() const;
	private:
		void setSpecificProperties(QString value);

	private:
		QString m_ufbSchemaId;
		QString m_ufbCaption;
		int m_ufbVersion = -1;

		QString m_specificPropertiesStruct;		// Description of the UFB's specific properties
	};
}
