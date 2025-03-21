#pragma once

namespace VFrame30
{

	class PropertyNames
	{
	  public:
		PropertyNames() = delete;

	  public:
		inline static const QString schemaId{"SchemaID"};

		inline static const QString top{"Top"};
		inline static const QString left{"Left"};
		inline static const QString width{"Width"};
		inline static const QString height{"Height"};

		inline static const QString lowLimit{"LowLimit"};
		inline static const QString highLimit{"HighLimit"};

		inline static const QString acceptClick{"AcceptClick"};
		inline static const QString clickScript{"ClickScript"};
		inline static const QString preDrawScript{"PreDrawScript"};
		inline static const QString onShowScript{"OnShowScript"};
		inline static const QString commented{"Commented"};
		inline static const QString objectName{"ObjectName"};
		inline static const QString name{"Name"};
		inline static const QString guid{"Uuid"};

		inline static const QString fontName{"FontName"};
		inline static const QString fontSize{"FontSize"};
		inline static const QString fontBold{"FontBold"};
		inline static const QString fontItalic{"FontItalic"};

		inline static const QString color{"Color"};

		inline static const QString rotationPoint{"RotationPoint"};
		inline static const QString rotationPointDescription{"The point around which the item is rotated."};
		inline static const QString angle{"Angle"};
		inline static const QString angleDescription{"Item rotation angle (rotates around RotationPoint)."};

		inline static const QString type{"Type"};
		inline static const QString value{"Value"};
		inline static const QString valueInteger{"ValueInteger"};
		inline static const QString valueFloat{"ValueFloat"};
		inline static const QString valueDiscrete{"ValueDiscrete"};
		inline static const QString precision{"Precision"};
		inline static const QString precisionPropText{"Number of decimals after period, -1: take value from the signal description"};
		inline static const QString analogFormat{"AnalogFormat"};
		inline static const QString customText{"CustomText"};
		inline static const QString columnCount{"ColumnCount"};
		inline static const QString pinCount{"PinCount"};
		inline static const QString showValidityPin{"ValidityPin"};

		inline static const QString lineCapSize{"LineCapSize"};
		inline static const QString lineCapFactor{"LineCapFactor"};
		inline static const QString lineCapStart{"LineCapStart"};
		inline static const QString lineCapEnd{"LineCapEnd"};
		inline static const QString lineStyle{"LineStyle"};
		inline static const QString lineStyleCap{"LineStyleCap"};
		inline static const QString lineColor{"LineColor"};
		inline static const QString lineWeight{"LineWeight"};
		inline static const QString fillColor{"FillColor"};
		inline static const QString backgroundColor{"BackgroundColor"};
		inline static const QString fill{"Fill"};
		inline static const QString drawRect{"DrawRect"};
		inline static const QString textFormat{"TextFormat"};
		inline static const QString textFormatDescription{"PlainText - Manual formatting.\nMarkdown - Markdown formatting, supports GitHub-style Markdown.\nHtmlSubset - HTML-formatted text in the html string. Support of the limited HTML Subset."};
		inline static const QString textColor{"TextColor"};
		inline static const QString text{"Text"};
		inline static const QString wordWrap{"WordWrap"};
		inline static const QString placeholderText{"PlaceholderText"};
		inline static const QString tags{"Tags"};
		inline static const QString label{"Label"};
		inline static const QString labelPos{"LabelPos"};
		inline static const QString caption{"Caption"};
		inline static const QString maxLength{"MaxLength"};
		inline static const QString multiLine{"MultiLine"};

		inline static const QString allowScale{"AllowScale"};
		inline static const QString keepAspectRatio{"KeepAspectRatio"};
		inline static const QString imageId{"ImageID"};
		inline static const QString image{"Image"};
		inline static const QString images{"Images"};
		inline static const QString currentImageId{"CurrentImageID"};

		inline static const QString svg{"Svg"};
		inline static const QString svgScaleFactor{"SvgScaleFactor"};
		inline static const QString svgScaleFactorDescription{"Multiplier for SVG image size."};

		inline static const QString orientation{"Orientation"};
		
