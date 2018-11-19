#pragma once

#include "VFrame30Lib_global.h"
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

		virtual QStringList getSignalList() const override;
		virtual QStringList getLabels() const override;

		// Properties
		//
	public:
		QString equipmentIds() const;
		QStringList equipmentIdList() const;
		void setEquipmentIds(const QString& s);
		void setEquipmentIdList(const QStringList& s);

		QStringList* mutable_equipmentIds();

		bool isMultichannelSchema() const;
		int channelCount() const;

		int nextCounterValue();

		QString lmDescriptionFile() const;
		void setLmDescriptionFile(QString value);

		// Data
		//
	private:
		QStringList m_equipmentIds;
		int m_counter = 0;										// Count is used to generate new uniques StrIDs for App Siagnals
		QString m_lmDescriptionFile = "LogicModule0000.xml";	// LogicModule Description
	};

}
