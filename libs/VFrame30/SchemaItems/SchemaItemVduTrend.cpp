#include <VFrame30/DrawParam.h>
#include <VFrame30/MacrosExpander.h>
#include <VFrame30/PropertyNames.h>
#include <VFrame30/SchemaItemVduTrend.h>


namespace VFrame30
{
	//
	// SchemaItemVduTrendSignalParam
	//
	SchemaItemVduTrendSignalParam::SchemaItemVduTrendSignalParam()
	{
		init();
		return;
	}

	SchemaItemVduTrendSignalParam::SchemaItemVduTrendSignalParam(const SchemaItemVduTrendSignalParam& src)
	{
		Proto::SchemaItemVduTrendSignal message;

		src.save(&message);
		load(message);

		init();
		return;
	}

	void SchemaItemVduTrendSignalParam::init()
	{
		removeAllProperties();

		Property* p = nullptr;

		ADD_PROPERTY_GETTER_SETTER(QString,
								   PropertyNames::appSignalID,
								   true,
								   SchemaItemVduTrendSignalParam::appSignalId,
								   SchemaItemVduTrendSignalParam::setAppSignalId)
			->setViewOrder(1);


		ADD_PROPERTY_GETTER_SETTER(QString,
								   PropertyNames::validityAppSignalID,
								   true,
								   SchemaItemVduTrendSignalParam::validityAppSignalId,
								   SchemaItemVduTrendSignalParam::setValidityAppSignalId)
			->setViewOrder(2);

		p = ADD_PROPERTY_GETTER_SETTER(int,
									   PropertyNames::precision,
									   true,
									   SchemaItemVduTrendSignalParam::precision,
									   SchemaItemVduTrendSignalParam::setPrecision);
		p->setDescription(PropertyNames::precisionPropText);
		p->setViewOrder(3);

		ADD_PROPERTY_GETTER_SETTER(E::DisplayValueFormat,
								   PropertyNames::valueFormat,
								   true,
								   SchemaItemVduTrendSignalParam::valueFormat,
								   SchemaItemVduTrendSignalParam::setValueFormat)
			->setViewOrder(4);

		ADD_PROPERTY_GETTER_SETTER(double,
								   PropertyNames::lowLimit,
								   true,
								   SchemaItemVduTrendSignalParam::lowViewLimit,
								   SchemaItemVduTrendSignalParam::setLowViewLimit)
			->setViewOrder(5);

		ADD_PROPERTY_GETTER_SETTER(double,
								   PropertyNames::highLimit,
								   true,
								   SchemaItemVduTrendSignalParam::highViewLimit,
								   SchemaItemVduTrendSignalParam::setHighViewLimit)
			->setViewOrder(6);

		ADD_PROPERTY_GETTER_SETTER(QColor,
								   PropertyNames::color,
								   true,
								   SchemaItemVduTrendSignalParam::color,
								   SchemaItemVduTrendSignalParam::setColor)
			->setViewOrder(7);

		ADD_PROPERTY_GETTER_SETTER(int,
								   PropertyNames::lineWeight,
								   true,
								   SchemaItemVduTrendSignalParam::lineWeight,
								   SchemaItemVduTrendSignalParam::setLineWeight)
			->setViewOrder(8);
	}

	void SchemaItemVduTrendSignalParam::save(Proto::SchemaItemVduTrendSignal* message) const
	{
		assert(message != nullptr);

		message->set_appsignalid(m_appSignalId.toStdString());
		message->set_validityappsignalid(m_validityAppSignalId.toStdString());

		message->set_precision(m_precision);
		message->set_valueformat(static_cast<int>(m_valueFormat));

		message->set_lowlimit(m_lowViewLimit);
		message->set_highlimit(m_highViewLimit);

		message->set_color(m_color.rgba());
		message->set_lineweight(m_lineWeight);

		return;
	}

	void SchemaItemVduTrendSignalParam::load(const Proto::SchemaItemVduTrendSignal& message)
	{
		m_appSignalId = QString::fromStdString(message.appsignalid());
		m_validityAppSignalId = QString::fromStdString(message.validityappsignalid());

		m_precision = message.precision();
		m_valueFormat = static_cast<E::DisplayValueFormat>(message.valueformat());

		m_lowViewLimit = message.lowlimit();
		m_highViewLimit = message.highlimit();

		m_color = QColor::fromRgba(message.color());
		m_lineWeight = message.lineweight();

		return;
	}

