#include "EditSchemaView.h"
#include "../GlobalMessanger.h"
#include "../Settings.h"
#include "../../VFrame30/DrawParam.h"
#include "../../VFrame30/LogicSchema.h"
#include "../../VFrame30/MonitorSchema.h"
#include "../../VFrame30/PosLineImpl.h"
#include "../../VFrame30/PosRectImpl.h"
#include "../../VFrame30/PosConnectionImpl.h"

//
//
// EditSchemaView
//
//
EditSchemaView::EditSchemaView(SignalSetProvider* signalSetProvider, QWidget* parent) :
	VFrame30::SchemaView(parent),
	m_activeLayer(0),
	m_mouseState(MouseState::None),
	m_appSignalProvider(signalSetProvider),
	m_tuningSignalProvider(signalSetProvider),
	m_appSignalController(&m_appSignalProvider, nullptr),
	m_tuningController(&m_tuningSignalProvider, nullptr, nullptr)
{
	Q_ASSERT(signalSetProvider);

	// Timer for updates of WRN/ERR count
	//
	m_updateDuringBuildTimer = startTimer(50);
}

EditSchemaView::EditSchemaView(SignalSetProvider* signalSetProvider, std::shared_ptr<VFrame30::Schema> schema, QWidget* parent)
	: VFrame30::SchemaView(schema, parent),
	m_activeLayer(0),
	m_mouseState(MouseState::None),
	m_appSignalProvider(signalSetProvider),
	m_tuningSignalProvider(signalSetProvider),
	m_appSignalController(&m_appSignalProvider, nullptr),
	m_tuningController(&m_tuningSignalProvider, nullptr, nullptr)
{
	Q_ASSERT(signalSetProvider);

	// Timer for updates of WRN/ERR count
	//
	m_updateDuringBuildTimer = startTimer(50);
}

EditSchemaView::~EditSchemaView()
{
}

void EditSchemaView::timerEvent(QTimerEvent* event)
{
	VFrame30::SchemaView::timerEvent(event);

	if (event->timerId() == m_updateDuringBuildTimer)
	{
		// Repaint schema if there are any new issues for it
		//
		Builder::BuildIssues::Counter schemaIssues = GlobalMessanger::instance().buildIssues().issueForSchema(schema()->schemaId());

		if (schemaIssues.errors !=  m_lastSchemaIssues.errors ||
			schemaIssues.warnings !=  m_lastSchemaIssues.warnings)
		{
			m_lastSchemaIssues = schemaIssues;
			update();
		}

		return;
	}

	return;
}

void EditSchemaView::paintEvent(QPaintEvent* paintEvent)
{
	// Draw schema
	//
	QRectF clipRect(0, 0, schema()->docWidth(), schema()->docHeight());

	if (schema() != nullptr && theSettings.infoMode() == false)
	{
		int dpiX = logicalDpiX();
		int dpiY = logicalDpiY();

		QRect updateRect = paintEvent->rect();
		updateRect.adjust(-dpiX, -dpiY / 4, dpiX, dpiY / 4);	// -one inch, to draw pin names

		QPointF cls;
		QPointF clf;

		bool mok = true;
		mok &= MousePosToDocPoint(updateRect.topLeft(), &cls, dpiX, dpiY);
		mok &= MousePosToDocPoint(updateRect.bottomRight(), &clf, dpiX, dpiY);

		if (mok == true)
		{
			clipRect.setTopLeft(cls);
			clipRect.setSize({clf.x() - cls.x(), clf.y() - cls.y()});
		}
	}

	// VFrame30::SchemaView::paintEvent(pe);
	//
	if (schema() != nullptr)
	{
		QPainter p(this);

		VFrame30::CDrawParam drawParam(&p, schema().get(), this, schema()->gridSize(), schema()->pinGridStep());
		drawParam.setControlBarSize(
			schema()->unit() == VFrame30::SchemaUnit::Display ?	10 * (100.0 / zoom()) : mm2in(2.4) * (100.0 / zoom()));

		drawParam.setInfoMode(theSettings.infoMode());
		drawParam.session() = session();

		drawParam.setAppSignalController(&m_appSignalController);

		draw(drawParam, clipRect);
	}

	// Draw other -- selection, grid, outlines, rulers, etc
	//
	QPainter p;
	p.begin(this);

	p.save();

	VFrame30::CDrawParam drawParam(&p, schema().get(), this, schema()->gridSize(), schema()->pinGridStep());
	drawParam.setInfoMode(theSettings.infoMode());

	// Calc size
	//
	p.setRenderHint(QPainter::Antialiasing);

	// Ajust QPainter
	//
	Ajust(&p, 0, 0, zoom());

	// Draw schema
	//
	drawParam.setControlBarSize(
		schema()->unit() == VFrame30::SchemaUnit::Display ?	10 * (100.0 / zoom()) : mm2in(2.4) * (100.0 / zoom()));

	// Draw Build Issues
	//
	drawBuildIssues(&drawParam, clipRect);

	// Draw run order
	//
	if (theSettings.isDebugMode() == true)
	{
		drawRunOrder(&drawParam, clipRect);
	}

	// Draw selection
	//
	if (m_selectedItems.empty() == false)
	{
		VFrame30::SchemaItem::drawSelection(&drawParam, m_selectedItems, m_selectedItems.size() == 1);
	}

	// Draw Edit Connection lines outlines
	//
	drawEditConnectionLineOutline(&drawParam);

	// Draw newItem outline
	//
	drawNewItemOutline(&p, &drawParam);

	// Draw selection bar
	//
	drawSelectionArea(&p);

	// Items are being moved drawing
	//
	drawMovingItems(&drawParam);

	// --
	//
	drawRectSizing(&drawParam);
	drawMovingLinePoint(&drawParam);
	drawMovingEdgesOrVertexConnectionLine(&drawParam);

	if (m_compareWidget == true)
	{
		drawCompareOutlines(&drawParam, clipRect);
	}

	p.restore();

	// Draw grid performed IS NOT AJUSTED PAINTER
	//
	{
		QRect updateRect = paintEvent->rect();
		updateRect.adjust(-1, -1, 1, 1);

		drawGrid(&p, updateRect);
	}

	// --
	//
	p.end();

	return;
}