		inline static const QString sliderInvertedAppearance{"InvertedAppearance"};
		inline static const QString sliderInvertedAppearanceToolTip{"This property holds whether or not a slider shows its values inverted.\nIf this property is false, the minimum and maximum will be shown in its classic position for the inherited widget.If the value is true, the minimum and maximum appear at their opposite location."};
		inline static const QString sliderInvertedControls{"InvertedControls"};
		inline static const QString sliderInvertedControlsToolTip{"This property holds whether or not the slider inverts its wheel and key events.\nIf this property is false, scrolling the mouse wheel \"up\" and using keys like page up will increase the slider's value towards its maximum. Otherwise pressing page up will move value towards the slider's minimum."};
		inline static const QString sliderEnableMouseWheel{"EnableMouseWheel"};
		inline static const QString sliderEnableMouseWheelToolTip{"If `true` then mouse wheel will affect slider value (note that zoom in/out by mouse wheel then will not work over the slider). If `false` then mouse wheel is ignored and event is propagated to the view."};
		inline static const QString sliderMaximum{"Maximum"};
		inline static const QString sliderMaximumToolTip{"This property holds the slider's maximum value.\nWhen setting this property, the minimum is adjusted if necessary to ensure that the range remains valid.Also the slider's current value is adjusted to be within the new range."};
		inline static const QString sliderMinimum{"Minimum"};
		inline static const QString sliderMinimumToolTip{"This property holds the slider's minimum value.\nWhen setting this property, the maximum is adjusted if necessary to ensure that the range remains valid.Also the slider's current value is adjusted to be within the new range."};
		inline static const QString sliderPageStep{"PageStep"};
		inline static const QString sliderPageStepToolTip{"This property holds the page step.\nThe larger of two natural steps that a slider provides and typically corresponds to the user pressing PageUp or PageDown."};
		inline static const QString sliderSingleStep{"SingleStep"};
		inline static const QString sliderSingleStepToolTip{"This property holds the single step.\nThe smaller of two natural steps that a slider provides and typically corresponds to the user pressing an arrow key."};
		inline static const QString sliderTracking{"Tracking"};
		inline static const QString sliderTrackingToolTip{"This property holds whether slider tracking is enabled.\nIf tracking is enabled, the slider emits the valueChanged() signal while the slider is being dragged.If tracking is disabled, the slider emits the valueChanged() signal only when the user releases the slider."};
		inline static const QString sliderTickInterval{"TickInterval"};
		inline static const QString sliderTickIntervalToolTip{"This property holds the interval between tickmarks.\nThis is a value interval, not a pixel interval.If it is 0, the slider will choose between singleStep and pageStep."};
		inline static const QString sliderTickPosition{"TickPosition"};
		inline static const QString sliderTickPositionToolTip{""};
		inline static const QString sliderDefaultValue{"DefaultValue"};
		inline static const QString sliderDefaultEventScript{"(function(schemaItem, sliderWidget, value)\n{\n})"};

		inline static const QString sliderMoved{"SliderMoved"};
		inline static const QString sliderMovedToolTip{"This signal is emitted when sliderDown is true and the slider moves. This usually happens when the user is dragging the slider. The value is the new slider position.\nThis signal is emitted even when tracking is turned off."};
		inline static const QString sliderPressed{"SliderPressed"};
		inline static const QString sliderPressedToolTip{"This signal is emitted when the user presses the slider with the mouse, or programmatically when setSliderDown(true) is called."};
		inline static const QString sliderReleased{"SliderReleased"};
		inline static const QString sliderReleasedToolTip{"This signal is emitted when the user releases the slider with the mouse, or programmatically when setSliderDown(false) is called."};
		inline static const QString sliderValueChanged{"ValueChanged"};
		inline static const QString sliderValueChangedToolTip{"This signal is emitted when the slider value has changed, with the new slider value as argument."};

		inline static const QString drawGrid{"DrawGrid"};
		inline static const QString drawGridForAllBars{"DrawGridForAllBars"};
		inline static const QString drawGridValues{"DrawGridValues"};
		inline static const QString drawGridValueForAllBars{"DrawGridValueForAllBars"};
		inline static const QString drawGridValueUnits{"DrawGridValueUnits"};
		inline static const QString linearGridMainStep{"LinearGridMainStep"};
		inline static const QString linearGridSmallStep{"LinearGridSmallStep"};
		inline static const QString logarithmicGridMainStep{"LogarithmicGridMainStep"};
		inline static const QString logarithmicGridSmallStep{"LogarithmicGridSmallStep"};

		inline static const QString indicatorSignalColors{"SignalColors"};
		inline static const QString indicatorType{"IndicatorType"};
		inline static const QString indicatorSettings{"IndicatorSettings"};
		inline static const QString indicator{"Indicator"};

		inline static const QString indicatorSetpointType{"SetpointType"};
		inline static const QString indicatorColorSource{"ColorSource"};
		inline static const QString indicatorSchemaItemLabel{"SchemaItemLabel"};
		inline static const QString indicatorOutputAppSignalId{"OutputAppSignalId"};
		inline static const QString indicatorStaticValue{"StaticValue"};
		inline static const QString indicatorStaticCompareType{"StaticCompareType"};

