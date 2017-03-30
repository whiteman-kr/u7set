#pragma once

#include "Schema.h"

namespace VFrame30
{

	class VFRAME30LIBSHARED_EXPORT UfbSchema : public Schema
	{
		Q_OBJECT

	public:
		UfbSchema(void);
		virtual ~UfbSchema(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Methods
		//
	public:
		virtual void Draw(CDrawParam* drawParam, const QRectF& clipRect) const override;

		// Properties
		//
	public:
		QString description() const;
		void setDescription(QString value);

		int version() const;

		QString lmDescriptionFile() const;
		void setLmDescriptionFile(QString value);

		// Data
		//
	private:
		QString m_description;
		int m_version = 1;							// Version is inceremented every save
		QString m_lmDescriptionFile = "LogicModule0000.xml";	// LogicModule Description
	};

}
