#pragma once

namespace VFrame30
{
	class PropertyNames
	{
	public:
		PropertyNames() = delete;

	public:
		// clang-format off

		inline static const QString schemaId = QStringLiteral("SchemaID");

		inline static const QString top = QStringLiteral("Top");
		inline static const QString left = QStringLiteral("Left");
		inline static const QString width = QStringLiteral("Width");
		inline static const QString height = QStringLiteral("Height");

		inline static const QString lowLimit = QStringLiteral("LowLimit");
		inline static const QString highLimit = QStringLiteral("HighLimit");

		inline static const QString acceptClick = QStringLiteral("AcceptClick");
		inline static const QString clickScript = QStringLiteral("ClickScript");
		inline static const QString preDrawScript = QStringLiteral("PreDrawScript");
		inline static const QString preDrawScriptDefault = QStringLiteral("(function(schemaItem) {\n})");
		inline static const QString onShowScript = QStringLiteral("OnShowScript");
		inline static const QString commented = QStringLiteral("Commented");
		inline static const QString objectName = QStringLiteral("ObjectName");
		inline static const QString name = QStringLiteral("Name");
		inline static const QString guid = QStringLiteral("Uuid");

		inline static const QString fontName = QStringLiteral("FontName");
		inline static const QString fontSize = QStringLiteral("FontSize");
		inline static const QString fontBold = QStringLiteral("FontBold");
		inline static const QString fontItalic = QStringLiteral("FontItalic");

		inline static const QString color = QStringLiteral("Color");

		inline static const QString rotationPoint = QStringLiteral("RotationPoint");
		inline static const QString rotationPointDescription = QStringLiteral("The point around which the item is rotated.");
		inline static const QString angle = QStringLiteral("Angle");
		inline static const QString angleDescription = QStringLiteral("Item rotation angle (rotates around RotationPoint).");

		inline static const QString type = QStringLiteral("Type");
		inline static const QString value = QStringLiteral("Value");
		inline static const QString valueInteger = QStringLiteral("ValueInteger");
		inline static const QString valueFloat = QStringLiteral("ValueFloat");
		inline static const QString valueDiscrete = QStringLiteral("ValueDiscrete");
		inline static const QString valueFormat = QStringLiteral("ValueFormat");
		inline static const QString precision = QStringLiteral("Precision");
		inline static const QString precisionPropText = QStringLiteral("Number of decimals after period, -1: take value from the signal description");
		inline static const QString analogFormat = QStringLiteral("AnalogFormat");
		inline static const QString analogFormatDescription = QStringLiteral(
			"Analog format for the value.\n"
			"Supported values:\n"
			"e_9e (0x65/101) scientific notation with lowercase exponent, for example 1.2e+03.\n"
			"E_9E (0x45/69) scientific notation with uppercase exponent, for example 1.2E+03.\n"
			"f_9 (0x66/102) fixed-point decimal format, for example 1234.56.\n"
			"g_9_or_9e (0x67/103) shortest representation using either fixed-point or scientific notation with lowercase exponent.\n"
			"G_9_or_9E (0x47/71)  shortest representation using either fixed-point or scientific notation with uppercase exponent."
		);
		inline static const QString customText = QStringLiteral("CustomText");
		inline static const QString columnCount = QStringLiteral("ColumnCount");
		inline static const QString pinCount = QStringLiteral("PinCount");
		inline static const QString showValidityPin = QStringLiteral("ValidityPin");

		inline static const QString inputs = QStringLiteral("Inputs");
		inline static const QString outputs = QStringLiteral("Outputs");

