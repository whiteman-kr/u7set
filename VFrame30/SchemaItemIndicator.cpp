#include "SchemaItemIndicator.h"
#include "PropertyNames.h"
#include "DrawParam.h"
#include "Schema.h"
#include "FblItemRect.h"
#include "MacrosExpander.h"
#include "TuningController.h"
#include "AppSignalController.h"
#include "../lib/AppSignal.h"
#include "../lib/Tuning/TuningSignalState.h"

namespace VFrame30
{
	//
	// IndicatorComponent base class
	//
	IndicatorObject::IndicatorObject(SchemaUnit itemUnit) :
		m_itemUnit(itemUnit)
	{
		Q_ASSERT(m_itemUnit == SchemaUnit::Display || m_itemUnit == SchemaUnit::Inch);
	}

	template <typename TYPE>
	TYPE IndicatorObject::regionalGetter(const TYPE& variable) const
	{
		if (m_itemUnit == SchemaUnit::Display)
		{
			return CUtils::RoundDisplayPoint(variable);
		}
		else
		{
			double pt = variable;
			pt = CUtils::ConvertPoint(pt, SchemaUnit::Inch, Settings::regionalUnit(), 0);
			return CUtils::RoundPoint(pt, Settings::regionalUnit());
		}
	}

	template <typename TYPE>
	void IndicatorObject::regionalSetter(TYPE value, TYPE* variable)
	{
		Q_ASSERT(variable);

		if (m_itemUnit == SchemaUnit::Display)
		{
			*variable = CUtils::RoundDisplayPoint(value);
		}
		else
		{
			*variable = CUtils::ConvertPoint(value, Settings::regionalUnit(), SchemaUnit::Inch, 0);
		}
	}

	void IndicatorObject::setUnits(SchemaUnit itemUnit)
	{
		Q_ASSERT(m_itemUnit == SchemaUnit::Display || m_itemUnit == SchemaUnit::Inch);
		m_itemUnit = itemUnit;
	}


