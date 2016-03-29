#pragma once

#include "Schema.h"

namespace VFrame30
{

	class VFRAME30LIBSHARED_EXPORT LogicSchema : public Schema
	{
		Q_OBJECT

	public:
		LogicSchema(void);
		virtual ~LogicSchema(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Methods
		//
	public:
		virtual void Draw(CDrawParam* pDrawParam, const QRectF& clipRect) const override;

		// Properties
		//
	public:
		QString hardwareStrIds() const;
		QStringList hardwareStrIdList() const;
		void setHardwareStrIds(const QString& s);

		QStringList* mutable_hardwareStrIds();

		bool isMultichannelSchema() const;
		int channelCount() const;

		// Data
		//
	private:
		QStringList m_hardwareStrIds;
	};

}
