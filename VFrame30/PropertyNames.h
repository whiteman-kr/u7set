#ifndef PROPERTYNAMES_H
#define PROPERTYNAMES_H

namespace VFrame30
{

	class VFRAME30LIBSHARED_EXPORT PropertyNames
	{
	public:
		PropertyNames() = delete;

	public:
		static const QString acceptClick;
		static const QString clickScript;
		static const QString commented;

		static const QString fontName;
		static const QString fontSize;
		static const QString fontBold;
		static const QString fontItalic;

		static const QString type;
		static const QString valueInteger;
		static const QString valueFloat;
		static const QString precision;
		static const QString analogFormat;
		static const QString columnCount;
		static const QString pinCount;
		static const QString showValidityPin;

		static const QString lineColor;
		static const QString lineWeight;
		static const QString fillColor;
		static const QString fill;
		static const QString drawRect;
		static const QString textColor;
		static const QString text;
		static const QString label;
		static const QString caption;

		static const QString userText;
		static const QString userTextPos;

		static const QString equipmentIds;
		static const QString lmDescriptionFile;

		static const QString ufbSchemaId;
		static const QString ufbSchemaVersion;

		static const QString width;
		static const QString height;
		static const QString locked;

		static const QString checkable;
		static const QString checkedDefault;
		static const QString autoRepeat;
		static const QString autoRepeatDelay;
		static const QString autoRepeatInterval;
		static const QString styleSheet;
		static const QString toolTip;

		static const QString afterCreate;
		static const QString clicked;
		static const QString pressed;
		static const QString released;
		static const QString toggled;

		static const QString alignHorz;
		static const QString alignVert;

		static const QString dataType;

		static const QString appSignalIDs;
		static const QString appSignalId;
		static const QString connectionId;

		static const QString behaviourCategory;
		static const QString appearanceCategory;
		static const QString functionalCategory;
		static const QString textCategory;
		static const QString monitorCategory;
		static const QString parametersCategory;
		static const QString controlCategory;
		static const QString scriptsCategory;

		static const QString widgetPropStyleSheet;
		static const QString widgetPropToolTip;
		static const QString widgetPropAfterCreate;

		static const QString pushButtonPropText;
		static const QString pushButtonDefaultStyleSheet;
		static const QString pushButtonDefaultEventScript;
		static const QString pushButtonPropCheckable;
		static const QString pushButtonPropCheckedDefault;
		static const QString pushButtonPropAutoRepeat;
		static const QString pushButtonPropAutoRepeatDelay;
		static const QString pushButtonPropAutoRepeatInterval;
		static const QString pushButtonPropClicked;
		static const QString pushButtonPropPressed;
		static const QString pushButtonPropReleased;
		static const QString pushButtonPropToggled;
	};

}
#endif // PROPERTYNAMES_H