	QString SchemaItemVduTrendSignalParam::appSignalId() const
	{
		return m_appSignalId;
	}

	void SchemaItemVduTrendSignalParam::setAppSignalId(const QString& value)
	{
		m_appSignalId = value.trimmed();
	}

	QString SchemaItemVduTrendSignalParam::validityAppSignalId() const
	{
		return m_validityAppSignalId;
	}

	void SchemaItemVduTrendSignalParam::setValidityAppSignalId(const QString& value)
	{
		m_validityAppSignalId = value.trimmed();
	}

	int SchemaItemVduTrendSignalParam::precision() const
	{
		return m_precision;
	}

	void SchemaItemVduTrendSignalParam::setPrecision(int value)
	{
		m_precision =
			std::clamp(value, -1, 16); // Assuming the valid range for precision is 0 to 16, and -1 to take precision from signal definition
	}

	E::DisplayValueFormat SchemaItemVduTrendSignalParam::valueFormat() const
	{
		return m_valueFormat;
	}

	void SchemaItemVduTrendSignalParam::setValueFormat(E::DisplayValueFormat value)
	{
		m_valueFormat = value;
	}

	double SchemaItemVduTrendSignalParam::lowViewLimit() const
	{
		return m_lowViewLimit;
	}

	void SchemaItemVduTrendSignalParam::setLowViewLimit(double value)
	{
		// Use float, as VDU works with float values.
		//
		m_lowViewLimit =
			std::clamp<double>(value, std::numeric_limits<float>::lowest(), m_highViewLimit - std::numeric_limits<float>::epsilon());
	}

	double SchemaItemVduTrendSignalParam::highViewLimit() const
	{
		return m_highViewLimit;
	}

	void SchemaItemVduTrendSignalParam::setHighViewLimit(double value)
	{
		// Use float, as VDU works with float values.
		m_highViewLimit =
			std::clamp<double>(value, m_lowViewLimit + std::numeric_limits<float>::epsilon(), std::numeric_limits<float>::max());
	}

	QColor SchemaItemVduTrendSignalParam::color() const
	{
		return m_color;
	}

	void SchemaItemVduTrendSignalParam::setColor(const QColor& value)
	{
		m_color = value;
	}

	int SchemaItemVduTrendSignalParam::lineWeight() const
	{
		return m_lineWeight;
	}

	void SchemaItemVduTrendSignalParam::setLineWeight(int value)
	{
		m_lineWeight = std::clamp(value, 1, 6); // Assuming the valid range for line weight is 1 to 6
	}

	//
	// SchemaItemVduTrend
	//
	SchemaItemVduTrend::SchemaItemVduTrend(void) :
		SchemaItemVduTrend(SchemaUnit::Display)
	{
	}

