#include "SchemaItemValue.h"
#include "SchemaView.h"
#include "MacrosExpander.h"
#include "PropertyNames.h"
#include "DrawParam.h"
#include "../lib/AppSignal.h"
#include "../lib/AppSignalManager.h"
#include "../lib/Tuning/TuningSignalState.h"


namespace VFrame30
{
	SchemaItemValue::SchemaItemValue(void) :
		SchemaItemValue(SchemaUnit::Inch)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	SchemaItemValue::SchemaItemValue(SchemaUnit unit)
	{
		Property* p = nullptr;

		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::lineWeight, PropertyNames::appearanceCategory, true, SchemaItemValue::lineWeight, SchemaItemValue::setLineWeight);

		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::lineColor, PropertyNames::appearanceCategory, true, SchemaItemValue::lineColor, SchemaItemValue::setLineColor);
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::fillColor, PropertyNames::appearanceCategory, true, SchemaItemValue::fillColor, SchemaItemValue::setFillColor)

		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::drawRect, PropertyNames::appearanceCategory, true, SchemaItemValue::drawRect, SchemaItemValue::setDrawRect);

		// Color Category
		//
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::fillColorNonValid0, PropertyNames::colorCategory, true, SchemaItemValue::fillColorNonValid0, SchemaItemValue::setFillColorNonValid0);
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::fillColorNonValid1, PropertyNames::colorCategory, true, SchemaItemValue::fillColorNonValid1, SchemaItemValue::setFillColorNonValid1);
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::textColorNonValid0, PropertyNames::colorCategory, true, SchemaItemValue::textColorNonValid0, SchemaItemValue::setTextColorNonValid0);
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::textColorNonValid1, PropertyNames::colorCategory, true, SchemaItemValue::textColorNonValid1, SchemaItemValue::setTextColorNonValid1);

//		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::fillColorOverflow0, PropertyNames::colorCategory, true, SchemaItemValue::fillColorOverflow0, SchemaItemValue::setFillColorOverflow0);
//		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::fillColorOverflow1, PropertyNames::colorCategory, true, SchemaItemValue::fillColorOverflow1, SchemaItemValue::setFillColorOverflow1);
//		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::textColorOverflow0, PropertyNames::colorCategory, true, SchemaItemValue::textColorOverflow0, SchemaItemValue::setTextColorOverflow0);
//		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::textColorOverflow1, PropertyNames::colorCategory, true, SchemaItemValue::textColorOverflow1, SchemaItemValue::setTextColorOverflow1);

