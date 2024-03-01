#pragma once
#include "../PropertyNames.h"
#include "SchemaItemControl.h"

namespace VFrame30
{
	/*! \class SchemaItemSlider
		\ingroup controlSchemaItems
		\brief This class is used to create a slider schema item.

		SchemaItemSlider class implements vertical or horizontal slider on schema.

		<b>Event handlers</b>

		To customize item's appearance and behaviour, event handler code is placed to following properties of the schema item using RPCT:

		- <b>PreDrawScript</b> contains pre-draw event handler code. Pre-draw event is generated each time before item is redrawn;<br>
		- <b>AfterCreate</b> contains creation event handler code. This event is generated when schema item is created;<br>
		- <b>SliderMoved</b> contains handler code for event when sliderDown is true and the slider moves. This usually happens when the user is dragging the slider. The value is the new slider position. This signal is emitted even when tracking is turned off;<br>
		- <b>SliderPressed</b> contains handler code for event when the user releases the slider with the mouse;<br>
		- <b>SliderReleased</b> contains handler code for event when the user presses the slider with the mouse;<br>
		- <b>ValueChanged</b> contains handler code for event when the slider value has changed, with the new slider value as argument. Typically the user should use this script to obtain a new value of the slider;<br>

		<b>ValueChanged</b> event handler function porotype:
		\code
		(function(schemaItem, sliderWidget, value)
		{
			log.writeText("SliderValue: " + value); // Property sliderWidget.value also can be used.
		})
		\endcode

		Parameters:<br>
		<i>schemaItem</i> - a handle to schema item, type: SchemaItemSlider.<br>

		<b>AfterCreate</b>, <b>SliderMoved</b>, <b>SliderPressed</b>, <b>SliderReleased</b> <b>ValueChanged</b> event handlers function porotypes:

		\code
		function(schemaItem, sliderWidget, value)
		\endcode

		Parameters:<br>
		<i>schemaItem</i> - a handle to schema item, type: SchemaItemSlider;<br>
		<i>sliderWidget</i> - a handle to a slider widget, type: SliderWidget;<br>
		<i>value</i> - slider current value, type: integer.<br>

		<b>Accessing slider widget</b>

		Slider widget is used to read or modify slider control properties. It is implemented by SliderWidget class.

		Widget can be accessed by <i>sliderWidget</i> parameter, requested by <i>findWidget</i> function of ScriptSchemaView class, or get by property <i>SchemaItemControl.widget</i>.
	*/
	class SchemaItemSlider final : public SchemaItemControl
	{
		Q_OBJECT

	public:
		SchemaItemSlider(void);
		explicit SchemaItemSlider(SchemaUnit unit);
		SchemaItemSlider(const SchemaItemSlider&) = delete;
		SchemaItemSlider(SchemaItemSlider&&) = delete;
		virtual ~SchemaItemSlider(void) = default;

		SchemaItemSlider& operator=(const SchemaItemSlider&) = delete;
		SchemaItemSlider& operator=(SchemaItemSlider&&) = delete;

		// Draw Functions
		//
	public:
		virtual void draw(CDrawParam* drawParam) const override;

	private:
		void drawSliderControl(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Methods
		//
	public:
		virtual QWidget* createWidgetImpl(QWidget* parent, bool editMode, double zoom) override;

		virtual void updateWidgetProperties(QWidget* widget, bool editMode) const override;

	protected:
		virtual void afterCreateImpl(QWidget* control) override;

	protected slots:
		void sliderMoved(int value);
		void sliderPressed();
		void sliderReleased();
		void valueChanged(int value);

		void runEventScript(QString scriptName, const QString& script, QJSValue& js, QSlider* sliderWidget);
		void runEventScript(QString scriptName, QJSValue& evaluatedJs, QSlider* sliderWidget, bool allowMessageBox);

		// Properties and Data
		//
	public:
		Qt::Orientation orientation() const;
		void setOrientation(Qt::Orientation orientation);

		bool invertedAppearance() const;
		void setInvertedAppearance(bool inverted);

		bool invertedControls() const;
		void setInvertedControls(bool inverted);

		bool enableMouseWheel() const;
		void setEnableMouseWheel(bool enable);

		int maximum() const;
		void setMaximum(int max);

		int minimum() const;
		void setMinimum(int min);

		int pageStep() const;
		void setPageStep(int step);

		int singleStep() const;
		void setSingleStep(int step);

		bool tracking() const;
		void setTracking(bool track);

		int tickInterval() const;
		void setTickInterval(int interval);

		QSlider::TickPosition tickPosition() const;
		void setTickPosition(QSlider::TickPosition position);

		int defaultValue() const;
		void setDefaultValue(int value);

		QString scriptAfterCreate() const;
		void setScriptAfterCreate(const QString& value);

		QString scriptSliderMoved() const;
		void setScriptSliderMoved(const QString& value);

		QString scriptSliderPressed() const;
		void setScriptSliderPressed(const QString& value);

		QString scriptSliderReleased() const;
		void setScriptSliderReleased(const QString& value);

		QString scriptValueChanged() const;
		void setScriptValueChanged(const QString& value);

	private:
		Qt::Orientation m_orientation = Qt::Vertical;
		bool m_invertedAppearance = false;
		bool m_invertedControls = false;
		bool m_enableMouseWheel = false;

		int m_maximum = 99;
		int m_minimum = 0;

		int m_pageStep = 10;
		int m_singleStep = 1;
		bool m_tracking = true;
		int m_tickInterval = 0;
		QSlider::TickPosition m_tickPosition = QSlider::NoTicks;

		int m_defaultValue = 0;

		QString m_scriptAfterCreate = PropertyNames::sliderDefaultEventScript;
		QString m_scriptSliderMoved = PropertyNames::sliderDefaultEventScript;
		QString m_scriptSliderPressed = PropertyNames::sliderDefaultEventScript;
		QString m_scriptSliderReleased = PropertyNames::sliderDefaultEventScript;
		QString m_scriptValueChanged = PropertyNames::sliderDefaultEventScript;

		// Evaluated scripts
		//
		QJSValue m_jsAfterCreate;
		QJSValue m_jsSliderMoved;
		QJSValue m_jsSliderPressed;
		QJSValue m_jsSliderReleased;
		QJSValue m_jsValueChanged;
	};