	SchemaItemVduTrend::SchemaItemVduTrend(SchemaUnit units) :
		PosRectImpl{}
	{
		assert(units == SchemaUnit::Display);
		setItemUnit(units);

		m_static = false;

		// --
		//
		Property* p{};

		// Durations
		//
		p = ADD_PROPERTY_GETTER_SETTER(QString,
									   PropertyNames::indicatorTrendLaneDurations,
									   true,
									   SchemaItemVduTrend::durationsSecondsStr,
									   SchemaItemVduTrend::setDurationsSecondsStr);
		p->setCategory(PropertyNames::indicatorSettings);
		p->setDescription(PropertyNames::indicatorTrendLaneDurationsToolTip);

		
		// viewMode
		//
		p = ADD_PROPERTY_GETTER_SETTER(E::TrendViewMode,
									   PropertyNames::indicatorTrendViewMode,
									   true,
									   SchemaItemVduTrend::viewMode,
									   SchemaItemVduTrend::setViewMode);
		p->setCategory(PropertyNames::indicatorSettings);

		// scaleType
		//
		p = ADD_PROPERTY_GETTER_SETTER(E::TrendScaleType,
									   PropertyNames::indicatorTrendScaleType,
									   true,
									   SchemaItemVduTrend::scaleType,
									   SchemaItemVduTrend::setScaleType);
		p->setCategory(PropertyNames::indicatorSettings);

		// indentLeft
		//
		p = ADD_PROPERTY_GETTER_SETTER(int,
									   PropertyNames::indicatorTrendIndentLeft,
									   true,
									   SchemaItemVduTrend::indentLeft,
									   SchemaItemVduTrend::setIndentLeft);
		p->setCategory(PropertyNames::positionAndSizeCategory);
		p->setDescription(PropertyNames::indicatorTrendIndentDescription);
		p->setPrecision(0);

		// indentRight
		//
		p = ADD_PROPERTY_GETTER_SETTER(int,
									   PropertyNames::indicatorTrendIndentRight,
									   true,
									   SchemaItemVduTrend::indentRight,
									   SchemaItemVduTrend::setIndentRight);
		p->setCategory(PropertyNames::positionAndSizeCategory);
		p->setDescription(PropertyNames::indicatorTrendIndentDescription);
		p->setPrecision(0);

		// indentTop
		//
		p = ADD_PROPERTY_GETTER_SETTER(int,
									   PropertyNames::indicatorTrendIndentTop,
									   true,
									   SchemaItemVduTrend::indentTop,
									   SchemaItemVduTrend::setIndentTop);
		p->setCategory(PropertyNames::positionAndSizeCategory);
		p->setDescription(PropertyNames::indicatorTrendIndentDescription);
		p->setPrecision(0);

		// indentBottom
		//
		p = ADD_PROPERTY_GETTER_SETTER(int,
									   PropertyNames::indicatorTrendIndentBottom,
									   true,
									   SchemaItemVduTrend::indentBottom,
									   SchemaItemVduTrend::setIndentBottom);
		p->setCategory(PropertyNames::positionAndSizeCategory);
		p->setDescription(PropertyNames::indicatorTrendIndentDescription);
		p->setPrecision(0);

		// lineColor
		//
		p = ADD_PROPERTY_GETTER_SETTER(QColor,
									   PropertyNames::lineColor,
									   true,
									   SchemaItemVduTrend::lineColor,
									   SchemaItemVduTrend::setLineColor);
		p->setCategory(PropertyNames::indicatorSettings);

		// backColor
		//
		p = ADD_PROPERTY_GETTER_SETTER(QColor,
									   PropertyNames::backColor,
									   true,
									   SchemaItemVduTrend::backColor,
									   SchemaItemVduTrend::setBackColor);
		p->setCategory(PropertyNames::indicatorSettings);

		// backColor1
		//
		p = ADD_PROPERTY_GETTER_SETTER(QColor,
									   PropertyNames::indicatorTrendBackColor1st,
									   true,
									   SchemaItemVduTrend::backColor1st,
									   SchemaItemVduTrend::setBackColor1st);
		p->setCategory(PropertyNames::indicatorSettings);

		// backColor2
		//
		p = ADD_PROPERTY_GETTER_SETTER(QColor,
									   PropertyNames::indicatorTrendBackColor2nd,
									   true,
									   SchemaItemVduTrend::backColor2nd,
									   SchemaItemVduTrend::setBackColor2nd);
		p->setCategory(PropertyNames::indicatorSettings);

		// showSignalIds
		//
		p = ADD_PROPERTY_GETTER_SETTER(bool,
									   PropertyNames::indicatorTrendShowSignalIds,
									   true,
									   SchemaItemVduTrend::showSignalIds,
									   SchemaItemVduTrend::setShowSignalIds);
		p->setCategory(PropertyNames::indicatorSettings);

		// showSignalCaptions
		//
		p = ADD_PROPERTY_GETTER_SETTER(bool,
									   PropertyNames::indicatorTrendShowSignalCaptions,
									   true,
									   SchemaItemVduTrend::showSignalCaptions,
									   SchemaItemVduTrend::setShowSignalCaptions);
		p->setCategory(PropertyNames::indicatorSettings);

		// showSignalScales
		//
		p = ADD_PROPERTY_GETTER_SETTER(bool,
									   PropertyNames::indicatorTrendShowSignalScales,
									   true,
									   SchemaItemVduTrend::showSignalScales,
									   SchemaItemVduTrend::setShowSignalScales);
		p->setCategory(PropertyNames::indicatorSettings);

		// showTimeLabels
		//
		p = ADD_PROPERTY_GETTER_SETTER(bool,
									   PropertyNames::indicatorTrendShowTimeLabels,
									   true,
									   SchemaItemVduTrend::showTimeLabels,
									   SchemaItemVduTrend::setShowTimeLabels);
		p->setCategory(PropertyNames::indicatorSettings);

		// showDateLabels
		//
		p = ADD_PROPERTY_GETTER_SETTER(bool,
									   PropertyNames::indicatorTrendShowDateLabels,
									   true,
									   SchemaItemVduTrend::showDateLabels,
									   SchemaItemVduTrend::setShowDateLabels);
		p->setCategory(PropertyNames::indicatorSettings);

		// trendSignalParams
		//
		ADD_PROPERTY_GETTER_SETTER(PropertyVector<SchemaItemVduTrendSignalParam>,
								   PropertyNames::trendSignalParams,
								   true,
								   SchemaItemVduTrend::signalParams,
								   SchemaItemVduTrend::setSignalParams)
			->setCategory(PropertyNames::indicatorSettings);

		// Font
		//
		ADD_PROPERTY_GET_SET_CAT(QString,
								 PropertyNames::fontName,
								 PropertyNames::textCategory,
								 true,
								 SchemaItemVduTrend::getFontName,
								 SchemaItemVduTrend::setFontName);

		ADD_PROPERTY_GET_SET_CAT(double,
								 PropertyNames::fontSize,
								 PropertyNames::textCategory,
								 true,
								 SchemaItemVduTrend::getFontSize,
								 SchemaItemVduTrend::setFontSize)
			->setPrecision(0);

		ADD_PROPERTY_GET_SET_CAT(bool,
								 PropertyNames::fontBold,
								 PropertyNames::textCategory,
								 true,
								 SchemaItemVduTrend::getFontBold,
								 SchemaItemVduTrend::setFontBold);

		ADD_PROPERTY_GET_SET_CAT(bool,
								 PropertyNames::fontItalic,
								 PropertyNames::textCategory,
								 true,
								 SchemaItemVduTrend::getFontItalic,
								 SchemaItemVduTrend::setFontItalic);

		m_font.setName(QStringLiteral("Arial"));
		Q_ASSERT(units == SchemaUnit::Display);

		m_font.setSize(12.0, units);

		return;
	}