		inline static const QString lineCapSize = QStringLiteral("LineCapSize");
		inline static const QString lineCapFactor = QStringLiteral("LineCapFactor");
		inline static const QString lineCapStart = QStringLiteral("LineCapStart");
		inline static const QString lineCapEnd = QStringLiteral("LineCapEnd");
		inline static const QString lineStyle = QStringLiteral("LineStyle");
		inline static const QString lineStyleCap = QStringLiteral("LineStyleCap");
		inline static const QString lineColor = QStringLiteral("LineColor");
		inline static const QString lineWeight = QStringLiteral("LineWeight");
		inline static const QString fillColor = QStringLiteral("FillColor");
		inline static const QString backgroundColor = QStringLiteral("BackgroundColor");
		inline static const QString backColor = QStringLiteral("BackColor");
		inline static const QString fill = QStringLiteral("Fill");
		inline static const QString drawRect = QStringLiteral("DrawRect");
		inline static const QString textFormat = QStringLiteral("TextFormat");
		inline static const QString textFormatDescription = QStringLiteral("PlainText - Manual formatting.\nMarkdown - Markdown formatting, supports GitHub-style Markdown.\nHtmlSubset - HTML-formatted text in the html string. Support of the limited HTML Subset.");
		inline static const QString textColor = QStringLiteral("TextColor");
		inline static const QString text = QStringLiteral("Text");
		inline static const QString wordWrap = QStringLiteral("WordWrap");
		inline static const QString placeholderText = QStringLiteral("PlaceholderText");
		inline static const QString tags = QStringLiteral("Tags");
		inline static const QString label = QStringLiteral("Label");
		inline static const QString labelPos = QStringLiteral("LabelPos");
		inline static const QString caption = QStringLiteral("Caption");
		inline static const QString userCaption = QStringLiteral("UserCaption");
		inline static const QString maxLength = QStringLiteral("MaxLength");
		inline static const QString multiLine = QStringLiteral("MultiLine");

		inline static const QString allowScale = QStringLiteral("AllowScale");
		inline static const QString keepAspectRatio = QStringLiteral("KeepAspectRatio");
		inline static const QString imageId = QStringLiteral("ImageID");
		inline static const QString image = QStringLiteral("Image");
		inline static const QString images = QStringLiteral("Images");
		inline static const QString currentImageId = QStringLiteral("CurrentImageID");

		inline static const QString svg = QStringLiteral("Svg");
		inline static const QString svgScaleFactor = QStringLiteral("SvgScaleFactor");
		inline static const QString svgScaleFactorDescription = QStringLiteral("Multiplier for SVG image size.");

		inline static const QString orientation = QStringLiteral("Orientation");
		
		inline static const QString sliderInvertedAppearance = QStringLiteral("InvertedAppearance");
		inline static const QString sliderInvertedAppearanceToolTip = QStringLiteral("This property holds whether or not a slider shows its values inverted.\nIf this property is false, the minimum and maximum will be shown in its classic position for the inherited widget.If the value is true, the minimum and maximum appear at their opposite location.");
		inline static const QString sliderInvertedControls = QStringLiteral("InvertedControls");
		inline static const QString sliderInvertedControlsToolTip = QStringLiteral("This property holds whether or not the slider inverts its wheel and key events.\nIf this property is false, scrolling the mouse wheel \"up\" and using keys like page up will increase the slider's value towards its maximum. Otherwise pressing page up will move value towards the slider's minimum.");
		inline static const QString sliderEnableMouseWheel = QStringLiteral("EnableMouseWheel");
		inline static const QString sliderEnableMouseWheelToolTip = QStringLiteral("If `true` then mouse wheel will affect slider value (note that zoom in/out by mouse wheel then will not work over the slider). If `false` then mouse wheel is ignored and event is propagated to the view.");
		inline static const QString sliderMaximum = QStringLiteral("Maximum");
		inline static const QString sliderMaximumToolTip = QStringLiteral("This property holds the slider's maximum value.\nWhen setting this property, the minimum is adjusted if necessary to ensure that the range remains valid.Also the slider's current value is adjusted to be within the new range.");
		inline static const QString sliderMinimum = QStringLiteral("Minimum");
		inline static const QString sliderMinimumToolTip = QStringLiteral("This property holds the slider's minimum value.\nWhen setting this property, the maximum is adjusted if necessary to ensure that the range remains valid.Also the slider's current value is adjusted to be within the new range.");
		inline static const QString sliderPageStep = QStringLiteral("PageStep");
		inline static const QString sliderPageStepToolTip = QStringLiteral("This property holds the page step.\nThe larger of two natural steps that a slider provides and typically corresponds to the user pressing PageUp or PageDown.");
		inline static const QString sliderSingleStep = QStringLiteral("SingleStep");
		inline static const QString sliderSingleStepToolTip = QStringLiteral("This property holds the single step.\nThe smaller of two natural steps that a slider provides and typically corresponds to the user pressing an arrow key.");
		inline static const QString sliderTracking = QStringLiteral("Tracking");
		inline static const QString sliderTrackingToolTip = QStringLiteral("This property holds whether slider tracking is enabled.\nIf tracking is enabled, the slider emits the valueChanged() signal while the slider is being dragged.If tracking is disabled, the slider emits the valueChanged() signal only when the user releases the slider.");
		inline static const QString sliderTickInterval = QStringLiteral("TickInterval");
		inline static const QString sliderTickIntervalToolTip = QStringLiteral("This property holds the interval between tickmarks.\nThis is a value interval, not a pixel interval.If it is 0, the slider will choose between singleStep and pageStep.");
		inline static const QString sliderTickPosition = QStringLiteral("TickPosition");
		inline static const QString sliderTickPositionToolTip = QStringLiteral("");
		inline static const QString sliderDefaultValue = QStringLiteral("DefaultValue");
		inline static const QString sliderDefaultEventScript = QStringLiteral("(function(schemaItem, sliderWidget, value)\n{\n})");

