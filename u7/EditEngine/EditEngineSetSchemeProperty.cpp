#include "EditEngineSetSchemeProperty.h"
#include "VideoFrameWidget.h"
#include "EditSchemeWidget.h"

namespace EditEngine
{

	SetSchemePropertyCommand::SetSchemePropertyCommand(
			EditSchemeView* videoFrameView,
			QString propertyName,
			QVariant value,
			std::shared_ptr<VFrame30::Scheme> scheme,
			QScrollBar* hScrollBar,
			QScrollBar* vScrollBar) :
		EditCommand(videoFrameView, hScrollBar, vScrollBar)
	{
		assert(propertyName.isEmpty() == false);
		assert(value.isValid() == true);
		assert(scheme.get());

		m_scheme = scheme;

		m_propertyName = propertyName;
		m_oldValue = m_scheme->property(m_propertyName.toStdString().c_str());
		m_newValue = value;

		assert(m_oldValue.isValid() == true);

		return;
	}

	void SetSchemePropertyCommand::executeCommand(EditSchemeView* /*videoFrameView*/)
	{
		m_scheme->setProperty(m_propertyName.toStdString().c_str(), m_newValue);
		return;
	}

	void SetSchemePropertyCommand::unExecuteCommand(EditSchemeView* /*videoFrameView*/)
	{
		m_scheme->setProperty(m_propertyName.toStdString().c_str(), m_oldValue);
		return;
	}

}
