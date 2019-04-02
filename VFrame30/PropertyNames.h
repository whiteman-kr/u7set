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
		static const QString preDrawScript;
		static const QString commented;
		static const QString objectName;
		static const QString name;
		static const QString guid;

		static const QString fontName;
		static const QString fontSize;
		static const QString fontBold;
		static const QString fontItalic;

		static const QString type;
		static const QString valueInteger;
		static const QString valueFloat;
		static const QString valueDiscrete;
		static const QString precision;
		static const QString precisionPropText;
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
		static const QString placeholderText;
		static const QString label;
		static const QString labelPos;
		static const QString caption;
		static const QString maxLength;
		static const QString multiLine;

		static const QString allowScale;
		static const QString keepAspectRatio;
		static const QString imageId;
		static const QString image;
		static const QString images;
		static const QString svg;
		static const QString initialImageId;

		// --
		//
		static const QString textAnalog;
		static const QString textDiscrete0;
		static const QString textDiscrete1;
		static const QString textNonValid;
		static const QString textValuePropDescription;

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
		static const QString readOnly;

		static const QString afterCreate;
		static const QString clicked;
		static const QString pressed;
		static const QString released;
		static const QString toggled;
		static const QString editingFinished;
		static const QString returnPressed;
		static const QString textChanged;

		static const QString alignHorz;
		static const QString alignVert;

		static const QString dataType;
		static const QString units;

		static const QString appSignalIDs;
		static const QString appSignalIDsValidator;
		static const QString appSignalId;
		static const QString appSignalIdValidator;
		static const QString connectionId;
		static const QString signalSource;

		static const QString coarseAperture;
		static const QString fineAperture;
		static const QString adaptiveAperture;

		static const QString loopbackId;

		static const QString busTypeId;
		static const QString busTypeFileName;
		static const QString busAutoSignalPlacemanet;
		static const QString busManualBusSize;
		static const QString busSignalId;
		static const QString busInbusOffset;
		static const QString busInbusDiscreteBitNo;
		static const QString busInbusAnalogSize;
		static const QString busInbusAnalogFormat;
		static const QString busInbusAnalogByteOrder;
		static const QString busAnalogLowLimit;
		static const QString busAnalogHightLimit;
		static const QString busInbusAnalogLowLimit;
		static const QString busInbusAnalogHightLimit;

		static const QString busSettingCategory;
		static const QString busInbusSettingCategory;

		static const QString behaviourCategory;
		static const QString appearanceCategory;
		static const QString functionalCategory;
		static const QString textCategory;
		static const QString colorCategory;
		static const QString monitorCategory;
		static const QString parametersCategory;
		static const QString controlCategory;
		static const QString scriptsCategory;
		static const QString apertureCategory;
		static const QString constCategory;
		static const QString imageCategory;

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

		static const QString lineEditDefaultStyleSheet;
		static const QString lineEditDefaultEventScript;
		static const QString lineEditPropText;
		static const QString lineEditPropPlaceholderText;
		static const QString lineEditPropMaxLength;
		static const QString lineEditPropReadOnly;
		static const QString lineEditPropEditingFinished;
		static const QString lineEditPropReturnPressed;
		static const QString lineEditPropTextChanged;

		static const QString scriptGlobalVariableView;
		static const QString scriptGlobalVariableTuning;
		static const QString scriptGlobalVariableSignals;
	};

}
#endif // PROPERTYNAMES_H