	// Vertical histogram, the base view
	//
	IndicatorHistogramVert::IndicatorHistogramVert(SchemaUnit itemUnit) :
		IndicatorObject(itemUnit),
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
											[this](){ return regionalGetter(m_rightMargin);},		// 2
											[this](const auto& value){ return regionalSetter(value, &m_rightMargin);})	// 3
				->setViewOrder(12);		// 4

		propertyObject->addProperty<double>(PropertyNames::indicatorMargingBottom,				// 1
											PropertyNames::indicatorSettings,
											true,
											[this](){ return regionalGetter(m_bottomMargin);},		// 2
											[this](const auto& value){ return regionalSetter(value, &m_bottomMargin);})	// 3
				->setViewOrder(13);		// 4

		propertyObject->ADD_PROPERTY_CAT_VAR(bool,
											 PropertyNames::indicatorDrawBarRect,
											 PropertyNames::indicatorSettings,
											 true,
											 m_drawBarRect);

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

		return true;
	}

	void IndicatorHistogramVert::draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer, const SchemaItemIndicator* item) const
	{
		if (drawParam == nullptr ||
			schema == nullptr ||
			layer == nullptr ||
			item == nullptr)
		{
			Q_ASSERT(drawParam);
			Q_ASSERT(schema);
			Q_ASSERT(layer);
			Q_ASSERT(item);

			return;
		}

		QPainter* p = drawParam->painter();
		Q_ASSERT(p);

		// --
		//
		const QRectF rect{item->boundingRectInDocPt(drawParam)};
		const QStringList appSignalIds = item->signalIds();

		// Bar Colors, if colors not enough then propagate the last one
		//
		QVector<QColor> signalColors = item->signalColors();
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
		p->fillRect(rect, item->backgroundColor());

		if (item->drawRect() == true)
		{
			QPen rectPen(item->lineColor(), item->lineWeightDraw() == 0.0 ? drawParam->cosmeticPenWidth() : item->lineWeightDraw());
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

		for (int barIndex = 0; barIndex < appSignalIds.size(); barIndex++)
		{
			const QString& appSignalId = appSignalIds[barIndex];
			QColor barColor = signalColors[barIndex];

			QRectF barRect;
			if (appSignalIds.size() == 1)
			{
				barRect = drawParam->gridToDpi({insideRect.left() + insideRect.width() / 2.0 - barWidth / 2, insideRect.top(), barWidth, insideRect.height()});
			}
			else
			{
				barRect = drawParam->gridToDpi({insideRect.left() + barIndex * (barWidth + barSpace), insideRect.top(), barWidth, insideRect.height()});
			};

			drawBar(drawParam, barRect, barIndex, appSignalId, barColor, item);
		}

		return;
	}

	void IndicatorHistogramVert::drawBar(CDrawParam* drawParam, const QRectF& barRect, int signalIndex, const QString appSignalId, QColor barColor, const SchemaItemIndicator* item) const
	{
		Q_ASSERT(drawParam);
		Q_ASSERT(item);

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

			item->getSignalState(drawParam, &signalParam, &appSignalState, &tuningSignalState);

			units = signalParam.unit();
			bool valid = false;
			double currentValue = 0;

			switch (item->signalSource())
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
			QPen rectPen(item->lineColor(), item->lineWeightDraw() == 0.0 ? drawParam->cosmeticPenWidth() : item->lineWeightDraw());
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
			double mainGridWidth = item->font().drawSize() / 2.0;
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
						topValue = QString("%1 %2 ").arg(m_endValue, 0, static_cast<char>(item->analogFormat()), item->precision()).arg(units);
						bottomValue = QString("%1 %2 ").arg(m_startValue, 0, static_cast<char>(item->analogFormat()), item->precision()).arg(units);
					}
					else
					{
						topValue = QString("%1 ").arg(m_endValue, 0, static_cast<char>(item->analogFormat()), item->precision());
						bottomValue = QString("%1 ").arg(m_startValue, 0, static_cast<char>(item->analogFormat()), item->precision());
					}
				}

				DrawGridStruct veryTopGrid{barRect.top(), mainGridWidth, topValue};
				DrawGridStruct veryBottomGrid{barRect.bottom(), mainGridWidth, bottomValue};

				grids.push_back(veryTopGrid);
				grids.push_back(std::move(veryBottomGrid));

				drawGrids(grids, drawParam, barRect, item);
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

				auto addGrid = [this, &grids, &barRect, &item, valueDiff, factor, signalIndex](double value, double gridWidth, bool drawValue) -> void
				{
					QString text;

					if (drawValue == true &&
						this->m_drawGridValues == true &&
						(this->m_drawGridValueForAllBars == true || signalIndex == 0))
					{
						text = QString("%1 ").arg(value, 0, static_cast<char>(item->analogFormat()), item->precision());
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

				drawGrids(grids, drawParam, barRect, item);
			}
		}

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

	//
	// ArrowIndicator
	//

	IndicatorArrowIndicator::IndicatorArrowIndicator(SchemaUnit itemUnit) :
		IndicatorObject(itemUnit)
	{
	}

	void IndicatorArrowIndicator::createProperties(SchemaItemIndicator* propertyObject, int /*signalCount*/)
	{
		propertyObject->ADD_PROPERTY_GETTER_SETTER(double, PropertyNames::indicatorStartValue, true, IndicatorArrowIndicator::startValue, IndicatorArrowIndicator::setStartValue)
				->setCategory(PropertyNames::indicatorSettings)
				.setViewOrder(0);

		propertyObject->ADD_PROPERTY_GETTER_SETTER(double, PropertyNames::indicatorEndValue, true, IndicatorArrowIndicator::endValue, IndicatorArrowIndicator::setEndValue)
				->setCategory(PropertyNames::indicatorSettings)
				.setViewOrder(1);

		propertyObject->ADD_PROPERTY_GETTER_SETTER(double, PropertyNames::indicatorStartAngle, true, IndicatorArrowIndicator::startAngle, IndicatorArrowIndicator::setStartAngle)
				->setCategory(PropertyNames::indicatorSettings)
				.setViewOrder(2);

		propertyObject->ADD_PROPERTY_GETTER_SETTER(double, PropertyNames::indicatorSpanAngle, true, IndicatorArrowIndicator::spanAngle, IndicatorArrowIndicator::setSpanAngle)
				->setCategory(PropertyNames::indicatorSettings)
				.setViewOrder(3);

		return;
	}

	bool IndicatorArrowIndicator::load(const Proto::SchemaItemIndicator& message, SchemaUnit unit)
	{
		m_itemUnit = unit;

		if (message.has_indicatorarrowindicator() == false)					// Line to change 1
		{
			// It can be just added new item, default values are taken
			//
			return true;
		}

		const ::Proto::IndicatorArrowIndicator& m = message.indicatorarrowindicator();	// Line to change 2

		m_startValue = m.startvalue();
		m_endValue = m.endvalue();

		m_startAngle = m.startangle();
		m_spanAngle = m.spanangle();

		return true;
	}

	bool IndicatorArrowIndicator::save(Proto::SchemaItemIndicator* message) const
	{
		if (message == nullptr)
		{
			Q_ASSERT(message);
			return false;
		}

		auto m = message->mutable_indicatorarrowindicator();		// Line to change 1

		m->set_startvalue(m_startValue);
		m->set_endvalue(m_endValue);

		m->set_startangle(m_startAngle);
		m->set_spanangle(m_spanAngle);

		return true;
	}

	void IndicatorArrowIndicator::draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer, const SchemaItemIndicator* item) const
	{
		if (drawParam == nullptr ||
			schema == nullptr ||
			layer == nullptr ||
			item == nullptr)
		{
			Q_ASSERT(drawParam);
			Q_ASSERT(schema);
			Q_ASSERT(layer);
			Q_ASSERT(item);

			return;
		}

		QPainter* p = drawParam->painter();
		Q_ASSERT(p);

		QRectF rect{item->boundingRectInDocPt(drawParam)};

		p->fillRect(rect, item->backgroundColor());

		return;
	}

	double IndicatorArrowIndicator::startValue() const
	{
		return m_startValue;
	}

	void IndicatorArrowIndicator::setStartValue(double value)
	{
		m_startValue = value;
	}

	double IndicatorArrowIndicator::endValue() const
	{
		return m_endValue;
	}

	void IndicatorArrowIndicator::setEndValue(double value)
	{
		m_endValue = value;
	}

	double IndicatorArrowIndicator::startAngle() const
	{
		return m_startAngle;
	}

	void IndicatorArrowIndicator::setStartAngle(double value)
	{
		m_startAngle = qBound(0.0, value, 360.0);
	}

	double IndicatorArrowIndicator::spanAngle() const
	{
		return m_spanAngle;
	}

	void IndicatorArrowIndicator::setSpanAngle(double value)
	{
		m_spanAngle = qBound(1.0, value, 360.0);
	}



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

	SchemaItemIndicator::~SchemaItemIndicator(void)
	{
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

		const IndicatorObject* io = indicatorObject();

		if (io == nullptr)
		{
			Q_ASSERT(io);
			return;
		}

		io->draw(drawParam,  schema, layer, this);

		return;
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

	IndicatorObject* SchemaItemIndicator::indicatorObject()
	{
		size_t index = static_cast<size_t>(m_indicatorType);

		if (index >= m_indicatorObjects.size())
		{
			Q_ASSERT(index < m_indicatorObjects.size());
			index = 0;
		}

		return m_indicatorObjects[index].get();
	}

	const IndicatorObject* SchemaItemIndicator::indicatorObject() const
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

