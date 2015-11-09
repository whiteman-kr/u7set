#pragma once

#include "Scheme.h"

namespace VFrame30
{

	class VFRAME30LIBSHARED_EXPORT LogicScheme : public Scheme
	{
		Q_OBJECT

		Q_PROPERTY(QString HardwareStrIDs READ hardwareStrIds WRITE setHardwareStrIds)

	public:
		LogicScheme(void);
		virtual ~LogicScheme(void);

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
		void setHardwareStrIds(const QString& s);
		QStringList* mutable_hardwareStrIds();

		bool isMultichannelScheme() const;
		int channelCount() const;

		// Data
		//
	private:
		QStringList m_hardwareStrIds;
	};

}