void EditSchemaView::drawBuildIssues(VFrame30::CDrawParam* drawParam, QRectF clipRect)
{
	if (drawParam == nullptr)
	{
		assert(drawParam != nullptr);
		return;
	}

	// Draw items by layers which has Show flag
	//
	double clipX = static_cast<double>(clipRect.left());
	double clipY = static_cast<double>(clipRect.top());
	double clipWidth = static_cast<double>(clipRect.width());
	double clipHeight = static_cast<double>(clipRect.height());

	// Find compile layer
	//
	for (auto layer = schema()->Layers.cbegin(); layer != schema()->Layers.cend(); ++layer)
	{
		const VFrame30::SchemaLayer* pLayer = layer->get();

		if (pLayer->compile() == false || pLayer->show() == false)
		{
			continue;
		}

		for (auto vi = pLayer->Items.cbegin(); vi != pLayer->Items.cend(); ++vi)
		{
			const SchemaItemPtr& item = *vi;

			OutputMessageLevel issue = GlobalMessanger::instance().issueForSchemaItem(item->guid());

			if ((issue == OutputMessageLevel::Warning0 ||
				 issue == OutputMessageLevel::Warning1 ||
				 issue == OutputMessageLevel::Warning2 ||
				 issue == OutputMessageLevel::Error) &&
				item->isIntersectRect(clipX, clipY, clipWidth, clipHeight) == true)
			{
				// Draw item issue
				//
				item->drawIssue(drawParam, issue);
			}
		}
	}

	return;
}

void EditSchemaView::drawRunOrder(VFrame30::CDrawParam* drawParam, QRectF clipRect)
{
	if (schema()->isLogicSchema() == false &&
		schema()->isUfbSchema() == false)
	{
		return;
	}

	if (drawParam == nullptr)
	{
		assert(drawParam != nullptr);
		return;
	}

	// Draw items by layers which has Show flag
	//
	double clipX = static_cast<double>(clipRect.left());
	double clipY = static_cast<double>(clipRect.top());
	double clipWidth = static_cast<double>(clipRect.width());
	double clipHeight = static_cast<double>(clipRect.height());

	// Find compile layer
	//
	for (auto layer = schema()->Layers.cbegin(); layer != schema()->Layers.cend(); ++layer)
	{
		const VFrame30::SchemaLayer* pLayer = layer->get();

		if (pLayer->compile() == false || pLayer->show() == false)
		{
			continue;
		}

		for (auto vi = pLayer->Items.cbegin(); vi != pLayer->Items.cend(); ++vi)
		{
			const SchemaItemPtr& item = *vi;

			QString orderIndexText;
			orderIndexText.reserve(32);

			if (item->isIntersectRect(clipX, clipY, clipWidth, clipHeight) == true)
			{
				orderIndexText = "?";

				if (schema()->isLogicSchema() == true)
				{
					VFrame30::LogicSchema* logicSchema = dynamic_cast<VFrame30::LogicSchema*>((schema().get()));
					if (logicSchema == nullptr)
					{
						assert(logicSchema);
						return;
					}

					if (logicSchema->isMultichannelSchema() == true)
					{
						QStringList eqIds = logicSchema->equipmentIdList();

						for (int i = 0; i < eqIds.size(); i++)
						{
							auto runIndex = GlobalMessanger::instance().schemaItemRunOrder(eqIds[i], item->guid());

							if (i == 0)
							{
								if (runIndex.first == runIndex.second)
								{
									orderIndexText = QString::number(runIndex.first);
								}
								else
								{
									orderIndexText = QString("%1-%2").arg(runIndex.first).arg(runIndex.second);
								}
							}
							else
							{
								if (runIndex.first == runIndex.second)
								{
									orderIndexText.append(QLatin1String(", ") + QString::number(runIndex.first));
								}
								else
								{
									orderIndexText.append(QString(", %1-%2").arg(runIndex.first).arg(runIndex.second));
								}
							}
						}
					}
					else
					{
						auto runIndex = GlobalMessanger::instance().schemaItemRunOrder(logicSchema->equipmentIds(), item->guid());

						if (runIndex.first == runIndex.second)
						{
							orderIndexText = QString::number(runIndex.first);
						}
						else
						{
							orderIndexText = QString("%1-%2").arg(runIndex.first).arg(runIndex.second);
						}
					}
				}

				if (schema()->isUfbSchema() == true)
				{
					auto runIndex = GlobalMessanger::instance().schemaItemRunOrder(schema()->schemaId(), item->guid());

					if (runIndex.first == runIndex.second)
					{
						orderIndexText = QString::number(runIndex.first);
					}
					else
					{
						orderIndexText = QString("%1-%2").arg(runIndex.first).arg(runIndex.second);
					}
				}

				item->drawDebugInfo(drawParam, orderIndexText);
			}
		}
	}

	return;
}

void EditSchemaView::drawEditConnectionLineOutline(VFrame30::CDrawParam* drawParam)
{
	bool ctrlIsPressed = QApplication::queryKeyboardModifiers().testFlag(Qt::ControlModifier);

	if (ctrlIsPressed == true ||
		m_doNotMoveConnectionLines == true)
	{
		return;
	}

	for (const EditConnectionLine& ecl : m_editConnectionLines)
	{
		ecl.drawOutline(drawParam);
	}

	return;
}