	// Serialization
	//
	bool SchemaItemVduTrend::SaveData(Proto::Envelope* message) const
	{
		bool result = PosRectImpl::SaveData(message);
		if (result == false || message->HasExtension(Proto::schemaitem) == false)
		{
			assert(result);
			assert(message->HasExtension(Proto::schemaitem));
			return false;
		}

		// --
		//
		auto trendMessage = message->MutableExtension(Proto::schemaitem)->mutable_vdutrend();

		trendMessage->set_durationssecs(m_durationsSecs.toStdString());

		trendMessage->set_viewmode(static_cast<int32_t>(m_viewMode));
		trendMessage->set_scaletype(static_cast<int32_t>(m_scaleType));

		trendMessage->set_indentleft(m_indentLeft);
		trendMessage->set_indentright(m_indentRight);
		trendMessage->set_indenttop(m_indentTop);
		trendMessage->set_indentbottom(m_indentBottom);

		trendMessage->set_linecolor(m_lineColor.rgba());
		trendMessage->set_backcolor(m_backColor.rgba());
		trendMessage->set_backcolor1st(m_backColor1st.rgba());
		trendMessage->set_backcolor2nd(m_backColor2nd.rgba());

		trendMessage->set_showsignalids(m_showSignalIds);
		trendMessage->set_showsignalcaptions(m_showSignalCaptions);
		trendMessage->set_showsignalscales(m_showSignalScales);
		trendMessage->set_showtimelabels(m_showTimeLabels);
		trendMessage->set_showdatelabels(m_showDateLabels);

		// save m_signalParams
		//
		for (const auto& signalParam : m_signalParams)
		{
			assert(signalParam != nullptr);

			auto signalMessage = trendMessage->add_signalparams();
			signalParam->save(signalMessage);
		}

		m_font.SaveData(trendMessage->mutable_font());

		return true;
	}

