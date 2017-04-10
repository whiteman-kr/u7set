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
	const QString PropertyNames::toolTip("ToolTip");

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

	const QString PropertyNames::widgetPropStyleSheet("Property holds the widget's style sheet.\nThe style sheet contains a textual description of customizations to the widget's style.");
	const QString PropertyNames::widgetPropToolTip("Property holds the widget's tooltip.");
	const QString PropertyNames::widgetPropAfterCreate("Script code to run after the control is created.");

	const QString PropertyNames::pushButtonDefaultStyleSheet(
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

	const QString PropertyNames::pushButtonDefaultEventScript(
R"_(function(schemaItem, schemaView, pushButtonWidget, checked)
{
}
)_");

	const QString PropertyNames::pushButtonPropText("Property holds the text shown on the button.");
	const QString PropertyNames::pushButtonPropCheckable("Property holds whether the button is checkable.");
	const QString PropertyNames::pushButtonPropCheckedDefault("If Checkable is set this property holds whether the button is checked by default.");
	const QString PropertyNames::pushButtonPropAutoRepeat("Property holds whether AutoRepeat is enabled.\nIf AutoRepeat is enabled, then the Pressed(), Released(), and Clicked() are emitted at regular intervals when the button is down.\nThe initial delay and the repetition interval are defined in milliseconds by AutoRepeatDelay and AutoRepeatInterval.");
	const QString PropertyNames::pushButtonPropAutoRepeatDelay("Property holds the initial delay of auto-repetition.\nIf AutoRepeat is enabled, then AutoRepeatDelay defines the initial delay in milliseconds before auto-repetition kicks in.");
	const QString PropertyNames::pushButtonPropAutoRepeatInterval("Property holds the interval of auto-repetition.\nIf AutoRepeat is enabled, then AutoRepeatInterval defines the length of the auto-repetition interval in millisecons.");
	const QString PropertyNames::pushButtonPropClicked("Script code for signal Clicked().\nThis signal is emitted when the button is activated (i.e., pressed down then released while the mouse cursor is inside the button).\nIf the button is checkable, checked is true if the button is checked, or false if the button is unchecked.");
	const QString PropertyNames::pushButtonPropPressed("Script code for signal Pressed().\nThis signal is emitted when the button is pressed down.");
	const QString PropertyNames::pushButtonPropReleased("Script code for signal Released().\nThis signal is emitted when the button is pressed released.");
	const QString PropertyNames::pushButtonPropToggled("Script code for signal Toggled().\nThis signal is emitted whenever a checkable button changes its state.\nChecked is true if the button is checked, or false if the button is unchecked. This may be the result of a user action, Click() slot activation, or because setChecked() is called.");
}