void EditSchemaView::drawNewItemOutline(QPainter* p, VFrame30::CDrawParam* drawParam)
{
	if (m_newItem == nullptr)
	{
		return;
	}

	if (drawParam == nullptr)
	{
		assert(drawParam != nullptr);
		return;
	}

	// Draw
	//
	std::vector<SchemaItemPtr> outlines;
	outlines.push_back(m_newItem);

	VFrame30::SchemaItem::drawOutline(drawParam, outlines);

	// Draw ruler for newItem
	//
	bool drawRulers = false;
	VFrame30::SchemaPoint rulerPoint;

	bool posInterfaceFound = false;

	if (dynamic_cast<VFrame30::IPosLine*>(m_newItem.get()) != nullptr)
	{
		if (mouseState() != MouseState::AddSchemaPosLineEndPoint)
		{
			return;
		}

		posInterfaceFound = true;

		VFrame30::IPosLine* pos = dynamic_cast<VFrame30::IPosLine*>(m_newItem.get());

		drawRulers = true;
		rulerPoint.X = pos->endXDocPt();
		rulerPoint.Y = pos->endYDocPt();
	}

	if (dynamic_cast<VFrame30::IPosRect*>(m_newItem.get()) != nullptr)
	{
		if (mouseState() != MouseState::AddSchemaPosRectEndPoint)
		{
			return;
		}

		posInterfaceFound = true;
		VFrame30::IPosRect* itemPos = dynamic_cast<VFrame30::IPosRect*>(m_newItem.get());

		drawRulers = true;

		rulerPoint.X = itemPos->leftDocPt() + itemPos->widthDocPt();
		rulerPoint.Y = itemPos->topDocPt() + itemPos->heightDocPt();
	}

	if (dynamic_cast<VFrame30::IPosConnection*>(m_newItem.get()) != nullptr)
	{
		if (mouseState() != MouseState::AddSchemaPosConnectionStartPoint &&
			mouseState() != MouseState::AddSchemaPosConnectionNextPoint)
		{
			return;
		}

		if (m_editConnectionLines.size() != 1)
		{
			return;
		}

		posInterfaceFound = true;
		const EditConnectionLine& ecl = m_editConnectionLines.front();

		if (ecl.extensionPoints().empty() == false)
		{
			drawRulers = true;
			rulerPoint = VFrame30::SchemaPoint(ecl.lastExtensionPoint());
		}
	}

	// --
	//
	if (posInterfaceFound == false)
	{
		assert(posInterfaceFound == true);
		return;
	}

	if (drawRulers == true)
	{
		QColor outlineColor(Qt::blue);
		outlineColor.setAlphaF(0.5);

		QPen outlinePen(outlineColor);
		outlinePen.setStyle(Qt::PenStyle::DashLine);
		outlinePen.setWidth(0);

		QPainter::RenderHints oldrenderhints = p->renderHints();
		p->setRenderHint(QPainter::Antialiasing, false);

		p->setPen(outlinePen);

		p->drawLine(QPointF(rulerPoint.X, 0.0), QPointF(rulerPoint.X, schema()->docHeight()));
		p->drawLine(QPointF(0.0, rulerPoint.Y), QPointF(schema()->docWidth(), rulerPoint.Y));

		p->setRenderHints(oldrenderhints);
	}

	return;
}

void EditSchemaView::drawSelectionArea(QPainter* p)
{
	QRectF r(m_mouseSelectionStartPoint, m_mouseSelectionEndPoint);

	QPen pen(QColor(0x33, 0x99, 0xFF, 0xE6));
	pen.setWidth(0);

	p->setPen(pen);
	p->setBrush(QColor(0x33, 0x99, 0xFF, 0x33));

	p->drawRect(r);

	return;
}

void EditSchemaView::drawMovingItems(VFrame30::CDrawParam* drawParam)
{
	if (mouseState() != MouseState::Moving ||
		m_selectedItems.empty() == true)
	{
		return;
	}

	double xdif = m_editEndDocPt.x() - m_editStartDocPt.x();
	double ydif = m_editEndDocPt.y() - m_editStartDocPt.y();

	// Shift position
	//
	bool ctrlIsPressed = QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);

	std::for_each(m_selectedItems.begin(), m_selectedItems.end(),
		[xdif, ydif, ctrlIsPressed](SchemaItemPtr si)
		{
			if (si->isLocked() == false ||
				(si->isLocked() == true && ctrlIsPressed == true))
			{
				si->moveItem(xdif, ydif);
			}
		});

	// Draw outline
	//
	VFrame30::SchemaItem::drawOutline(drawParam, m_selectedItems);

	// Get bounding rect
	//
	double left = 0.0;
	double right = 0.0;
	double top = 0.0;
	double bottom = 0.0;

	for (auto it = std::begin(m_selectedItems); it != std::end(m_selectedItems); it++)
	{
		SchemaItemPtr si = *it;

		if ((si->isLocked() == true && ctrlIsPressed == false) ||
			si->isLocked() == true)
		{
			continue;
		}

		VFrame30::IPointList* ipoint = dynamic_cast<VFrame30::IPointList*>(si.get());
		if (ipoint == nullptr)
		{
			assert(ipoint);
			continue;
		}

		std::vector<VFrame30::SchemaPoint> points = ipoint->getPointList();

		for (size_t i = 0; i < points.size(); i++)
		{
			const VFrame30::SchemaPoint& p = points[i];

			if (it == std::begin(m_selectedItems) && i == 0)
			{
				left = p.X;
				right = p.X;
				top = p.Y;
				bottom = p.Y;
				continue;
			}

			left = std::min(left, p.X);
			right = std::max(right, p.X);
			top = std::min(top, p.Y);
			bottom = std::max(bottom, p.Y);
		}
	}

	// Shift position back
	//
	std::for_each(m_selectedItems.begin(), m_selectedItems.end(),
		[xdif, ydif, ctrlIsPressed](SchemaItemPtr si)
		{
			if (si->isLocked() == false ||
				(si->isLocked() == true && ctrlIsPressed == true))
			{
				si->moveItem(-xdif, -ydif);
			}
		});

	// Draw rulers by bounding rect
	//
	QPainter* p = drawParam->painter();

	QPen outlinePen(Qt::blue);
	outlinePen.setStyle(Qt::PenStyle::DashLine);
	outlinePen.setWidth(0);

	QPainter::RenderHints oldrenderhints = p->renderHints();
	p->setRenderHint(QPainter::Antialiasing, false);

	p->setPen(outlinePen);
	p->drawLine(QPointF(left, 0.0), QPointF(left, schema()->docHeight()));
	p->drawLine(QPointF(right, 0.0), QPointF(right, schema()->docHeight()));

	p->drawLine(QPointF(0.0, top), QPointF(schema()->docWidth(), top));
	p->drawLine(QPointF(0.0, bottom), QPointF(schema()->docWidth(), bottom));

	// --
	//
	p->setRenderHints(oldrenderhints);

	return;
}

