#pragma once
#include "ClientBehavior.h"

class QColor;

namespace Behavior
{
	//
	// TuningClientBehavior
	//
	class TuningClientBehavior : public ClientBehavior
	{
	public:
		TuningClientBehavior();

		TuningClientBehavior& operator=(const TuningClientBehavior& That)
		{
			m_tagToColor = That.m_tagToColor;

			ClientBehavior::operator=(That);
			return *this;
		}

	private:
		virtual void saveToXml(QXmlStreamWriter& writer) override;
		virtual bool loadFromXml(QXmlStreamReader& reader) override;

	private:
		QHash<QString, QColor> m_tagToColor;
	};
} // namespace Behavior