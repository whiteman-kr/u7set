#include "PropertyNames.h"

namespace VFrame30
{

	const QString PropertyNames::acceptClick("AcceptClick");
	const QString PropertyNames::clickScript("ClickScript");
	const QString PropertyNames::preDrawScript("PreDrawScript");
	const QString PropertyNames::commented("Commented");
	const QString PropertyNames::objectName("ObjectName");
	const QString PropertyNames::name("Name");
	const QString PropertyNames::guid("Uuid");

	const QString PropertyNames::fontName("FontName");
	const QString PropertyNames::fontSize("FontSize");
	const QString PropertyNames::fontBold("FontBold");
	const QString PropertyNames::fontItalic("FontItalic");

	const QString PropertyNames::type("Type");
	const QString PropertyNames::valueInteger("ValueInteger");
	const QString PropertyNames::valueFloat("ValueFloat");
	const QString PropertyNames::precision("Precision");
	const QString PropertyNames::precisionPropText("Number of decimals after period, -1: take value from the signal description");
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
	const QString PropertyNames::placeholderText("PlaceholderText");
	const QString PropertyNames::label("Label");
	const QString PropertyNames::caption("Caption");
	const QString PropertyNames::maxLength("MaxLength");
	const QString PropertyNames::multiLine("MultiLine");

	// SchemaItemValue Colors
	//
	const QString PropertyNames::fillColorNonValid0("ColorNonValidFill0");
	const QString PropertyNames::fillColorNonValid1("ColorNonValidFill1");
	const QString PropertyNames::textColorNonValid0("ColorNonValidText0");
	const QString PropertyNames::textColorNonValid1("ColorNonValidText1");

	const QString PropertyNames::fillColorOverflow0("ColorOverflowFill0");
	const QString PropertyNames::fillColorOverflow1("ColorOverflowFill1");
	const QString PropertyNames::textColorOverflow0("ColorOverflowText0");
	const QString PropertyNames::textColorOverflow1("ColorOverflowText1");

	const QString PropertyNames::fillColorUnderflow0("ColorUnderflowFill0");
	const QString PropertyNames::fillColorUnderflow1("ColorUnderflowFill1");
	const QString PropertyNames::textColorUnderflow0("ColorUnderflowText0");
	const QString PropertyNames::textColorUnderflow1("ColorUnderflowText1");

	const QString PropertyNames::fillColorAnalog0("ColorAnalogFill0");
	const QString PropertyNames::fillColorAnalog1("ColorAnalogFill1");
	const QString PropertyNames::textColorAnalog0("ColorAnalogText0");
	const QString PropertyNames::textColorAnalog1("ColorAnalogText1");

	const QString PropertyNames::fillColorDiscrYes0("ColorDiscrYesFill0");
	const QString PropertyNames::fillColorDiscrYes1("ColorDiscrYesFill1");
	const QString PropertyNames::textColorDiscrYes0("ColorDiscrYesText0");
	const QString PropertyNames::textColorDiscrYes1("ColorDiscrYesText1");

	const QString PropertyNames::fillColorDiscrNo0("ColorDiscrNoFill0");
	const QString PropertyNames::fillColorDiscrNo1("ColorDiscrNoFill1");
	const QString PropertyNames::textColorDiscrNo0("ColorDiscrNoText0");
	const QString PropertyNames::textColorDiscrNo1("ColorDiscrNoText1");

	const QString PropertyNames::textAnalog("TextAnalog");
	const QString PropertyNames::textDiscrete0("TextDiscrete0");
	const QString PropertyNames::textDiscrete1("TextDiscrete1");
	const QString PropertyNames::textNonValid("TextNonValid");
	const QString PropertyNames::textValuePropDescription("$(value) Signal value\n"
														  "$(caption) Signal caption\n"
														  "$(signalid) SignalID (CustomSignalID)\n"
														  "$(appsignalid) AppSignalID (#APPSIGANLID)\n"
														  "$(equipmentid) Signal EquipmentID (LM for internal signals, input/output equipment port for IO signals)\n");
														  //"$(highlimit) High limit\n"
														  //"$(lowlimit) Low limit");

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
	const QString PropertyNames::readOnly("ReadOnly");

	const QString PropertyNames::afterCreate("AfterCreate");
	const QString PropertyNames::clicked("Clicked");
	const QString PropertyNames::pressed("Pressed");
	const QString PropertyNames::released("Released");
	const QString PropertyNames::toggled("Toggled");
	const QString PropertyNames::editingFinished("EditingFinished");
	const QString PropertyNames::returnPressed("ReturnPressed");
	const QString PropertyNames::textChanged("TextChanged");

	const QString PropertyNames::alignHorz("AlignHorz");
	const QString PropertyNames::alignVert("AlignVert");

	const QString PropertyNames::dataType("DataType");
	const QString PropertyNames::units("Units");

	const QString PropertyNames::appSignalIDs("AppSignalIDs");
	const QString PropertyNames::appSignalId("AppSignalID");
	const QString PropertyNames::connectionId("ConnectionID");
	const QString PropertyNames::signalSource("SignalSource");

	const QString PropertyNames::coarseAperture("CoarseAperture");
	const QString PropertyNames::fineAperture("FineAperture");
	const QString PropertyNames::adaptiveAperture("AdaptiveAperture");