		inline static const QString sliderMoved = QStringLiteral("SliderMoved");
		inline static const QString sliderMovedToolTip = QStringLiteral("This signal is emitted when sliderDown is true and the slider moves. This usually happens when the user is dragging the slider. The value is the new slider position.\nThis signal is emitted even when tracking is turned off.");
		inline static const QString sliderPressed = QStringLiteral("SliderPressed");
		inline static const QString sliderPressedToolTip = QStringLiteral("This signal is emitted when the user presses the slider with the mouse, or programmatically when setSliderDown(true) is called.");
		inline static const QString sliderReleased = QStringLiteral("SliderReleased");
		inline static const QString sliderReleasedToolTip = QStringLiteral("This signal is emitted when the user releases the slider with the mouse, or programmatically when setSliderDown(false) is called.");
		inline static const QString sliderValueChanged = QStringLiteral("ValueChanged");
		inline static const QString sliderValueChangedToolTip = QStringLiteral("This signal is emitted when the slider value has changed, with the new slider value as argument.");

		inline static const QString drawGrid = QStringLiteral("DrawGrid");
		inline static const QString drawGridForAllBars = QStringLiteral("DrawGridForAllBars");
		inline static const QString drawGridValues = QStringLiteral("DrawGridValues");
		inline static const QString drawGridValueForAllBars = QStringLiteral("DrawGridValueForAllBars");
		inline static const QString drawGridValueUnits = QStringLiteral("DrawGridValueUnits");
		inline static const QString linearGridMainStep = QStringLiteral("LinearGridMainStep");
		inline static const QString linearGridSmallStep = QStringLiteral("LinearGridSmallStep");
		inline static const QString logarithmicGridMainStep = QStringLiteral("LogarithmicGridMainStep");
		inline static const QString logarithmicGridSmallStep = QStringLiteral("LogarithmicGridSmallStep");

		inline static const QString indicatorSignalColors = QStringLiteral("SignalColors");
		inline static const QString indicatorType = QStringLiteral("IndicatorType");
		inline static const QString indicatorSettings = QStringLiteral("IndicatorSettings");
		inline static const QString indicator = QStringLiteral("Indicator");

		inline static const QString indicatorSetpointType = QStringLiteral("SetpointType");
		inline static const QString indicatorColorSource = QStringLiteral("ColorSource");
		inline static const QString indicatorSchemaItemLabel = QStringLiteral("SchemaItemLabel");
		inline static const QString indicatorOutputAppSignalId = QStringLiteral("OutputAppSignalId");
		inline static const QString indicatorStaticValue = QStringLiteral("StaticValue");
		inline static const QString indicatorStaticCompareType = QStringLiteral("StaticCompareType");

		inline static const QString indicatorSetpointTypeByLabelCategory = QStringLiteral("Type By Label");
		inline static const QString indicatorSetpointTypeBySignalIdCategory = QStringLiteral("Type By Output SignalID");
		inline static const QString indicatorSetpointTypeStaticCategory = QStringLiteral("Type Static");

		inline static const QString indicatorStartValue = QStringLiteral("StartValue");
		inline static const QString indicatorEndValue = QStringLiteral("EndValue");
		inline static const QString indicatorBarWidth = QStringLiteral("BarWidth");
		inline static const QString indicatorDrawBarRect = QStringLiteral("DrawBarRect");
		inline static const QString indicatorStartAngle = QStringLiteral("StartAngle");
		inline static const QString indicatorSpanAngle = QStringLiteral("SpanAngle");

