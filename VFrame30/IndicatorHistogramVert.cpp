#include "IndicatorHistogramVert.h"
#include "SchemaItemIndicator.h"
#include "PropertyNames.h"
#include "DrawParam.h"
#include "../lib/AppSignal.h"
#include "../lib/ComparatorSet.h"
#include "AppSignalController.h"


namespace VFrame30
{
	void CustomSetPoint::propertyDemand(const QString&)
	{
		// Common category
		//
		ADD_PROPERTY_GETTER_SETTER(E::IndicatorColorSource, PropertyNames::indicatorColorSource, true, CustomSetPoint::colorSource, CustomSetPoint::setColorSource);
		ADD_PROPERTY_GETTER_SETTER(QColor, PropertyNames::color, true, CustomSetPoint::color, CustomSetPoint::setColor);
		ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::indicatorOutputAppSignalId, true, CustomSetPoint::outputAppSignalId, CustomSetPoint::setOutputAppSignalId);

		return;
	}

	bool CustomSetPoint::save(Proto::VFrameSetPoint* message) const
	{
		if (message == nullptr)
		{
			assert(message);
			return false;
		}

		message->set_colorsource(static_cast<int32_t>(m_colorSource));
		message->set_color(m_color.rgba());

		message->set_outputappsignalid(m_outputAppSignalId.toStdString());

		return true;
	}

	bool CustomSetPoint::load(const Proto::VFrameSetPoint& message)
	{
		m_colorSource = static_cast<E::IndicatorColorSource>(message.colorsource());
		m_color = message.color();

		m_outputAppSignalId = QString::fromStdString(message.outputappsignalid());

		return true;
	}

	E::IndicatorColorSource CustomSetPoint::colorSource() const
	{
		return m_colorSource;
	}

	void CustomSetPoint::setColorSource(E::IndicatorColorSource value)
	{
		m_colorSource = value;
	}

	QColor CustomSetPoint::color() const
	{
		return m_color;
	}

	void CustomSetPoint::setColor(const QColor& value)
	{
		m_color = value;
	}

	const QString& CustomSetPoint::outputAppSignalId() const
	{
		return m_outputAppSignalId;
	}

	void CustomSetPoint::setOutputAppSignalId(const QString& value)
	{
		m_outputAppSignalId = value;
	}


	//
	// Vertical histogram, the base view
	//
	IndicatorHistogramVert::IndicatorHistogramVert(SchemaUnit itemUnit) :
		Indicator(itemUnit),
		m_barWidth(itemUnit == SchemaUnit::Display ? 20 : mm2in(4)),
		m_leftMargin(itemUnit == SchemaUnit::Display ? 50 : mm2in(15)),
		m_topMargin(itemUnit == SchemaUnit::Display ? 10 : mm2in(5)),
		m_rightMargin(itemUnit == SchemaUnit::Display ? 10 : mm2in(5)),
		m_bottomMargin(itemUnit == SchemaUnit::Display ? 10 : mm2in(5))
	{
	}

	E::IndicatorScaleType IndicatorHistogramVert::scaleType() const
	{
		return m_scaleType;
	}

	void IndicatorHistogramVert::setScaleType(E::IndicatorScaleType value)
	{
		m_scaleType = value;

		// If user changes scale type with default range - set start value to 1 to make logarithmic scale correct
		//
		if (m_scaleType == E::IndicatorScaleType::Logarithmic && m_startValue == 0 && m_endValue == 100)
		{
			m_startValue = 1;
		}

		emit updatePropertiesList();

		return;
	}

	void IndicatorHistogramVert::createProperties(SchemaItemIndicator* propertyObject, int /*signalCount*/)
	{
		propertyObject->ADD_PROPERTY_CAT_VAR(double,
											 PropertyNames::indicatorEndValue,
											 PropertyNames::indicatorSettings,
											 true,
											 m_endValue)
				->setViewOrder(0);

		propertyObject->ADD_PROPERTY_CAT_VAR(double,
											 PropertyNames::indicatorStartValue,
											 PropertyNames::indicatorSettings,
											 true,
											 m_startValue)
				->setViewOrder(1);

		propertyObject->addProperty<double>(PropertyNames::indicatorBarWidth,
											PropertyNames::indicatorSettings,
											true,
											[this](){ return regionalGetter(m_barWidth);},
		[this](const auto& value){ return regionalSetter(value, &m_barWidth);})
				->setViewOrder(2);

		propertyObject->addProperty<double>(PropertyNames::indicatorMargingLeft,
											PropertyNames::indicatorSettings,
											true,
											[this](){ return regionalGetter(m_leftMargin);},
		[this](const auto& value){ return regionalSetter(value, &m_leftMargin);})
				->setViewOrder(10);

		propertyObject->addProperty<double>(PropertyNames::indicatorMargingTop,
											PropertyNames::indicatorSettings,
											true,
											[this](){ return regionalGetter(m_topMargin);},
		[this](const auto& value){ return regionalSetter(value, &m_topMargin);})
				->setViewOrder(11);

		propertyObject->addProperty<double>(PropertyNames::indicatorMargingRight,				// 1
											PropertyNames::indicatorSettings,
											true,
											[this](){ return regionalGetter(m_rightMargin);},	// 2
		[this](const auto& value){ return regionalSetter(value, &m_rightMargin);})				// 3
				->setViewOrder(12);																// 4

		propertyObject->addProperty<double>(PropertyNames::indicatorMargingBottom,				// 1
											PropertyNames::indicatorSettings,
											true,
											[this](){ return regionalGetter(m_bottomMargin);},	// 2
		[this](const auto& value){ return regionalSetter(value, &m_bottomMargin);})				// 3
				->setViewOrder(13);																// 4

		propertyObject->ADD_PROPERTY_CAT_VAR(bool,
											 PropertyNames::indicatorDrawBarRect,
											 PropertyNames::indicatorSettings,
											 true,
											 m_drawBarRect);

		// bool m_drawGrid = true;					// Draw grids
		// bool m_drawGridForAllBars = false;		// Draw grids for all bars
		// bool m_drawGridValues = true;			// Draw values for grid (only if m_drawGrid == true, for next bars depends on DrawGridOnlyForFirstBar)
		// bool m_drawGridValueForAllBars = false;	// Draw values for grid for all bars (true) or just for the first one (false) (only if drawGrid == true && drawGridValues == true)
		// bool m_drawGridValueUnits = true;		// Draw units for limits values (only if DrawGrid == true && DrawGridValues == true)
		// double m_gridMainStep = 50.0;			// Step for main grids (only if DrawGrid == true)
		// double m_gridSmallStep = 10.0;			// Step for small grids (only if DrawGrid == true)
		//
		propertyObject->ADD_PROPERTY_CAT_VAR(bool,
											 PropertyNames::drawGrid,
											 PropertyNames::indicatorSettings,
											 true,
											 m_drawGrid)
				->setDescription(QStringLiteral("Draw grids"));

		propertyObject->ADD_PROPERTY_CAT_VAR(bool,
											 PropertyNames::drawGridForAllBars,
											 PropertyNames::indicatorSettings,
											 true,
											 m_drawGridForAllBars)
				->setDescription(QStringLiteral("Draw grids for all bars (if DrawGrid == true)"));

		propertyObject->ADD_PROPERTY_CAT_VAR(bool,
											 PropertyNames::drawGridValues,
											 PropertyNames::indicatorSettings,
											 true,
											 m_drawGridValues)
				->setDescription(QStringLiteral("Draw values for grid (only if DrawGrid == true)"));

		propertyObject->ADD_PROPERTY_CAT_VAR(bool,
											 PropertyNames::drawGridValueForAllBars,
											 PropertyNames::indicatorSettings,
											 true,
											 m_drawGridValueForAllBars)
				->setDescription(QStringLiteral("Draw values for grid for all bars (true) or just for the first one (false) (only if drawGrid == true && drawGridValues == true)"));

		propertyObject->ADD_PROPERTY_CAT_VAR(bool,
											 PropertyNames::drawGridValueUnits,
											 PropertyNames::indicatorSettings,
											 true,
											 m_drawGridValueUnits)
				->setDescription(QStringLiteral("Draw units for limits values (only if DrawGrid == true && DrawGridValues == true)"));

		if (m_scaleType == E::IndicatorScaleType::Linear)
		{
			propertyObject->ADD_PROPERTY_CAT_VAR(double,
												 PropertyNames::linearGridMainStep,
												 PropertyNames::indicatorSettings,
												 true,
												 m_linearGridMainStep)
					->setDescription(QStringLiteral("Step for main linear grids (only if DrawGrid == true)"));

			propertyObject->ADD_PROPERTY_CAT_VAR(double,
												 PropertyNames::linearGridSmallStep,
												 PropertyNames::indicatorSettings,
												 true,
												 m_linearGridSmallStep)
					->setDescription(QStringLiteral("Step for small linear grids (only if DrawGrid == true)"));
		}

		if (m_scaleType == E::IndicatorScaleType::Logarithmic)
		{
			propertyObject->ADD_PROPERTY_CAT_VAR(double,
												 PropertyNames::logarithmicGridMainStep,
												 PropertyNames::indicatorSettings,
												 true,
												 m_logarithmicGridMainStep)
					->setDescription(QStringLiteral("Step for main logarighmic grids (only if DrawGrid == true)"));

			propertyObject->ADD_PROPERTY_CAT_VAR(double,
												 PropertyNames::logarithmicGridSmallStep,
												 PropertyNames::indicatorSettings,
												 true,
												 m_logarithmicGridSmallStep)
					->setDescription(QStringLiteral("Step for small logarighmic grids (only if DrawGrid == true)"));
		}

		propertyObject->ADD_PROPERTY_GETTER_SETTER(E::IndicatorScaleType,
												   PropertyNames::indicatorScaleType,
												   true,
												   IndicatorHistogramVert::scaleType,
												   IndicatorHistogramVert::setScaleType)
				->setCategory(PropertyNames::indicatorSettings);

		// bool m_drawAutoSetpoints = true;						// Draw all auto generated setpoints
		// bool m_drawCustomSetpoints = true;					// Draw custom setpoints
		// PropertyVector<CustomSetPoint> m_customSetPoints;	// Custom setpoint list
		//
		propertyObject->ADD_PROPERTY_CAT_VAR(E::IndicatorDrawSetpoints,
											 PropertyNames::drawSetpoints,
											 PropertyNames::setpointsCategory,
											 true,
											 m_drawSetpoints);

		propertyObject->ADD_PROPERTY_CAT_VAR(PropertyVector<CustomSetPoint>,
											 PropertyNames::customSetpoints,
											 PropertyNames::setpointsCategory,
											 true,
											 m_customSetPoints)
				->setDescription(QStringLiteral("CustomSetPoints if DrawSetpoints is set to CustomSetpoints"));

		return;
	}

	bool IndicatorHistogramVert::save(Proto::SchemaItemIndicator* message) const
	{
		if (message == nullptr)
		{
			Q_ASSERT(message);
			return false;
		}

		auto m = message->mutable_indicatorhistogramvert();				// Line to change 1

		m->set_startvalue(m_startValue);
		m->set_endvalue(m_endValue);

		m->set_barwidth(m_barWidth);

		m->set_leftmargin(m_leftMargin);
		m->set_topmargin(m_topMargin);
		m->set_rightmargin(m_rightMargin);
		m->set_bottommargin(m_bottomMargin);

		m->set_drawbarrect(m_drawBarRect);

		m->set_drawgrid(m_drawGrid);
		m->set_drawgridforallbars(m_drawGridForAllBars);
		m->set_drawgridvalues(m_drawGridValues);
		m->set_drawgridvalueforallbars(m_drawGridValueForAllBars);
		m->set_drawgridvalueunits(m_drawGridValueUnits);

		m->set_lineargridmainstep(m_linearGridMainStep);
		m->set_lineargridsmallstep(m_linearGridSmallStep);

		m->set_logarithmicgridmainstep(m_logarithmicGridMainStep);
		m->set_logarithmicgridsmallstep(m_logarithmicGridSmallStep);

		m->set_scaletype(static_cast<int32_t>(m_scaleType));

		m->set_drawsetpoints(static_cast<int32_t>(m_drawSetpoints));

		for (const auto& csp : m_customSetPoints)
		{
			::Proto::VFrameSetPoint* mcsp = m->add_customsetpoints();
			csp->save(mcsp);
		}

		return true;
	}

	bool IndicatorHistogramVert::load(const Proto::SchemaItemIndicator& message, SchemaUnit unit)
	{
		m_itemUnit = unit;

		if (message.has_indicatorhistogramvert() == false)				// Line to change 1
		{
			// It can be just added new item, default values are taken
			//
			return true;
		}

		const auto& m = message.indicatorhistogramvert();	// Line to change 2

		m_startValue = m.startvalue();
		m_endValue = m.endvalue();

		m_barWidth = m.barwidth();

		m_leftMargin = m.leftmargin();
		m_topMargin = m.topmargin();
		m_rightMargin = m.rightmargin();
		m_bottomMargin = m.bottommargin();

		m_drawBarRect = m.drawbarrect();

		m_drawGrid = m.drawgrid();
		m_drawGridForAllBars = m.drawgridforallbars();
		m_drawGridValues = m.drawgridvalues();
		m_drawGridValueForAllBars = m.drawgridvalueforallbars();
		m_drawGridValueUnits = m.drawgridvalueunits();

		m_linearGridMainStep = m.lineargridmainstep();
		m_linearGridSmallStep = m.lineargridsmallstep();

		m_logarithmicGridMainStep = m.logarithmicgridmainstep();
		m_logarithmicGridSmallStep = m.logarithmicgridsmallstep();

		m_scaleType = static_cast<E::IndicatorScaleType>(m.scaletype());

		m_drawSetpoints = static_cast<E::IndicatorDrawSetpoints>(m.drawsetpoints());

		m_customSetPoints.clear();
		m_customSetPoints.reserve(static_cast<size_t>(m.customsetpoints_size()));
		for (int i = 0; i < m.customsetpoints_size(); i++)
		{
			auto csp = m_customSetPoints.createItem();
			csp->load(m.customsetpoints(i));
			m_customSetPoints.push_back(csp);
		}

		return true;
	}

	void IndicatorHistogramVert::draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer, const SchemaItemIndicator* schemaItem) const
	{
		if (drawParam == nullptr ||

			schema == nullptr ||
			layer == nullptr ||
			schemaItem == nullptr)
		{
			Q_ASSERT(drawParam);
			Q_ASSERT(schema);
			Q_ASSERT(layer);
			Q_ASSERT(schemaItem);
			return;
		}

		QPainter* p = drawParam->painter();
		Q_ASSERT(p);

		// --
		//
		const QRectF rect{schemaItem->boundingRectInDocPt(drawParam)};
		const QStringList appSignalIds = schemaItem->signalIds();

		// Bar Colors, if colors not enough then propagate the last one
		//
		QVector<QColor> signalColors = schemaItem->signalColors();
		if (signalColors.isEmpty() == true)
		{
			signalColors.push_back(Qt::darkBlue);
		}

		while (signalColors.size() < appSignalIds.size())
		{
			signalColors.push_back(signalColors.back());
		}

		// Draw background
		//
		p->fillRect(rect, schemaItem->backgroundColor());

		if (schemaItem->drawRect() == true)
		{
			QPen rectPen(schemaItem->lineColor(), schemaItem->lineWeightDraw() == 0.0 ? drawParam->cosmeticPenWidth() : schemaItem->lineWeightDraw());
			rectPen.setJoinStyle(Qt::MiterJoin);

			p->setPen(rectPen);
			p->drawRect(rect);
		}

		// Calc inside rect
		//
		QMarginsF margins{m_leftMargin, m_topMargin, m_rightMargin, m_bottomMargin};
		QRectF insideRect = rect.marginsRemoved(margins).normalized().intersected(rect);

		// Draw bars
		//
		if (appSignalIds.isEmpty() == true)
		{
			return;
		}

		double barWidth = qBound<double>(0.0, m_barWidth, insideRect.width() / appSignalIds.size());

		double barSpace = 0;
		if (appSignalIds.size() > 1)
		{
			barSpace = (insideRect.width() - barWidth * appSignalIds.size()) / static_cast<double>(appSignalIds.size() - 1);
		}

		std::vector<QRectF> barRects;
		barRects.reserve(static_cast<size_t>(appSignalIds.size()));

		std::map<QString, std::vector<IndicatorSetpoint>> setpoints;

		for (int barIndex = 0; barIndex < appSignalIds.size(); barIndex++)
		{
			const QString& appSignalId = appSignalIds[barIndex];

			// Calc bar rect
			//
			QRectF barRect{};
			if (appSignalIds.size() == 1)
			{
				barRect = drawParam->gridToDpi({insideRect.left() + insideRect.width() / 2.0 - barWidth / 2, insideRect.top(), barWidth, insideRect.height()});
			}
			else
			{
				barRect = drawParam->gridToDpi({insideRect.left() + barIndex * (barWidth + barSpace), insideRect.top(), barWidth, insideRect.height()});
			}

			barRects.push_back(barRect);

			// Get bar color
			// signalColors[barIndex] - is a base color
			// if any setpoint is alerted and it has tag, try to fetch color for this tag from behavior
			//
			std::vector<IndicatorSetpoint> signalSetpoints = comparators(drawParam, appSignalId, schemaItem);
			std::optional<QRgb> alertedColor = getAlertColor(signalSetpoints, drawParam, schemaItem);

			QColor barColor = alertedColor.value_or(signalColors[barIndex].rgb());

			setpoints[appSignalId] = std::move(signalSetpoints);

			// --
			//
			drawBar(drawParam, barRect, barIndex, appSignalId, barColor, schemaItem);
		}

		// Draw setpoints
		//
		drawSetpoints(drawParam, setpoints, barRects, schemaItem);

		return;
	}

	void IndicatorHistogramVert::drawBar(CDrawParam* drawParam,
										 const QRectF& barRect,
										 int signalIndex,
										 const QString& appSignalId,
										 const QColor& barColor,
										 const SchemaItemIndicator* schemaItem) const
	{
		Q_ASSERT(drawParam);
		Q_ASSERT(schemaItem);

		QPainter* p = drawParam->painter();
		Q_ASSERT(p);

		double lowLimit = pointToScaleValue(m_startValue);
		double highLimit = pointToScaleValue(m_endValue);

		double valueDiff = highLimit - lowLimit;	// if valueDiff is negative, then draw bar upside down

		if (std::abs(valueDiff) <= std::numeric_limits<double>::epsilon())
		{
			return;
		}

		// --
		//
		QString units;

		// Draw valued bar
		//
		if (drawParam->isEditMode() == true)
		{
			QRectF signalValueRect = barRect;

			// Draw half of bar, just as example of bar color
			//
			if (valueDiff > 0)
			{
				signalValueRect.setTop(signalValueRect.bottom() - signalValueRect.height() / 2);
			}
			else
			{
				signalValueRect.setBottom(signalValueRect.bottom() - signalValueRect.height() / 2);
			}

			p->fillRect(signalValueRect, barColor);
		}
		else
		{
			// Get signal description and state
			//
			AppSignalParam signalParam;
			AppSignalState appSignalState;
			TuningSignalState tuningSignalState;

			signalParam.setAppSignalId(appSignalId);
			signalParam.setCustomSignalId(appSignalId);

			schemaItem->getSignalState(drawParam, &signalParam, &appSignalState, &tuningSignalState);

			units = signalParam.unit();
			bool valid = false;
			double currentValue = 0;

			switch (schemaItem->signalSource())
			{
				case E::SignalSource::AppDataService:
					valid = appSignalState.isValid();
					currentValue = pointToScaleValue(appSignalState.value());
					break;

				case E::SignalSource::TuningService:
					valid = tuningSignalState.valid();
					currentValue = pointToScaleValue(tuningSignalState.toDouble());
					break;

				default:
					Q_ASSERT(false);
			}

			if (valid == true)
			{
				QRectF signalValueRect = barRect;

				if (valueDiff > 0)
				{
					if (currentValue >= highLimit)
					{
						signalValueRect = barRect;
					}
					else
					{
						if (currentValue <= lowLimit)
						{
							signalValueRect.setTop(barRect.bottom());
						}
						else
						{
							const double factor = barRect.height() / valueDiff;
							double top = barRect.bottom() - (currentValue - lowLimit) * factor;

							signalValueRect.setTop(top);
						}
					}
				}
				else
				{
					if (currentValue <= highLimit)
					{
						signalValueRect.setBottom(barRect.top());
					}
					else
					{
						if (currentValue >= lowLimit)
						{
							signalValueRect = barRect;
						}
						else
						{
							const double factor = barRect.height() / valueDiff;
							double bottom = barRect.bottom() - (currentValue - lowLimit) * factor;

							signalValueRect.setBottom(bottom);
						}
					}
				}

				p->fillRect(signalValueRect, barColor);
			} // if (valid == true)
		}

		// Draw rect around bar
		//
		if (m_drawBarRect == true)
		{
			QPen rectPen(schemaItem->lineColor(), schemaItem->lineWeightDraw() == 0.0 ? drawParam->cosmeticPenWidth() : schemaItem->lineWeightDraw());
			rectPen.setJoinStyle(Qt::MiterJoin);

			p->setPen(rectPen);
			p->drawRect(barRect);
		}

		// Draw grid values
		//
		if (m_drawGrid == true &&
			m_drawBarRect == true &&
			(m_drawGridForAllBars == true || signalIndex == 0))
		{
			double mainGridWidth = schemaItem->font().drawSize() / 2.0;
			double smallGridWidth = mainGridWidth / 2.0;

			// Draw top and bottom grids
			//
			{
				std::vector<DrawGridStruct> grids;
				grids.reserve(2);

				QString topValue;
				QString bottomValue;

				if (m_drawGridValues == true &&
					(m_drawGridValueForAllBars == true || signalIndex == 0))
				{
					if (m_drawGridValueUnits == true && units.isEmpty() == false)
					{
						topValue = QString("%1 %2 ").arg(m_endValue, 0, static_cast<char>(schemaItem->analogFormat()), schemaItem->precision()).arg(units);
						bottomValue = QString("%1 %2 ").arg(m_startValue, 0, static_cast<char>(schemaItem->analogFormat()), schemaItem->precision()).arg(units);
					}
					else
					{
						topValue = QString("%1 ").arg(m_endValue, 0, static_cast<char>(schemaItem->analogFormat()), schemaItem->precision());
						bottomValue = QString("%1 ").arg(m_startValue, 0, static_cast<char>(schemaItem->analogFormat()), schemaItem->precision());
					}
				}

				DrawGridStruct veryTopGrid{barRect.top(), mainGridWidth, topValue};
				DrawGridStruct veryBottomGrid{barRect.bottom(), mainGridWidth, bottomValue};

				grids.push_back(veryTopGrid);
				grids.push_back(std::move(veryBottomGrid));

				drawGrids(grids, drawParam, barRect, schemaItem);
			}

			// Draw main and small grids
			//
			double gridMainStep = m_scaleType == E::IndicatorScaleType::Linear ? m_linearGridMainStep : m_logarithmicGridMainStep;
			double gridSmallStep = m_scaleType == E::IndicatorScaleType::Linear ? m_linearGridSmallStep : m_logarithmicGridSmallStep;

			if ((m_drawGridForAllBars == true || signalIndex == 0) &&
				gridMainStep > std::numeric_limits<double>::epsilon() &&
				gridSmallStep > std::numeric_limits<double>::epsilon() &&
				std::abs(valueDiff) / gridMainStep < 100 &&
				std::abs(valueDiff) / gridSmallStep < 500)
			{
				int gridsCount = static_cast<int>(std::abs(valueDiff) / gridMainStep) +
								 static_cast<int>(std::abs(valueDiff) / gridSmallStep) + 2;

				std::vector<DrawGridStruct> grids;
				grids.reserve(gridsCount);

				const double factor = barRect.height() / valueDiff;

				// Add a grid point
				//
				auto addGridPoint = [this, &grids, &barRect, &schemaItem, factor, signalIndex, lowLimit](double value, double gridWidth, bool drawValue) -> void
				{
					QString text;

					if (drawValue == true &&
						this->m_drawGridValues == true &&
						(this->m_drawGridValueForAllBars == true || signalIndex == 0))
					{
						double gridValue = pointFromScaleValue(value);

						if (std::fabs(gridValue) <= DBL_MIN)
						{
							gridValue = 0;
						}

						text = QString("%1 ").arg(gridValue, 0, static_cast<char>(schemaItem->analogFormat()), schemaItem->precision());
					}

					double vertPos = barRect.bottom() - (value -  lowLimit) * factor;

					grids.emplace_back(DrawGridStruct{vertPos, gridWidth, text});
				};

				// Create a grid with specified step
				//
				auto createGrid = [this, valueDiff, lowLimit, highLimit, addGridPoint](double step, double gridWidth, bool drawValue) -> void
				{
					double prevCurrentValue = 0;
					double currentValue = lowLimit;

					for (int i = 0; i < 1000; i++)	// Loop is for safety
					{
						if (valueDiff > 0)
						{
							currentValue += step;
						}
						else
						{
							currentValue -= step;
						}

						if ((valueDiff > 0 && currentValue >= highLimit) ||
							(valueDiff < 0 && currentValue <= highLimit))
						{
							break;
						}

						if ((prevCurrentValue < 0 && currentValue > 0) ||
							(prevCurrentValue > 0 && currentValue < 0))
						{
							// Zero was crossed - next point should be mirrored to previous point

							if (step > std::fabs(currentValue * 2))
							{
								addGridPoint(0, gridWidth, drawValue);	// Draw zero point if necessary
							}

							currentValue = -prevCurrentValue;
						}

						addGridPoint(currentValue, gridWidth, drawValue);

						prevCurrentValue = currentValue;
					}
				};

				// Draw main grids
				//
				createGrid(gridMainStep, mainGridWidth, true);

				createGrid(gridSmallStep, smallGridWidth, false);

				drawGrids(grids, drawParam, barRect, schemaItem);
			}
		} // Draw grid values

		return;
	}

	void IndicatorHistogramVert::drawGrids(const std::vector<DrawGridStruct> grids, CDrawParam* drawParam, const QRectF barRect, const SchemaItemIndicator* item) const
	{
		QPainter* p = drawParam->painter();
		Q_ASSERT(p);

		QPen pen(item->lineColor(), item->lineWeightDraw() == 0.0 ? drawParam->cosmeticPenWidth() : item->lineWeightDraw());
		p->setPen(pen);

		for (const DrawGridStruct& g : grids)
		{
			p->drawLine(QPointF{barRect.left() - g.gridWidth, drawParam->gridToDpiY(g.gridVertPos)},
						QPointF{barRect.left(), drawParam->gridToDpiY(g.gridVertPos)});

			if (g.text.isEmpty() == false)
			{
				QRectF textRect{barRect.left() - g.gridWidth, g.gridVertPos, 0, 0};

				DrawHelper::drawText(p, item->font(), item->itemUnit(), g.text, textRect, Qt::AlignRight | Qt::AlignVCenter | Qt::TextDontClip | Qt::TextSingleLine);
			}
		}

		return;
	}

	std::vector<IndicatorSetpoint> IndicatorHistogramVert::comparators(CDrawParam* drawParam,
																	   const QString& appSignalId,
																	   const SchemaItemIndicator* schemaItem) const
	{
		Q_ASSERT(drawParam);

		std::vector<IndicatorSetpoint> result;
		result.reserve(8);

		if (drawParam->isMonitorMode() == false ||
			drawParam->appSignalController() == nullptr)
		{
			return result;
		}

		switch (m_drawSetpoints)
		{
		case E::IndicatorDrawSetpoints::AutoGenerated:
			{
				std::vector<std::shared_ptr<Comparator>> setpoints = drawParam->appSignalController()->setpointsByInputSignalId(appSignalId);

				for (const std::shared_ptr<Comparator>& sp : setpoints)
				{
					if (sp == nullptr)
					{
						Q_ASSERT(sp);
						continue;
					}

					if (sp->compare().isConst() == false &&
						sp->compare().isAcquired() == false)
					{
						// skip this setpoint, the value compare with is unknown
						//
						continue;
					}

					result.push_back({sp, SetpointSource::AutoGenerated, {}, {}, {}, qRgb(0x00, 0x00, 0xC0)});
				}
			}
			break;
		case E::IndicatorDrawSetpoints::CustomSetpoints:
			{
				for (const std::shared_ptr<CustomSetPoint>& csp : m_customSetPoints)
				{
					std::vector<std::shared_ptr<Comparator>> setpoints = drawParam->appSignalController()->setpointsByInputSignalId(appSignalId);
					for (const std::shared_ptr<Comparator>& sp : setpoints)
					{
						if (sp == nullptr)
						{
							Q_ASSERT(sp);
							continue;
						}

						Q_ASSERT(appSignalId == sp->input().appSignalID());

						if (sp->output().isAcquired() == false)
						{
							// skip this setpoint, the value compare with is unknown
							//
							continue;
						}

						if (sp->output().appSignalID() == csp->outputAppSignalId())
						{
							// Setpoint is found
							//
							result.push_back({sp, SetpointSource::Custom, csp, {}, {}, qRgb(0x00, 0x00, 0xC0)});
							break;
						}
					}
				}
			}
			break;
		case E::IndicatorDrawSetpoints::NoSetpoints:
			break;
		default:
			Q_ASSERT(false);
		}

		// Get value and alerted for setpoints
		//
		for (IndicatorSetpoint& sp : result)
		{
			std::optional<double> comparatorValue;
			std::optional<double> alertedValue;
			std::optional<bool> alerted;
			QRgb foundColor{qRgb(0x00, 0x00, 0xC0)};

			if (const ComparatorSignal& valueSignal = sp.comparator->compare();		// This signal contains value for setpoint
				valueSignal.isConst() == true)
			{
				comparatorValue = valueSignal.constValue();
			}
			else
			{
				comparatorValue = schemaItem->getSignalState(drawParam, valueSignal.appSignalID());
			}

			if (comparatorValue.has_value() == true)
			{
				// Getting setpoint state
				//
				const ComparatorSignal& stateSignal = sp.comparator->output();	// This signal contains value for setpoint result
				//Q_ASSERT(stateSignal.isConst() == false);

				if (stateSignal.isAcquired() == true)
				{
					alertedValue = schemaItem->getSignalState(drawParam, stateSignal.appSignalID());
				}

				// if setting outputs state is not valid it is also indicated as alerted
				//
				alerted = (alertedValue.has_value() == true && alertedValue.value() != 0) ||
								 alertedValue.has_value() == false;

				// Get color by output signal tags and behavior
				//
				std::optional<QRgb> overrideColor;

				switch (sp.source)
				{
				case SetpointSource::AutoGenerated:
					overrideColor = {};
					break;
				case SetpointSource::Custom:
					Q_ASSERT(sp.customSetpointData);
					if (sp.customSetpointData != nullptr)
					{
						overrideColor = (sp.customSetpointData->colorSource() == E::IndicatorColorSource::StaticColorFromStruct)
										? std::optional<QRgb>(sp.customSetpointData->color().rgb())
										: std::optional<QRgb>();
					}
					break;
				}

				if (overrideColor.has_value() == false)
				{
					QStringList setpointSignalTags = drawParam->appSignalController()->signalTags(stateSignal.appSignalID());

					const MonitorBehavior& monitorBehavior = drawParam->monitorBehavor();

					std::optional<std::pair<QRgb, QRgb>> color = monitorBehavior.tagToColors(setpointSignalTags);

					if (color.has_value() == true)
					{
						foundColor = drawParam->blinkPhase() ? color.value().first : color.value().second;
					}
				}
				else
				{
					foundColor = overrideColor.value();
				}
			}

			// Assign results
			//
			sp.value = comparatorValue;
			sp.alerted = alerted;
			sp.setpointColor = foundColor;
		}

		return result;
	}


	std::optional<QRgb> IndicatorHistogramVert::getAlertColor(const std::vector<IndicatorSetpoint>& setpoints, CDrawParam* drawParam, const SchemaItemIndicator* schemaItem) const
	{
		Q_ASSERT(drawParam);
		Q_ASSERT(schemaItem);

		if (drawParam->isMonitorMode() == false)
		{
			return {};
		}

		// Custom alerted setpoints with m_colorSource == StaticColorFromStruct
		// have the highest priority
		//
		for (const IndicatorSetpoint& isp : setpoints)
		{
			if (isp.source == SetpointSource::Custom &&
				isp.alerted.has_value() == true &&
				isp.alerted.value() == true &&
				isp.customSetpointData->colorSource() == E::IndicatorColorSource::StaticColorFromStruct)
			{
				return {isp.customSetpointData->color().rgb()};
			}
		}

		// If there are any alerted setpoints with output signals' tags,
		// then try to get color from MonitorBehavior by these tags
		//
		std::set<QString> alertedTags;

		for (const IndicatorSetpoint& isp : setpoints)
		{
			if (isp.alerted.has_value() && isp.alerted.value() == true)
			{
				const ComparatorSignal& output = isp.comparator->output();

				if (output.isAcquired() == true)
				{
					auto signalTags = schemaItem->getSignalTags(drawParam, output.appSignalID());

					for (const QString& t : signalTags)
					{
						alertedTags.insert(t);
					}
				}
			}
		}

		std::optional<std::pair<QRgb, QRgb>> result = drawParam->monitorBehavor().tagToColors(alertedTags);

		if (result.has_value() == false)
		{
			return {};
		}

		return drawParam->blinkPhase() ? result.value().first : result.value().second;
	}

	void IndicatorHistogramVert::drawSetpoints(CDrawParam* drawParam,
											   const std::map<QString, std::vector<IndicatorSetpoint>>& setpoints,
											   const std::vector<QRectF>& barRects,
											   const SchemaItemIndicator* schemaItem) const
	{
		if (drawParam->isMonitorMode() == false)
		{
			// No access to setpoints
			//
			return;
		}

		std::vector<DrawSetpointStruct> drawSetpoints;
		drawSetpoints.reserve(16);

		// Convert setpoints to DrawSetpointStruct whick is convinirnt for drawing
		//
		const QStringList appSignalIds = schemaItem->signalIds();

		for (int signalIndex = 0; signalIndex < appSignalIds.size(); signalIndex++)
		{
			const QString& appSignalId = appSignalIds[signalIndex];

			auto setpointsIt = setpoints.find(appSignalId);
			if (setpointsIt == setpoints.end())
			{
				Q_ASSERT(setpointsIt != setpoints.end());
				return;
			}

			const std::vector<IndicatorSetpoint>& signalSetpoints = setpointsIt->second;

			for (const IndicatorSetpoint& is : signalSetpoints)
			{
				drawSetpoints.push_back(DrawSetpointStruct{signalIndex, barRects[signalIndex], is});
			}
		}

		// Draw setpoints
		//
		drawSetpointItems(drawParam, drawSetpoints, schemaItem);

		return;
	}

	void IndicatorHistogramVert::drawSetpointItems(CDrawParam* drawParam, const std::vector<DrawSetpointStruct>& setpoints, const SchemaItemIndicator* schemaItem) const
	{
		QPainter* p = drawParam->painter();
		Q_ASSERT(p);

		double lowLimit = pointToScaleValue(m_startValue);
		double highLimit = pointToScaleValue(m_endValue);

		double valueDiff = highLimit - lowLimit;	// if valueDiff is negative, then draw bar upside down
		if (std::abs(valueDiff) <= std::numeric_limits<double>::epsilon())
		{
			return;
		}

		double mainGridWidth = schemaItem->font().drawSize() / 1.8;
		QString valueString;

		for (const DrawSetpointStruct& ds : setpoints)
		{
			const QRectF& barRect = ds.barRect;
			bool alerted = ds.indicatorSetpoint.alerted.value_or(true);

			const double factor = barRect.height() / valueDiff;
			double value = ds.indicatorSetpoint.value.value_or(lowLimit);
			double scaleValue = pointToScaleValue(value);
			double y = barRect.bottom() - (scaleValue - lowLimit) * factor;

			QRgb color{ds.indicatorSetpoint.setpointColor};
//			if (alerted == true)
//			{
//				color = drawParam->blinkPhase() ? ds.color : 0xF0F0F0;	// What second color can be!?
//			}

			QBrush brush{color};

			QPen pen(brush, schemaItem->lineWeightDraw() == 0.0 ? drawParam->cosmeticPenWidth() : schemaItem->lineWeightDraw());
			p->setPen(pen);

			// Draw horz line
			//
			p->drawLine(QPointF{barRect.left() - mainGridWidth, drawParam->gridToDpiY(y)},
						QPointF{barRect.right() + mainGridWidth, drawParam->gridToDpiY(y)});

			// Draw setpoint value
			//
			QChar cmpSymbol;
			switch (ds.indicatorSetpoint.comparator->cmpType())
			{
			case E::CmpType::Equal:		cmpSymbol = QChar('=');		break;
			case E::CmpType::Greate:	cmpSymbol = QChar(0x25B2);	break;
			case E::CmpType::Less:		cmpSymbol = QChar(0x25BC);	break;
			case E::CmpType::NotEqual:	cmpSymbol = QChar(0x2260);	break;
			default:
				Q_ASSERT(false);
			}

			if (ds.indicatorSetpoint.value.has_value() == true)
			{
				if (alerted == false ||
					(alerted == true && drawParam->blinkPhase() == true))
				{
					valueString = QString(" %1 %2")
									.arg(value, 0, static_cast<char>(schemaItem->analogFormat()), schemaItem->precision())
									.arg(cmpSymbol);

					QRectF textRect{barRect.right() + mainGridWidth, drawParam->gridToDpiY(y), 0, 0};

					DrawHelper::drawText(p, schemaItem->font(), schemaItem->itemUnit(), valueString, textRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextDontClip | Qt::TextSingleLine);
				}
			}
		}

		return;
	}

	double IndicatorHistogramVert::indicatorLog10(double value) const
	{
		// Logarithm calculation.
		// The result is shifted up by DBL_MAX_10_EXP.
		// For negative value, logarithm is taken from absolute value and then shifted and multiplied by -1.
		// This means that we take a "ghost" logarithm from negative value.

		double result = std::fabs(value);

		if (result < DBL_MIN)
		{
			result = DBL_MIN;
		}

		result = std::log10(result);

		result += DBL_MAX_10_EXP;

		if (value < 0)
		{
			result = -result;
		}

		return result;
	}

	double IndicatorHistogramVert::indicatorPow10(double value) const
	{
		// Power calculation, reverse function for trendLog10.
		// Input value is shifted down by DBL_MAX_10_EXP and power is calculated from its absoulte value.
		// The sign of the result depened on input value sign.

		double result = std::fabs(value);

		result -= DBL_MAX_10_EXP;

		result = std::pow(10, result);

		if (value < 0)
		{
			result = -result;
		}

		return result;
	}

	double IndicatorHistogramVert::pointToScaleValue(double value) const
	{
		switch (m_scaleType)
		{
		case E::IndicatorScaleType::Linear:
			{
				return value;
			}
		case E::IndicatorScaleType::Logarithmic:
			{
				return indicatorLog10(value);
			}
		default:
			Q_ASSERT(false);
		}

		return 0;
	}

	double IndicatorHistogramVert::pointFromScaleValue(double scaleValue) const
	{
		switch (m_scaleType)
		{
		case E::IndicatorScaleType::Linear:
			{
				return scaleValue;
			}
		case E::IndicatorScaleType::Logarithmic:
			{
				scaleValue = indicatorPow10(scaleValue);

				return scaleValue;
			}

		default:
			Q_ASSERT(false);
		}
		return 0;
	}

}