	bool SchemaItemVduTrend::LoadData(const Proto::Envelope& message)
	{
		if (message.HasExtension(Proto::schemaitem) == false)
		{
			assert(message.HasExtension(Proto::schemaitem));
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
		const auto& schemaItemMessage = message.GetExtension(Proto::schemaitem);

		if (schemaItemMessage.has_vdutrend() == false)
		{
			assert(schemaItemMessage.has_vdutrend());
			return false;
		}

		const auto& trendMessage = schemaItemMessage.vdutrend();

		m_durationsSecs = QString::fromStdString(trendMessage.durationssecs());

		m_viewMode = static_cast<E::TrendViewMode>(trendMessage.viewmode());
		m_scaleType = static_cast<E::TrendScaleType>(trendMessage.scaletype());

		m_indentLeft = trendMessage.indentleft();
		m_indentRight = trendMessage.indentright();
		m_indentTop = trendMessage.indenttop();
		m_indentBottom = trendMessage.indentbottom();

		m_lineColor = QColor::fromRgba(trendMessage.linecolor());
		m_backColor = QColor::fromRgba(trendMessage.backcolor());
		m_backColor1st = QColor::fromRgba(trendMessage.backcolor1st());
		m_backColor2nd = QColor::fromRgba(trendMessage.backcolor2nd());

		m_showSignalIds = trendMessage.showsignalids();
		m_showSignalCaptions = trendMessage.showsignalcaptions();
		m_showSignalScales = trendMessage.showsignalscales();
		m_showTimeLabels = trendMessage.showtimelabels();
		m_showDateLabels = trendMessage.showdatelabels();

		// Load m_signalParams
		//
		m_signalParams.clear();
		m_signalParams.reserve(trendMessage.signalparams_size());

		for (const auto& signalMessage : trendMessage.signalparams())
		{
			auto signalParam = std::make_shared<SchemaItemVduTrendSignalParam>();
			signalParam->load(signalMessage);
			m_signalParams.push_back(signalParam);
		}

		m_font.LoadData(trendMessage.font());

		return true;
	}

	// Drawing Functions
	//
	void SchemaItemVduTrend::draw(CDrawParam* drawParam) const
	{
		QPainter* painter = drawParam->painter();
		QRectF boundingRect = boundingRectInDocPt(drawParam);

		painter->save();

		auto restorePainter = qScopeGuard(
			[painter]()
			{
				painter->restore();
			});

		painter->fillRect(boundingRect, m_backColor);

		QRectF trendAreaRect = boundingRect;

		if (m_indentLeft < 0)
		{
			// Auto indent left by 5% of width
			//
			trendAreaRect.setLeft(boundingRect.left() + boundingRect.width() * 0.05);
		}
		else
		{
			trendAreaRect.setLeft(boundingRect.left() + m_indentLeft);
		}

		if (m_indentRight < 0)
		{
			// Auto indent right by 5% of width
			//
			trendAreaRect.setRight(boundingRect.right() - boundingRect.width() * 0.05);
		}
		else
		{
			trendAreaRect.setRight(boundingRect.right() - m_indentRight);
		}

		if (m_indentTop < 0)
		{
			// Auto indent top by 5% of height
			//
			trendAreaRect.setTop(boundingRect.top() + boundingRect.height() * 0.05);
		}
		else
		{
			trendAreaRect.setTop(boundingRect.top() + m_indentTop);
		}

		if (m_indentBottom < 0)
		{
			// Auto indent bottom by 5% of height
			//
			trendAreaRect.setBottom(boundingRect.bottom() - boundingRect.height() * 0.05);
		}
		else
		{
			trendAreaRect.setBottom(boundingRect.bottom() - m_indentBottom);
		}

		if (trendAreaRect.width() < 0)
		{
			trendAreaRect.setWidth(0);
		}

		if (trendAreaRect.height() < 0)
		{
			trendAreaRect.setHeight(0);
		}

		if (trendAreaRect.isEmpty() == false)
		{
			// Draw signal lane 1
			//
			painter->setPen(Qt::NoPen);
			painter->fillRect(trendAreaRect, m_backColor2nd);

			// Draw signal lane 2
			//
			auto halfRect = trendAreaRect;
			halfRect.setTop(trendAreaRect.top() + trendAreaRect.height() / 2);
			painter->fillRect(halfRect, m_backColor1st);

			// Draw border
			//
			painter->setPen(m_lineColor);
			painter->setBrush(Qt::NoBrush);
			painter->drawRect(trendAreaRect);
		}

		// Draw signal ids
		//
		painter->setClipRect(trendAreaRect);

		QFont font = m_font.qfont(itemUnit(), 0);
		font.setStyleStrategy(QFont::PreferAntialias);

		painter->setFont(font);

		QRectF textBounds;
		QPen savedPen = painter->pen();
		QPen measurePen = savedPen;
		measurePen.setColor(Qt::transparent);
		painter->setPen(measurePen);
		painter->drawText(QRectF{},
						  static_cast<int>(Qt::AlignLeft) | static_cast<int>(Qt::AlignTop) | static_cast<int>(Qt::TextSingleLine),
						  QStringLiteral("Ag"),
						  &textBounds);
		painter->setPen(savedPen);

		double rowHeight = textBounds.height();
		if (rowHeight <= 0.0)
		{
			rowHeight = painter->fontMetrics().height();
		}

		const double rowSpacing = rowHeight * 1.15;
		const double textLeft = trendAreaRect.left() + 4.0;
		double textTop = trendAreaRect.top();

		for (const auto& signalParamPtr : m_signalParams)
		{
			assert(signalParamPtr != nullptr);

			const SchemaItemVduTrendSignalParam& signalParam = *signalParamPtr;
			QRectF rowRect{textLeft, textTop, trendAreaRect.right() - textLeft, rowSpacing};

			painter->setPen(signalParam.color());
			painter->drawText(rowRect, static_cast<int>(Qt::AlignLeft) | static_cast<int>(Qt::AlignVCenter), signalParam.appSignalId());

			textTop += rowSpacing;
			if (textTop > trendAreaRect.bottom())
			{
				break;
			}
		}

		return;
	}

	double SchemaItemVduTrend::minimumPossibleHeightDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
	}