//		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::fillColorUnderflow0, PropertyNames::colorCategory, true, SchemaItemValue::fillColorUnderflow0, SchemaItemValue::setFillColorUnderflow0);
//		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::fillColorUnderflow1, PropertyNames::colorCategory, true, SchemaItemValue::fillColorUnderflow1, SchemaItemValue::setFillColorUnderflow1);
//		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::textColorUnderflow0, PropertyNames::colorCategory, true, SchemaItemValue::textColorUnderflow0, SchemaItemValue::setTextColorUnderflow0);
//		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::textColorUnderflow1, PropertyNames::colorCategory, true, SchemaItemValue::textColorUnderflow1, SchemaItemValue::setTextColorUnderflow1);

		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::fillColorAnalog0, PropertyNames::colorCategory, true, SchemaItemValue::fillColorAnalog0, SchemaItemValue::setFillColorAnalog0);
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::fillColorAnalog1, PropertyNames::colorCategory, true, SchemaItemValue::fillColorAnalog1, SchemaItemValue::setFillColorAnalog1);
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::textColorAnalog0, PropertyNames::colorCategory, true, SchemaItemValue::textColorAnalog0, SchemaItemValue::setTextColorAnalog0);
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::textColorAnalog1, PropertyNames::colorCategory, true, SchemaItemValue::textColorAnalog1, SchemaItemValue::setTextColorAnalog1);

		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::fillColorDiscrYes0, PropertyNames::colorCategory, true, SchemaItemValue::fillColorDiscrYes0, SchemaItemValue::setFillColorDiscrYes0);
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::fillColorDiscrYes1, PropertyNames::colorCategory, true, SchemaItemValue::fillColorDiscrYes1, SchemaItemValue::setFillColorDiscrYes1);
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::textColorDiscrYes0, PropertyNames::colorCategory, true, SchemaItemValue::textColorDiscrYes0, SchemaItemValue::setTextColorDiscrYes0);
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::textColorDiscrYes1, PropertyNames::colorCategory, true, SchemaItemValue::textColorDiscrYes1, SchemaItemValue::setTextColorDiscrYes1);

		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::fillColorDiscrNo0, PropertyNames::colorCategory, true, SchemaItemValue::fillColorDiscrNo0, SchemaItemValue::setFillColorDiscrNo0);
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::fillColorDiscrNo1, PropertyNames::colorCategory, true, SchemaItemValue::fillColorDiscrNo1, SchemaItemValue::setFillColorDiscrNo1);
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::textColorDiscrNo0, PropertyNames::colorCategory, true, SchemaItemValue::textColorDiscrNo0, SchemaItemValue::setTextColorDiscrNo0);
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::textColorDiscrNo1, PropertyNames::colorCategory, true, SchemaItemValue::textColorDiscrNo1, SchemaItemValue::setTextColorDiscrNo1);

		// Text Category Properties
		//
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::textColor, PropertyNames::textCategory, true, SchemaItemValue::textColor, SchemaItemValue::setTextColor);

		ADD_PROPERTY_GET_SET_CAT(E::HorzAlign, PropertyNames::alignHorz, PropertyNames::textCategory, true, SchemaItemValue::horzAlign, SchemaItemValue::setHorzAlign);
		ADD_PROPERTY_GET_SET_CAT(E::VertAlign, PropertyNames::alignVert, PropertyNames::textCategory, true, SchemaItemValue::vertAlign, SchemaItemValue::setVertAlign);

		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::fontName, PropertyNames::textCategory, true, SchemaItemValue::getFontName, SchemaItemValue::setFontName);
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::fontSize, PropertyNames::textCategory, true, SchemaItemValue::getFontSize, SchemaItemValue::setFontSize);
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::fontBold, PropertyNames::textCategory, true, SchemaItemValue::getFontBold, SchemaItemValue::setFontBold);
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::fontItalic, PropertyNames::textCategory, true,  SchemaItemValue::getFontItalic, SchemaItemValue::setFontItalic);

		// Funtional
		//
		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::appSignalId, PropertyNames::functionalCategory, true, SchemaItemValue::signalId, SchemaItemValue::setSignalId);
		ADD_PROPERTY_GET_SET_CAT(E::SignalSource, PropertyNames::signalSource, PropertyNames::functionalCategory, true, SchemaItemValue::signalSource, SchemaItemValue::setSignalSource);

		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::textAnalog, PropertyNames::functionalCategory, true, SchemaItemValue::textAnalog, SchemaItemValue::setTextAnalog);
		p->setDescription(PropertyNames::textValuePropDescription);

		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::textDiscrete0, PropertyNames::functionalCategory, true, SchemaItemValue::textDiscreteNo, SchemaItemValue::setTextDiscreteNo);
		p->setDescription(PropertyNames::textValuePropDescription);

		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::textDiscrete1, PropertyNames::functionalCategory, true, SchemaItemValue::textDiscreteYes, SchemaItemValue::setTextDiscreteYes);
		p->setDescription(PropertyNames::textValuePropDescription);

		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::textNonValid, PropertyNames::functionalCategory, true, SchemaItemValue::textNonValid, SchemaItemValue::setTextNonValid);
		p->setDescription(PropertyNames::textValuePropDescription);

		p = ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::precision, PropertyNames::functionalCategory, true, SchemaItemValue::precision, SchemaItemValue::setPrecision);
		p->setDescription(PropertyNames::precisionPropText);

		// --
		//
		m_font.setName("Arial");

		switch (unit)
		{
		case SchemaUnit::Display:
			m_font.setSize(12.0, unit);
			break;
		case SchemaUnit::Inch:
			m_font.setSize(1.0 / 8.0, unit);		// 1/8"
			break;
		case SchemaUnit::Millimeter:
			m_font.setSize(mm2in(3), unit);
			break;
		default:
			assert(false);
		}

		m_static = false;
		setItemUnit(unit);
	}

	SchemaItemValue::~SchemaItemValue(void)
	{
	}

	// Serialization
	//
	bool SchemaItemValue::SaveData(Proto::Envelope* message) const
	{
		bool result = PosRectImpl::SaveData(message);
		if (result == false ||
			message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}
		
		// --
		//
		Proto::SchemaItemValue* valueMessage = message->mutable_schemaitem()->mutable_value();

		valueMessage->set_weight(m_lineWeight);
		valueMessage->set_linecolor(m_lineColor.rgba());
		valueMessage->set_fillcolor(m_fillColor.rgba());
		valueMessage->set_drawrect(m_drawRect);
		valueMessage->set_textcolor(m_textColor.rgba());
		m_font.SaveData(valueMessage->mutable_font());

		valueMessage->set_horzalign(static_cast<int32_t>(m_horzAlign));
		valueMessage->set_vertalign(static_cast<int32_t>(m_vertAlign));

		valueMessage->set_textanalog(m_textAnalog.toStdString());
		valueMessage->set_textdiscrete0(m_textDiscreteNo.toStdString());
		valueMessage->set_textdiscrete1(m_textDiscreteYes.toStdString());
		valueMessage->set_textnonvalid(m_textNonValid.toStdString());

		valueMessage->set_signalid(m_signalId.toStdString());
		valueMessage->set_signalsource(static_cast<int32_t>(m_signalSource));
		valueMessage->set_precision(m_precision);

		valueMessage->set_fillcolornonvalid0(m_fillColorNonValid0.rgba());
		valueMessage->set_fillcolornonvalid1(m_fillColorNonValid1.rgba());
		valueMessage->set_textcolornonvalid0(m_textColorNonValid0.rgba());
		valueMessage->set_textcolornonvalid1(m_textColorNonValid1.rgba());

//		valueMessage->set_fillcoloroverflow0(m_fillColorOverflow0.rgba());
//		valueMessage->set_fillcoloroverflow1(m_fillColorOverflow1.rgba());
//		valueMessage->set_textcoloroverflow0(m_textColorOverflow0.rgba());
//		valueMessage->set_textcoloroverflow1(m_textColorOverflow1.rgba());

//		valueMessage->set_fillcolorunderflow0(m_fillColorUnderflow0.rgba());
//		valueMessage->set_fillcolorunderflow1(m_fillColorUnderflow1.rgba());
//		valueMessage->set_textcolorunderflow0(m_textColorUnderflow0.rgba());
//		valueMessage->set_textcolorunderflow1(m_textColorUnderflow1.rgba());

		valueMessage->set_fillcoloranalog0(m_fillColorAnalog0.rgba());
		valueMessage->set_fillcoloranalog1(m_fillColorAnalog1.rgba());
		valueMessage->set_textcoloranalog0(m_textColorAnalog0.rgba());
		valueMessage->set_textcoloranalog1(m_textColorAnalog1.rgba());

		valueMessage->set_fillcolordiscryes0(m_fillColorDiscrYes0.rgba());
		valueMessage->set_fillcolordiscryes1(m_fillColorDiscrYes1.rgba());
		valueMessage->set_textcolordiscryes0(m_textColorDiscrYes0.rgba());
		valueMessage->set_textcolordiscryes1(m_textColorDiscrYes1.rgba());

		valueMessage->set_fillcolordiscrno0(m_fillColorDiscrNo0.rgba());
		valueMessage->set_fillcolordiscrno1(m_fillColorDiscrNo1.rgba());
		valueMessage->set_textcolordiscrno0(m_textColorDiscrNo0.rgba());
		valueMessage->set_textcolordiscrno1(m_textColorDiscrNo1.rgba());

		return true;
	}

	bool SchemaItemValue::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemaitem() == false)
		{
			assert(message.has_schemaitem());
			return false;
		}

		// --
		//
		bool result = PosRectImpl::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemaitem().has_value() == false)
		{
			assert(message.schemaitem().has_value());
			return false;
		}

		const Proto::SchemaItemValue& valueMessage = message.schemaitem().value();

		m_lineWeight = valueMessage.weight();
		m_lineColor = QColor::fromRgba(valueMessage.linecolor());
		m_fillColor = QColor::fromRgba(valueMessage.fillcolor());
		m_textColor = QColor::fromRgba(valueMessage.textcolor());
		m_drawRect = valueMessage.drawrect();

		m_textAnalog = QString::fromStdString(valueMessage.textanalog());
		m_textDiscreteNo = QString::fromStdString(valueMessage.textdiscrete0());
		m_textDiscreteYes = QString::fromStdString(valueMessage.textdiscrete1());
		m_textNonValid = QString::fromStdString(valueMessage.textnonvalid());

		m_horzAlign = static_cast<E::HorzAlign>(valueMessage.horzalign());
		m_vertAlign = static_cast<E::VertAlign>(valueMessage.vertalign());

		m_font.LoadData(valueMessage.font());

		m_signalId = QString::fromStdString(valueMessage.signalid());
		m_signalSource = static_cast<E::SignalSource>(valueMessage.signalsource());
		m_precision = valueMessage.precision();

		m_fillColorNonValid0 = valueMessage.fillcolornonvalid0();
		m_fillColorNonValid1 = valueMessage.fillcolornonvalid1();
		m_textColorNonValid0 = valueMessage.textcolornonvalid0();
		m_textColorNonValid1 = valueMessage.textcolornonvalid1();

