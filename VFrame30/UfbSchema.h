#pragma once

#include "Schema.h"

namespace VFrame30
{

	class UfbSchema : public Schema
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
		[[nodiscard]]QString description() const;
		void setDescription(QString value);

		[[nodiscard]]int version() const;

		[[nodiscard]]QString lmDescriptionFile() const;
		void setLmDescriptionFile(QString value);

		[[nodiscard]] QString specificProperties() const;
		void setSpecificProperties(QString value);

		// Data
		//
	private:
		QString m_description;
		mutable int m_version = 1;								// Version is inceremented every save
		QString m_lmDescriptionFile = "LogicModule0000.xml";	// LogicModule Description

		QString m_specificPropertiesStruct;						// Description of the UFB's specific properties
	};

}
