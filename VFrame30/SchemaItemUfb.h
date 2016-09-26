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
	class VFRAME30LIBSHARED_EXPORT SchemaItemUfb : public FblItemRect
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
		virtual void Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* pLayer) const override;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Methods
		//
	public:
		virtual QString buildName() const override;

	protected:
		bool updateElement(const UfbSchema* ufbSchema, QString* errorMsg);

		// Properties and Data
		//
	public:
		QString ufbSchemaId() const;
		QString ufbCaption() const;
		int ufbSchemaVersion() const;

		QString label() const;
		void setLabel(QString value);

	private:
		QString m_ufbSchemaId;
		QString m_ufbCaption;
		int m_version = -1;

		QString m_label;
	};
}
