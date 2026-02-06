#include "EditSchemaView.h"

#include <VFrame30/Context.h>
#include <VFrame30/DrawParam.h>
#include <VFrame30/PosConnectionImpl.h>
#include <VFrame30/PosLineImpl.h>
#include <VFrame30/PosRectImpl.h>
#include <VFrame30/PosRectRotatable.h>
#include <VFrame30/SchemaLayer.h>
#include <ClientLib/TuningConnectionStub.h>

#include "../GlobalMessanger.h"
#include "../Settings.h"

//
//
// EditSchemaView
//
//
EditSchemaView::EditSchemaView(AppSignalSetProvider* signalSetProvider, QWidget* parent) :
	VFrame30::SchemaViewWidget(parent),
	m_activeLayer(0),
	m_mouseState(MouseState::None),
	m_appSignalProvider(signalSetProvider),
	m_tuningSignalProvider(signalSetProvider),
	m_diagStateController(m_diagStateProvider, nullptr),
	m_appSignalController(m_appSignalProvider, nullptr),
	
	m_tuningConnectionStub(std::make_unique<ClientLib::TuningConnectionStub>()),
	m_tuningAuthorizationStub(std::make_unique<TuningAuthorizationStub>()),
	m_tuningController(m_tuningSignalProvider, *m_tuningConnectionStub, *m_tuningAuthorizationStub, nullptr)
{
	Q_ASSERT(signalSetProvider);

	// Timer for updates of WRN/ERR count
	//
	m_updateDuringBuildTimer = startTimer(50);
}

EditSchemaView::EditSchemaView(AppSignalSetProvider* signalSetProvider, std::shared_ptr<VFrame30::Schema> schema, QWidget* parent) :
	VFrame30::SchemaViewWidget(schema, parent),
	m_activeLayer(0),
	m_mouseState(MouseState::None),
	m_diagStateProvider(),
	m_appSignalProvider(signalSetProvider),
	m_tuningSignalProvider(signalSetProvider),
	m_diagStateController(m_diagStateProvider, nullptr),
	m_appSignalController(m_appSignalProvider, nullptr),

	m_tuningConnectionStub(std::make_unique<ClientLib::TuningConnectionStub>()),
	m_tuningAuthorizationStub(std::make_unique<TuningAuthorizationStub>()),
	m_tuningController(m_tuningSignalProvider, *m_tuningConnectionStub, *m_tuningAuthorizationStub, nullptr)
{
	Q_ASSERT(signalSetProvider);

	auto context =
		VFrame30::Context::create(&m_diagStateController, &m_appSignalController, &m_tuningController, nullptr, nullptr);
	schema->setContext(std::move(context));

	// Timer for updates of WRN/ERR count
	//
	m_updateDuringBuildTimer = startTimer(50);
}

EditSchemaView::~EditSchemaView() {}

VFrame30::DrawMode EditSchemaView::drawMode() const
{
	return VFrame30::DrawMode::Editor;
}