void EditSchemaView::drawRectSizing(VFrame30::CDrawParam* drawParam)
{
	if (mouseState() != MouseState::SizingTopLeft &&
		mouseState() != MouseState::SizingTop &&
		mouseState() != MouseState::SizingTopRight &&
		mouseState() != MouseState::SizingRight &&
		mouseState() != MouseState::SizingBottomRight &&
		mouseState() != MouseState::SizingBottom &&
		mouseState() != MouseState::SizingBottomLeft &&
		mouseState() != MouseState::SizingLeft)
	{
		return;
	}

	if (m_editStartDocPt.isNull() == true ||
		m_editEndDocPt.isNull() == true)
	{
		assert(m_editStartDocPt.isNull() == false);
		assert(m_editEndDocPt.isNull() == false);
		return;
	}

	if (selectedItems().size() != 1)
	{
		assert(selectedItems().size() == 1);
		return;
	}

	double xdif = m_editEndDocPt.x() - m_editStartDocPt.x();
	double ydif = m_editEndDocPt.y() - m_editStartDocPt.y();

	VFrame30::IPosRect* itemPos = dynamic_cast<VFrame30::IPosRect*>(selectedItems().front().get());
	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		return;
	}

	auto si = selectedItems().front();

	// Get new rect
	//
	QRectF newItemRect = sizingRectItem(xdif, ydif, itemPos);
	newItemRect = newItemRect.normalized();

	// Save old state
	//
	std::vector<VFrame30::SchemaPoint> oldPos = si->getPointList();

	// Set new pos
	//
	itemPos->setLeftDocPt(newItemRect.left());
	itemPos->setTopDocPt(newItemRect.top());
	itemPos->setWidthDocPt(newItemRect.width());
	itemPos->setHeightDocPt(newItemRect.height());

	// Save result for drawing rulers
	//
	m_addRectStartPoint = VFrame30::SchemaPoint(newItemRect.topLeft());
	m_addRectEndPoint = VFrame30::SchemaPoint(newItemRect.bottomRight());

	// Draw rulers by bounding rect
	//
	QPainter* p = drawParam->painter();

	QRectF rulerRect(m_addRectStartPoint, m_addRectEndPoint);

	QPen outlinePen(Qt::blue);
	outlinePen.setStyle(Qt::PenStyle::DashLine);
	outlinePen.setWidth(0);

	QPainter::RenderHints oldrenderhints = p->renderHints();
	p->setRenderHint(QPainter::Antialiasing, false);

	p->setPen(outlinePen);

	switch (mouseState())
	{
	case MouseState::SizingTopLeft:
		p->drawLine(QPointF(rulerRect.left(), 0.0), QPointF(rulerRect.left(), schema()->docHeight()));
		p->drawLine(QPointF(0.0, rulerRect.top()), QPointF(schema()->docWidth(), rulerRect.top()));
		break;
	case MouseState::SizingTop:
		p->drawLine(QPointF(0.0, rulerRect.top()), QPointF(schema()->docWidth(), rulerRect.top()));
		break;
	case MouseState::SizingTopRight:
		p->drawLine(QPointF(rulerRect.right(), 0.0), QPointF(rulerRect.right(), schema()->docHeight()));
		p->drawLine(QPointF(0.0, rulerRect.top()), QPointF(schema()->docWidth(), rulerRect.top()));
		break;
	case MouseState::SizingRight:
		p->drawLine(QPointF(rulerRect.right(), 0.0), QPointF(rulerRect.right(), schema()->docHeight()));
		break;
	case MouseState::SizingBottomRight:
		p->drawLine(QPointF(rulerRect.right(), 0.0), QPointF(rulerRect.right(), schema()->docHeight()));
		p->drawLine(QPointF(0.0, rulerRect.bottom()), QPointF(schema()->docWidth(), rulerRect.bottom()));
		break;
	case MouseState::SizingBottom:
		p->drawLine(QPointF(0.0, rulerRect.bottom()), QPointF(schema()->docWidth(), rulerRect.bottom()));
		break;
	case MouseState::SizingBottomLeft:
		p->drawLine(QPointF(rulerRect.left(), 0.0), QPointF(rulerRect.left(), schema()->docHeight()));
		p->drawLine(QPointF(0.0, rulerRect.bottom()), QPointF(schema()->docWidth(), rulerRect.bottom()));
		break;
	case MouseState::SizingLeft:
		p->drawLine(QPointF(rulerRect.left(), 0.0), QPointF(rulerRect.left(), schema()->docHeight()));
		break;
	default:
		assert(false);
		break;
	}
	p->setRenderHints(oldrenderhints);

	// Draw item outline
	//
	VFrame30::SchemaItem::drawOutline(drawParam, m_selectedItems);

	// restore position
	//
	si->setPointList(oldPos);
	return;
}

