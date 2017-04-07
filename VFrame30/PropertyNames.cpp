#include "PropertyNames.h"

namespace VFrame30
{

	const QString PropertyNames::acceptClick("AcceptClick");
	const QString PropertyNames::clickScript("ClickScript");
	const QString PropertyNames::commented("Commented");

	const QString PropertyNames::fontName("FontName");
	const QString PropertyNames::fontSize("FontSize");
	const QString PropertyNames::fontBold("FontBold");
	const QString PropertyNames::fontItalic("FontItalic");

	const QString PropertyNames::type("Type");
	const QString PropertyNames::valueInteger("ValueInteger");
	const QString PropertyNames::valueFloat("ValueFloat");
	const QString PropertyNames::precision("Precision");
	const QString PropertyNames::analogFormat("AnalogFormat");
	const QString PropertyNames::columnCount("ColumnCount");
	const QString PropertyNames::pinCount("PinCount");
	const QString PropertyNames::showValidityPin("ValidityPin");

	const QString PropertyNames::lineColor("LineColor");
	const QString PropertyNames::lineWeight("LineWeight");
	const QString PropertyNames::fillColor("FillColor");
	const QString PropertyNames::fill("Fill");
	const QString PropertyNames::drawRect("DrawRect");
	const QString PropertyNames::textColor("TextColor");
	const QString PropertyNames::text("Text");
	const QString PropertyNames::label("Label");
	const QString PropertyNames::caption("Caption");

	const QString PropertyNames::userText("UserText");
	const QString PropertyNames::userTextPos("UserTextPos");

	const QString PropertyNames::equipmentIds("EquipmentIDs");
	const QString PropertyNames::lmDescriptionFile("LmDescriptionFile");

	const QString PropertyNames::ufbSchemaId("UFBSchemaID");
	const QString PropertyNames::ufbSchemaVersion("UFBSchemaVersion");

	const QString PropertyNames::width("Width");
	const QString PropertyNames::height("Height");
	const QString PropertyNames::locked("Locked");

	const QString PropertyNames::checkable("Checkable");
	const QString PropertyNames::checkedDefault("CheckedDefault");
	const QString PropertyNames::autoRepeat("AutoRepeat");
	const QString PropertyNames::autoRepeatDelay("AutoRepeatDelay");
	const QString PropertyNames::autoRepeatInterval("AutoRepeatInterval");
	const QString PropertyNames::styleSheet("StyleSheet");

	const QString PropertyNames::afterCreate("AfterCreate");
	const QString PropertyNames::clicked("Clicked");
	const QString PropertyNames::pressed("Pressed");
	const QString PropertyNames::released("Released");
	const QString PropertyNames::toggled("Toggled");

	const QString PropertyNames::alignHorz("AlignHorz");
	const QString PropertyNames::alignVert("AlignVert");

	const QString PropertyNames::dataType("DataType");

	const QString PropertyNames::appSignalIDs("AppSignalIDs");
	const QString PropertyNames::appSignalId("AppSignalID");
	const QString PropertyNames::connectionId("ConnectionID");

	const QString PropertyNames::behaviourCategory("Behaviour");
	const QString PropertyNames::appearanceCategory("Appearance");
	const QString PropertyNames::functionalCategory("Functional");
	const QString PropertyNames::textCategory("Text");
	const QString PropertyNames::monitorCategory("Monitor");
	const QString PropertyNames::parametersCategory("Parameters");
	const QString PropertyNames::controlCategory("Control");
	const QString PropertyNames::scriptsCategory("Scripts");

	const QString PropertyNames::defaultPushButtonStyleSheet(
R"_(QPushButton {
	border: 1px outset #8f8f91;
	border-radius: 4px;
	background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #f6f7fa, stop: 1 #dadbde);
}
QPushButton:pressed {
	border: 1px inset #8f8f91;
	border-radius: 4px;
	background-color: #f6f7fa;
}
QPushButton:checked {
	border: 1px inset #8f8f91;
	border-radius: 4px;
	background-color: #f6f7fa;
}
QPushButton:focus {
	border-width: 2px
}
QPushButton:hover {
	border-width: 2px
}
QPushButton:default {
	border-color: navy;
}
)_");

	const QString PropertyNames::defaultPushButtonAfterCreateScript(
R"_(function(schemaItem, schemaView, pushButtonWidget)
{
}
)_");

	const QString PropertyNames::defaultPushButtonEventScript(
R"_(function(schemaItem, schemaView, pushButtonWidget, checked)
{
}
)_");

}
