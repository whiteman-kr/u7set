#include <VFrame30/SchemaItemDiagValue.h>
#include <VFrame30/DiagStateController.h>
#include <VFrame30/DrawParam.h>
#include <VFrame30/MacrosExpander.h>
#include <VFrame30/PropertyNames.h>
#include <VFrame30/SchemaView.h>


namespace VFrame30
{
	SchemaItemDiagValue::SchemaItemDiagValue(void) :
		SchemaItemDiagValue(SchemaUnit::Inch)
	{
		// This constructor can called while serialization
		//
	}

	SchemaItemDiagValue::SchemaItemDiagValue(SchemaUnit unit)
	{
		m_font.setName(QStringLiteral("Arial"));

		switch (unit)
		{
		case SchemaUnit::Display:
			m_font.setSize(14.0, unit);
			break;
		case SchemaUnit::Inch:
			m_font.setSize(mm2in(4), unit);
			// m_font.setSize(1.0 / 8.0, unit);		// 1/8"
			break;
		case SchemaUnit::Millimeter:
			m_font.setSize(mm2in(4), unit);
			break;
		default:
			assert(false);
		}

		m_static = false;
		setItemUnit(unit);
	}

	void SchemaItemDiagValue::propertyDemand(const QString& prop)
	{
		PosRectRotatable::propertyDemand(prop);

		// clang-format off

		// Functional
		//
		if (prop.isEmpty() == true || prop == PropertyNames::diagSignalIDs)
		{
			ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::diagSignalIDs, PropertyNames::functionalCategory, true, SchemaItemDiagValue::diagSignalIdsString, SchemaItemDiagValue::setDiagSignalIdsString);
		}