	// Warning! SliderWidget class is used only for documentation generated by Doxygen and should not be used in code!
	//

	/*! \class SliderWidget
		\ingroup widgets
		\brief This class is used to control appearance and behaviour of a slider widget.
	*/
	class SliderWidget : public QSlider
	{
		Q_OBJECT

		/// \brief This property holds the orientation of the slider.
		///
		/// The orientation can be Qt::Vertical or Qt::Horizontal.
		/// Qt::Horizontal - 0x1
		/// Qt::Vertical - 0x2
		Q_PROPERTY(int orientation READ someIntProperty WRITE setSomeIntProperty)

		/// \brief This property holds whether or not a slider shows its values inverted.
		///
		/// If this property is false, the minimum and maximum will be shown in its classic position for the inherited widget.
		/// If the value is true, the minimum and maximum appear at their opposite location.
		Q_PROPERTY(bool invertedAppearance READ someBoolProperty WRITE setSomeBoolProperty)

		/// \brief This property holds whether or not the slider inverts its wheel and key events.
		///
		/// If this property is false, scrolling the mouse wheel "up" and using keys like page up will increase the slider's value towards its maximum.
		/// Otherwise pressing page up will move value towards the slider's minimum.
		Q_PROPERTY(bool invertedControls READ someBoolProperty WRITE setSomeBoolProperty)

		/// \brief This property holds the slider's maximum value.
		///
		/// When setting this property, the minimum is adjusted if necessary to ensure that the range remains valid.
		/// Also the slider's current value is adjusted to be within the new range.
		Q_PROPERTY(int maximum READ someIntProperty WRITE setSomeIntProperty)

		/// \brief This property holds the slider's minimum value.
		///
		/// When setting this property, the maximum is adjusted if necessary to ensure that the range remains valid.
		/// Also the slider's current value is adjusted to be within the new range.
		Q_PROPERTY(int minimum READ someIntProperty WRITE setSomeIntProperty)

		/// \brief This property holds the page step.
		///
		/// The larger of two natural steps that a slider provides and typically corresponds to the user pressing PageUp or PageDown.
		Q_PROPERTY(int pageStep READ someIntProperty WRITE setSomeIntProperty)

		/// \brief This property holds the single step.
		///
		/// The smaller of two natural steps that a sliders provides and typically corresponds to the user pressing an arrow key.
		Q_PROPERTY(int singleStep READ someIntProperty WRITE setSomeIntProperty)

		/// \brief This property holds whether slider tracking is enabled.
		///
		/// If tracking is enabled, the slider emits the valueChanged() signal while the slider is being dragged.
		/// If tracking is disabled, the slider emits the valueChanged() signal only when the user releases the slider.
		Q_PROPERTY(bool tracking READ someBoolProperty WRITE setSomeBoolProperty)

		/// \brief This property holds the interval between tickmarks.
		///
		/// This is a value interval, not a pixel interval. If it is 0, the slider will choose between singleStep and pageStep.
		Q_PROPERTY(int tickInterval READ someIntProperty WRITE setSomeIntProperty)

		/// \brief This property holds the tickmark position for this slider.
		///
		/// The valid values are described by the QSlider::TickPosition enum.
		/// QSlider::NoTicks - 0 - Do not draw any tick marks.
		/// QSlider::TicksBothSides - 3 - Draw tick marks on both sides of the groove.
		/// QSlider::TicksAbove - 1 - Draw tick marks above the (horizontal) slider.
		/// QSlider::TicksBelow - 2 - Draw tick marks below the (horizontal) slider.
		/// QSlider::TicksLeft - TicksAbove - Draw tick marks to the left of the (vertical) slider.
		/// QSlider::TicksRight - TicksBelow - Draw tick marks to the right of the (vertical) slider.
		Q_PROPERTY(int tickPosition READ someIntProperty WRITE setSomeIntProperty)

		/// \brief This property holds the slider's current value.
		///
		/// The slider forces the value to be within the legal range: minimum <= value <= maximum.
		Q_PROPERTY(int value READ someIntProperty WRITE setSomeIntProperty)

		/// \brief This property holds the widget's style sheet
		// The style sheet contains a textual description of customizations to the widget's style, as described in the <a href="https://doc.qt.io/qt-6/stylesheet.html">Qt Style Sheets</a> document.
		Q_PROPERTY(QString styleSheet READ someStringProperty WRITE setSomeStringProperty)

		/// \brief This property holds the widget's tooltip
		Q_PROPERTY(QString toolTip READ someStringProperty WRITE setSomeStringProperty)

		/// \brief Specifies how long time the tooltip will be displayed, in milliseconds.
		Q_PROPERTY(int toolTipDuration READ someIntProperty WRITE setSomeIntProperty)

		// Empty property getters and setters.
		//
		bool someBoolProperty() const { return false; }
		void setSomeBoolProperty(bool value) { Q_UNUSED(value); }

		int someIntProperty() const { return 0; }
		void setSomeIntProperty(int value) { Q_UNUSED(value); }

		QString someStringProperty() const { return {}; }
		void setSomeStringProperty(QString value) { Q_UNUSED(value); }
	};
} // namespace VFrame30