void EditSchemaView::drawMovingLinePoint(VFrame30::CDrawParam* drawParam)
{
	if (mouseState() != MouseState::MovingStartLinePoint &&
		mouseState() != MouseState::MovingEndLinePoint)
	{
		return;
	}

	if (m_editStartDocPt.isNull() == true ||
		m_editEndDocPt.isNull() == true)
	{
		assert(m_editStartDocPt.isNull() == false);
		assert(m_editEndDocPt.isNull() == false);
		return;
	}

	if (m_selectedItems.size() != 1)
	{
		assert(m_selectedItems.size() == 1);
		return;
	}

	auto si = m_selectedItems.front();
	VFrame30::IPosLine* itemPos = dynamic_cast<VFrame30::IPosLine*>(m_selectedItems.front().get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		return;
	}

	// Save current pos
	//

	auto oldPos = si->getPointList();

	// Set new pos
	//
	double xdif = m_editEndDocPt.x() - m_editStartDocPt.x();
	double ydif = m_editEndDocPt.y() - m_editStartDocPt.y();

	if (mouseState() == MouseState::MovingStartLinePoint)
	{
		itemPos->setStartXDocPt(itemPos->startXDocPt() + xdif);
		itemPos->setStartYDocPt(itemPos->startYDocPt() + ydif);
	}

	if (mouseState() == MouseState::MovingEndLinePoint)
	{
		itemPos->setEndXDocPt(itemPos->endXDocPt() + xdif);
		itemPos->setEndYDocPt(itemPos->endYDocPt() + ydif);
	}

	// Draw rulers
	//
	QPainter* p = drawParam->painter();

	QPen outlinePen(Qt::blue);
	outlinePen.setStyle(Qt::PenStyle::DashLine);
	outlinePen.setWidth(0);

	QPainter::RenderHints oldrenderhints = p->renderHints();
	p->setRenderHint(QPainter::Antialiasing, false);

	p->setPen(outlinePen);

	switch (mouseState())
	{
	case MouseState::MovingStartLinePoint:
		p->drawLine(QPointF(itemPos->startXDocPt(), 0.0), QPointF(itemPos->startXDocPt(), schema()->docHeight()));
		p->drawLine(QPointF(0.0, itemPos->startYDocPt()), QPointF(schema()->docWidth(), itemPos->startYDocPt()));
		break;
	case MouseState::MovingEndLinePoint:
		p->drawLine(QPointF(itemPos->endXDocPt(), 0.0), QPointF(itemPos->endXDocPt(), schema()->docHeight()));
		p->drawLine(QPointF(0.0, itemPos->endYDocPt()), QPointF(schema()->docWidth(), itemPos->endYDocPt()));
		break;
	default:
		assert(false);
		break;
	}
	p->setRenderHints(oldrenderhints);

	// Draw outline
	//
	VFrame30::SchemaItem::drawOutline(drawParam, m_selectedItems);

	// Resotore points
	//
	si->setPointList(oldPos);

	return;
}

void EditSchemaView::drawMovingEdgesOrVertexConnectionLine(VFrame30::CDrawParam* drawParam)
{
	if (mouseState() != MouseState::MovingHorizontalEdge &&
		mouseState() != MouseState::MovingVerticalEdge &&
		mouseState() != MouseState::MovingConnectionLinePoint)
	{
		return;
	}

	if (m_editConnectionLines.size() != 1)
	{
		assert(m_editConnectionLines.size() == 1);
		return;
	}

	const EditConnectionLine& ecl = m_editConnectionLines.front();

	// Draw rulers
	//
	QPainter* p = drawParam->painter();

	QColor outlineColor(Qt::blue);
	outlineColor.setAlphaF(0.5);

	QPen outlinePen(outlineColor);
	outlinePen.setStyle(Qt::PenStyle::DashLine);
	outlinePen.setWidth(0);

	QPainter::RenderHints oldrenderhints = p->renderHints();
	p->setRenderHint(QPainter::Antialiasing, false);

	p->setPen(outlinePen);

	switch (mouseState())
	{
	case MouseState::MovingHorizontalEdge:
		{
			double rulerPoint = ecl.editEdgetCurrState();
			p->drawLine(QPointF(0.0, rulerPoint), QPointF(schema()->docWidth(), rulerPoint));
		}
		break;
	case MouseState::MovingVerticalEdge:
		{
			double rulerPoint = ecl.editEdgetCurrState();
			p->drawLine(QPointF(rulerPoint, 0.0), QPointF(rulerPoint, schema()->docHeight()));
		}
		break;
	case MouseState::MovingConnectionLinePoint:
		{
			QPointF rulerPoint;
			switch (ecl.mode())
			{
			case EditConnectionLine::EditMode::AddToBegin:
			case EditConnectionLine::EditMode::AddToEnd:
				rulerPoint = ecl.lastExtensionPoint();
				break;
			case EditConnectionLine::EditMode::EditPoint:
				rulerPoint = ecl.editPointCurrState();
				break;
			default:
				assert(false);
			}

			p->drawLine(QPointF(rulerPoint.x(), 0.0), QPointF(rulerPoint.x(), schema()->docHeight()));
			p->drawLine(QPointF(0.0, rulerPoint.y()), QPointF(schema()->docWidth(), rulerPoint.y()));
		}
		break;
	default:
		assert(false);
		break;
	}

	p->setRenderHints(oldrenderhints);

	return;
}

void EditSchemaView::drawCompareOutlines(VFrame30::CDrawParam* drawParam, const QRectF& clipRect)
{
	if (drawParam == nullptr)
	{
		assert(drawParam != nullptr);
		return;
	}

	// Draw items by layers which has Show flag
	//
	double clipX = clipRect.left();
	double clipY = clipRect.top();
	double clipWidth = clipRect.width();
	double clipHeight = clipRect.height();

	// Find compile layer
	//
	for (auto layer = schema()->Layers.cbegin(); layer != schema()->Layers.cend(); ++layer)
	{
		const VFrame30::SchemaLayer* pLayer = layer->get();

		if (pLayer->show() == false)
		{
			continue;
		}

		for (auto vi = pLayer->Items.cbegin(); vi != pLayer->Items.cend(); ++vi)
		{
			const SchemaItemPtr& item = *vi;

			auto actionIt = m_itemsActions.find(item->guid());
			if (actionIt == m_itemsActions.end())
			{
				assert(actionIt != m_itemsActions.end());
				continue;
			}

			CompareAction compareAction = actionIt->second;

			QColor color;

			switch (compareAction)
			{
			case CompareAction::Unmodified:
				color = QColor(0, 0, 0, 0);			// Full transparent, as is
				break;
			case CompareAction::Modified:
				color = QColor(0, 0, 0xC0, 128);
				break;
			case CompareAction::Added:
				color = QColor(0, 0xC0, 0, 128);
				break;
			case CompareAction::Deleted:
				color = QColor(0xC0, 0, 0, 128);
				break;
			default:
				assert(false);
			}

			if (compareAction != CompareAction::Unmodified &&
				item->isIntersectRect(clipX, clipY, clipWidth, clipHeight) == true)
			{
				// Draw item issue
				//
				item->drawCompareAction(drawParam, color);
			}
		}
	}

}