	double SchemaItemVduTrend::minimumPossibleWidthDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
	}

	// IMatsSchemaItemAssociations implementation.
	//
	QStringList SchemaItemVduTrend::associatedDiagObjectIds() const
	{
		return {};
	};

	QStringList SchemaItemVduTrend::associatedAppSignalIds() const
	{
		QStringList result;
		result.reserve(m_signalParams.size() * 2);

		for (const auto& signalParamPtr : m_signalParams)
		{
			assert(signalParamPtr != nullptr);
			const SchemaItemVduTrendSignalParam& signalParam = *signalParamPtr;

			if (signalParam.appSignalId().isEmpty() == false)
			{
				result << signalParam.appSignalId();
			}

			if (signalParam.validityAppSignalId().trimmed().isEmpty() == false)
			{
				result << signalParam.validityAppSignalId();
			}
		}

		return result;
	}

	QStringList SchemaItemVduTrend::associatedImpactAppSignalIds() const
	{
		return {};
	}

	QStringList SchemaItemVduTrend::associatedConnectionIds() const
	{
		return {};
	}

	QStringList SchemaItemVduTrend::associatedLoopbackIds() const
	{
		return {};
	}

	QStringList SchemaItemVduTrend::associatedSchemaItemLabels() const
	{
		return {};
	}

	QString SchemaItemVduTrend::durationsSecondsStr() const
	{ 
		return m_durationsSecs;
	}

	void SchemaItemVduTrend::setDurationsSecondsStr(QString value)
	{ 
		if (value.isEmpty() == true) 
		{
			return;
		}

		static const auto re = QRegularExpression("\\W+");
		QStringList l = value.split(re, Qt::SkipEmptyParts);

		for (const QString& s : l)
		{
			bool ok = false;
			s.toULong(&ok);
			if (ok == false)
			{
				return;
			}
		}

		m_durationsSecs = value;
	}

	std::vector<uint32_t> SchemaItemVduTrend::durationsSeconds() const
	{
		std::vector<uint32_t> result;

		static const auto re = QRegularExpression("\\W+");
		QStringList l = m_durationsSecs.split(re, Qt::SkipEmptyParts);

		for (const QString& s : l) 
		{
			bool ok = false;
			uint32_t v = s.toULong(&ok);
			if (ok == true) 
			{
				result.push_back(v);
			}
		}

		return result;
	}

	int SchemaItemVduTrend::columnCount() const
	{
		int result = width();

		const int TrendDefaultItend = 5;
		const int TrendDefaultScaleIdent = 100;

		if (indentLeft() != -1)
		{
			result -= (indentLeft() + showSignalScales() ? TrendDefaultScaleIdent : TrendDefaultItend);
		}
		else
		{
			result -= (TrendDefaultItend + showSignalScales() ? TrendDefaultScaleIdent : TrendDefaultItend);
		}

		if (indentRight() != -1)
		{
			result -= indentRight();
		}
		else
		{
			result -= TrendDefaultItend;
		}

		if (result < 0) 
		{
			Q_ASSERT(result >= 0);
			result = 0;
		}

		return result;
	}

	
	E::TrendViewMode SchemaItemVduTrend::viewMode() const
	{
		return m_viewMode;
	}

	void SchemaItemVduTrend::setViewMode(E::TrendViewMode value)
	{
		m_viewMode = value;
	}

	E::TrendScaleType SchemaItemVduTrend::scaleType() const
	{
		return m_scaleType;
	}

	void SchemaItemVduTrend::setScaleType(E::TrendScaleType value)
	{
		m_scaleType = value;
	}

	int SchemaItemVduTrend::indentLeft() const
	{
		return m_indentLeft;
	}

	void SchemaItemVduTrend::setIndentLeft(int value)
	{
		m_indentLeft = value;
	}

	int SchemaItemVduTrend::indentRight() const
	{
		return m_indentRight;
	}

	void SchemaItemVduTrend::setIndentRight(int value)
	{
		m_indentRight = value;
	}

	int SchemaItemVduTrend::indentTop() const
	{
		return m_indentTop;
	}

	void SchemaItemVduTrend::setIndentTop(int value)
	{
		m_indentTop = value;
	}

	int SchemaItemVduTrend::indentBottom() const
	{
		return m_indentBottom;
	}

	void SchemaItemVduTrend::setIndentBottom(int value)
	{
		m_indentBottom = value;
	}

	QColor SchemaItemVduTrend::lineColor() const
	{
		return m_lineColor;
	}

	void SchemaItemVduTrend::setLineColor(const QColor& value)
	{
		m_lineColor = value;
	}

	QColor SchemaItemVduTrend::backColor() const
	{
		return m_backColor;
	}

	void SchemaItemVduTrend::setBackColor(const QColor& value)
	{
		m_backColor = value;
	}

	QColor SchemaItemVduTrend::backColor1st() const
	{
		return m_backColor1st;
	}

	void SchemaItemVduTrend::setBackColor1st(const QColor& value)
	{
		m_backColor1st = value;
	}

	QColor SchemaItemVduTrend::backColor2nd() const
	{
		return m_backColor2nd;
	}

	void SchemaItemVduTrend::setBackColor2nd(const QColor& value)
	{
		m_backColor2nd = value;
	}

	bool SchemaItemVduTrend::showSignalIds() const
	{
		return m_showSignalIds;
	}

	void SchemaItemVduTrend::setShowSignalIds(bool value)
	{
		m_showSignalIds = value;
	}

	bool SchemaItemVduTrend::showSignalCaptions() const
	{
		return m_showSignalCaptions;
	}

	void SchemaItemVduTrend::setShowSignalCaptions(bool value)
	{
		m_showSignalCaptions = value;
	}

	bool SchemaItemVduTrend::showSignalScales() const
	{
		return m_showSignalScales;
	}

	void SchemaItemVduTrend::setShowSignalScales(bool value)
	{
		m_showSignalScales = value;
	}

	bool SchemaItemVduTrend::showTimeLabels() const
	{
		return m_showTimeLabels;
	}

	void SchemaItemVduTrend::setShowTimeLabels(bool value)
	{
		m_showTimeLabels = value;
	}

	bool SchemaItemVduTrend::showDateLabels() const
	{
		return m_showDateLabels;
	}

	void SchemaItemVduTrend::setShowDateLabels(bool value)
	{
		m_showDateLabels = value;
	}

	PropertyVector<SchemaItemVduTrendSignalParam> SchemaItemVduTrend::signalParams() const
	{
		return m_signalParams;
	}

	void SchemaItemVduTrend::setSignalParams(const PropertyVector<SchemaItemVduTrendSignalParam>& value)
	{
		m_signalParams = value;

		while (m_signalParams.size() > SchemaItemVduTrend::MaxSignalCount)
		{
			m_signalParams.pop_back();
		}
	}

	IMPLEMENT_FONT_PROPERTIES(SchemaItemVduTrend, Font, m_font);

} // namespace VFrame30
