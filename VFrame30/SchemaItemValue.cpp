#include "SchemaItemValue.h"
#include "SchemaView.h"
#include "MacrosExpander.h"
#include "PropertyNames.h"
#include "DrawParam.h"
#include "TuningController.h"
#include "AppSignalController.h"
#include "ClientSchemaView.h"
#include "../AppSignalLib/AppSignalParam.h"
#include "../lib/Tuning/TuningSignalState.h"


namespace VFrame30
{
	SchemaItemValue::SchemaItemValue(void) :
		SchemaItemValue(SchemaUnit::Inch)
	{
		// This constructor can called while serialization
		//
	}

	SchemaItemValue::SchemaItemValue(SchemaUnit unit)
	{
		m_font.setName(QStringLiteral("Arial"));

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

	void SchemaItemValue::propertyDemand(const QString& prop)
	{
		PosRectImpl::propertyDemand(prop);

		// Functional
		//
		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::appSignalIDs, PropertyNames::functionalCategory, true, SchemaItemValue::signalIdsString, SchemaItemValue::setSignalIdsString);
		ADD_PROPERTY_GET_SET_CAT(E::SignalSource, PropertyNames::signalSource, PropertyNames::functionalCategory, true, SchemaItemValue::signalSource, SchemaItemValue::setSignalSource);

		// Appearance
		//
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::lineWeight, PropertyNames::appearanceCategory, true, SchemaItemValue::lineWeight, SchemaItemValue::setLineWeight);

		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::lineColor, PropertyNames::appearanceCategory, true, SchemaItemValue::lineColor, SchemaItemValue::setLineColor);
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::fillColor, PropertyNames::appearanceCategory, true, SchemaItemValue::fillColor, SchemaItemValue::setFillColor);
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::textColor, PropertyNames::appearanceCategory, true, SchemaItemValue::textColor, SchemaItemValue::setTextColor);

		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::drawRect, PropertyNames::appearanceCategory, true, SchemaItemValue::drawRect, SchemaItemValue::setDrawRect);

		// Text Category Properties
		//
		ADD_PROPERTY_GET_SET_CAT(E::HorzAlign, PropertyNames::alignHorz, PropertyNames::textCategory, true, SchemaItemValue::horzAlign, SchemaItemValue::setHorzAlign);
		ADD_PROPERTY_GET_SET_CAT(E::VertAlign, PropertyNames::alignVert, PropertyNames::textCategory, true, SchemaItemValue::vertAlign, SchemaItemValue::setVertAlign);

		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::fontName, PropertyNames::textCategory, true, SchemaItemValue::getFontName, SchemaItemValue::setFontName);
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::fontSize, PropertyNames::textCategory, true, SchemaItemValue::getFontSize, SchemaItemValue::setFontSize);
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::fontBold, PropertyNames::textCategory, true, SchemaItemValue::getFontBold, SchemaItemValue::setFontBold);
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::fontItalic, PropertyNames::textCategory, true,  SchemaItemValue::getFontItalic, SchemaItemValue::setFontItalic);

		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::text, PropertyNames::functionalCategory, true, SchemaItemValue::text, SchemaItemValue::setText)
			->setDescription(PropertyNames::textValuePropDescription);

		ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::precision, PropertyNames::functionalCategory, true, SchemaItemValue::precision, SchemaItemValue::setPrecision)
			->setDescription(PropertyNames::precisionPropText);

		return;
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

		valueMessage->set_signalids(signalIdsString().toStdString());
		valueMessage->set_signalsource(static_cast<int32_t>(m_signalSource));

		valueMessage->set_lineweight(m_lineWeight);

		valueMessage->set_linecolor(m_lineColor.rgba());
		valueMessage->set_fillcolor(m_fillColor.rgba());
		valueMessage->set_textcolor(m_textColor.rgba());

		valueMessage->set_text(m_text.toStdString());

		m_font.SaveData(valueMessage->mutable_font());

		valueMessage->set_drawrect(m_drawRect);

		valueMessage->set_horzalign(static_cast<int32_t>(m_horzAlign));
		valueMessage->set_vertalign(static_cast<int32_t>(m_vertAlign));

		valueMessage->set_precision(m_precision);
		valueMessage->set_analogformat(static_cast<int32_t>(m_analogFormat));

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

		setSignalIdsString(valueMessage.signalids().data());
		m_signalSource = static_cast<E::SignalSource>(valueMessage.signalsource());

		m_lineWeight = valueMessage.lineweight();

		m_lineColor = QColor::fromRgba(valueMessage.linecolor());
		m_fillColor = QColor::fromRgba(valueMessage.fillcolor());
		m_textColor = QColor::fromRgba(valueMessage.textcolor());

		m_text = QString::fromStdString(valueMessage.text());

		m_font.LoadData(valueMessage.font());

		m_drawRect = valueMessage.drawrect();

		m_horzAlign = static_cast<E::HorzAlign>(valueMessage.horzalign());
		m_vertAlign = static_cast<E::VertAlign>(valueMessage.vertalign());

		m_precision = valueMessage.precision();
		m_analogFormat = static_cast<E::AnalogFormat>(valueMessage.analogformat());

		return true;
	}

	// Drawing Functions
	//
	void SchemaItemValue::draw(CDrawParam* drawParam, const Schema* /*schema*/, const SchemaLayer* /*layer*/) const
	{
		QPainter* p = drawParam->painter();

		// Initialization drawing resources
		//
		initDrawingResources();
						
		// Calculate rectangle
		//
		QRectF r = boundingRectInDocPt(drawParam);

		// Drawing background
		//
		m_fillBrush->setColor(m_fillColor);
		drawParam->painter()->fillRect(r, *m_fillBrush);

		// Drawing text
		//
		drawText(drawParam, r);

		// Remove brush to draw non-filled rects
		//
		p->setBrush(Qt::NoBrush);

		// Drawing frame rect
		//
		if (drawRect() == true)
		{
			m_rectPen->setWidthF(m_lineWeight == 0.0 ? drawParam->cosmeticPenWidth() : m_lineWeight);

			p->setPen(*m_rectPen);
			p->drawRect(r);
		}

		// Draw highlights for m_appSignalIds
		//
		for (const QString& appSignalId : m_signalIds)
		{
			if (drawParam->hightlightIds().contains(appSignalId) == true)
			{
				QRectF highlightRect = boundingRectInDocPt(drawParam);
				drawHighlightRect(drawParam, highlightRect);
				break;
			}
		}

		return;
	}

	void SchemaItemValue::initDrawingResources() const
	{
		if (m_rectPen.get() == nullptr)
		{
			m_rectPen = std::make_unique<QPen>();
		}

		if (m_rectPen->color() != lineColor())
		{
			m_rectPen->setColor(lineColor());
		}

		// --
		//
		if (m_fillBrush.get() == nullptr)
		{
			m_fillBrush = std::make_unique<QBrush>(Qt::SolidPattern);
		}

		return;
	}

	void SchemaItemValue::drawText(CDrawParam* drawParam, const QRectF& rect) const
	{
		QPainter* painter = drawParam->painter();
		QString text;

		if (drawParam->isEditMode() == true)
		{
			text = m_text;
		}
		else
		{
			if (m_text.contains(QLatin1String("$(")) == true)
			{
				// m_text contains some variables, which need to be parsed
				//

				// Get signal description and state
				//
				AppSignalParam signalParam;
				AppSignalState signalState;
				TuningSignalState tuningSignalState;

				QString signalId;

				if (signalIds().empty() == false)
				{
					signalId = signalIds().front();

					signalParam.setAppSignalId(signalId);
					signalParam.setCustomSignalId(signalId);
				}

				getSignalState(signalId, drawParam, &signalParam, &signalState, &tuningSignalState);

				text = parseText(m_text, drawParam, signalParam, signalState);
			}
			else
			{
				// Most likely text was set in PreDrawScript, or it is just text
				//
				text = m_text;
			}
		}

		if (text.isEmpty() == false)
		{
			painter->setPen(textColor());
			DrawHelper::drawText(painter, m_font, itemUnit(), text, rect, horzAlign() | vertAlign());
		}

		return;
	}

	QString SchemaItemValue::parseText(QString text, CDrawParam* drawParam, const AppSignalParam& signal, const AppSignalState& signalState) const
	{
		if (drawParam == nullptr)
		{
			Q_ASSERT(drawParam);
		}

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
			std::optional<QString> replaceText;
			do
			{
				if (macro.compare(QLatin1String("value"), Qt::CaseInsensitive) == 0)
				{
					if (signalState.isValid() == true)
					{
						replaceText = formatNumber(signalState.m_value, signal);
					}
					else
					{
						replaceText = QStringLiteral("?");
					}
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
			}
			while (false);

			// Replace text in result
			//
			if (replaceText.has_value() == true)
			{
				result.replace(startIndexOfMacro, endIndexOfMacro - startIndexOfMacro + 1, *replaceText);
				index = startIndexOfMacro + replaceText->size();
			}
			else
			{
				index = endIndexOfMacro;
			}
		}

		// Expand all other macroses
		//
		result = MacrosExpander::parse(result, drawParam, this);

		return result;
	}

	QString SchemaItemValue::formatNumber(double value, const AppSignalParam& signal) const
	{
		if (signal.isDiscrete() == true)
		{
			return QString::number(value, 'f', 0);
		}

		assert(signal.isAnalog());

		int p = m_precision;
		if (m_precision == -1)
		{
			p = signal.precision();
		}

		return QString::number(value, static_cast<char>(analogFormat()), p);
	}

	bool SchemaItemValue::getSignalState(QString appSignalId, CDrawParam* drawParam, AppSignalParam* signalParam, AppSignalState* appSignalState, TuningSignalState* tuningSignalState) const
	{
		if (drawParam == nullptr ||
			signalParam == nullptr ||
			appSignalState == nullptr ||
			tuningSignalState == nullptr)
		{
			Q_ASSERT(drawParam);
			Q_ASSERT(signalParam);
			Q_ASSERT(appSignalState);
			Q_ASSERT(tuningSignalState);
			return false;
		}

		bool ok = false;

		switch (signalSource())
		{
		case E::SignalSource::AppDataService:
			if (auto appSignalController = drawParam->appSignalController();
				appSignalController == nullptr)
			{
			}
			else
			{
				if (appSignalId.startsWith('@') == true)
				{
					appSignalId = appSignalController->appSignalManager()->equipmentToAppSiganlId(appSignalId);
				}

				*signalParam = drawParam->appSignalController()->signalParam(appSignalId, &ok);
				*appSignalState = drawParam->appSignalController()->signalState(appSignalId, nullptr);
			}
			break;

		case E::SignalSource::TuningService:
			if (drawParam->tuningController() == nullptr)
			{
			}
			else
			{
				*signalParam = drawParam->tuningController()->signalParam(appSignalId, &ok);
				*tuningSignalState = drawParam->tuningController()->signalState(appSignalId, nullptr);

				appSignalState->m_hash = signalParam->hash();
				appSignalState->m_flags.valid = tuningSignalState->valid();
				appSignalState->m_value = tuningSignalState->value().toDouble();
			}
			break;

		default:
			Q_ASSERT(false);
			ok = false;
		}

		return ok;
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

	QString SchemaItemValue::signalIdsString() const
	{
		QStringList resultList = m_signalIds;

		// Expand variables in AppSignalIDs in MonitorMode, if applicable (m_drawParam is set and is monitor mode)
		//
		if (m_drawParam != nullptr &&
			m_drawParam->isMonitorMode() == true &&
			m_drawParam->clientSchemaView() != nullptr)
		{
			resultList = MacrosExpander::parse(resultList, m_drawParam, this);

			for (QString& s : resultList)
			{
				if (s.startsWith('@') == true)
				{
					s = m_drawParam->clientSchemaView()->appSignalController()->appSignalManager()->equipmentToAppSiganlId(s);
				}
			}
		}

		return resultList.join(QChar::LineFeed);
	}

	void SchemaItemValue::setSignalIdsString(const QString& value)
	{
		m_signalIds = value.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
	}

	QStringList SchemaItemValue::signalIds() const
	{
		QStringList resultList = m_signalIds;

		// Expand variables in AppSignalIDs in MonitorMode, if applicable
		//
		if (m_drawParam != nullptr &&
			m_drawParam->isMonitorMode() == true &&
			m_drawParam->clientSchemaView() != nullptr)
		{
			resultList = MacrosExpander::parse(resultList, m_drawParam, this);

			for (QString& s : resultList)
			{
				if (s.startsWith('@') == true)
				{
					s = m_drawParam->clientSchemaView()->appSignalController()->appSignalManager()->equipmentToAppSiganlId(s);
				}
			}
		}

		return resultList;
	}

	void SchemaItemValue::setSignalIds(const QStringList& value)
	{
		m_signalIds = value;
	}

	E::SignalSource SchemaItemValue::signalSource() const
	{
		return m_signalSource;
	}

	void SchemaItemValue::setSignalSource(E::SignalSource value)
	{
		m_signalSource = value;
	}

	// Weight property
	//
	double SchemaItemValue::lineWeight() const
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			return VFrame30::RoundDisplayPoint(m_lineWeight);
		}
		else
		{
			double pt = VFrame30::ConvertPoint(m_lineWeight, SchemaUnit::Inch, Settings::regionalUnit(), 0);
			pt = VFrame30::RoundPoint(pt, Settings::regionalUnit());
			return pt;
		}
	}

	void SchemaItemValue::setLineWeight(double weight)
	{
		if (weight < 0)
		{
			weight = 0;
		}

		if (itemUnit() == SchemaUnit::Display)
		{
			m_lineWeight = VFrame30::RoundDisplayPoint(weight);
		}
		else
		{
			double pt = VFrame30::ConvertPoint(weight, Settings::regionalUnit(), SchemaUnit::Inch, 0);
			m_lineWeight = pt;
		}
	}

	// LineColor property
	//
	const QColor& SchemaItemValue::lineColor() const
	{
		return m_lineColor;
	}
	void SchemaItemValue::setLineColor(const QColor& color)
	{
		m_lineColor = color;
	}

	// FillColor property
	//
	const QColor& SchemaItemValue::fillColor() const
	{
		return m_fillColor;
	}
	void SchemaItemValue::setFillColor(const QColor& color)
	{
		m_fillColor = color;
	}

	// TextColor property
	//
	const QColor& SchemaItemValue::textColor() const
	{
		return m_textColor;
	}
	void SchemaItemValue::setTextColor(const QColor& color)
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

	const QString& SchemaItemValue::text() const
	{
		return m_text;
	}
	void SchemaItemValue::setText(QString value)
	{
		m_text = value;
	}

	int SchemaItemValue::precision() const
	{
		return m_precision;
	}

	void SchemaItemValue::setPrecision(int value)
	{
		m_precision = qBound(-1, value, 64);
	}

	E::AnalogFormat SchemaItemValue::analogFormat() const
	{
		return m_analogFormat;
	}

	void SchemaItemValue::setAnalogFormat(E::AnalogFormat value)
	{
		m_analogFormat = value;
	}
}