void EditSchemaView::drawGrid(QPainter* p, const QRectF& clipRect)
{
	assert(p);

//	if (m_mouseSelectionStartPoint.isNull() == false &&
//		m_mouseSelectionEndPoint.isNull() == false)
//	{
//		// Don't draw grid if selection now,
//		// just speed optimization
//		//
//		return;
//	}

	auto unit = schema()->unit();

	double frameWidth = schema()->docWidth();
	double frameHeight = schema()->docHeight();
	double gridSize = schema()->gridSize();
	double scale = zoom() / 100.0;

	// Thin out the grid
	//
	if (unit == VFrame30::SchemaUnit::Display)
	{
		while (gridSize * scale < 11.0)
		{
			gridSize *= 2;
		}
	}
	else
	{
		while (gridSize * scale < 2.6 / 25.4)
		{
			gridSize *= 2;
		}
	}

	// Calculate points count
	//
	if (gridSize == 0)
	{
		assert(gridSize);
		return;
	}

	int horzGridCount = qBound(0, static_cast<int>(frameWidth / gridSize), 1024);
	int vertGridCount = qBound(0, static_cast<int>(frameHeight / gridSize), 1024);

	// Drawing grid
	//
	p->setPen(QColor{0x00, 0x00, 0x80, 0xB4});

	const double dpiX = unit == VFrame30::SchemaUnit::Display ? 1.0 : p->device()->logicalDpiX();
	const double dpiY = unit == VFrame30::SchemaUnit::Display ? 1.0 : p->device()->logicalDpiY();

	const double dpiXScale = gridSize * dpiX * scale;
	const double dpiYScale = gridSize * dpiY * scale;

	std::vector<QPointF> points;
	points.reserve(1024);

	QPointF pt;

	for (int v = 0; v < vertGridCount; ++v)
	{
		pt.setY(static_cast<double>(v + 1) * dpiYScale);
		points.clear();

		for (int h = 0; h < horzGridCount; ++h)
		{
			pt.setX(static_cast<double>(h + 1) * dpiXScale);

			if (clipRect.contains(pt) == true)
			{
				points.push_back(pt);
			}
		}

		p->drawPoints(points.data(), static_cast<int>(points.size()));
	}

	return;
}

