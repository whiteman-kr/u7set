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
		ADD_PROPERTY_GETTER_SETTER(E::IndicatorSetpointType, PropertyNames::indicatorSetpointType, true, CustomSetPoint::setpointType, CustomSetPoint::setSetpointType);
		ADD_PROPERTY_GETTER_SETTER(QColor, PropertyNames::color, true, CustomSetPoint::color, CustomSetPoint::setColor);

		// Type AutoByLabel category
		//
		ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::indicatorSchemaItemLabel, true, CustomSetPoint::schemaItemLabel, CustomSetPoint::setSchemaItemLabel)
				->setCategory(PropertyNames::indicatorSetpointTypeByLabelCategory);

		// Type AutoBySignalID category
		//
		ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::indicatorOutputAppSignalId, true, CustomSetPoint::outputAppSignalId, CustomSetPoint::setOutputAppSignalId)
				->setCategory(PropertyNames::indicatorSetpointTypeBySignalIdCategory);

		// Type Static category
		//
		ADD_PROPERTY_GETTER_SETTER(double, PropertyNames::indicatorStaticValue, true, CustomSetPoint::staticValue, CustomSetPoint::setStaticValue)
			->setCategory(PropertyNames::indicatorSetpointTypeStaticCategory);

		ADD_PROPERTY_GETTER_SETTER(E::CmpType, PropertyNames::indicatorStaticCompareType, true, CustomSetPoint::staticCompareType, CustomSetPoint::setStaticCompareType)
			->setCategory(PropertyNames::indicatorSetpointTypeStaticCategory);

		return;
	}

	bool CustomSetPoint::save(Proto::VFrameSetPoint* message) const
	{
		if (message == nullptr)
		{
			assert(message);
			return false;
		}

		message->set_setpointtype(static_cast<int32_t>(m_setpointType));
		message->set_color(m_color.rgba());

		message->set_schemaitemlabel(m_schemaItemLabel.toStdString());

		message->set_outputappsignalid(m_outputAppSignalId.toStdString());

		message->set_staticvalue(m_staticValue);
		message->set_staticcomparetype(static_cast<int32_t>(m_staticCompareType));

		return true;
	}

	bool CustomSetPoint::load(const Proto::VFrameSetPoint& message)
	{
		m_setpointType = static_cast<E::IndicatorSetpointType>(message.setpointtype());
		m_color = message.color();

		m_schemaItemLabel = QString::fromStdString(message.schemaitemlabel());

		m_outputAppSignalId = QString::fromStdString(message.outputappsignalid());

		m_staticValue = message.staticvalue();
		m_staticCompareType = static_cast<E::CmpType>(message.staticcomparetype());

		return true;
	}

	E::IndicatorSetpointType CustomSetPoint::setpointType() const
	{
		return m_setpointType;
	}

	void CustomSetPoint::setSetpointType(E::IndicatorSetpointType value)
	{
		m_setpointType = value;
	}

	QColor CustomSetPoint::color() const
	{
		return m_color;
	}

	void CustomSetPoint::setColor(const QColor& value)
	{
		m_color = value;
	}

	const QString& CustomSetPoint::schemaItemLabel() const
	{
		return m_schemaItemLabel;
	}

	void CustomSetPoint::setSchemaItemLabel(const QString& value)
	{
		m_schemaItemLabel = value;
	}

	const QString& CustomSetPoint::outputAppSignalId() const
	{
		return m_outputAppSignalId;
	}

	void CustomSetPoint::setOutputAppSignalId(const QString& value)
	{
		m_outputAppSignalId = value;
	}

	double CustomSetPoint::staticValue() const
	{
		return m_staticValue;
	}

	void CustomSetPoint::setStaticValue(double value)
	{
		m_staticValue = value;
	}

	E::CmpType CustomSetPoint::staticCompareType() const
	{
		return m_staticCompareType;
	}

	void CustomSetPoint::setStaticCompareType(E::CmpType value)
	{
		m_staticCompareType = value;
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

		propertyObject->ADD_PROPERTY_CAT_VAR(double,
											 PropertyNames::gridMainStep,
											 PropertyNames::indicatorSettings,
											 true,
											 m_gridMainStep)
				->setDescription(QStringLiteral("Step for main grids (only if DrawGrid == true)"));

		propertyObject->ADD_PROPERTY_CAT_VAR(double,
											 PropertyNames::gridSmallStep,
											 PropertyNames::indicatorSettings,
											 true,
											 m_gridSmallStep)
				->setDescription(QStringLiteral("Step for small grids (only if DrawGrid == true)"));

		// bool m_drawAutoSetpoints = true;						// Draw all auto generated setpoints
		// bool m_drawCustomSetpoints = true;					// Draw custom setpoints
		// PropertyVector<CustomSetPoint> m_customSetPoints;	// Custom setpoint list
		//
		propertyObject->ADD_PROPERTY_CAT_VAR(bool,
											 PropertyNames::drawAutoSetpoints,
											 PropertyNames::setpointsCategory,
											 true,
											 m_drawAutoSetpoints)
				->setDescription(QStringLiteral("Draw all auto generated setpoints"));

		propertyObject->ADD_PROPERTY_CAT_VAR(bool,
											 PropertyNames::drawCustomSetpoints,
											 PropertyNames::setpointsCategory,
											 true,
											 m_drawCustomSetpoints)
				->setDescription(QStringLiteral("Draw custom setpoints added to CustomSetPoints property"));

		propertyObject->ADD_PROPERTY_CAT_VAR(PropertyVector<CustomSetPoint>,
											 PropertyNames::customSetpoints,
											 PropertyNames::setpointsCategory,
											 true,
											 m_customSetPoints)
				->setDescription(QStringLiteral("CustomSetPoints"));

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

		m->set_gridmainstep(m_gridMainStep);
		m->set_gridsmallstep(m_gridSmallStep);

		m->set_drawautosetpoints(m_drawAutoSetpoints);
		m->set_drawcustomsetpoints(m_drawCustomSetpoints);

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

		m_gridMainStep = m.gridmainstep();
		m_gridSmallStep = m.gridsmallstep();

		m_drawAutoSetpoints = m.drawautosetpoints();
		m_drawCustomSetpoints = m.drawcustomsetpoints();

		m_customSetPoints.clear();
		m_customSetPoints.reserve(m.customsetpoints_size());
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
		barRects.reserve(appSignalIds.size());

		for (int barIndex = 0; barIndex < appSignalIds.size(); barIndex++)
		{
			const QString& appSignalId = appSignalIds[barIndex];
			QColor barColor = signalColors[barIndex];

			QRectF barRect{};
			if (appSignalIds.size() == 1)
			{
				barRect = drawParam->gridToDpi({insideRect.left() + insideRect.width() / 2.0 - barWidth / 2, insideRect.top(), barWidth, insideRect.height()});
			}
			else
			{
				barRect = drawParam->gridToDpi({insideRect.left() + barIndex * (barWidth + barSpace), insideRect.top(), barWidth, insideRect.height()});
			};

			barRects.push_back(barRect);

			drawBar(drawParam, barRect, barIndex, appSignalId, barColor, schemaItem);
		}

		// Draw setpoints
		//
		drawSetpoints(drawParam, barRects, schemaItem);

		return;
	}

	void IndicatorHistogramVert::drawBar(CDrawParam* drawParam, const QRectF& barRect, int signalIndex, const QString appSignalId, QColor barColor, const SchemaItemIndicator* schemaItem) const
	{
		Q_ASSERT(drawParam);
		Q_ASSERT(schemaItem);

		QPainter* p = drawParam->painter();
		Q_ASSERT(p);

		double valueDiff = m_endValue - m_startValue;	// if valueDiff is negative, then draw bar upside down

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
					currentValue = appSignalState.value();
					break;

				case E::SignalSource::TuningService:
					valid = tuningSignalState.valid();
					currentValue = tuningSignalState.toDouble();
					break;

				default:
					Q_ASSERT(false);
			}

			if (valid == true)
			{
				QRectF signalValueRect = barRect;

				if (valueDiff > 0)
				{
					if (currentValue >= m_endValue)
					{
						signalValueRect = barRect;
					}
					else
					{
						if (currentValue <= m_startValue)
						{
							signalValueRect.setTop(barRect.bottom());
						}
						else
						{
							const double factor = barRect.height() / valueDiff;
							double top = barRect.bottom() - (currentValue - this->m_startValue) * factor;

							signalValueRect.setTop(top);
						}
					}
				}
				else
				{
					if (currentValue <= m_endValue)
					{
						signalValueRect.setBottom(barRect.top());
					}
					else
					{
						if (currentValue >= m_startValue)
						{
							signalValueRect = barRect;

						}
						else
						{
							const double factor = barRect.height() / valueDiff;
							double bottom = barRect.bottom() - (currentValue - this->m_startValue) * factor;

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
			if ((m_drawGridForAllBars == true || signalIndex == 0) &&
				m_gridMainStep > std::numeric_limits<double>::epsilon() &&
				m_gridSmallStep > std::numeric_limits<double>::epsilon() &&
				std::abs(valueDiff) / m_gridMainStep < 100 &&
				std::abs(valueDiff) / m_gridSmallStep < 500)
			{
				int gridsCount = static_cast<int>(std::abs(valueDiff) / m_gridMainStep) +
								 static_cast<int>(std::abs(valueDiff) / m_gridSmallStep) + 2;

				std::vector<DrawGridStruct> grids;
				grids.reserve(gridsCount);

				const double factor = barRect.height() / valueDiff;

				auto addGrid = [this, &grids, &barRect, &schemaItem, valueDiff, factor, signalIndex](double value, double gridWidth, bool drawValue) -> void
				{
					QString text;

					if (drawValue == true &&
						this->m_drawGridValues == true &&
						(this->m_drawGridValueForAllBars == true || signalIndex == 0))
					{
						text = QString("%1 ").arg(value, 0, static_cast<char>(schemaItem->analogFormat()), schemaItem->precision());
					}

					double vertPos = barRect.bottom() - (value - this->m_startValue) * factor;

					grids.emplace_back(DrawGridStruct{vertPos, gridWidth, text});
				};

				// Draw main grids
				//
				if (valueDiff > 0)
				{
					for (double currentValue = m_startValue + m_gridMainStep;
						 currentValue < m_endValue;
						 currentValue += m_gridMainStep)
					{
						addGrid(currentValue, mainGridWidth, true);
					}
				}
				else
				{
					for (double currentValue = m_startValue - m_gridMainStep;
						 currentValue > m_endValue;
						 currentValue -= m_gridMainStep)
					{
						addGrid(currentValue, mainGridWidth, true);
					}
				}

				// draw small grids
				//
				if (valueDiff > 0)
				{
					for (double currentValue = m_startValue + m_gridSmallStep;
						 currentValue < m_endValue;
						 currentValue += m_gridSmallStep)
					{
						addGrid(currentValue, smallGridWidth, false);
					}
				}
				else
				{
					for (double currentValue = m_startValue - m_gridSmallStep;
						 currentValue > m_endValue;
						 currentValue -= m_gridSmallStep)
					{
						addGrid(currentValue, smallGridWidth, false);
					}
				}

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

	void IndicatorHistogramVert::drawSetpoints(CDrawParam* drawParam, const std::vector<QRectF>& barRects, const SchemaItemIndicator* schemaItem) const
	{
		// --
		//
		double valueDiff = m_endValue - m_startValue;	// if valueDiff is negative, then draw bar upside down
		if (std::abs(valueDiff) <= std::numeric_limits<double>::epsilon())
		{
			return;
		}

		// --
		//
		struct DrawSetpointStruct
		{
			DrawSetpointStruct(int signalIndex_, double value_, E::CmpType type_, QRgb color_) :
				signalIndex(signalIndex_),
				value(value_),
				type(type_),
				color(color_)
			{
			}

			int signalIndex = 0;
			double value = 0;
			E::CmpType type = E::CmpType::Greate;
			QRgb color = qRgb(0x80, 0x00, 0x00);
			//AppSignalState outputState;
		};

		std::vector<DrawSetpointStruct> drawSetpoints;
		drawSetpoints.reserve(16);

		// Get all automaticaly generated (by u7) setpoints
		//
		if (m_drawAutoSetpoints == true &&
			drawParam->isMonitorMode() == true &&
			drawParam->appSignalController() != nullptr)
		{
			const QStringList appSignalIds = schemaItem->signalIds();

			for (int i = 0; i < appSignalIds.size(); i++)
			{
				const QString& appSignalId = appSignalIds[i];
				std::vector<std::shared_ptr<Comparator>> setpoints = drawParam->appSignalController()->setpointsByInputSignalId(appSignalId);

				for (const std::shared_ptr<Comparator>& sp : setpoints)
				{
					Q_ASSERT(sp);

					std::optional<double> value;

					if (const ComparatorSignal& valueSignal = sp->compare();	// This signal contains value for setpoint
						valueSignal.isConst() == true)
					{
						value = valueSignal.constValue();
					}
					else
					{
						if (valueSignal.isAcquired() == false)
						{
							continue;
						}
						else
						{
							value = schemaItem->getSignalState(drawParam, appSignalId);
						}
					}

					if (value.has_value() == true)
					{
						int warning_to_do_color;
						drawSetpoints.emplace_back(i, value.value(), sp->cmpType(), qRgb(0x00, 0x00, 0xC0));
					}
				}
			}
		}

		// Get all custom setpoints
		//
		if (m_drawCustomSetpoints == true && m_customSetPoints.empty() == false)
		{
		}

		// Draw setpoints
		//
		QPainter* p = drawParam->painter();
		Q_ASSERT(p);

		double mainGridWidth = schemaItem->font().drawSize() / 1.8;
		QString valueString;



		for (const DrawSetpointStruct& ds : drawSetpoints)
		{
			const QRectF& barRect = barRects[ds.signalIndex];

			const double factor = barRect.height() / valueDiff;
			double y = barRect.bottom() - (ds.value - m_startValue) * factor;

//			QColor color1 = ds.color;
//			QColor color2{(~color1.red()) & 0xFF,
//						(~color1.green()) & 0xFF,
//						(~color1.blue()) & 0xFF};

			//QBrush brush{drawParam->blinkPhase() ? color1 : color2};
			QBrush brush{ds.color};

			QPen pen(brush, schemaItem->lineWeightDraw() == 0.0 ? drawParam->cosmeticPenWidth() : schemaItem->lineWeightDraw());
			p->setPen(pen);

			// Draw horz line
			//
			p->drawLine(QPointF{barRect.left() - mainGridWidth, drawParam->gridToDpiY(y)},
						QPointF{barRect.right() + mainGridWidth, drawParam->gridToDpiY(y)});

			// Draw setpoint value
			//
			QChar cmpSymbol;
			switch (ds.type)
			{
			case E::CmpType::Equal:		cmpSymbol = QChar('=');		break;
			case E::CmpType::Greate:	cmpSymbol = QChar(0x25B2);	break;
			case E::CmpType::Less:		cmpSymbol = QChar(0x25BC);	break;
			case E::CmpType::NotEqual:	cmpSymbol = QChar(0x2260);	break;
			default:
				Q_ASSERT(false);
			}

			if (drawParam->blinkPhase() == true)
			{
				valueString = QString(" %1 %2")
								.arg(ds.value, 0, static_cast<char>(schemaItem->analogFormat()), schemaItem->precision())
								.arg(cmpSymbol);

				QRectF textRect{barRect.right() + mainGridWidth, drawParam->gridToDpiY(y), 0, 0};

				DrawHelper::drawText(p, schemaItem->font(), schemaItem->itemUnit(), valueString, textRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextDontClip | Qt::TextSingleLine);
			}
		}

		return;
	}

}
