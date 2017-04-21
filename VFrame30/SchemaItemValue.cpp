#include "SchemaItemValue.h"
#include "SchemaView.h"
#include "MacrosExpander.h"
#include "PropertyNames.h"
#include "DrawParam.h"
#include "../lib/Signal.h"
#include "../lib/AppSignalState.h"
#include "../lib/AppSignalManager.h"

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

		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::textDiscrete0, PropertyNames::functionalCategory, true, SchemaItemValue::textDiscrete0, SchemaItemValue::setTextDiscrete0);
		p->setDescription(PropertyNames::textValuePropDescription);

		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::textDiscrete1, PropertyNames::functionalCategory, true, SchemaItemValue::textDiscrete1, SchemaItemValue::setTextDiscrete1);
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
		valueMessage->set_textdiscrete0(m_textDiscrete0.toStdString());
		valueMessage->set_textdiscrete1(m_textDiscrete1.toStdString());
		valueMessage->set_textnonvalid(m_textNonValid.toStdString());

				valueMessage->set_signalid(m_signalId.toStdString());
		valueMessage->set_signalsource(static_cast<int32_t>(m_signalSource));
		valueMessage->set_precision(m_precision);

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
		m_textDiscrete0 = QString::fromStdString(valueMessage.textdiscrete0());
		m_textDiscrete1 = QString::fromStdString(valueMessage.textdiscrete1());
		m_textNonValid = QString::fromStdString(valueMessage.textnonvalid());

		m_horzAlign = static_cast<E::HorzAlign>(valueMessage.horzalign());
		m_vertAlign = static_cast<E::VertAlign>(valueMessage.vertalign());

		m_font.LoadData(valueMessage.font());

		m_signalId = QString::fromStdString(valueMessage.signalid());
		m_signalSource = static_cast<E::SignalSource>(valueMessage.signalsource());
		m_precision = valueMessage.precision();

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
		Signal signal;

		signal.setAppSignalID(signalId());
		signal.setCustomAppSignalID(signalId());
		bool ok = false;

		if (drawParam->isMonitorMode() == true)
		{
			switch (signalSource())
			{
			case E::SignalSource::AppDataService:
				assert(drawParam->appSignalManager());
				signal = drawParam->appSignalManager()->signal(signalId(), &ok);
				signalState = drawParam->appSignalManager()->signalState(signalId(), nullptr);
				break;
			case E::SignalSource::TuningService:
				assert(false);
				break;
			default:
				assert(false);
			}
		}

		// Draw background
		//
		if (drawParam->isEditMode())
		{
			QPainter::RenderHints oldrenderhints = p->renderHints();
			p->setRenderHint(QPainter::Antialiasing, false);

			m_fillBrush->setColor(fillColor());
			p->fillRect(r, *m_fillBrush);		// 22% if use QColor and much less in case of using ready brush

			p->setRenderHints(oldrenderhints);
		}
		else
		{
			drawBackground(p, r, signal, signalState);
		}

		// Drawing Text
		//
		drawText(drawParam, r, signal, signalState);

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

	void SchemaItemValue::drawBackground(QPainter* painter, const QRectF& rect, const Signal& signal, const AppSignalState& signalState) const
	{
		QColor color = fillColor();

		if (signal.isAnalog() == true)
		{
		}

		if (signal.isDiscrete() == true)
		{
		}

		QPainter::RenderHints oldrenderhints = painter->renderHints();
		painter->setRenderHint(QPainter::Antialiasing, false);

		m_fillBrush->setColor(color);
		painter->fillRect(rect, *m_fillBrush);		// 22% если использовать Qcolor и намного меньше если использовать готовый Brush

		painter->setRenderHints(oldrenderhints);

		return;
	}

	void SchemaItemValue::drawText(CDrawParam* drawParam, const QRectF& rect, const Signal& signal, const AppSignalState& signalState) const
	{
		QPainter* painter = drawParam->painter();

		if (drawParam->isEditMode() == true)
		{
			painter->setPen(textColor());
			DrawHelper::drawText(painter, m_font, itemUnit(), signalId(), rect, horzAlign() | vertAlign());
			return;
		}

		QString text;

		if (signalState.flags.valid == true)
		{
			if (signal.isAnalog() == true)
			{
				text = parseText(m_textAnalog, signal, signalState);
			}
			else
			{
				if (signalState.value == 0)
				{
					text = parseText(m_textDiscrete0, signal, signalState);
				}
				else
				{
					text = parseText(m_textDiscrete1, signal, signalState);
				}
			}
		}
		else
		{
			text = parseText(m_textNonValid, signal, signalState);
		}

		painter->setPen(textColor());
		DrawHelper::drawText(painter, m_font, itemUnit(), text, rect, horzAlign() | vertAlign());

		return;
	}

	QString SchemaItemValue::parseText(QString text, const Signal& signal, const AppSignalState& signalState) const
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
					replaceText = signal.customAppSignalID();
					break;
				}

				if (macro.compare(QLatin1String("appsignalid"), Qt::CaseInsensitive) == 0)
				{
					replaceText = signal.appSignalID();
					break;
				}

				if (macro.compare(QLatin1String("equipmentid"), Qt::CaseInsensitive) == 0)
				{
					replaceText = signal.equipmentID();
					break;
				}

				if (macro.compare(QLatin1String("highlimit"), Qt::CaseInsensitive) == 0)
				{
					replaceText = formatNumber(signal.highValidRange(), signal);
					break;
				}

				if (macro.compare(QLatin1String("lowlimit"), Qt::CaseInsensitive) == 0)
				{
					replaceText = formatNumber(signal.lowValidRange(), signal);
					break;
				}

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

	QString SchemaItemValue::formatNumber(double value, const Signal& signal) const
	{
		if (signal.isDiscrete() == true)
		{
			return QString::number(value, 'f', 0);
		}

		int p = m_precision;
		if (m_precision == -1)
		{
			p = signal.decimalPlaces();
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

	const QString& SchemaItemValue::textDiscrete0() const
	{
		return m_textDiscrete0;
	}

	void SchemaItemValue::setTextDiscrete0(QString value)
	{
		m_textDiscrete0 = value;
	}

	const QString& SchemaItemValue::textDiscrete1() const
	{
		return m_textDiscrete1;
	}

	void SchemaItemValue::setTextDiscrete1(QString value)
	{
		m_textDiscrete1 = value;
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