SchemaItemAction EditSchemaView::getPossibleAction(VFrame30::SchemaItem* schemaItem, QPointF point, int* outMovingEdgePointIndex)
{
	// Params checks
	//
	if (schemaItem == nullptr)
	{
		Q_ASSERT(schemaItem != nullptr);
		return SchemaItemAction::NoAction;
	}

	if (schemaItem->itemUnit() != schema()->unit())
	{
		Q_ASSERT(schemaItem->itemUnit() == schema()->unit());
		return SchemaItemAction::NoAction;
	}

	if (outMovingEdgePointIndex != nullptr)
	{
		*outMovingEdgePointIndex = -1;
	}

	// --
	//
	double controlBarSize = ControlBar(schemaItem->itemUnit(), zoom());
	bool ctrlIsPressed = QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);

	// SchemaItem position and point are the same units
	//
	if (dynamic_cast<VFrame30::IPosRect*>(schemaItem) != nullptr)
	{
		VFrame30::IPosRect* itemPos = dynamic_cast<VFrame30::IPosRect*>(schemaItem) ;

		// If inside the rect then SchemaItemAction.MoveItem
		//
		if (schemaItem->isIntersectPoint(point.x(), point.y()) == true)
		{
			if (schemaItem->isLocked() == false ||
				(schemaItem->isLocked() == true && ctrlIsPressed == true))
			{
				return SchemaItemAction::MoveItem;
			}
			else
			{
				return SchemaItemAction::NoAction;
			}
		}

		if (schemaItem->isLocked() == true)
		{
			return SchemaItemAction::NoAction;
		}

		// �������� �� ������ ����������� ��������������� ControlBarSizeIn
		//
		QRectF itemRectangle;		// ����������������� �������������

		itemRectangle.setX(itemPos->leftDocPt());
		itemRectangle.setY(itemPos->topDocPt());
		itemRectangle.setWidth(itemPos->widthDocPt());
		itemRectangle.setHeight(itemPos->heightDocPt());

		if (QRectF(itemRectangle.left() - controlBarSize, itemRectangle.top() - controlBarSize, controlBarSize, controlBarSize).contains(point) == true)
		{
			return SchemaItemAction::ChangeSizeTopLeft;
		}

		if (QRectF(itemRectangle.left() + itemRectangle.width() / 2 - controlBarSize / 2, itemRectangle.top() - controlBarSize, controlBarSize, controlBarSize).contains(point) == true)
		{
			return SchemaItemAction::ChangeSizeTop;
		}

		if (QRectF(itemRectangle.right(), itemRectangle.top() - controlBarSize, controlBarSize, controlBarSize).contains(point) == true)
		{
			return SchemaItemAction::ChangeSizeTopRight;
		}

		if (QRectF(itemRectangle.right(), itemRectangle.top() + itemRectangle.height() / 2 - controlBarSize / 2, controlBarSize, controlBarSize).contains(point) == true)
		{
			return SchemaItemAction::ChangeSizeRight;
		}

		if (QRectF(itemRectangle.right(), itemRectangle.top() + itemRectangle.height(), controlBarSize, controlBarSize).contains(point) == true)
		{
			return SchemaItemAction::ChangeSizeBottomRight;
		}

		if (QRectF(itemRectangle.left() + itemRectangle.width() / 2 - controlBarSize / 2, itemRectangle.top() + itemRectangle.height(), controlBarSize, controlBarSize).contains(point) == true)
		{
			return SchemaItemAction::ChangeSizeBottom;
		}

		if (QRectF(itemRectangle.left() - controlBarSize, itemRectangle.top() + itemRectangle.height(), controlBarSize, controlBarSize).contains(point) == true)
		{
			return SchemaItemAction::ChangeSizeBottomLeft;
		}

		if (QRectF(itemRectangle.left() - controlBarSize, itemRectangle.top() + itemRectangle.height() / 2 - controlBarSize / 2, controlBarSize, controlBarSize).contains(point) == true)
		{
			return SchemaItemAction::ChangeSizeLeft;
		}

		return SchemaItemAction::NoAction;
	}

	if (dynamic_cast<VFrame30::IPosLine*>(schemaItem) != nullptr)
	{
		VFrame30::IPosLine* itemPos = dynamic_cast<VFrame30::IPosLine*>(schemaItem) ;

		double x1 = itemPos->startXDocPt();
		double y1 = itemPos->startYDocPt();
		double x2 = itemPos->endXDocPt();
		double y2 = itemPos->endYDocPt();

		// �������� �� ������ ����������� ��������������� ControlBarSizeIn
		// ��������������, �� ������� ����� ��������� � �������� �������
		//
		QRectF controlRectangles[2] = {QRectF{x1 - controlBarSize / 2, y1 - controlBarSize / 2, controlBarSize, controlBarSize},
									   QRectF{x2 - controlBarSize / 2, y2 - controlBarSize/ 2, controlBarSize, controlBarSize}};

		if (controlRectangles[0].contains(point) == true && schemaItem->isLocked() == false)
		{
			return SchemaItemAction::MoveStartLinePoint;
		}

		if (controlRectangles[1].contains(point) == true && schemaItem->isLocked() == false)
		{
			return SchemaItemAction::MoveEndLinePoint;
		}

		// ���� ������ �� �����, �� SchemaItemAction.MoveItem
		//
		if (schemaItem->isIntersectPoint(point.x(), point.y()) == true)
		{
			if (schemaItem->isLocked() == false ||
				(schemaItem->isLocked() == true && ctrlIsPressed == true))
			{
				return SchemaItemAction::MoveItem;
			}
			else
			{
				return SchemaItemAction::NoAction;
			}
		}

		return SchemaItemAction::NoAction;
	}


	if (dynamic_cast<VFrame30::IPosConnection*>(schemaItem) != nullptr)
	{
		VFrame30::IPosConnection* itemPos = dynamic_cast<VFrame30::IPosConnection*>(schemaItem) ;

		if (schemaItem->isLocked() == true)
		{
			return SchemaItemAction::NoAction;
		}

		// �������� �� ������ ����������� ��������������� ControlBarSizeIn
		//
		std::list<VFrame30::SchemaPoint> points = itemPos->GetPointList();

		int pointIndex = 0;
		for (auto pt = points.begin(); pt != points.end(); pt++, pointIndex++)
		{
			QRectF controlRect(pt->X - controlBarSize / 2, pt->Y - controlBarSize / 2, controlBarSize, controlBarSize);

			if (controlRect.contains(point.x(), point.y()) == true)
			{
				*outMovingEdgePointIndex = pointIndex;
				return SchemaItemAction::MoveConnectionLinePoint;
			}
		}

		// �������� ���� ��������
		//
		VFrame30::SchemaPoint lastPoint;

		pointIndex = 0;
		for (auto pt = points.begin(); pt != points.end(); pt++, pointIndex++)
		{
			if (pt == points.begin())
			{
				lastPoint = *pt;
				continue;
			}

			// ������������ �����
			//
			double x1 = std::min(lastPoint.X, pt->X);
			double y1 = std::min(lastPoint.Y, pt->Y);
			double x2 = std::max(lastPoint.X, pt->X);
			double y2 = std::max(lastPoint.Y, pt->Y);

			// ���������� �����
			//
			if (std::abs(x1 - x2) < std::abs(y1 - y2))
			{
				// The line is vertical
				//
				x1 -= controlBarSize / 4.0;
				x2 += controlBarSize / 4.0;

				if (point.x() >= x1 && point.x() <= x2 &&
					point.y() >= y1 && point.y() <= y2)
				{
					*outMovingEdgePointIndex = pointIndex - 1;
					return SchemaItemAction::MoveVerticalEdge;
				}
			}
			else
			{
				// The line is horizontal
				//
				y1 -= controlBarSize / 4.0;
				y2 += controlBarSize / 4.0;

				if (point.x() >= x1 && point.x() <= x2 &&
					point.y() >= y1 && point.y() <= y2)
				{
					*outMovingEdgePointIndex = pointIndex - 1;
					return SchemaItemAction::MoveHorizontalEdge;
				}
			}

			//--
			//
			lastPoint = *pt;
		}

		return SchemaItemAction::NoAction;
	}

	assert(false);

	return SchemaItemAction::NoAction;
}