void EditSchemaView::timerEvent(QTimerEvent* event)
{
	VFrame30::SchemaViewWidget::timerEvent(event);

	if (event->timerId() == m_updateDuringBuildTimer)
	{
		// Repaint schema if there are any new issues for it
		//
		Builder::BuildIssues::Counter schemaIssues = GlobalMessanger::instance().buildIssues().issueForSchema(schema()->schemaId());

		if (schemaIssues.errors != m_lastSchemaIssues.errors || schemaIssues.warnings != m_lastSchemaIssues.warnings)
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
		QRect updateRect = paintEvent->rect();
		updateRect.adjust(-logicalDpiX(), -logicalDpiY() / 4, logicalDpiX(), logicalDpiY() / 4); // some space to draw pin names

		QPointF cls;
		QPointF clf;

		bool mouseOk = true;
		mouseOk &= MousePosToDocPoint(updateRect.topLeft(), &cls);
		mouseOk &= MousePosToDocPoint(updateRect.bottomRight(), &clf);

		if (mouseOk == true)
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

		VFrame30::CDrawParam drawParam(&p, this, schema()->gridSize(), schema()->pinGridStep(), schema()->unit());
		drawParam.setControlBarSize(CONTROL_BAR(schema()->unit(), p.device()->devicePixelRatioF(), zoom()));
		drawParam.setInfoMode(theSettings.infoMode());

		// QElapsedTimer et;
		// et.start();

		draw(drawParam, clipRect);

		// qDebug() << "EditSchemaView::paintEvent, draw time " << et.elapsed() << "ms";
	}

	// Draw other -- selection, grid, outlines, rulers, etc
	//
	QPainter p;
	p.begin(this);

	VFrame30::CDrawParam drawParam(&p, this, schema()->gridSize(), schema()->pinGridStep(), schema()->unit());
	drawParam.setInfoMode(theSettings.infoMode());

	// Calc size
	//
	p.setRenderHint(QPainter::Antialiasing);

	// Adjust QPainter
	//
	Ajust(&p, schema()->unit(), 0, 0, zoom());

	// Draw schema
	//
	drawParam.setControlBarSize(CONTROL_BAR(schema()->unit(), p.device()->devicePixelRatioF(), zoom()));

	// Draw Build Issues
	//
	drawBuildIssues(&drawParam, clipRect);

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

	// Draw proposed auto connections.
	//
	drawAutoFblItemConnection(drawParam);

	// --
	//
	drawRectSizing(&drawParam);
	drawMovingLinePoint(&drawParam);
	drawMovingEdgesOrVertexConnectionLine(&drawParam);

	if (m_compareWidget == true)
	{
		drawCompareOutlines(&drawParam, clipRect);
	}

	// Draw grid performed IN NOT AJUSTED PAINTER
	//
	{
		p.resetTransform();

		QRectF updateRect = paintEvent->rect();
		updateRect.moveTopLeft(updateRect.topLeft() * devicePixelRatioF());
		updateRect.setSize(updateRect.size() * devicePixelRatioF());

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
	for (const auto& layer : schema()->layers())
	{
		if (layer->compile() == false || layer->show() == false)
		{
			continue;
		}

		for (const auto& item : layer->items())
		{
			OutputMessageLevel issue = GlobalMessanger::instance().issueForSchemaItem(item->guid());

			if ((issue == OutputMessageLevel::Warning0 || issue == OutputMessageLevel::Warning1 || issue == OutputMessageLevel::Warning2 ||
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

void EditSchemaView::drawEditConnectionLineOutline(VFrame30::CDrawParam* drawParam)
{
	bool ctrlIsPressed = QApplication::queryKeyboardModifiers().testFlag(Qt::ControlModifier);

	if (ctrlIsPressed == true || m_doNotMoveConnectionLines == true)
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
		if (mouseState() != MouseState::AddSchemaPosConnectionStartPoint && mouseState() != MouseState::AddSchemaPosConnectionNextPoint)
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
	if (mouseState() != MouseState::Moving || m_selectedItems.empty() == true)
	{
		return;
	}

	double xdif = m_editEndDocPt.x() - m_editStartDocPt.x();
	double ydif = m_editEndDocPt.y() - m_editStartDocPt.y();

	// Shift position
	//
	bool ctrlIsPressed = QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);

	std::for_each(m_selectedItems.begin(),
				  m_selectedItems.end(),
				  [xdif, ydif, ctrlIsPressed](SchemaItemPtr si)
				  {
					  if (si->isLocked() == false || (si->isLocked() == true && ctrlIsPressed == true))
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

		if ((si->isLocked() == true && ctrlIsPressed == false) || si->isLocked() == true)
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

		// If item is rotated we need to take just the first point (top left), all other points are outside of the bounding rect.
		//
		VFrame30::PosRectRotatable* itemPosRotatable = dynamic_cast<VFrame30::PosRectRotatable*>(si.get());
		bool rotated = itemPosRotatable != nullptr && itemPosRotatable->angle() != 0;
		if (rotated)
		{
			points.resize(1);
		}

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
	std::for_each(m_selectedItems.begin(),
				  m_selectedItems.end(),
				  [xdif, ydif, ctrlIsPressed](SchemaItemPtr si)
				  {
					  if (si->isLocked() == false || (si->isLocked() == true && ctrlIsPressed == true))
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
	if (mouseState() != MouseState::SizingTopLeft && mouseState() != MouseState::SizingTop && mouseState() != MouseState::SizingTopRight &&
		mouseState() != MouseState::SizingRight && mouseState() != MouseState::SizingBottomRight &&
		mouseState() != MouseState::SizingBottom && mouseState() != MouseState::SizingBottomLeft && mouseState() != MouseState::SizingLeft)
	{
		return;
	}

	if (m_editStartDocPt.isNull() == true || m_editEndDocPt.isNull() == true)
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

	auto schemaItem = selectedItems().front();

	VFrame30::IPosRect* itemPos = dynamic_cast<VFrame30::IPosRect*>(schemaItem.get());
	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		return;
	}

	// Get new rect
	//
	double xdif = m_editEndDocPt.x() - m_editStartDocPt.x();
	double ydif = m_editEndDocPt.y() - m_editStartDocPt.y();

	QRectF newItemRect = sizingRectItem(xdif, ydif, itemPos);
	newItemRect = newItemRect.normalized();

	// Save old state
	//
	std::vector<VFrame30::SchemaPoint> oldPos = schemaItem->getPointList();

	// Set new pos
	//
	itemPos->setLeftDocPt(newItemRect.left());
	itemPos->setTopDocPt(newItemRect.top());
	itemPos->setWidthDocPt(newItemRect.width());
	itemPos->setHeightDocPt(newItemRect.height());

	// Save result for drawing rulers
	//
	m_addRectStartPoint = newItemRect.topLeft();
	m_addRectEndPoint = newItemRect.bottomRight();

	// Rotate m_addRectEndPoint if needed.
	//
	auto itemPosRotatable = dynamic_cast<VFrame30::PosRectRotatable*>(selectedItems().front().get());
	bool rotated = itemPosRotatable != nullptr && itemPosRotatable->angle() != 0;
	if (rotated == true)
	{
		auto rotatePoint = itemPosRotatable->rotationPointInDocPt();

		QTransform transform;
		transform.translate(rotatePoint.x(), rotatePoint.y());
		transform.rotate(itemPosRotatable->angle());
		transform.translate(-rotatePoint.x(), -rotatePoint.y());

		m_addRectEndPoint = transform.map(m_addRectEndPoint);
	}

	// Draw rulers by bounding rect only if rect is not rotated or if it is rotated and we are sizing bottom right corner.
	//
	if (rotated == false || (rotated == true && mouseState() == MouseState::SizingBottomRight))
	{
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
	}

	// Draw item outline
	//
	VFrame30::SchemaItem::drawOutline(drawParam, m_selectedItems);

	// restore position
	//
	schemaItem->setPointList(oldPos);
	return;
}

void EditSchemaView::drawMovingLinePoint(VFrame30::CDrawParam* drawParam)
{
	if (mouseState() != MouseState::MovingStartLinePoint && mouseState() != MouseState::MovingEndLinePoint)
	{
		return;
	}

	if (m_editStartDocPt.isNull() == true || m_editEndDocPt.isNull() == true)
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

	// Restore points
	//
	si->setPointList(oldPos);

	return;
}

void EditSchemaView::drawMovingEdgesOrVertexConnectionLine(VFrame30::CDrawParam* drawParam)
{
	if (mouseState() != MouseState::MovingHorizontalEdge && mouseState() != MouseState::MovingVerticalEdge &&
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

	// Draw items by layers which has show flag
	//
	double clipX = clipRect.left();
	double clipY = clipRect.top();
	double clipWidth = clipRect.width();
	double clipHeight = clipRect.height();

	// Find compile layer
	//
	for (const auto& layer : schema()->layers())
	{
		if (layer->show() == false)
		{
			continue;
		}

		for (const auto& item : layer->items())
		{
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
				color = QColor(0, 0, 0, 0); // Full transparent, as is
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

			if (compareAction != CompareAction::Unmodified && item->isIntersectRect(clipX, clipY, clipWidth, clipHeight) == true)
			{
				// Draw item issue
				//
				item->drawCompareAction(drawParam, color);
			}
		}
	}
}

void EditSchemaView::drawAutoFblItemConnection(VFrame30::CDrawParam& drawParam)
{
	QPainter* painter = drawParam.painter();

	QPen pen{Qt::darkGray};
	pen.setCosmetic(true);
	pen.setStyle(Qt::PenStyle::DashLine);

	painter->setPen(pen);
	painter->setBrush(QColor{0xF0, 0xF0, 0xF0, 0xE0});

	static const VFrame30::FontParam font{"Arial", 1.0 / 8.0, false, false};

	for (auto lines = m_autoFblItemConnection.getPropositions(); const auto& line : lines)
	{
		painter->drawLine(line.from, line.to);
		painter->drawRect(line.addButtonRect);

		VFrame30::DrawHelper::drawText(painter, font, SchemaUnit::Inch, "+", line.addButtonRect, Qt::AlignCenter);
	}

	return;
}

void EditSchemaView::drawGrid(QPainter* p, const QRectF& clipRect)
{
	Q_ASSERT(p);

	//	if (m_mouseSelectionStartPoint.isNull() == false &&
	//		m_mouseSelectionEndPoint.isNull() == false)
	//	{
	//		// Don't draw grid if selection now,
	//		// just speed optimization
	//		//
	//		return;
	//	}

	auto unit = schema()->unit();
	double documentWidth = schema()->docWidth();
	double documentHeight = schema()->docHeight();
	double gridSize = schema()->gridSize();
	double scale = zoom() / 100.0;

	// Thin out the grid
	//
	if (unit == SchemaUnit::Display)
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
		Q_ASSERT(gridSize);
		return;
	}

	int horzGridCount = qBound(0, static_cast<int>(documentWidth / gridSize), 1024);
	int vertGridCount = qBound(0, static_cast<int>(documentHeight / gridSize), 1024);

	// Drawing grid
	//
	p->setPen(QColor{0x00, 0x00, 0x80, 0xFF});

	auto screen = this->screen();
	assert(screen);

	const double dpiX = (unit == SchemaUnit::Display) ? (1.0) : (screen->physicalDotsPerInchX() * devicePixelRatioF());

	const double dpiY = (unit == SchemaUnit::Display) ? (1.0) : (screen->physicalDotsPerInchY() * devicePixelRatioF());

	const double dpiXScale = gridSize * dpiX * scale;
	const double dpiYScale = gridSize * dpiY * scale;

	std::vector<QPointF> points;
	points.reserve(1024);

	QPointF pt;

	p->scale(1.0 / devicePixelRatioF(), 1.0 / devicePixelRatioF());

	for (int v = 0; v < vertGridCount; ++v)
	{
		pt.setY(static_cast<double>(v + 1) * dpiYScale);
		points.clear();

		for (int h = 0; h < horzGridCount; ++h)
		{
			pt.setX(static_cast<double>(h + 1) * dpiXScale);

			if (clipRect.contains(pt) == true)
			{
				points.emplace_back(pt);
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
	const double controlBarSize = CONTROL_BAR(schemaItem->itemUnit(), this->devicePixelRatioF(), zoom());
	const bool ctrlIsPressed = QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);

	// SchemaItem position and point are the same units
	//
	if (dynamic_cast<VFrame30::IPosRect*>(schemaItem) != nullptr)
	{
		VFrame30::IPosRect* itemPos = dynamic_cast<VFrame30::IPosRect*>(schemaItem);

		// If inside the rect then SchemaItemAction.MoveItem
		//
		if (schemaItem->isIntersectPoint(point.x(), point.y()) == true)
		{
			if (schemaItem->isLocked() == false || (schemaItem->isLocked() == true && ctrlIsPressed == true))
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

		// Check control bars.
		//
		QRectF itemRectangle{itemPos->leftDocPt(), itemPos->topDocPt(), itemPos->widthDocPt(), itemPos->heightDocPt()};

		std::vector<std::pair<QRectF, SchemaItemAction>> barRects;
		barRects.reserve(8);

		// If this is the rotatable item and the angle is not 0, for different rotation points deferent control bars.
		// Attention, this code has duplication with PosRectRotatable::drawSelectionPrivate() method.
		//
		if (const auto rotatableItem = dynamic_cast<const VFrame30::PosRectRotatable*>(schemaItem); rotatableItem != nullptr)
		{
			auto controlBarArray = rotatableItem->controlBarRects(controlBarSize);
			static_assert(controlBarArray.size() == 8);

			if (controlBarArray[0].isNull() == false)
			{
				barRects.emplace_back(controlBarArray[0], SchemaItemAction::ChangeSizeTopLeft);
			}
			if (controlBarArray[1].isNull() == false)
			{
				barRects.emplace_back(controlBarArray[1], SchemaItemAction::ChangeSizeTop);
			}
			if (controlBarArray[2].isNull() == false)
			{
				barRects.emplace_back(controlBarArray[2], SchemaItemAction::ChangeSizeTopRight);
			}
			if (controlBarArray[3].isNull() == false)
			{
				barRects.emplace_back(controlBarArray[3], SchemaItemAction::ChangeSizeRight);
			}
			if (controlBarArray[4].isNull() == false)
			{
				barRects.emplace_back(controlBarArray[4], SchemaItemAction::ChangeSizeBottomRight);
			}
			if (controlBarArray[5].isNull() == false)
			{
				barRects.emplace_back(controlBarArray[5], SchemaItemAction::ChangeSizeBottom);
			}
			if (controlBarArray[6].isNull() == false)
			{
				barRects.emplace_back(controlBarArray[6], SchemaItemAction::ChangeSizeBottomLeft);
			}
			if (controlBarArray[7].isNull() == false)
			{
				barRects.emplace_back(controlBarArray[7], SchemaItemAction::ChangeSizeLeft);
			}
		}
		else
		{
			barRects.emplace_back(
				QRectF{itemRectangle.left() - controlBarSize, itemRectangle.top() - controlBarSize, controlBarSize, controlBarSize},
				SchemaItemAction::ChangeSizeTopLeft);
			barRects.emplace_back(QRectF{itemRectangle.left() + itemRectangle.width() / 2 - controlBarSize / 2,
										 itemRectangle.top() - controlBarSize,
										 controlBarSize,
										 controlBarSize},
								  SchemaItemAction::ChangeSizeTop);
			barRects.emplace_back(QRectF{itemRectangle.right(), itemRectangle.top() - controlBarSize, controlBarSize, controlBarSize},
								  SchemaItemAction::ChangeSizeTopRight);
			barRects.emplace_back(QRectF{itemRectangle.right(),
										 itemRectangle.top() + itemRectangle.height() / 2 - controlBarSize / 2,
										 controlBarSize,
										 controlBarSize},
								  SchemaItemAction::ChangeSizeRight);
			barRects.emplace_back(
				QRectF{itemRectangle.right(), itemRectangle.top() + itemRectangle.height(), controlBarSize, controlBarSize},
				SchemaItemAction::ChangeSizeBottomRight);
			barRects.emplace_back(QRectF{itemRectangle.left() + itemRectangle.width() / 2 - controlBarSize / 2,
										 itemRectangle.top() + itemRectangle.height(),
										 controlBarSize,
										 controlBarSize},
								  SchemaItemAction::ChangeSizeBottom);
			barRects.emplace_back(
				QRectF{itemRectangle.left() - controlBarSize, itemRectangle.top() + itemRectangle.height(), controlBarSize, controlBarSize},
				SchemaItemAction::ChangeSizeBottomLeft);
			barRects.emplace_back(QRectF{itemRectangle.left() - controlBarSize,
										 itemRectangle.top() + itemRectangle.height() / 2 - controlBarSize / 2,
										 controlBarSize,
										 controlBarSize},
								  SchemaItemAction::ChangeSizeLeft);
		}

		if (VFrame30::PosRectRotatable* rotatableItem = dynamic_cast<VFrame30::PosRectRotatable*>(schemaItem);
			rotatableItem != nullptr && rotatableItem->angle() != 0.0)
		{
			// Check BarRectangles with rotations
			//
			auto rotatePoint = rotatableItem->rotationPointInDocPt();

			QTransform transform;
			transform.translate(rotatePoint.x(), rotatePoint.y());
			transform.rotate(rotatableItem->angle());
			transform.translate(-rotatePoint.x(), -rotatePoint.y());

			QPainterPath path;
			for (const auto& [barRect, action] : barRects)
			{
				QPolygonF rotatedRect = transform.map(barRect);

				path.clear();
				path.addPolygon(rotatedRect);
				path.closeSubpath();

				if (path.contains(point) == true)
				{
					return action;
				}
			}
		}
		else
		{
			// Check BarRectangles without rotations
			//
			for (const auto& [rect, action] : barRects)
			{
				if (rect.contains(point) == true)
				{
					return action;
				}
			}
		}

		return SchemaItemAction::NoAction;
	}

	if (dynamic_cast<VFrame30::IPosLine*>(schemaItem) != nullptr)
	{
		VFrame30::IPosLine* itemPos = dynamic_cast<VFrame30::IPosLine*>(schemaItem);

		double x1 = itemPos->startXDocPt();
		double y1 = itemPos->startYDocPt();
		double x2 = itemPos->endXDocPt();
		double y2 = itemPos->endYDocPt();

		QRectF controlRectangles[2] = {QRectF{x1 - controlBarSize / 2, y1 - controlBarSize / 2, controlBarSize, controlBarSize},
									   QRectF{x2 - controlBarSize / 2, y2 - controlBarSize / 2, controlBarSize, controlBarSize}};

		if (controlRectangles[0].contains(point) == true && schemaItem->isLocked() == false)
		{
			return SchemaItemAction::MoveStartLinePoint;
		}

		if (controlRectangles[1].contains(point) == true && schemaItem->isLocked() == false)
		{
			return SchemaItemAction::MoveEndLinePoint;
		}

		if (schemaItem->isIntersectPoint(point.x(), point.y()) == true)
		{
			if (schemaItem->isLocked() == false || (schemaItem->isLocked() == true && ctrlIsPressed == true))
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
		if (schemaItem->isLocked() == true)
		{
			return SchemaItemAction::NoAction;
		}

		VFrame30::PosConnectionImpl* itemPos = dynamic_cast<VFrame30::PosConnectionImpl*>(schemaItem);
		std::list<VFrame30::SchemaPoint> points = itemPos->GetPointList();

		int pointIndex = 0;
		for (auto pt = points.begin(); pt != points.end(); pt++, pointIndex++)
		{
			QRectF controlRect{pt->X - controlBarSize / 2, pt->Y - controlBarSize / 2, controlBarSize, controlBarSize};

			if (controlRect.contains(point.x(), point.y()) == true)
			{
				*outMovingEdgePointIndex = pointIndex;
				return SchemaItemAction::MoveConnectionLinePoint;
			}
		}

		// --
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

			// --
			//
			double x1 = std::min(lastPoint.X, pt->X);
			double y1 = std::min(lastPoint.Y, pt->Y);
			double x2 = std::max(lastPoint.X, pt->X);
			double y2 = std::max(lastPoint.Y, pt->Y);

			// --
			//
			if (std::abs(x1 - x2) < std::abs(y1 - y2))
			{
				// The line is vertical
				//
				x1 -= controlBarSize / 4.0;
				x2 += controlBarSize / 4.0;

				if (point.x() >= x1 && point.x() <= x2 && point.y() >= y1 && point.y() <= y2)
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

				if (point.x() >= x1 && point.x() <= x2 && point.y() >= y1 && point.y() <= y2)
				{
					*outMovingEdgePointIndex = pointIndex - 1;
					return SchemaItemAction::MoveHorizontalEdge;
				}
			}

			//--
			//
			lastPoint = *pt;
		}

		// Move Item
		//
		QRectF br = itemPos->boundingRectInDocPt(nullptr);
		QRectF moveBarRect = br;

		moveBarRect.setWidth(controlBarSize * 2);
		moveBarRect.setHeight(controlBarSize * 2);
		moveBarRect.moveCenter(br.center());

		if (moveBarRect.contains(point.x(), point.y()) == true)
		{
			return SchemaItemAction::MoveItem;
		}

		return SchemaItemAction::NoAction;
	}

	assert(false);

	return SchemaItemAction::NoAction;
}

QRectF EditSchemaView::sizingRectItem(double xdif, double ydif, const VFrame30::IPosRect* itemPos)
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
		if (x2 - x1 < minWidth)  // x1
		{
			x1 = x2 - minWidth;
		}
		if (y2 - y1 < minHeight) // y1
		{
			y1 = y2 - minHeight;
		}
		break;
	case MouseState::SizingTop:
		y1 += ydif;
		if (y2 - y1 < minHeight) // y1
		{
			y1 = y2 - minHeight;
		}
		break;
	case MouseState::SizingTopRight:
		x2 += xdif;
		y1 += ydif;
		if (x2 - x1 < minWidth)  // x2
		{
			x2 = x1 + minWidth;
		}
		if (y2 - y1 < minHeight) // y1
		{
			y1 = y2 - minHeight;
		}
		break;
	case MouseState::SizingRight:
		x2 += xdif;
		if (x2 - x1 < minWidth) // x2
		{
			x2 = x1 + minWidth;
		}
		break;
	case MouseState::SizingBottomRight:
		x2 += xdif;
		y2 += ydif;
		if (x2 - x1 < minWidth)  // x2
		{
			x2 = x1 + minWidth;
		}
		if (y2 - y1 < minHeight) // y2
		{
			y2 = y1 + minHeight;
		}
		break;
	case MouseState::SizingBottom:
		y2 += ydif;
		if (y2 - y1 < minHeight) // y2
		{
			y2 = y1 + minHeight;
		}
		break;
	case MouseState::SizingBottomLeft:
		x1 += xdif;
		y2 += ydif;
		if (x2 - x1 < minWidth)  // x1
		{
			x1 = x2 - minWidth;
		}
		if (y2 - y1 < minHeight) // y2
		{
			y2 = y1 + minHeight;
		}
		break;
	case MouseState::SizingLeft:
		x1 += xdif;
		if (x2 - x1 < minWidth) // x1
		{
			x1 = x2 - minWidth;
		}
		break;
	default:
		assert(false);
		break;
	}

	QRectF result(std::min(x1, x2), std::min(y1, y2), std::abs(x2 - x1), std::abs(y2 - y1));

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

	// Check if the selected items are the same, don't do anything and don't emit selectionChanged
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

	m_autoFblItemConnection.setItems(m_selectedItems);

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

	m_autoFblItemConnection.setItems(m_selectedItems);

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

	m_autoFblItemConnection.setItems(m_selectedItems);

	emit selectionChanged();

	return;
}

void EditSchemaView::addSelection(const SchemaItemPtr& item, bool emitSectionChanged)
{
	auto fp = std::find(std::begin(m_selectedItems), std::end(m_selectedItems), item);

	if (fp == std::end(m_selectedItems))
	{
		m_selectedItems.push_back(item);
		m_autoFblItemConnection.setItems(m_selectedItems);

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
	m_autoFblItemConnection.setItems(m_selectedItems);

	emit selectionChanged();

	return;
}

bool EditSchemaView::removeFromSelection(const SchemaItemPtr& item, bool emitSectionChanged)
{
	auto findResult = std::find(m_selectedItems.begin(), m_selectedItems.end(), item);

	if (findResult != m_selectedItems.end())
	{
		m_selectedItems.erase(findResult);
		m_autoFblItemConnection.setItems(m_selectedItems);

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

void EditSchemaView::exportToPdf(const QString& fileName, bool infoMode)
{
	if (schema() == nullptr)
	{
		return;
	}

	// --
	//
	QPdfWriter pdfWriter(fileName);

	pdfWriter.setTitle(schema()->caption());

	QPageSize pageSize;
	double pageWidth = schema()->docWidth();
	double pageHeight = schema()->docHeight();

	if (schema()->unit() == SchemaUnit::Inch)
	{
		pageSize = QPageSize(QSizeF(pageWidth, pageHeight), QPageSize::Inch);
	}
	else
	{
		assert(schema()->unit() == SchemaUnit::Display);
		pageSize = QPageSize(QSize(static_cast<int>(pageWidth), static_cast<int>(pageHeight)));

		pdfWriter.setResolution(72); // 72 is from enum QPageLayout::Unit help,
									 // QPageLayout::Point	1	1/!!! 72th !!!! of an inch
	}

	pdfWriter.setPageSize(pageSize);
	pdfWriter.setPageMargins(QMarginsF(0, 0, 0, 0));

	// --
	//
	QPainter p(&pdfWriter);
	VFrame30::CDrawParam drawParam(&p, this, schema()->gridSize(), schema()->pinGridStep(), schema()->unit());

	drawParam.setInfoMode(infoMode);
	drawParam.setControlBarSize(
		CONTROL_BAR(schema()->unit(), p.device()->devicePixelRatioF(), 100.0)); // Compare outlines depends on ControlBarSize
	drawParam.setPdfMode(true);

	// Calc size
	//
	int widthInPixel = schema()->GetDocumentWidth(pdfWriter.resolution(), 100.0);   // Export 100% zoom
	int heightInPixel = schema()->GetDocumentHeight(pdfWriter.resolution(), 100.0); // Export 100% zoom

	// Clear device
	//
	p.fillRect(QRectF(0, 0, widthInPixel + 1, heightInPixel + 1), QColor(0xB0, 0xB0, 0xB0));
	p.setRenderHint(QPainter::Antialiasing);

	// Ajust QPainter
	//
	Ajust(&p, schema()->unit(), 0, 0, 100.0); // Export 100% zoom

	// Draw Schema
	//
	{
		QApplication::setOverrideCursor(Qt::WaitCursor);

		QRectF clipRect(0, 0, schema()->docWidth(), schema()->docHeight());
		schema()->Draw(&drawParam, clipRect);

		if (m_compareWidget == true)
		{
			drawCompareOutlines(&drawParam, clipRect);
		}

		QApplication::restoreOverrideCursor();
	}

	// Ending
	//

	return;
}