	const QString PropertyNames::busTypeId("BusTypeID");
	const QString PropertyNames::busTypeFileName("FileName");
	const QString PropertyNames::busAutoSignalPlacemanet("AutoSignalPlacement");
	const QString PropertyNames::busManualBusSize("ManualBusSize");
	const QString PropertyNames::busSignalId("SignalID");

	const QString PropertyNames::busInbusOffset("Offset");
	const QString PropertyNames::busInbusDiscreteBitNo("BitNo");
	const QString PropertyNames::busInbusAnalogSize("Size");
	const QString PropertyNames::busInbusAnalogFormat("Format");
	const QString PropertyNames::busInbusAnalogByteOrder("ButeOrder");
	const QString PropertyNames::busAnalogLowLimit("BusSignalLowLimit");
	const QString PropertyNames::busAnalogHightLimit("BusSignalHighLimit");
	const QString PropertyNames::busInbusAnalogLowLimit("InbusSignalLowLimit");
	const QString PropertyNames::busInbusAnalogHightLimit("InbusSignalHighLimit");

	const QString PropertyNames::busSettingCategory("Bus Settings");
	const QString PropertyNames::busInbusSettingCategory("InBus Settings (Manual Signal Placement)");

	const QString PropertyNames::behaviourCategory("Behaviour");
	const QString PropertyNames::appearanceCategory("Appearance");
	const QString PropertyNames::functionalCategory("Functional");
	const QString PropertyNames::textCategory("Text");
	const QString PropertyNames::colorCategory("Color");
	const QString PropertyNames::monitorCategory("Monitor");
	const QString PropertyNames::parametersCategory("Parameters");
	const QString PropertyNames::controlCategory("Control");
	const QString PropertyNames::scriptsCategory("Scripts");
	const QString PropertyNames::apertureCategory("Aperture");

	const QString PropertyNames::widgetPropStyleSheet("Property holds the widget's style sheet.\nThe style sheet contains a textual description of customizations to the widget's style.");
	const QString PropertyNames::widgetPropToolTip("Property holds the widget's tooltip.");
	const QString PropertyNames::widgetPropAfterCreate("Script code to run after the control is created.");

	const QString PropertyNames::pushButtonDefaultStyleSheet(
R"_(QPushButton {
	border: 1px outset #8f8f91;
	border-radius: 4px;
	background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #f6f7fa, stop: 1 #dadbde);
	font-family: "Arial";
	font-size: 14pt;		/* px: pixels, pt: the size of one point (i.e., 1/72 of an inch) */
	font-style: Normal;	/* Normal, Italic, Oblique */
	font-weight: Normal; 	/* Normal, Bold, 100, 200 ... 900 */
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
R"_(function(schemaItem, pushButtonWidget, checked)
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

	const QString PropertyNames::lineEditDefaultStyleSheet = {
R"_(QLineEdit {
	border-width: 1px;
	border-style: solid;
	border-color: #404040;
	background-color: white;

	font-family: "Consolas";
	font-size: 14pt;		/* px: pixels, pt: the size of one point (i.e., 1/72 of an inch) */
	font-style: Normal;		/* Normal, Italic, Oblique */
	font-weight: Normal;	/* Normal, Bold, 100, 200 ... 900 */
}
QLineEdit:focus {
	border-width: 2px
}
QLineEdit:hover {
	border-width: 2px
})_"};

	const QString PropertyNames::lineEditDefaultEventScript = {"function(schemaItem, lineEditWidget, text)\n{\n}"};
	const QString PropertyNames::lineEditPropText("Property holds the line edit's text.");
	const QString PropertyNames::lineEditPropPlaceholderText("Property holds the line edit's placeholder text.\n"
															 "Setting this property makes the line edit display a grayed-out placeholder text as long as the line edit is empty.\n"
															 "Normally, an empty line edit shows the placeholder text even when it has focus.\n"
															 "However, if the content is horizontally centered, the placeholder text is not displayed under the cursor when the line edit has focus.\n"
															 "By default, this property contains an empty string.");
	const QString PropertyNames::lineEditPropMaxLength("Property holds the maximum permitted length of the text.\n"
													   "If the text is too long, it is truncated at the limit.\n"
													   "If truncation occurs any selected text will be unselected, the cursor position is set to 0 and the first part of the string is shown.");

   const QString PropertyNames::lineEditPropReadOnly("This property holds whether the line edit is read only.\n"
													 "In read-only mode, the user can still copy the text to the clipboard, or drag and drop the text, but cannot edit it.\n"
													 "LineEdit does not show a cursor in read-only mode.\n"
													 "By default, this property is false.");

   const QString PropertyNames::lineEditPropEditingFinished("This signal is emitted when the Return or Enter key is pressed or the line edit loses focus.");
   const QString PropertyNames::lineEditPropReturnPressed("This signal is emitted when the Return or Enter key is pressed.");
   const QString PropertyNames::lineEditPropTextChanged("This signal is emitted whenever the text changes.\nThis signal is also emitted when the text is changed programmatically.");

   const QString PropertyNames::scriptGlobalVariableView = {"view"};
   const QString PropertyNames::scriptGlobalVariableTuning = {"tuning"};
   const QString PropertyNames::scriptGlobalVariableSignals = {"signals"};
}