		inline static const QString indicatorSetpointTypeByLabelCategory{"Type By Label"};
		inline static const QString indicatorSetpointTypeBySignalIdCategory{"Type By Output SignalID"};
		inline static const QString indicatorSetpointTypeStaticCategory{"Type Static"};

		inline static const QString indicatorStartValue{"StartValue"};
		inline static const QString indicatorEndValue{"EndValue"};
		inline static const QString indicatorBarWidth{"BarWidth"};
		inline static const QString indicatorDrawBarRect{"DrawBarRect"};
		inline static const QString indicatorStartAngle{"StartAngle"};
		inline static const QString indicatorSpanAngle{"SpanAngle"};

		inline static const QString timeType{"TimeType"};
		inline static const QString timeTypeToolTip{"Plant: plant time received from LogicModule, System: server time (UTC+0), Local: localized server time (UTC+Time Zone)"};

		inline static const QString indicatorTrendBackColor1st{"BackColor1st"};
		inline static const QString indicatorTrendBackColor2nd{"BackColor2nd"};
		inline static const QString indicatorTrendLaneCount{"LaneCount"};
		inline static const QString indicatorTrendLaneDuration{"LaneDuration"};
		inline static const QString indicatorTrendLaneDurationToolTip{"Lane duration, seconds"};
		inline static const QString indicatorTrendRedrawInterval{"RedrawInterval"};
		inline static const QString indicatorTrendRedrawIntervalToolTip{"Trend image update time, ms"};
		inline static const QString indicatorTrendSamplePeriod{"SamplePeriod"};
		inline static const QString indicatorTrendScaleType{"ScaleType"};
		inline static const QString indicatorTrendViewMode{"ViewMode"};

		inline static const QString indicatorMargingLeft{"MarginLeft"};
		inline static const QString indicatorMargingTop{"MarginTop"};
		inline static const QString indicatorMargingRight{"MarginRight"};
		inline static const QString indicatorMargingBottom{"MarginBottom"};
		inline static const QString indicatorScaleType{"ScaleType"};

		inline static const QString drawSetpoints{"DrawSetpoints"};
		inline static const QString customSetpoints{"CustomSetpoints"};
		inline static const QString trendSignalParams{"TrendSignalParams"};

		// --
		//
		inline static const QString textAnalog{"TextAnalog"};
		inline static const QString textDiscrete0{"TextDiscrete0"};
		inline static const QString textDiscrete1{"TextDiscrete1"};
		inline static const QString textNonValid{"TextNonValid"};
		inline static const QString textValuePropDescription{"$(value) Signal value\n"
															 "$(caption) Signal caption\n"
															 "$(signalid) SignalID (CustomSignalID)\n"
															 "$(appsignalid) AppSignalID (#APPSIGNALID)\n"
															 "$(equipmentid) Signal EquipmentID (LM for internal signals, input/output equipment port for IO signals)\n"
															 "$(units) Signal units\n"};
		//"$(highlimit) High limit\n"
		//"$(lowlimit) Low limit"};

		inline static const QString textVduItemValueDescription = R"($Text to display, may contain placeholders:\n
Example: "Value %i: %E %u" -> "Value YCB10B23: 1.0E-11 kg"\n
%% - Percent\n
%i - CustomAppSignalID\n
%c - Signal caption\n
%v - Signal value\n
%V - Signal value + unit\n
%s - +/- signal value\n
%S - +/- signal value + unit\n
%u - unit\n
%e - Value in exponential form (1.0e-11)\n
%E - Value in exponential form (1.0E-11)\n
%X - Value in HEX (only for integer signal type). m_precision plays the role of the number of zeros to add (00009abc).\n
%X - Value in HEX (only for integer signal type). m_precision plays the role of the number of zeros to add (00009ABC).$)";

		inline static const QString userText{"UserText"};
		inline static const QString userTextPos{"UserTextPos"};

		inline static const QString equipmentIds{"EquipmentIDs"};
		inline static const QString lmDescriptionFile{"LmDescriptionFile"};

		inline static const QString ufbSchemaId{"UFBSchemaID"};
		inline static const QString ufbSchemaVersion{"UFBSchemaVersion"};

		inline static const QString specificProperties{"SpecificProperties"};

		inline static const QString locked{"Locked"};

		inline static const QString checkable{"Checkable"};
		inline static const QString checkedDefault{"CheckedDefault"};
		inline static const QString autoRepeat{"AutoRepeat"};
		inline static const QString autoRepeatDelay{"AutoRepeatDelay"};
		inline static const QString autoRepeatInterval{"AutoRepeatInterval"};
		inline static const QString styleSheet{"StyleSheet"};
		inline static const QString toolTip{"ToolTip"};
		inline static const QString readOnly{"ReadOnly"};