		// Appearance
		//
		if (prop.isEmpty() == true || prop == PropertyNames::lineWeight)
		{
			ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::lineWeight, PropertyNames::appearanceCategory, true, SchemaItemDiagValue::lineWeight, SchemaItemDiagValue::setLineWeight);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::lineColor)
		{
			ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::lineColor, PropertyNames::appearanceCategory, true, SchemaItemDiagValue::lineColor, SchemaItemDiagValue::setLineColor);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::fillColor)
		{
			ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::fillColor, PropertyNames::appearanceCategory, true, SchemaItemDiagValue::fillColor, SchemaItemDiagValue::setFillColor);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::textColor)
		{
			ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::textColor, PropertyNames::appearanceCategory, true, SchemaItemDiagValue::textColor, SchemaItemDiagValue::setTextColor);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::drawRect)
		{
			ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::drawRect, PropertyNames::appearanceCategory, true, SchemaItemDiagValue::drawRect, SchemaItemDiagValue::setDrawRect);
		}

		// Text Category Properties
		//
		if (prop.isEmpty() == true || prop == PropertyNames::alignHorz)
		{
			ADD_PROPERTY_GET_SET_CAT(E::HorzAlign, PropertyNames::alignHorz, PropertyNames::textCategory, true, SchemaItemDiagValue::horzAlign, SchemaItemDiagValue::setHorzAlign);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::alignVert)
		{
			ADD_PROPERTY_GET_SET_CAT(E::VertAlign, PropertyNames::alignVert, PropertyNames::textCategory, true, SchemaItemDiagValue::vertAlign, SchemaItemDiagValue::setVertAlign);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::fontName)
		{
			ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::fontName, PropertyNames::textCategory, true, SchemaItemDiagValue::getFontName, SchemaItemDiagValue::setFontName);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::fontSize)
		{
			ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::fontSize, PropertyNames::textCategory, true, SchemaItemDiagValue::getFontSize, SchemaItemDiagValue::setFontSize);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::fontBold)
		{
			ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::fontBold, PropertyNames::textCategory, true, SchemaItemDiagValue::getFontBold, SchemaItemDiagValue::setFontBold);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::fontItalic)
		{
			ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::fontItalic, PropertyNames::textCategory, true, SchemaItemDiagValue::getFontItalic, SchemaItemDiagValue::setFontItalic);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::text)
		{
			ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::text, PropertyNames::functionalCategory, true, SchemaItemDiagValue::text, SchemaItemDiagValue::setText)
				->setDescription(PropertyNames::textValuePropDescription);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::precision)
		{
			ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::precision, PropertyNames::functionalCategory, true, SchemaItemDiagValue::precision, SchemaItemDiagValue::setPrecision)
				->setDescription(PropertyNames::precisionPropText);
		}

		// clang-format on
		return;
	}

	// Serialization
	//
	bool SchemaItemDiagValue::SaveData(Proto::Envelope* message) const
	{
		bool result = PosRectRotatable::SaveData(message);
		if (result == false ||
			message->HasExtension(Proto::schemaitem) == false)
		{
			assert(result);
			assert(message->HasExtension(Proto::schemaitem));
			return false;
		}

		// --
		//
		Proto::SchemaItemDiagValue* valueMessage = message->MutableExtension(Proto::schemaitem)->mutable_diagvalue();

		valueMessage->set_signalids(diagSignalIdsString(nullptr).toStdString()); // Set context to nullptr so ids WILL NOT be expanded

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

	bool SchemaItemDiagValue::LoadData(const Proto::Envelope& message)
	{
		if (message.HasExtension(Proto::schemaitem) == false)
		{
			assert(message.HasExtension(Proto::schemaitem));
			return false;
		}

		// --
		//
		bool result = PosRectRotatable::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.GetExtension(Proto::schemaitem).has_value() == false)
		{
			assert(message.GetExtension(Proto::schemaitem).has_diagvalue());
			return false;
		}

		const Proto::SchemaItemDiagValue& valueMessage = message.GetExtension(Proto::schemaitem).diagvalue();

		setDiagSignalIdsString(valueMessage.signalids().data());

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
	void SchemaItemDiagValue::draw(CDrawParam* drawParam) const
	{
		return drawRotated(drawParam, [drawParam, this]()
						   {
							   return drawPrivate(drawParam);
						   });
	}

	void SchemaItemDiagValue::drawPrivate(CDrawParam* drawParam) const
	{
		QPainter* painter = drawParam->painter();

		const std::shared_ptr<Context> context = this->context();
		if (context == nullptr)
		{
			Q_ASSERT(context);
			return;
		}

		// Calculate rectangle
		//
		QRectF r = boundingRectInDocPt(drawParam);

		// Drawing background
		//
		QBrush fillBrush{fillColor()};
		drawParam->painter()->fillRect(r, fillBrush);

		// Drawing text
		//
		drawText(drawParam, context.get(), r);

		// Remove brush to draw non-filled rectangle
		//
		painter->setBrush(Qt::NoBrush);

		// Drawing frame rect
		//
		if (drawRect() == true)
		{
			QPen rectPen{lineColor()};
			rectPen.setWidthF(m_lineWeight == 0.0 ? drawParam->cosmeticPenWidth() : m_lineWeight);

			painter->setPen(rectPen);
			painter->drawRect(r);
		}

		return;
	}

	void SchemaItemDiagValue::drawHighlight(CDrawParam* drawParam) const
	{
		return drawRotated(drawParam, [drawParam, this]()
						   {
							   return drawHighlightPrivate(drawParam);
						   });
	}

	void SchemaItemDiagValue::drawHighlightPrivate(CDrawParam* drawParam) const
	{
		bool highlight = drawParam->highlightIds().contains(label());

		// Draw highlights for m_diagSignalIds
		//
		for (const QString& diagSignalId : m_diagSignalIds)
		{
			if (drawParam->highlightIds().contains(diagSignalId) == true)
			{
				highlight = true;
				break;
			}
		}

		if (highlight == true)
		{
			QRectF highlightRect = boundingRectInDocPt(drawParam);
			drawHighlightRect(drawParam, highlightRect);
		}

		return;
	}

	void SchemaItemDiagValue::drawText(CDrawParam* drawParam, const Context* context, const QRectF& /*rect*/) const
	{
		Q_ASSERT(drawParam);
		Q_ASSERT(context);

		// TODO: SchemaItemDiagValue::drawText

		/*

		QPainter* painter = drawParam->painter();
		QString text;

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

			if (auto signalIdList = diagSignalIds(context);
				signalIdList.empty() == false)
			{
				signalId = signalIdList.front();

				signalParam.setAppSignalId(signalId);
				signalParam.setCustomSignalId(signalId);
			}

			bool ok = getSignalState(signalId, context, &signalParam, &signalState, &tuningSignalState);
			if (ok == false)
			{
				// Display signalId in case of error.
				//
				signalParam.setAppSignalId(signalId);
				signalParam.setCustomSignalId(signalId);
				signalParam.setCaption(signalId);
			}

			text = parseText(m_text, context, drawParam->session(), signalParam, signalState);
		}
		else
		{
			// Most likely text was set in PreDrawScript, or it is just text
			//
			text = m_text;
		}

		if (text.isEmpty() == true)
		{
			return;
		}

		painter->setPen(textColor());

#ifdef VFRAME30_CACHE_DRAW_TEXT
		if (drawParam->pdfMode() == true)
		{
			DrawHelper::drawText(painter,
								 m_font,
								 itemUnit(),
								 text,
								 rect,
								 static_cast<int>(horzAlign()) | static_cast<int>(vertAlign()));
		}
		else
		{
			DrawHelper::drawTextCahed(painter,
									  m_font,
									  itemUnit(),
									  text,
									  rect,
									  static_cast<int>(horzAlign()) | static_cast<int>(vertAlign()),
									  drawParam->schemaView()->zoom());
		}
#else
		DrawHelper::drawText(painter,
							 m_font,
							 itemUnit(),
							 text,
							 rect,
							 static_cast<int>(horzAlign()) | static_cast<int>(vertAlign()));
#endif
*/
		return;
	}

	QString SchemaItemDiagValue::parseText(QString text,
										   const Context* context,
										   const Session& session,
										   const HardwareLib::DiagSignal& /*signal*/
										   /*const AppSignalState& signalState*/) const
	{
		QString result = text;

		if (context == nullptr)
		{
			Q_ASSERT(context);
			return result;
		}

		QRegularExpression reStartIndex("\\$\\([a-zA-Z0-9]+"); // Search for $([SomeText])

		qsizetype index = 0;
		while (index < result.size())
		{
			// Find macro bounds
			//
			qsizetype startIndexOfMacro = result.indexOf(reStartIndex, index);
			if (startIndexOfMacro == -1)
			{
				break;
			}

			qsizetype endIndexOfMacro = result.indexOf(')', startIndexOfMacro + 1);
			if (endIndexOfMacro == -1)
			{
				break;
			}

			// Extract macro string
			//
			QString macro = result.mid(startIndexOfMacro + 2, endIndexOfMacro - startIndexOfMacro - 2); // +2 is $(, -2 is $()

			// Get value string
			//
			std::optional<QString> replaceText;
			do
			{
				//if (macro.compare(QLatin1String("value"), Qt::CaseInsensitive) == 0)
				//{
				//	if (signalState.isValid() == true)
				//	{
				//		replaceText = formatNumber(signalState.m_value, signal);
				//	}
				//	else
				//	{
				//		replaceText = QStringLiteral("?");
				//	}
				//	break;
				//}

				//if (macro.compare(QLatin1String("caption"), Qt::CaseInsensitive) == 0)
				//{
				//	replaceText = signal.caption();
				//	break;
				//}

				//if (macro.compare(QLatin1String("signalid"), Qt::CaseInsensitive) == 0)
				//{
				//	replaceText = signal.customSignalId();
				//	break;
				//}

				//if (macro.compare(QLatin1String("appsignalid"), Qt::CaseInsensitive) == 0)
				//{
				//	replaceText = signal.appSignalId();
				//	break;
				//}

				//if (macro.compare(QLatin1String("equipmentid"), Qt::CaseInsensitive) == 0)
				//{
				//	replaceText = signal.equipmentId();
				//	break;
				//}
			} while (false);

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

		// Expand all other macros.
		//
		result = MacrosExpander::parse(result, context, &session, this);

		return result;
	}

	QString SchemaItemDiagValue::formatNumber(double /*value*/, const HardwareLib::DiagSignal& /*signal*/) const
	{
		// TODO: SchemaItemDiagValue::formatNumber
		return {"TODO: SchemaItemDiagValue::formatNumber"};
		// 
		//if (signal.isDiscrete() == true)
		//{
		//	return QString::number(value, 'f', 0);
		//}

		//assert(signal.isAnalog());

		//int p = m_precision;
		//if (m_precision == -1)
		//{
		//	p = signal.precision();
		//}

		//return QString::number(value, static_cast<char>(analogFormat()), p);
	}

	bool SchemaItemDiagValue::getSignalState(QString appSignalId, const Context* context, HardwareLib::DiagSignal* signalParam) const
	{
		if (context == nullptr ||
			signalParam == nullptr /*||
			appSignalState == nullptr ||
			tuningSignalState == nullptr*/)
		{
			Q_ASSERT(context);
			Q_ASSERT(signalParam);
			// Q_ASSERT(appSignalState);
			// Q_ASSERT(tuningSignalState);
			return false;
		}

		// TODO: SchemaItemDiagValue::getSignalState
		return true;

		//bool ok = false;

		//switch (signalSource())
		//{
		//case E::SignalSource::AppDataService:
		//	if (auto appSignalController = context->appSignalController();
		//		appSignalController == nullptr)
		//	{
		//	}
		//	else
		//	{
		//		if (appSignalId.startsWith('@') == true)
		//		{
		//			appSignalId = appSignalController->appSignalManager().equipmentToAppSignalId(appSignalId);
		//		}

		//		*signalParam = context->appSignalController()->signalParam(appSignalId, &ok);
		//		*appSignalState = context->appSignalController()->signalState(appSignalId, nullptr);
		//	}
		//	break;

		//case E::SignalSource::TuningService:
		//	if (context->tuningController() == nullptr)
		//	{
		//	}
		//	else
		//	{
		//		*signalParam = context->tuningController()->signalParam(appSignalId, &ok);
		//		*tuningSignalState = context->tuningController()->signalState(appSignalId, nullptr);

		//		appSignalState->m_hash = signalParam->hash();
		//		appSignalState->m_flags.valid = tuningSignalState->valid();
		//		appSignalState->m_value = tuningSignalState->value().toDouble();
		//	}
		//	break;

		//default:
		//	Q_ASSERT(false);
		//	ok = false;
		//}

		//return ok;
	}

	double SchemaItemDiagValue::minimumPossibleHeightDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
	}

	double SchemaItemDiagValue::minimumPossibleWidthDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
	}

	// IMatsSchemaItemAssociations implementation.
	//
	QStringList SchemaItemDiagValue::associatedDiagObjectIds() const
	{
		return diagSignalIds();
	}

	QStringList SchemaItemDiagValue::associatedAppSignalIds() const
	{
		return diagSignalIds();
	}

	QStringList SchemaItemDiagValue::associatedImpactAppSignalIds() const
	{
		return {};
	}

	QStringList SchemaItemDiagValue::associatedConnectionIds() const
	{
		return {};
	}

	QStringList SchemaItemDiagValue::associatedLoopbackIds() const
	{
		return {};
	}

	QStringList SchemaItemDiagValue::associatedSchemaItemLabels() const
	{
		return {};
	}

	// Properties and Data
	//
	IMPLEMENT_FONT_PROPERTIES(SchemaItemDiagValue, Font, m_font);

	QString SchemaItemDiagValue::diagSignalIdsString() const
	{
		auto context = this->context();
		return diagSignalIdsString(context.get());
	}

	QString SchemaItemDiagValue::diagSignalIdsString(const Context* context) const
	{
		QStringList resultList = m_diagSignalIds;

		// Expand variables in AppSignalIDs in Monitor or Simulator modes.
		//
		if (context != nullptr &&
			context->viewVariables() != nullptr &&
			context->appSignalController() != nullptr)
		{
			resultList = MacrosExpander::parse(resultList, context, nullptr, this);

			for (QString& s : resultList)
			{
				if (s.startsWith('@') == true)
				{
					s = context->appSignalController()->appSignalManager().equipmentToAppSignalId(s);
				}
			}
		}

		return resultList.join(QChar::LineFeed);
	}

	void SchemaItemDiagValue::setDiagSignalIdsString(const QString& value)
	{
		m_diagSignalIds = value.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
	}

	QStringList SchemaItemDiagValue::diagSignalIds() const
	{
		auto context = this->context().get();
		return diagSignalIds(context);
	}

	QStringList SchemaItemDiagValue::diagSignalIds(const Context* context) const
	{
		QStringList resultList = m_diagSignalIds;

		// Expand variables in AppSignalIDs in MonitorMode, if applicable
		//
		if (context != nullptr &&
			context->viewVariables() != nullptr &&
			context->appSignalController() != nullptr)
		{
			resultList = MacrosExpander::parse(resultList, context, nullptr, this);

			for (QString& s : resultList)
			{
				if (s.startsWith('@') == true)
				{
					s = context->appSignalController()->appSignalManager().equipmentToAppSignalId(s);
				}
			}
		}

		return resultList;
	}

	void SchemaItemDiagValue::setDiagSignalIds(const QStringList& value)
	{
		m_diagSignalIds = value;
	}

	// Weight property
	//
	double SchemaItemDiagValue::lineWeight() const
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

	void SchemaItemDiagValue::setLineWeight(double weight)
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
	const QColor& SchemaItemDiagValue::lineColor() const
	{
		return m_lineColor;
	}
	void SchemaItemDiagValue::setLineColor(const QColor& color)
	{
		m_lineColor = color;
	}

	// FillColor property
	//
	const QColor& SchemaItemDiagValue::fillColor() const
	{
		return m_fillColor;
	}
	void SchemaItemDiagValue::setFillColor(const QColor& color)
	{
		m_fillColor = color;
	}

	// TextColor property
	//
	const QColor& SchemaItemDiagValue::textColor() const
	{
		return m_textColor;
	}
	void SchemaItemDiagValue::setTextColor(const QColor& color)
	{
		m_textColor = color;
	}

	// Align properties
	//
	E::HorzAlign SchemaItemDiagValue::horzAlign() const
	{
		return m_horzAlign;
	}
	void SchemaItemDiagValue::setHorzAlign(E::HorzAlign align)
	{
		m_horzAlign = align;
	}

	E::VertAlign SchemaItemDiagValue::vertAlign() const
	{
		return m_vertAlign;
	}

	void SchemaItemDiagValue::setVertAlign(E::VertAlign align)
	{
		m_vertAlign = align;
	}

	bool SchemaItemDiagValue::drawRect() const
	{
		return m_drawRect;
	}

	void SchemaItemDiagValue::setDrawRect(bool value)
	{
		m_drawRect = value;
	}

	const QString& SchemaItemDiagValue::text() const
	{
		return m_text;
	}
	void SchemaItemDiagValue::setText(QString value)
	{
		m_text = std::move(value);
	}

	int SchemaItemDiagValue::precision() const
	{
		return m_precision;
	}

	void SchemaItemDiagValue::setPrecision(int value)
	{
		m_precision = qBound(-1, value, 64);
	}

	E::AnalogFormat SchemaItemDiagValue::analogFormat() const
	{
		return m_analogFormat;
	}

	void SchemaItemDiagValue::setAnalogFormat(E::AnalogFormat value)
	{
		m_analogFormat = value;
	}
} // namespace VFrame30