		inline static const QString timeType = QStringLiteral("TimeType");
		inline static const QString timeTypeToolTip = QStringLiteral("Plant: plant time received from LogicModule, System: server time (UTC+0), Local: localized server time (UTC+Time Zone)");

		inline static const QString indicatorTrendBackColor1st = QStringLiteral("BackColor1st");
		inline static const QString indicatorTrendBackColor2nd = QStringLiteral("BackColor2nd");
		inline static const QString indicatorTrendShowSignalIds = QStringLiteral("ShowSignalIDs");
		inline static const QString indicatorTrendShowSignalCaptions = QStringLiteral("ShowSignalCaptions");
		inline static const QString indicatorTrendShowSignalScales = QStringLiteral("ShowSignalScales");
		inline static const QString indicatorTrendShowTimeLabels = QStringLiteral("ShowTimeLabels");
		inline static const QString indicatorTrendShowDateLabels = QStringLiteral("ShowDateLabels");
		inline static const QString indicatorTrendIndentLeft = QStringLiteral("IndentLeft");
		inline static const QString indicatorTrendIndentRight = QStringLiteral("IndentRight");
		inline static const QString indicatorTrendIndentTop = QStringLiteral("IndentTop");
		inline static const QString indicatorTrendIndentBottom = QStringLiteral("IndentBottom");
		inline static const QString indicatorTrendIndentDescription = QStringLiteral("Indent; if set to -1, the auto indent is used.");
		inline static const QString indicatorTrendLaneCount = QStringLiteral("LaneCount");
		inline static const QString indicatorTrendLaneDuration = QStringLiteral("LaneDuration");
		inline static const QString indicatorTrendLaneDurationToolTip = QStringLiteral("Lane duration, seconds");
		inline static const QString indicatorTrendRedrawInterval = QStringLiteral("RedrawInterval");
		inline static const QString indicatorTrendRedrawIntervalToolTip = QStringLiteral("Trend image update time, ms");
		inline static const QString indicatorTrendSamplePeriod = QStringLiteral("SamplePeriod");
		inline static const QString indicatorTrendScaleType = QStringLiteral("ScaleType");
		inline static const QString indicatorTrendViewMode = QStringLiteral("ViewMode");

		inline static const QString indicatorMarginLeft = QStringLiteral("MarginLeft");
		inline static const QString indicatorMarginTop = QStringLiteral("MarginTop");
		inline static const QString indicatorMarginRight = QStringLiteral("MarginRight");
		inline static const QString indicatorMarginBottom = QStringLiteral("MarginBottom");
		inline static const QString indicatorScaleType = QStringLiteral("ScaleType");

		inline static const QString drawSetpoints = QStringLiteral("DrawSetpoints");
		inline static const QString customSetpoints = QStringLiteral("CustomSetpoints");
		inline static const QString trendSignalParams = QStringLiteral("TrendSignalParams");