		inline static const QString afterCreate{"AfterCreate"};
		inline static const QString clicked{"Clicked"};
		inline static const QString pressed{"Pressed"};
		inline static const QString released{"Released"};
		inline static const QString toggled{"Toggled"};
		inline static const QString editingFinished{"EditingFinished"};
		inline static const QString returnPressed{"ReturnPressed"};
		inline static const QString textChanged{"TextChanged"};

		inline static const QString alignHorz{"AlignHorz"};
		inline static const QString alignVert{"AlignVert"};

		inline static const QString dataType{"DataType"};
		inline static const QString units{"Units"};

		inline static const QString diagSignalIDs = QStringLiteral("DiagSignalIDs");

		inline static const QString signalIDs = QStringLiteral("SignalIDs");
		inline static const QString appSignalIDs{"AppSignalIDs"};
		inline static const QString appSignalIDsValidator{"^[#]?([A-Za-z\\d_]+((;[#]?)?\\r?(\\n[#]?)?))+$"};
		inline static const QString appSignalIDsOrReferenceValidator{R"(^[#a-zA-Z0-9\$_\(\).;\n\r]*$)"};

		inline static const QString appSignalId{"AppSignalID"};
		inline static const QString appSignalIdValidator{"^[#]?[A-Za-z\\d_]+$"};
		inline static const QString impactAppSignalIDs{"ImpactAppSignalIDs"};
		inline static const QString connectionId{"ConnectionID"};
		inline static const QString signalSource{"SignalSource"};

		inline static const QString coarseAperture{"CoarseAperture"};
		inline static const QString fineAperture{"FineAperture"};
		inline static const QString adaptiveAperture{"AdaptiveAperture"};

		inline static const QString loopbackId{"LoopbackID"};
		inline static const QString packedLogicId{"PackedLogicID"};

		inline static const QString compareType{"CompareType"};

		inline static const QString busTypeId{"BusTypeID"};
		inline static const QString busTypeFileName{"FileName"};
		inline static const QString busAutoSignalPlacement{"AutoSignalPlacement"};
		inline static const QString busEnableManualBusSize{"EnableManualBusSize"};
		inline static const QString busManualBusSize{"ManualBusSize"};
		inline static const QString busSignalId{"SignalID"};
		inline static const QString busInbusOffset{"Offset"};
		inline static const QString busInbusDiscreteBitNo{"BitNo"};
		inline static const QString busInbusAnalogSize{"Size"};
		inline static const QString busInbusAnalogFormat{"Format"};
		inline static const QString busInbusAnalogByteOrder{"ByteOrder"};
		inline static const QString busAnalogLowLimit{"BusSignalLowLimit"};
		inline static const QString busAnalogHightLimit{"BusSignalHighLimit"};
		inline static const QString busInbusAnalogLowLimit{"InbusSignalLowLimit"};
		inline static const QString busInbusAnalogHightLimit{"InbusSignalHighLimit"};

		inline static const QString busSettingCategory{"Bus Settings"};
		inline static const QString busInbusSettingCategory{"InBus Settings (Manual Signal Placement)"};

		inline static const QString commonCategory{"Common"};
		inline static const QString behaviourCategory{"Behaviour"};
		inline static const QString appearanceCategory{"Appearance"};
		inline static const QString functionalCategory{"Functional"};
		inline static const QString textCategory{"Text"};
		inline static const QString colorCategory{"Color"};
		inline static const QString monitorCategory{"Monitor"};
		inline static const QString parametersCategory{"Parameters"};
		inline static const QString controlCategory{"Control"};
		inline static const QString scriptsCategory{"Scripts"};
		inline static const QString apertureCategory{"Aperture"};
		inline static const QString constCategory{"Const"};
		inline static const QString imageCategory{"Image"};
		inline static const QString positionAndSizeCategory{"Position and Size"};
		inline static const QString setpointsCategory{"Setpoints"};

		inline static const QString widgetPropStyleSheet{"Property holds the widget's style sheet.\nThe style sheet contains a textual description of customizations to the widget's style."};
		inline static const QString widgetPropToolTip{"Property holds the widget's tooltip."};
		inline static const QString widgetPropAfterCreate{"Script code to run after the control is created."};

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

		inline static const QString scriptGlobalVariableApp{"app"};
		inline static const QString scriptGlobalVariableView{"view"};
		inline static const QString scriptGlobalVariableTuning{"tuning"};
		inline static const QString scriptGlobalVariableSignals{"signals"};
		inline static const QString scriptGlobalVariableEquipment{"equipment"};
		inline static const QString scriptGlobalVariableLog{"log"};
	};
} // namespace VFrame30