QRectF EditSchemaView::sizingRectItem(double xdif, double ydif, VFrame30::IPosRect* itemPos)
{
	if (itemPos == nullptr)
	{
		assert(itemPos);
		return QRectF();
	}

	double x1 = itemPos->leftDocPt();
	double y1 = itemPos->topDocPt();
	double x2 = x1 + itemPos->widthDocPt();
	double y2 = y1 + itemPos->heightDocPt();

	double minWidth = itemPos->minimumPossibleWidthDocPt(schema()->gridSize(), schema()->pinGridStep());
	double minHeight = itemPos->minimumPossibleHeightDocPt(schema()->gridSize(), schema()->pinGridStep());

	switch (mouseState())
	{
	case MouseState::SizingTopLeft:
		x1 += xdif;
		y1 += ydif;
		if (x2 - x1 < minWidth)		// x1
		{
			x1 = x2 - minWidth;
		}
		if (y2 - y1 < minHeight)	// y1
		{
			y1 = y2 - minHeight;
		}
		break;
	case MouseState::SizingTop:
		y1 += ydif;
		if (y2 - y1 < minHeight)	// y1
		{
			y1 = y2 - minHeight;
		}
		break;
	case MouseState::SizingTopRight:
		x2 += xdif;
		y1 += ydif;
		if (x2 - x1 < minWidth)		// x2
		{
			x2 = x1 + minWidth;
		}
		if (y2 - y1 < minHeight)	// y1
		{
			y1 = y2 - minHeight;
		}
		break;
	case MouseState::SizingRight:
		x2 += xdif;
		if (x2 - x1 < minWidth)		// x2
		{
			x2 = x1 + minWidth;
		}
		break;
	case MouseState::SizingBottomRight:
		x2 += xdif;
		y2 += ydif;
		if (x2 - x1 < minWidth)		// x2
		{
			x2 = x1 + minWidth;
		}
		if (y2 - y1 < minHeight)	// y2
		{
			y2 = y1 + minHeight;
		}
		break;
	case MouseState::SizingBottom:
		y2 += ydif;
		if (y2 - y1 < minHeight)	// y2
		{
			y2 = y1 + minHeight;
		}
		break;
	case MouseState::SizingBottomLeft:
		x1 += xdif;
		y2 += ydif;
		if (x2 - x1 < minWidth)		// x1
		{
			x1 = x2 - minWidth;
		}
		if (y2 - y1 < minHeight)	// y2
		{
			y2 = y1 + minHeight;
		}
		break;
	case MouseState::SizingLeft:
		x1 += xdif;
		if (x2 - x1 < minWidth)		// x1
		{
			x1 = x2 - minWidth;
		}
		break;
	default:
		assert(false);
		break;
	}


	QRectF result(std::min(x1, x2),
				  std::min(y1, y2),
				  std::abs(x2 - x1),
				  std::abs(y2 - y1));

	return result;
}


QUuid EditSchemaView::activeLayerGuid() const
{
	return schema()->activeLayerGuid();
}

std::shared_ptr<VFrame30::SchemaLayer> EditSchemaView::activeLayer()
{
	return schema()->activeLayer();
}

void EditSchemaView::setActiveLayer(std::shared_ptr<VFrame30::SchemaLayer> layer)
{
	return schema()->setActiveLayer(layer);
}

MouseState EditSchemaView::mouseState() const
{
	return m_mouseState;
}

void EditSchemaView::setMouseState(MouseState state)
{
	m_mouseState = state;
}

const std::vector<SchemaItemPtr>& EditSchemaView::selectedItems() const
{
	return m_selectedItems;
}

std::vector<SchemaItemPtr> EditSchemaView::selectedNonLockedItems() const
{
	std::vector<SchemaItemPtr> result;
	result.reserve(m_selectedItems.size());

	for (const SchemaItemPtr& si : m_selectedItems)
	{
		if (si->isLocked() == false)
		{
			result.push_back(si);
		}
	}

	return result;
}


void EditSchemaView::setSelectedItems(const std::vector<SchemaItemPtr>& items)
{
	std::vector<SchemaItemPtr> uniqueItems;
	uniqueItems.reserve(16);

	// In some cases items can be duplicated (batch command), make them unique
	// We need to keep order of items
	//
	for (auto i : items)
	{
		auto foundIt = std::find(uniqueItems.begin(), uniqueItems.end(), i);

		if (foundIt == uniqueItems.end())
		{
			uniqueItems.push_back(i);
		}
	}

	// Check if the selected items are the same, don't do anything and don't emit selectionCanged
	//
	if (uniqueItems.size() == m_selectedItems.size())
	{
		bool differs = false;

		auto i = std::begin(uniqueItems);
		for (auto s = std::begin(m_selectedItems); s != std::end(m_selectedItems) && differs == false; ++s, ++i)
		{
			if (*s != *i)
			{
				differs = true;
				break;
			}
		}

		if (differs == false)
		{
			return;
		}
	}

	// Set new selection
	//
	m_selectedItems = uniqueItems;

	emit selectionChanged();

	return;
}

void EditSchemaView::setSelectedItems(const std::list<SchemaItemPtr>& items)
{
	// Check if the selected items are the same, don't do anything and don't emit selectionCanged
	//
	if (items.size() == m_selectedItems.size())
	{
		bool differs = false;

		auto i = std::begin(items);
		for (auto s = std::begin(m_selectedItems); s != std::end(m_selectedItems) && differs == false; ++s, ++i)
		{
			if (*s != *i)
			{
				differs = true;
				break;
			}
		}

		if (differs == false)
		{
			return;
		}
	}

	// Set new selection
	//
	m_selectedItems.clear();
	m_selectedItems.insert(m_selectedItems.begin(), items.begin(), items.end());

	emit selectionChanged();

	return;
}

void EditSchemaView::setSelectedItem(const SchemaItemPtr& item)
{
	if (m_selectedItems.size() == 1 && item == m_selectedItems.back())
	{
		return;
	}

	m_selectedItems.clear();
	m_selectedItems.push_back(item);

	emit selectionChanged();

	return;
}

void EditSchemaView::addSelection(const SchemaItemPtr& item, bool emitSectionChanged)
{
	auto fp = std::find(std::begin(m_selectedItems), std::end(m_selectedItems), item);

	if (fp == std::end(m_selectedItems))
	{
		m_selectedItems.push_back(item);

		if (emitSectionChanged == true)
		{
			emit selectionChanged();
		}
	}

	return;
}

void EditSchemaView::clearSelection()
{
	if (m_selectedItems.empty() == true)
	{
		return;
	}

	m_selectedItems.clear();
	emit selectionChanged();

	return;
}

bool EditSchemaView::removeFromSelection(const SchemaItemPtr& item, bool emitSectionChanged)
{
	auto findResult = std::find(m_selectedItems.begin(), m_selectedItems.end(), item);

	if (findResult != m_selectedItems.end())
	{
		m_selectedItems.erase(findResult);

		if (emitSectionChanged == true)
		{
			emit selectionChanged();
		}

		// Was found and deleted
		//
		return true;
	}

	// Was not found in selection list
	//
	return false;
}

bool EditSchemaView::isItemSelected(const SchemaItemPtr& item)
{
	auto findResult = std::find(m_selectedItems.begin(), m_selectedItems.end(), item);
	return findResult != m_selectedItems.end();
}
