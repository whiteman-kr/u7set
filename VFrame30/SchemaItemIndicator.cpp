#include "SchemaItemIndicator.h"
#include "../lib/AppSignal.h"
#include "../lib/Tuning/TuningSignalState.h"
#include "PropertyNames.h"
#include "DrawParam.h"
#include "Schema.h"
#include "FblItemRect.h"
#include "MacrosExpander.h"
#include "TuningController.h"
#include "AppSignalController.h"
#include "IndicatorHistogramVert.h"
#include "IndicatorArrowIndicator.h"

namespace VFrame30
{
	//
	// SchemaItemIndicator
	//
	SchemaItemIndicator::SchemaItemIndicator(void) :
		SchemaItemIndicator(SchemaUnit::Inch)
	{
		// This constructor can be called during serialization, then all variables
		// are initialized in loading process
		//
	}

	SchemaItemIndicator::SchemaItemIndicator(SchemaUnit unit) :
		m_indicatorObjects
			{
				std::make_unique<IndicatorHistogramVert>(unit),		// E::IndicatorType::HistogramVert
				std::make_unique<IndicatorArrowIndicator>(unit),	// E::IndicatorType::ArrowIndicator
			}
	{
		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::appSignalIDs, PropertyNames::functionalCategory, true, SchemaItemIndicator::signalIdsString, SchemaItemIndicator::setSignalIdsString);
		ADD_PROPERTY_GET_SET_CAT(E::SignalSource, PropertyNames::signalSource, PropertyNames::functionalCategory, true, SchemaItemIndicator::signalSource, SchemaItemIndicator::setSignalSource);