//		m_fillColorOverflow0 = valueMessage.fillcoloroverflow0();
//		m_fillColorOverflow1 = valueMessage.fillcoloroverflow1();
//		m_textColorOverflow0 = valueMessage.textcoloroverflow0();
//		m_textColorOverflow1 = valueMessage.textcoloroverflow1();

//		m_fillColorUnderflow0 = valueMessage.fillcolorunderflow0();
//		m_fillColorUnderflow1 =	valueMessage.fillcolorunderflow1();
//		m_textColorUnderflow0 =	valueMessage.textcolorunderflow0();
//		m_textColorUnderflow1 =	valueMessage.textcolorunderflow1();

		m_fillColorAnalog0 = valueMessage.fillcoloranalog0();
		m_fillColorAnalog1 = valueMessage.fillcoloranalog1();
		m_textColorAnalog0 = valueMessage.textcoloranalog0();
		m_textColorAnalog1 = valueMessage.textcoloranalog1();

		m_fillColorDiscrYes0 = valueMessage.fillcolordiscryes0();
		m_fillColorDiscrYes1 = valueMessage.fillcolordiscryes1();
		m_textColorDiscrYes0 = valueMessage.textcolordiscryes0();
		m_textColorDiscrYes1 = valueMessage.textcolordiscryes1();

		m_fillColorDiscrNo0 = valueMessage.fillcolordiscrno0();
		m_fillColorDiscrNo1 = valueMessage.fillcolordiscrno1();
		m_textColorDiscrNo0 = valueMessage.textcolordiscrno0();
		m_textColorDiscrNo1 = valueMessage.textcolordiscrno1();

		return true;
	}

	// Drawing Functions
	//

	// Рисование элемента, выполняется в 100% масштабе.
	// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
	//
	void SchemaItemValue::Draw(CDrawParam* drawParam, const Schema* /*schema*/, const SchemaLayer* /*layer*/) const
	{
		QPainter* p = drawParam->painter();

		// Initialization drawing resources
		//
		initDrawingResources();
						
		// Calculate rectangle
		//
		QRectF r = boundingRectInDocPt();

		// Get signal description and state
		//
		AppSignalState signalState;
		AppSignalParam signalParam;
		TuningSignalState tuningSignalState;

		signalParam.setAppSignalId(signalId());
		signalParam.setCustomSignalId(signalId());

		bool ok = false;

		if (drawParam->isMonitorMode() == true)
		{
			switch (signalSource())
			{
			case E::SignalSource::AppDataService:
				if (drawParam->appSignalManager() == nullptr)
				{
				}
				else
				{
					signalParam = drawParam->appSignalManager()->signal(signalId(), &ok);
					signalState = drawParam->appSignalManager()->signalState(signalId(), nullptr);
				}
				break;

			case E::SignalSource::TuningService:
				if (drawParam->tuningController() == nullptr)
				{

				}
				else
				{
					signalParam = drawParam->tuningController()->signalParam(signalId(), &ok);
					tuningSignalState = drawParam->tuningController()->signalState(signalId(), nullptr);

					// This is for temporary debugging
					signalState.hash = signalParam.hash();
					signalState.flags.valid = tuningSignalState.valid();
					signalState.value = tuningSignalState.value();
					//
				}
				break;

			default:
				assert(false);
			}
		}

		// Drawing background and text
		//
		drawLogic(drawParam, r, signalParam, signalState);

		// Drawing frame rect
		//
		if (drawRect() == true)
		{
			m_rectPen->setWidthF(m_lineWeight == 0.0 ? drawParam->cosmeticPenWidth() : m_lineWeight);

			p->setPen(*m_rectPen);
			p->drawRect(r);
		}

		return;
	}

	void SchemaItemValue::initDrawingResources() const
	{
		if (m_rectPen.get() == nullptr)
		{
			m_rectPen = std::make_shared<QPen>();
		}

		QColor qlinecolor(lineColor());
		if (m_rectPen->color() !=qlinecolor )
		{
			m_rectPen->setColor(qlinecolor);
		}

		if (m_fillBrush.get() == nullptr)
		{
			m_fillBrush = std::make_shared<QBrush>(Qt::SolidPattern);
		}

		return;
	}

	void SchemaItemValue::drawLogic(CDrawParam* drawParam, const QRectF& rect, const AppSignalParam& signal, const AppSignalState& signalState) const
	{
		QPainter* painter = drawParam->painter();
		QColor text_color = textColor();
		QColor back_color = fillColor();
		QString text;
		bool blinkPhase = drawParam->blinkPhase();

		if (drawParam->isEditMode() == true)
		{
			text = signalId();
			back_color = fillColor();
			text_color = textColor();
		}
		else
		{
			if (signalState.isValid() == false)
			{
				text = parseText(m_textNonValid, signal, signalState);
				back_color = blinkPhase ? fillColorNonValid0() : fillColorNonValid1();
				text_color = blinkPhase ? textColorNonValid0() : textColorNonValid1();
			}
			else
			{
				if (signal.isAnalog() == true)
				{
					text = parseText(m_textAnalog, signal, signalState);

					back_color = blinkPhase ? fillColorAnalog0() : fillColorAnalog1();
					text_color = blinkPhase ? textColorAnalog0() : textColorAnalog1();

//					if (signalState.isOverflow() == true)
//					{
//						back_color = blinkPhase ? fillColorOverflow0() : fillColorOverflow1();
//						text_color = blinkPhase ? textColorOverflow0() : textColorOverflow1();
//					}
//					else
//					{
//						if (signalState.isUnderflow() == true)
//						{
//							back_color = blinkPhase ? fillColorUnderflow0() : fillColorUnderflow1();
//							text_color = blinkPhase ? textColorUnderflow0() : textColorUnderflow1();
//						}
//						else
//						{
//							back_color = blinkPhase ? fillColorAnalog0() : fillColorAnalog1();
//							text_color = blinkPhase ? textColorAnalog0() : textColorAnalog1();
//						}
//					}
				}
				else
				{
					if (signalState.value == 0)
					{
						text = parseText(m_textDiscreteNo, signal, signalState);
						back_color = blinkPhase ? fillColorDiscrNo0() : fillColorDiscrNo1();
						text_color = blinkPhase ? textColorDiscrNo0() : textColorDiscrNo1();
					}
					else
					{
						text = parseText(m_textDiscreteYes, signal, signalState);
						back_color = blinkPhase ? fillColorDiscrYes0() : fillColorDiscrYes1();
						text_color = blinkPhase ? textColorDiscrYes0() : textColorDiscrYes1();
					}
				}
			}
		}

		m_fillBrush->setColor(back_color);
		drawParam->painter()->fillRect(rect, *m_fillBrush);

		if (text.isEmpty() == false)
		{
			painter->setPen(text_color);
			DrawHelper::drawText(painter, m_font, itemUnit(), text, rect, horzAlign() | vertAlign());
		}

		return;
	}

	QString SchemaItemValue::parseText(QString text, const AppSignalParam& signal, const AppSignalState& signalState) const
	{
		QString result = text;

		QRegExp reStartIndex("\\$\\([a-zA-Z0-9]+");	// Search for $([SomeText])

		int index = 0;
		while (index < result.size())
		{
			// Find macro bounds
			//
			int startIndexOfMacro = result.indexOf(reStartIndex, index);
			if (startIndexOfMacro == -1)
			{
				break;
			}

			int endIndexOfMacro = result.indexOf(')', startIndexOfMacro + 1);
			if (endIndexOfMacro == -1)
			{
				break;
			}

			// Extract macro string
			//
			QString macro = result.mid(startIndexOfMacro + 2, endIndexOfMacro - startIndexOfMacro - 2);		// +2 is $(, -2 is $()

			// Get value string
			//
			QString replaceText;
			do
			{
				if (macro.compare(QLatin1String("value"), Qt::CaseInsensitive) == 0)
				{
					replaceText = formatNumber(signalState.value, signal);
					break;
				}

				if (macro.compare(QLatin1String("caption"), Qt::CaseInsensitive) == 0)
				{
					replaceText = signal.caption();
					break;
				}

				if (macro.compare(QLatin1String("signalid"), Qt::CaseInsensitive) == 0)
				{
					replaceText = signal.customSignalId();
					break;
				}

				if (macro.compare(QLatin1String("appsignalid"), Qt::CaseInsensitive) == 0)
				{
					replaceText = signal.appSignalId();
					break;
				}

				if (macro.compare(QLatin1String("equipmentid"), Qt::CaseInsensitive) == 0)
				{
					replaceText = signal.equipmentId();
					break;
				}

//				if (macro.compare(QLatin1String("highlimit"), Qt::CaseInsensitive) == 0)
//				{
//					replaceText = formatNumber(signal.highValidRange(), signal);
//					break;
//				}

//				if (macro.compare(QLatin1String("lowlimit"), Qt::CaseInsensitive) == 0)
//				{
//					replaceText = formatNumber(signal.lowValidRange(), signal);
//					break;
//				}

				// Unknown macro
				//
				replaceText = QLatin1String("[UnknownProp]");
			}
			while (false);

			// Replace text in result
			//
			result.replace(startIndexOfMacro, endIndexOfMacro - startIndexOfMacro + 1, replaceText);

			// Iterate
			//
			index = startIndexOfMacro + replaceText.size();
		}

		return result;
	}

	QString SchemaItemValue::formatNumber(double value, const AppSignalParam& signal) const
	{
		if (signal.isDiscrete() == true)
		{
			return QString::number(value, 'f', 0);
		}

		int p = m_precision;
		if (m_precision == -1)
		{
			p = signal.precision();
		}

		return QString::number(value, 'f', p);
	}

	bool SchemaItemValue::searchText(const QString& text) const
	{
		return SchemaItem::searchText(text) ||
				m_textAnalog.contains(text, Qt::CaseInsensitive);
	}

	double SchemaItemValue::minimumPossibleHeightDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
	}

	double SchemaItemValue::minimumPossibleWidthDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
	}

	// Properties and Data
	//
	IMPLEMENT_FONT_PROPERTIES(SchemaItemValue, Font, m_font);

	// Weight property
	//
	double SchemaItemValue::lineWeight() const
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			return CUtils::RoundDisplayPoint(m_lineWeight);
		}
		else
		{
			double pt = CUtils::ConvertPoint(m_lineWeight, SchemaUnit::Inch, Settings::regionalUnit(), 0);
			pt = CUtils::RoundPoint(pt, Settings::regionalUnit());
			return pt;
		}
	}

	void SchemaItemValue::setLineWeight(double weight)
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			m_lineWeight = CUtils::RoundDisplayPoint(weight);
		}
		else
		{
			double pt = CUtils::ConvertPoint(weight, Settings::regionalUnit(), SchemaUnit::Inch, 0);
			m_lineWeight = pt;
		}
	}

	// LineColor property
	//
	QColor SchemaItemValue::lineColor() const
	{
		return m_lineColor;
	}
	void SchemaItemValue::setLineColor(QColor color)
	{
		m_lineColor = color;
	}

	// FillColor property
	//
	QColor SchemaItemValue::fillColor() const
	{
		return m_fillColor;
	}
	void SchemaItemValue::setFillColor(QColor color)
	{
		m_fillColor = color;
	}

	// TextColor property
	//
	QColor SchemaItemValue::textColor() const
	{
		return m_textColor;
	}
	void SchemaItemValue::setTextColor(QColor color)
	{
		m_textColor = color;
	}

	// Align propertis
	//
	E::HorzAlign SchemaItemValue::horzAlign() const
	{
		return m_horzAlign;
	}
	void SchemaItemValue::setHorzAlign(E::HorzAlign align)
	{
		m_horzAlign = align;
	}

	E::VertAlign SchemaItemValue::vertAlign() const
	{
		return m_vertAlign;
	}

	void SchemaItemValue::setVertAlign(E::VertAlign align)
	{
		m_vertAlign = align;
	}

	bool SchemaItemValue::drawRect() const
	{
		return m_drawRect;
	}

	void SchemaItemValue::setDrawRect(bool value)
	{
		m_drawRect = value;
	}

	QString SchemaItemValue::signalId() const
	{
		return m_signalId;
	}

	void SchemaItemValue::setSignalId(const QString& value)
	{
		m_signalId = value.trimmed();
	}

	E::SignalSource SchemaItemValue::signalSource() const
	{
		return m_signalSource;
	}

	void SchemaItemValue::setSignalSource(E::SignalSource value)
	{
		m_signalSource = value;
	}

	const QString& SchemaItemValue::textAnalog() const
	{
		return m_textAnalog;
	}
	void SchemaItemValue::setTextAnalog(QString value)
	{
		m_textAnalog = value;
	}

	const QString& SchemaItemValue::textDiscreteNo() const
	{
		return m_textDiscreteNo;
	}

	void SchemaItemValue::setTextDiscreteNo(QString value)
	{
		m_textDiscreteNo = value;
	}

	const QString& SchemaItemValue::textDiscreteYes() const
	{
		return m_textDiscreteYes;
	}

	void SchemaItemValue::setTextDiscreteYes(QString value)
	{
		m_textDiscreteYes = value;
	}

	const QString& SchemaItemValue::textNonValid() const
	{
		return m_textNonValid;
	}

	void SchemaItemValue::setTextNonValid(QString value)
	{
		m_textNonValid = value;
	}

	int SchemaItemValue::precision() const
	{
		return m_precision;
	}

	void SchemaItemValue::setPrecision(int value)
	{
		if (value < -1)
		{
			value = -1;
		}

		if (value > 64)
		{
			value = 64;
		}

		m_precision = value;
	}
}