		// --
		//
		inline static const QString textAnalog = QStringLiteral("TextAnalog");
		inline static const QString textDiscrete0 = QStringLiteral("TextDiscrete0");
		inline static const QString textDiscrete1 = QStringLiteral("TextDiscrete1");
		inline static const QString textNonValid = QStringLiteral("TextNonValid");
		inline static const QString textValuePropDescription = QStringLiteral(R"($(value) - Signal value
$(value_[e/E]) - Signal value format as [-]9.9e[+|-]999/, lowercase/uppercase accordingly.
$(value_f) - Signal value format as [-]9.9, same as  $(value).
$(value_[g/G]) - Signal value format as 'e' or 'f', whichever is the most concise, lowercase/uppercase accordingly.
$(value_[hex/HEX]) - Signal value shown in hex, precision determines the number of digits, lowercase/uppercase accordingly.
$(value_[stag/STAG]) - Signal value formatted according to signal tag (view_linear - f/F, view_log10 or view_period - e/E).
$(value_[itag/ITAG]) - Signal value formatted according to schema item tag (view_linear - f/F, view_log10 or view_period - e/E).
$(caption) - Caption.
$(signalid) - SignalID (CustomSignalID).
$(appsignalid) - AppSignalID (#APPSIGANLID).
$(equipmentid) - Signal EquipmentID (LM for internal signals, input/output equipment port for IO signals).
$(units) - Signal units.
$(AppSignalParam.[property]) - AppSignal parameter, example: AppSignalParam.tuningDefaultValue.
$(AppSignalState.[property]) - AppSignal state, example: AppSignalState.blocked.)");

		inline static const QString textVduItemValueDescription = QStringLiteral(R"(Text to display, may contain placeholders:
Example: "Value %i: %E %u" -> "Value YCB10B23: 1.0E-11 kg"
%% - Percent
%i - CustomAppSignalID
%c - Signal caption
%v - Signal value
%V - Signal value + units
%s - +/- signal value
%S - +/- signal value + units
%u - units
%e - Value in exponential form (1.0e-11)
%E - Value in exponential form (1.0E-11)
%X - Value in HEX (only for integer signal type). m_precision plays the role of the number of zeros to add (00009abc).
%X - Value in HEX (only for integer signal type). m_precision plays the role of the number of zeros to add (00009ABC).)");

		inline static const QString userText = QStringLiteral("UserText");
		inline static const QString userTextPos = QStringLiteral("UserTextPos");

		inline static const QString equipmentIds = QStringLiteral("EquipmentIDs");
		inline static const QString lmDescriptionFile = QStringLiteral("LmDescriptionFile");

		inline static const QString ufbSchemaId = QStringLiteral("UFBSchemaID");
		inline static const QString ufbSchemaVersion = QStringLiteral("UFBSchemaVersion");

		inline static const QString specificProperties = QStringLiteral("SpecificProperties");

		inline static const QString locked = QStringLiteral("Locked");

		inline static const QString checkable = QStringLiteral("Checkable");
		inline static const QString checkedDefault = QStringLiteral("CheckedDefault");
		inline static const QString autoRepeat = QStringLiteral("AutoRepeat");
		inline static const QString autoRepeatDelay = QStringLiteral("AutoRepeatDelay");
		inline static const QString autoRepeatInterval = QStringLiteral("AutoRepeatInterval");
		inline static const QString styleSheet = QStringLiteral("StyleSheet");
		inline static const QString toolTip = QStringLiteral("ToolTip");
		inline static const QString readOnly = QStringLiteral("ReadOnly");

		inline static const QString afterCreate = QStringLiteral("AfterCreate");
		inline static const QString clicked = QStringLiteral("Clicked");
		inline static const QString pressed = QStringLiteral("Pressed");
		inline static const QString released = QStringLiteral("Released");
		inline static const QString toggled = QStringLiteral("Toggled");
		inline static const QString editingFinished = QStringLiteral("EditingFinished");
		inline static const QString returnPressed = QStringLiteral("ReturnPressed");
		inline static const QString textChanged = QStringLiteral("TextChanged");

		inline static const QString alignHorz = QStringLiteral("AlignHorz");
		inline static const QString alignVert = QStringLiteral("AlignVert");

		inline static const QString dataType = QStringLiteral("DataType");
		inline static const QString units = QStringLiteral("Units");

		inline static const QString diagSignalIDs = QStringLiteral("DiagSignalIDs");

		inline static const QString signalIDs = QStringLiteral("SignalIDs");
		inline static const QString signalId = QStringLiteral("SignalID");
		inline static const QString appSignalID = QStringLiteral("AppSignalID");
		inline static const QString validityAppSignalID = QStringLiteral("ValidityAppSignalID");
		inline static const QString appSignalIDs = QStringLiteral("AppSignalIDs");
		inline static const QString appSignalIDsValidator = QStringLiteral("^[#]?([A-Za-z\\d_]+((;[#]?)?\\r?(\\n[#]?)?))+$");
		inline static const QString appSignalIDsOrReferenceValidator = QStringLiteral(R"(^[#a-zA-Z0-9\$_\(\).;\n\r]*$)");

		inline static const QString signalType = QStringLiteral("SignalType");

		inline static const QString appSignalIdValidator = QStringLiteral("^[#]?[A-Za-z\\d_]+$");
		inline static const QString impactAppSignalIDs = QStringLiteral("ImpactAppSignalIDs");
		inline static const QString connectionId = QStringLiteral("ConnectionID");
		inline static const QString signalSource = QStringLiteral("SignalSource");

		inline static const QString coarseAperture = QStringLiteral("CoarseAperture");
		inline static const QString fineAperture = QStringLiteral("FineAperture");
		inline static const QString adaptiveAperture = QStringLiteral("AdaptiveAperture");

		inline static const QString loopbackId = QStringLiteral("LoopbackID");
		inline static const QString packedLogicId = QStringLiteral("PackedLogicID");

		inline static const QString compareType = QStringLiteral("CompareType");

		inline static const QString busTypeId = QStringLiteral("BusTypeID");
		inline static const QString busTypeFileName = QStringLiteral("FileName");
		inline static const QString busAutoSignalPlacement = QStringLiteral("AutoSignalPlacement");
		inline static const QString busEnableManualBusSize = QStringLiteral("EnableManualBusSize");
		inline static const QString busManualBusSize = QStringLiteral("ManualBusSize");
		inline static const QString busSignalId = QStringLiteral("SignalID");
		inline static const QString busInbusOffset = QStringLiteral("Offset");
		inline static const QString busInbusDiscreteBitNo = QStringLiteral("BitNo");
		inline static const QString busInbusAnalogSize = QStringLiteral("Size");
		inline static const QString busInbusAnalogFormat = QStringLiteral("Format");
		inline static const QString busInbusAnalogByteOrder = QStringLiteral("ByteOrder");
		inline static const QString busAnalogLowLimit = QStringLiteral("BusSignalLowLimit");
		inline static const QString busAnalogHighLimit = QStringLiteral("BusSignalHighLimit");
		inline static const QString busInbusAnalogLowLimit = QStringLiteral("InbusSignalLowLimit");
		inline static const QString busInbusAnalogHighLimit = QStringLiteral("InbusSignalHighLimit");

		inline static const QString busSettingCategory = QStringLiteral("Bus Settings");
		inline static const QString busInbusSettingCategory = QStringLiteral("InBus Settings (Manual Signal Placement)");

		inline static const QString commonCategory = QStringLiteral("Common");
		inline static const QString actuatorCategory = QStringLiteral("Actuator");
		inline static const QString behaviourCategory = QStringLiteral("Behaviour");
		inline static const QString appearanceCategory = QStringLiteral("Appearance");
		inline static const QString functionalCategory = QStringLiteral("Functional");
		inline static const QString textCategory = QStringLiteral("Text");
		inline static const QString colorCategory = QStringLiteral("Color");
		inline static const QString monitorCategory = QStringLiteral("Monitor");
		inline static const QString parametersCategory = QStringLiteral("Parameters");
		inline static const QString controlCategory = QStringLiteral("Control");
		inline static const QString scriptsCategory = QStringLiteral("Scripts");
		inline static const QString apertureCategory = QStringLiteral("Aperture");
		inline static const QString constCategory = QStringLiteral("Const");
		inline static const QString imageCategory = QStringLiteral("Image");
		inline static const QString positionAndSizeCategory = QStringLiteral("Position and Size");
		inline static const QString setpointsCategory = QStringLiteral("Setpoints");

		inline static const QString widgetPropStyleSheet = QStringLiteral("Property holds the widget's style sheet.\nThe style sheet contains a textual description of customizations to the widget's style.");
		inline static const QString widgetPropToolTip = QStringLiteral("Property holds the widget's tooltip.");
		inline static const QString widgetPropAfterCreate = QStringLiteral("Script code to run after the control is created.");

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

		inline static const QString scriptGlobalVariableApp = QStringLiteral("app");
		inline static const QString scriptGlobalVariableView = QStringLiteral("view");
		inline static const QString scriptGlobalVariableTuning = QStringLiteral("tuning");
		inline static const QString scriptGlobalVariableSignals = QStringLiteral("signals");
		inline static const QString scriptGlobalVariableEquipment = QStringLiteral("equipment");
		inline static const QString scriptGlobalVariableLog = QStringLiteral("log");

		inline static const QString ActuatorTypeId = QStringLiteral("ActuatorTypeID");
		inline static const QString acmPreset = QStringLiteral("AcmPreset");
		inline static const QString descriptionFile = QStringLiteral("DescriptionFile");
		inline static const QString lmNumber = QStringLiteral("LmNumber");
		inline static const QString subsystemId = QStringLiteral("SubsystemID");
		inline static const QString excludeFromBuild = QStringLiteral("ExcludeFromBuild");

		// clang-format on
	};
} // namespace VFrame30
