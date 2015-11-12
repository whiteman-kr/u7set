#include "EditEngineSetSchemeProperty.h"
#include "EditSchemeWidget.h"

namespace EditEngine
{

	SetSchemePropertyCommand::SetSchemePropertyCommand(EditSchemeView* schemeView,
			QString propertyName,
			QVariant value,
			std::shared_ptr<VFrame30::Scheme> scheme,
			QScrollBar* hScrollBar,
			QScrollBar* vScrollBar) :
		EditCommand(schemeView, hScrollBar, vScrollBar)
	{
		assert(propertyName.isEmpty() == false);
		assert(value.isValid() == true);
		assert(scheme.get());

		m_scheme = scheme;

		m_propertyName = propertyName;
		m_oldValue = m_scheme->propertyValue(m_propertyName);
		m_newValue = value;

		assert(m_oldValue.isValid() == true);

		return;
	}

	void SetSchemePropertyCommand::executeCommand(EditSchemeView*)
	{
		m_scheme->setPropertyValue(m_propertyName, m_newValue);
		return;
	}

	void SetSchemePropertyCommand::unExecuteCommand(EditSchemeView*)
	{
		m_scheme->setPropertyValue(m_propertyName, m_oldValue);
		return;
	}

}