		ADD_PROPERTY_GETTER_SETTER(E::AnalogFormat, PropertyNames::analogFormat, true, SchemaItemIndicator::analogFormat, SchemaItemIndicator::setAnalogFormat);
		ADD_PROPERTY_GETTER_SETTER(int, PropertyNames::precision, true, SchemaItemIndicator::precision, SchemaItemIndicator::setPrecision);

		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::fontName, PropertyNames::textCategory, true, SchemaItemIndicator::getFontName, SchemaItemIndicator::setFontName);
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::fontSize, PropertyNames::textCategory, true, SchemaItemIndicator::getFontSize, SchemaItemIndicator::setFontSize);
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::fontBold, PropertyNames::textCategory, true, SchemaItemIndicator::getFontBold, SchemaItemIndicator::setFontBold);
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::fontItalic, PropertyNames::textCategory, true,  SchemaItemIndicator::getFontItalic, SchemaItemIndicator::setFontItalic);

		ADD_PROPERTY_GETTER_SETTER(E::IndicatorType, PropertyNames::indicatorType, true, SchemaItemIndicator::indicatorType, SchemaItemIndicator::setIndicatorType);

		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::drawRect, PropertyNames::appearanceCategory, true, SchemaItemIndicator::drawRect, SchemaItemIndicator::setDrawRect);

		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::lineWeight, PropertyNames::appearanceCategory, true, SchemaItemIndicator::lineWeight, SchemaItemIndicator::setLineWeight);

		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::backgroundColor, PropertyNames::appearanceCategory, true, SchemaItemIndicator::backgroundColor, SchemaItemIndicator::setBackgroundColor);
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::lineColor, PropertyNames::appearanceCategory, true, SchemaItemIndicator::lineColor, SchemaItemIndicator::setLineColor);

		ADD_PROPERTY_GET_SET_CAT(QVector<QColor>, PropertyNames::indicatorSignalColors, PropertyNames::appearanceCategory, true, SchemaItemIndicator::signalColors, SchemaItemIndicator::setSignalColors);

		m_static = false;
		setItemUnit(unit);

		setIndicatorType(E::IndicatorType::HistogramVert);

		// Set font
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
			Q_ASSERT(false);
		}

		return;
	}

	// Serialization
	//
	bool SchemaItemIndicator::SaveData(Proto::Envelope* message) const
	{
		bool result = PosRectImpl::SaveData(message);
		if (result == false || message->has_schemaitem() == false)
		{
			Q_ASSERT(result);
			Q_ASSERT(message->has_schemaitem());
			return false;
		}
		
		// --
		//
		Proto::SchemaItemIndicator* indicatorMessage = message->mutable_schemaitem()->mutable_indicator();

		indicatorMessage->set_signalids(signalIdsString().toStdString());
		indicatorMessage->set_signalsource(static_cast<int32_t>(m_signalSource));

		indicatorMessage->set_analogformat(static_cast<int32_t>(m_analogFormat));
		indicatorMessage->set_precision(m_precision);

		m_font.SaveData(indicatorMessage->mutable_font());

		indicatorMessage->set_drawrect(m_drawRect);
		indicatorMessage->set_lineweight(m_lineWeight);

		indicatorMessage->set_backgroundcolor(m_backgroundColor.rgba());
		indicatorMessage->set_linecolor(m_lineColor.rgba());

		indicatorMessage->mutable_signalcolors()->Reserve(m_signalColors.size());
		for (const QColor& bc : m_signalColors)
		{
			indicatorMessage->mutable_signalcolors()->Add(bc.rgba());
		}

		indicatorMessage->set_type(static_cast<int>(m_indicatorType));

		for (auto& io : m_indicatorObjects)
		{
			Q_ASSERT(io);
			io->save(indicatorMessage);
		}

		return true;
	}

	bool SchemaItemIndicator::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemaitem() == false)
		{
			Q_ASSERT(message.has_schemaitem());
			return false;
		}

		bool result = PosRectImpl::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemaitem().has_indicator() == false)
		{
			Q_ASSERT(message.schemaitem().has_indicator());
			return false;
		}

		const Proto::SchemaItemIndicator& indicatorMessage = message.schemaitem().indicator();

		setSignalIdsString(indicatorMessage.signalids().data());
		m_signalSource = static_cast<E::SignalSource>(indicatorMessage.signalsource());

		m_analogFormat = static_cast<E::AnalogFormat>(indicatorMessage.analogformat());
		m_precision = indicatorMessage.precision();

		m_font.LoadData(indicatorMessage.font());

		m_drawRect = indicatorMessage.drawrect();
		m_lineWeight = indicatorMessage.lineweight();

		m_backgroundColor = QColor::fromRgba(indicatorMessage.backgroundcolor());
		m_lineColor = QColor::fromRgba(indicatorMessage.linecolor());

		m_signalColors.clear();
		m_signalColors.reserve(indicatorMessage.signalcolors_size());

		const auto& bcfiled = indicatorMessage.signalcolors();
		for (auto bc : bcfiled)
		{
			m_signalColors.push_back(QColor::fromRgba(bc));
		}

		// Set iunits for the IndicatorObject, as it is not saved in proto container and it must be the same with schema item
		//
		for (auto& io : m_indicatorObjects)
		{
			io->setUnits(itemUnit());
			io->load(indicatorMessage, itemUnit());
		}

		// --
		//
		setIndicatorType(static_cast<E::IndicatorType>(indicatorMessage.type()));	// call setter to create properties

		return true;
	}

	// Drawing Functions
	//
	void SchemaItemIndicator::draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const
	{
		QPainter* p = drawParam->painter();
		Q_ASSERT(p);

		const Indicator* io = indicatorObject();

		if (io == nullptr)
		{
			Q_ASSERT(io);
			return;
		}

		io->draw(drawParam,  schema, layer, this);

		return;
	}

	std::set<QString> SchemaItemIndicator::getSignalTags(CDrawParam* drawParam, const QString& appSignalId) const
	{
		Q_ASSERT(drawParam);

		std::set<QString> result;

		switch (signalSource())
		{
		case E::SignalSource::AppDataService:
			if (drawParam->appSignalController() == nullptr)
			{
				Q_ASSERT(drawParam->appSignalController());
			}
			else
			{
				QStringList tags = drawParam->appSignalController()->signalTags(appSignalId);
				for (const QString& t : tags)
				{
					result.insert(t);
				}
			}
			break;

		case E::SignalSource::TuningService:
			if (drawParam->tuningController() == nullptr)
			{
			}
			else
			{
				AppSignalParam param;
				param.setAppSignalId(appSignalId);

				bool ok = getSignalParam(drawParam, &param);
				if (ok == true)
				{
					for (const QString& t : param.tags())
					{
						result.insert(t);
					}
				}

			}
			break;

		default:
			Q_ASSERT(false);
		}

		return result;
	}

	bool SchemaItemIndicator::getSignalParam(CDrawParam* drawParam, AppSignalParam* signalParam) const
	{
		if (drawParam == nullptr ||
			signalParam == nullptr)
		{
			Q_ASSERT(drawParam);
			Q_ASSERT(signalParam);
			return false;
		}

		if (drawParam->isMonitorMode() == false)
		{
			Q_ASSERT(drawParam->isMonitorMode());
			return false;
		}

		bool ok = false;

		switch (signalSource())
		{
		case E::SignalSource::AppDataService:
			if (drawParam->appSignalController() == nullptr)
			{
			}
			else
			{
				*signalParam = drawParam->appSignalController()->signalParam(signalParam->appSignalId(), &ok);
			}
			break;

		case E::SignalSource::TuningService:
			if (drawParam->tuningController() == nullptr)
			{
			}
			else
			{
				*signalParam = drawParam->tuningController()->signalParam(signalParam->appSignalId(), &ok);
			}
			break;

		default:
			Q_ASSERT(false);
			ok = false;
		}

		return ok;
	}

	bool SchemaItemIndicator::getSignalState(CDrawParam* drawParam, AppSignalParam* signalParam, AppSignalState* appSignalState, TuningSignalState* tuningSignalState) const
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

		if (drawParam->isMonitorMode() == false)
		{
			Q_ASSERT(drawParam->isMonitorMode());
			return false;
		}

		bool ok = false;

		switch (signalSource())
		{
		case E::SignalSource::AppDataService:
			if (drawParam->appSignalController() == nullptr)
			{
			}
			else
			{
				*signalParam = drawParam->appSignalController()->signalParam(signalParam->appSignalId(), &ok);
				*appSignalState = drawParam->appSignalController()->signalState(signalParam->appSignalId(), nullptr);
			}
			break;

		case E::SignalSource::TuningService:
			if (drawParam->tuningController() == nullptr)
			{
			}
			else
			{
				*signalParam = drawParam->tuningController()->signalParam(signalParam->appSignalId(), &ok);
				*tuningSignalState = drawParam->tuningController()->signalState(signalParam->appSignalId(), nullptr);

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

	std::optional<double> SchemaItemIndicator::getSignalState(CDrawParam* drawParam, const QString& appSignalId) const
	{
		if (drawParam->isMonitorMode() == false)
		{
			return {};
		}

		bool valid = false;
		double value = -1;

		switch (signalSource())
		{
		case E::SignalSource::AppDataService:
			if (drawParam->appSignalController() == nullptr)
			{
			}
			else
			{
				AppSignalState state = drawParam->appSignalController()->signalState(appSignalId, nullptr);

				valid = state.isValid();
				value = state.value();
			}
			break;

		case E::SignalSource::TuningService:
			if (drawParam->tuningController() == nullptr)
			{
			}
			else
			{
				TuningSignalState state = drawParam->tuningController()->signalState(appSignalId, nullptr);

				valid = state.valid();
				value = state.value().toDouble();
			}
			break;

		default:
			Q_ASSERT(false);
		}

		if (valid == false)
		{
			return {};
		}
		else
		{
			return {value};
		}
	}

	// Properties and Data
	//
	// AppSignalIDs
	//
	QString SchemaItemIndicator::signalIdsString() const
	{
		QString result = m_signalIds.join(QChar::LineFeed);

		// Expand variables in AppSignalIDs in MonitorMode, if applicable (m_drawParam is set and is monitor mode)
		//
		if (m_drawParam != nullptr &&
			m_drawParam->isMonitorMode() == true &&
			m_drawParam->clientSchemaView() != nullptr)
		{
			result = MacrosExpander::parse(result, m_drawParam, this);
		}

		return result;
	}

	void SchemaItemIndicator::setSignalIdsString(const QString& value)
	{
		setSignalIds(value.split(QRegExp("\\s+"), QString::SkipEmptyParts));
	}

	QStringList SchemaItemIndicator::signalIds() const
	{
		QStringList resultList = m_signalIds;

		// Expand variables in AppSignalIDs in MonitorMode, if applicable
		//
		if (m_drawParam != nullptr &&
			m_drawParam->isMonitorMode() == true &&
			m_drawParam->clientSchemaView() != nullptr)
		{
			resultList = MacrosExpander::parse(resultList, m_drawParam, this);
		}

		return resultList;
	}

	void SchemaItemIndicator::setSignalIds(const QStringList& value)
	{
		m_signalIds = value;

		while (m_signalIds.size() > MaxSignalsCound)
		{
			m_signalIds.removeLast();
		}

		return;
	}

	E::SignalSource SchemaItemIndicator::signalSource() const
	{
		return m_signalSource;
	}

	void SchemaItemIndicator::setSignalSource(E::SignalSource value)
	{
		m_signalSource = value;
	}

	E::AnalogFormat SchemaItemIndicator::analogFormat() const
	{
		return m_analogFormat;
	}

	void SchemaItemIndicator::setAnalogFormat(E::AnalogFormat value)
	{
		m_analogFormat = value;
	}

	int SchemaItemIndicator::precision() const
	{
		return qBound(-1, m_precision, 32);
	}

	void SchemaItemIndicator::setPrecision(int value)
	{
		m_precision = qBound(-1, value, 32);
	}

	IMPLEMENT_FONT_PROPERTIES(SchemaItemIndicator, Font, m_font);

	FontParam& SchemaItemIndicator::font()
	{
		return m_font;
	}

	const FontParam& SchemaItemIndicator::font() const
	{
		return m_font;
	}

	bool SchemaItemIndicator::drawRect() const
	{
		return m_drawRect;
	}

	void SchemaItemIndicator::setDrawRect(bool value)
	{
		m_drawRect = value;
	}

	double SchemaItemIndicator::lineWeight() const
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

	double SchemaItemIndicator::lineWeightDraw() const noexcept
	{
		return m_lineWeight;
	}

	void SchemaItemIndicator::setLineWeight(double weight)
	{
		if (weight < 0)
		{
			weight = 0;
		}

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

	const QColor& SchemaItemIndicator::backgroundColor() const
	{
		return m_backgroundColor;
	}

	void SchemaItemIndicator::setBackgroundColor(const QColor& color)
	{
		m_backgroundColor = color;
	}

	const QColor& SchemaItemIndicator::lineColor() const
	{
		return m_lineColor;
	}

	void SchemaItemIndicator::setLineColor(const QColor& color)
	{
		m_lineColor = color;
	}

	const QVector<QColor>& SchemaItemIndicator::signalColors() const
	{
		return m_signalColors;
	}

	void SchemaItemIndicator::setSignalColors(const QVector<QColor>& value)
	{
		m_signalColors = value;
		//m_signalColors.resize(12);
	}

	E::IndicatorType SchemaItemIndicator::indicatorType() const
	{
		return m_indicatorType;
	}

	void SchemaItemIndicator::setIndicatorType(E::IndicatorType value)
	{
		if (static_cast<size_t>(value) >= m_indicatorObjects.size())
		{
			Q_ASSERT(static_cast<size_t>(value) < m_indicatorObjects.size());
			value = E::IndicatorType::HistogramVert;
		}

		m_indicatorType = value;		// Getter will other object with different properties

		// Update specific properties for current indiocator type
		//
		removeCategoryProperties(PropertyNames::indicatorSettings);
		indicatorObject()->createProperties(this, m_signalIds.size());

		emit propertyListChanged();

		return;
	}

	Indicator* SchemaItemIndicator::indicatorObject()
	{
		size_t index = static_cast<size_t>(m_indicatorType);

		if (index >= m_indicatorObjects.size())
		{
			Q_ASSERT(index < m_indicatorObjects.size());
			index = 0;
		}

		return m_indicatorObjects[index].get();
	}

	const Indicator* SchemaItemIndicator::indicatorObject() const
	{
		size_t index = static_cast<size_t>(m_indicatorType);

		if (index >= m_indicatorObjects.size())
		{
			Q_ASSERT(index < m_indicatorObjects.size());
			index = 0;
		}

		return m_indicatorObjects[index].get();
	}

}

