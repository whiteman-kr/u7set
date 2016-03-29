#ifndef EDITENGINESETSCHEMEPROPERTIY_H
#define EDITENGINESETSCHEMEPROPERTIY_H

#pragma once

#include "EditEngine.h"

namespace EditEngine
{

	class SetSchemePropertyCommand : public EditCommand
	{
		SetSchemePropertyCommand();
	public:
		SetSchemePropertyCommand(
				EditSchemeView* schemeView,
				QString propertyName,
				QVariant value,
				std::shared_ptr<VFrame30::Schema> scheme,
				QScrollBar* hScrollBar,
				QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(EditSchemeView* schemeView) override;
		virtual void unExecuteCommand(EditSchemeView* schemeView) override;

		// Data
		//
	private:
		QString m_propertyName;
		QVariant m_oldValue;
		QVariant m_newValue;

		std::shared_ptr<VFrame30::Schema> m_scheme;
	};

}


#endif // EDITENGINESETPROPERTIY_H
