#include "Stable.h"
#include <QRubberBand>
#include <QClipboard>
#include <QCompleter>
#include <QMimeData>
#include "EditEngine/EditEngine.h"
#include "EditSchemaWidget.h"
#include "SchemaPropertiesDialog.h"
#include "SchemaLayersDialog.h"
#include "SchemaItemPropertiesDialog.h"
#include "ChooseAfbDialog.h"
#include "ChooseUfbDialog.h"
#include "SignalPropertiesDialog.h"
#include "GlobalMessanger.h"
#include "../VFrame30/UfbSchema.h"
#include "../VFrame30/SchemaItemLine.h"
#include "../VFrame30/SchemaItemRect.h"
#include "../VFrame30/SchemaItemPath.h"
#include "../VFrame30/SchemaItemSignal.h"
#include "../VFrame30/SchemaItemAfb.h"
#include "../VFrame30/SchemaItemLink.h"
#include "../VFrame30/SchemaItemConst.h"
#include "../VFrame30/SchemaItemConnection.h"
#include "../VFrame30/SchemaItemUfb.h"
#include "../VFrame30/SchemaItemTerminator.h"
#include "../VFrame30/Session.h"
#include "SignalsTabPage.h"
#include "Forms/ComparePropertyObjectDialog.h"

const EditSchemaWidget::MouseStateCursor EditSchemaWidget::m_mouseStateCursor[] =
	{
		{MouseState::Scrolling, Qt::CursorShape::ArrowCursor},
		{MouseState::Selection, Qt::CursorShape::CrossCursor},
		{MouseState::Moving, Qt::CursorShape::SizeAllCursor},
		{MouseState::SizingTopLeft, Qt::CursorShape::SizeFDiagCursor},
		{MouseState::SizingTop, Qt::CursorShape::SizeVerCursor},
		{MouseState::SizingTopRight, Qt::CursorShape::SizeBDiagCursor},
		{MouseState::SizingRight, Qt::CursorShape::SizeHorCursor},
		{MouseState::SizingBottomRight, Qt::CursorShape::SizeFDiagCursor},
		{MouseState::SizingBottom, Qt::CursorShape::SizeVerCursor},
		{MouseState::SizingBottomLeft, Qt::CursorShape::SizeBDiagCursor},
		{MouseState::SizingLeft, Qt::CursorShape::SizeHorCursor},
		{MouseState::MovingStartLinePoint, Qt::CursorShape::SizeAllCursor},
		{MouseState::MovingEndLinePoint, Qt::CursorShape::SizeAllCursor},
		{MouseState::MovingHorizontalEdge, Qt::CursorShape::SplitVCursor},
		{MouseState::MovingVerticalEdge, Qt::CursorShape::SplitHCursor},
		{MouseState::MovingConnectionLinePoint, Qt::CursorShape::SizeAllCursor},
	};

const EditSchemaWidget::SizeActionToMouseCursor EditSchemaWidget::m_sizeActionToMouseCursor[] =
	{
		{SchemaItemAction::ChangeSizeTopLeft, MouseState::SizingTopLeft, Qt::SizeFDiagCursor},
		{SchemaItemAction::ChangeSizeTop, MouseState::SizingTop, Qt::SizeVerCursor},
		{SchemaItemAction::ChangeSizeTopRight, MouseState::SizingTopRight, Qt::SizeBDiagCursor},
		{SchemaItemAction::ChangeSizeRight, MouseState::SizingRight, Qt::SizeHorCursor},
		{SchemaItemAction::ChangeSizeBottomRight, MouseState::SizingBottomRight, Qt::SizeFDiagCursor},
		{SchemaItemAction::ChangeSizeBottom, MouseState::SizingBottom, Qt::SizeVerCursor},
		{SchemaItemAction::ChangeSizeBottomLeft, MouseState::SizingBottomLeft, Qt::SizeBDiagCursor},
		{SchemaItemAction::ChangeSizeLeft, MouseState::SizingLeft, Qt::SizeHorCursor}
	};


void addSchemaItem(const QByteArray& itemData);



//
// SchemaItemsClipboard
//
const char* SchemaItemClipboardData::mimeType = "application/x-radiyschemaset";

//
//
// EditSchemaView
//
//
EditSchemaView::EditSchemaView(QWidget* parent) :
	VFrame30::SchemaView(parent),
	m_activeLayer(0),
	m_mouseState(MouseState::None),
	m_editStartMovingEdge(0),
	m_editEndMovingEdge(0),
	m_editStartMovingEdgeX(0),
	m_editStartMovingEdgeY(0),
	m_editEndMovingEdgeX(0),
	m_editEndMovingEdgeY(0),
	m_movingEdgePointIndex(0)
	//m_rubberBand(new QRubberBand(QRubberBand::Rectangle, this))
{
}

EditSchemaView::EditSchemaView(std::shared_ptr<VFrame30::Schema>& schema, QWidget* parent)
	: VFrame30::SchemaView(schema, parent),
	m_activeLayer(0),
	m_mouseState(MouseState::None),
	m_editStartMovingEdge(0),
	m_editEndMovingEdge(0),
	m_editStartMovingEdgeX(0),
	m_editStartMovingEdgeY(0),
	m_editEndMovingEdgeX(0),
	m_editEndMovingEdgeY(0),
	m_movingEdgePointIndex(0)
	//m_rubberBand(new QRubberBand(QRubberBand::Rectangle, this))
{
}

EditSchemaView::~EditSchemaView()
{
}

void EditSchemaView::paintEvent(QPaintEvent* /*pe*/)
{
	// Draw schema
	//

	// VFrame30::SchemaView::paintEvent(pe);
	//
	if (schema().get() != nullptr)
	{
		QPainter p(this);

		VFrame30::CDrawParam drawParam(&p, schema().get(), schema()->gridSize(), schema()->pinGridStep());
		drawParam.setControlBarSize(
			schema()->unit() == VFrame30::SchemaUnit::Display ?	10 * (100.0 / zoom()) : mm2in(2.4) * (100.0 / zoom()));

		drawParam.setInfoMode(theSettings.infoMode());

		drawParam.session() = session();

		draw(drawParam);
	}

	// Draw other -- selection, grid, outlines, rullers, etc
	//
	QPainter p;
	p.begin(this);

	p.save();

	VFrame30::CDrawParam drawParam(&p, schema().get(), schema()->gridSize(), schema()->pinGridStep());
	drawParam.setInfoMode(theSettings.infoMode());

	// Calc size
	//
	p.setRenderHint(QPainter::Antialiasing);

	// Ajust QPainter
	//
	Ajust(&p, 0, 0, zoom());

	// Draw schema
	//
	QRectF clipRect(0, 0, schema()->docWidth(), schema()->docHeight());

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
		VFrame30::SchemaItem::DrawSelection(&drawParam, m_selectedItems, m_selectedItems.size() == 1);
	}

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

	// Draw grid performed in not ajusted painter
	//
	drawGrid(&p);

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
			const std::shared_ptr<VFrame30::SchemaItem>& item = *vi;

			OutputMessageLevel issue = GlobalMessanger::instance()->issueForSchemaItem(item->guid());

			if ((issue == OutputMessageLevel::Warning0 ||
				 issue == OutputMessageLevel::Warning1 ||
				 issue == OutputMessageLevel::Warning2 ||
				 issue == OutputMessageLevel::Error) &&
				item->IsIntersectRect(clipX, clipY, clipWidth, clipHeight) == true)
			{
				// Draw item issue
				//
				item->DrawIssue(drawParam, issue);
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
			const std::shared_ptr<VFrame30::SchemaItem>& item = *vi;

			QString orderIndexText;
			orderIndexText.reserve(32);

			if (item->IsIntersectRect(clipX, clipY, clipWidth, clipHeight) == true)
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
							auto runIndex = GlobalMessanger::instance()->schemaItemRunOrder(eqIds[i], item->guid());

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
						auto runIndex = GlobalMessanger::instance()->schemaItemRunOrder(logicSchema->equipmentIds(), item->guid());

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
					auto runIndex = GlobalMessanger::instance()->schemaItemRunOrder(schema()->schemaId(), item->guid());

					if (runIndex.first == runIndex.second)
					{
						orderIndexText = QString::number(runIndex.first);
					}
					else
					{
						orderIndexText = QString("%1-%2").arg(runIndex.first).arg(runIndex.second);
					}
				}

				item->DrawDebugInfo(drawParam, orderIndexText);
			}
		}
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
	std::vector<std::shared_ptr<VFrame30::SchemaItem>> outlines;
	outlines.push_back(m_newItem);

	VFrame30::SchemaItem::DrawOutline(drawParam, outlines);

	// Draw ruller for newItem
	//
	bool drawRullers = false;
	VFrame30::SchemaPoint rullerPoint;

	bool posInterfaceFound = false;

	if (dynamic_cast<VFrame30::IPosLine*>(m_newItem.get()) != nullptr)
	{
		if (mouseState() != MouseState::AddSchemaPosLineEndPoint)
		{
			return;
		}

		posInterfaceFound = true;

		VFrame30::IPosLine* pos = dynamic_cast<VFrame30::IPosLine*>(m_newItem.get());

		drawRullers = true;
		rullerPoint.X = pos->endXDocPt();
		rullerPoint.Y = pos->endYDocPt();
	}

	if (dynamic_cast<VFrame30::IPosRect*>(m_newItem.get()) != nullptr)
	{
		if (mouseState() != MouseState::AddSchemaPosRectEndPoint)
		{
			return;
		}

		posInterfaceFound = true;
		VFrame30::IPosRect* itemPos = dynamic_cast<VFrame30::IPosRect*>(m_newItem.get());

		drawRullers = true;

		rullerPoint.X = itemPos->leftDocPt() + itemPos->widthDocPt();
		rullerPoint.Y = itemPos->topDocPt() + itemPos->heightDocPt();
	}

	if (dynamic_cast<VFrame30::IPosConnection*>(m_newItem.get()) != nullptr)
	{
		if (mouseState() != MouseState::AddSchemaPosConnectionStartPoint &&
			mouseState() != MouseState::AddSchemaPosConnectionNextPoint)
		{
			return;
		}

		posInterfaceFound = true;
		VFrame30::IPosConnection* pos = dynamic_cast<VFrame30::IPosConnection*>(m_newItem.get());

		const std::list<VFrame30::SchemaPoint>& extPoints = pos->GetExtensionPoints();

		if (extPoints.empty() == false)
		{
			drawRullers = true;
			rullerPoint.X = extPoints.back().X;
			rullerPoint.Y = extPoints.back().Y;
		}
	}

	if (posInterfaceFound == false)
	{
		assert(posInterfaceFound == true);
		return;
	}

	if (drawRullers == true)
	{
		QColor outlineColor(Qt::blue);
		outlineColor.setAlphaF(0.5);

		QPen outlinePen(outlineColor);
		outlinePen.setWidth(0);

		QPainter::RenderHints oldrenderhints = p->renderHints();
		p->setRenderHint(QPainter::Antialiasing, false);

		p->setPen(outlinePen);

		p->drawLine(QPointF(rullerPoint.X, 0.0), QPointF(rullerPoint.X, schema()->docHeight()));
		p->drawLine(QPointF(0.0, rullerPoint.Y), QPointF(schema()->docWidth(), rullerPoint.Y));

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

	float xdif = m_editEndDocPt.x() - m_editStartDocPt.x();
	float ydif = m_editEndDocPt.y() - m_editStartDocPt.y();

	// Shift position
	//
	bool ctrlIsPressed = QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);

	std::for_each(m_selectedItems.begin(), m_selectedItems.end(),
		[xdif, ydif, ctrlIsPressed](std::shared_ptr<VFrame30::SchemaItem> si)
		{
			if (si->isLocked() == false ||
				(si->isLocked() == true && ctrlIsPressed == true))
			{
				si->MoveItem(xdif, ydif);
			}
		});

	// Draw outline
	//
	VFrame30::SchemaItem::DrawOutline(drawParam, m_selectedItems);

	// Get bounding rect
	//
	double left = 0.0;
	double right = 0.0;
	double top = 0.0;
	double bottom = 0.0;

	for (auto it = std::begin(m_selectedItems); it != std::end(m_selectedItems); it++)
	{
		std::shared_ptr<VFrame30::SchemaItem> si = *it;

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
		[xdif, ydif, ctrlIsPressed](std::shared_ptr<VFrame30::SchemaItem> si)
		{
			if (si->isLocked() == false ||
				(si->isLocked() == true && ctrlIsPressed == true))
			{
				si->MoveItem(-xdif, -ydif);
			}
		});

	// Draw rullers by bounding rect
	//
	QPainter* p = drawParam->painter();

	QPen outlinePen(Qt::blue);
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

	float xdif = m_editEndDocPt.x() - m_editStartDocPt.x();
	float ydif = m_editEndDocPt.y() - m_editStartDocPt.y();

	VFrame30::IPosRect* itemPos = dynamic_cast<VFrame30::IPosRect*>(selectedItems().front().get());
	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		return;
	}

	auto si = selectedItems().front();

	// save old state
	//
	std::vector<VFrame30::SchemaPoint> oldPos = si->getPointList();

	// set new pos
	//
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

	itemPos->setLeftDocPt(std::min(x1, x2));
	itemPos->setTopDocPt(std::min(y1, y2));

	double width = std::max(std::abs(x2 - x1), itemPos->minimumPossibleWidthDocPt(schema()->gridSize(), schema()->pinGridStep()));
	double height = std::max(std::abs(y2 - y1), itemPos->minimumPossibleHeightDocPt(schema()->gridSize(), schema()->pinGridStep()));

	itemPos->setWidthDocPt(width);
	itemPos->setHeightDocPt(height);

	// Save result for drawing rullers
	//
	m_addRectStartPoint = VFrame30::SchemaPoint(x1, y1);
	m_addRectEndPoint = VFrame30::SchemaPoint(x2, y2);

	// Draw rullers by bounding rect
	//
	QPainter* p = drawParam->painter();

	QRectF rullerRect(m_addRectStartPoint, m_addRectEndPoint);

	QPen outlinePen(Qt::blue);
	outlinePen.setWidth(0);

	QPainter::RenderHints oldrenderhints = p->renderHints();
	p->setRenderHint(QPainter::Antialiasing, false);

	p->setPen(outlinePen);

	switch (mouseState())
	{
	case MouseState::SizingTopLeft:
		p->drawLine(QPointF(rullerRect.left(), 0.0), QPointF(rullerRect.left(), schema()->docHeight()));
		p->drawLine(QPointF(0.0, rullerRect.top()), QPointF(schema()->docWidth(), rullerRect.top()));
		break;
	case MouseState::SizingTop:
		p->drawLine(QPointF(0.0, rullerRect.top()), QPointF(schema()->docWidth(), rullerRect.top()));
		break;
	case MouseState::SizingTopRight:
		p->drawLine(QPointF(rullerRect.right(), 0.0), QPointF(rullerRect.right(), schema()->docHeight()));
		p->drawLine(QPointF(0.0, rullerRect.top()), QPointF(schema()->docWidth(), rullerRect.top()));
		break;
	case MouseState::SizingRight:
		p->drawLine(QPointF(rullerRect.right(), 0.0), QPointF(rullerRect.right(), schema()->docHeight()));
		break;
	case MouseState::SizingBottomRight:
		p->drawLine(QPointF(rullerRect.right(), 0.0), QPointF(rullerRect.right(), schema()->docHeight()));
		p->drawLine(QPointF(0.0, rullerRect.bottom()), QPointF(schema()->docWidth(), rullerRect.bottom()));
		break;
	case MouseState::SizingBottom:
		p->drawLine(QPointF(0.0, rullerRect.bottom()), QPointF(schema()->docWidth(), rullerRect.bottom()));
		break;
	case MouseState::SizingBottomLeft:
		p->drawLine(QPointF(rullerRect.left(), 0.0), QPointF(rullerRect.left(), schema()->docHeight()));
		p->drawLine(QPointF(0.0, rullerRect.bottom()), QPointF(schema()->docWidth(), rullerRect.bottom()));
		break;
	case MouseState::SizingLeft:
		p->drawLine(QPointF(rullerRect.left(), 0.0), QPointF(rullerRect.left(), schema()->docHeight()));
		break;
	default:
		assert(false);
		break;
	}
	p->setRenderHints(oldrenderhints);

	// Draw item outline
	//
	VFrame30::SchemaItem::DrawOutline(drawParam, m_selectedItems);

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

	// Draw rullers
	//
	QPainter* p = drawParam->painter();

	QPen outlinePen(Qt::blue);
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
	VFrame30::SchemaItem::DrawOutline(drawParam, m_selectedItems);

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

	if (m_movingEdgePointIndex == -1)
	{
		assert(m_movingEdgePointIndex != -1);
		return;
	}

	if (selectedItems().size() != 1)
	{
		assert(selectedItems().size() == 1);
		return;
	}

	auto si = selectedItems().front();

	VFrame30::IPosConnection* itemPos = dynamic_cast<VFrame30::IPosConnection*>(si.get());
	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		return;
	}

	std::list<VFrame30::SchemaPoint> pointsList = itemPos->GetPointList();
	std::vector<VFrame30::SchemaPoint> points(pointsList.begin(), pointsList.end());

	// Save position
	//
	auto oldPos = si->getPointList();

	if (m_movingEdgePointIndex < 0 || m_movingEdgePointIndex >= static_cast<int>(points.size()))
	{
		assert(m_movingEdgePointIndex >= 0);
		assert(m_movingEdgePointIndex < static_cast<int>(points.size()));
		return;
	}

	QPointF rullerPoint;

	// Calculate new position
	//
	switch (mouseState())
	{
	case MouseState::MovingHorizontalEdge:
		{
			double diff = m_editEndMovingEdge - m_editStartMovingEdge;

			VFrame30::SchemaPoint oldEdgeStart = points[m_movingEdgePointIndex];
			VFrame30::SchemaPoint oldEdgeEnd = points[m_movingEdgePointIndex + 1];

			VFrame30::SchemaPoint op = points[m_movingEdgePointIndex];
			op.Y += diff;

			points[m_movingEdgePointIndex] = op;

			//
			op = points[m_movingEdgePointIndex + 1];
			op.Y += diff;

			points[m_movingEdgePointIndex + 1] = op;

			rullerPoint.setY(op.Y);

			// ���� �� �������� ���� ��� �������������� ����� �� �������� �����,
			// ��� �� ����� �� ������ �� ��������� �������� �������
			//
			if (m_movingEdgePointIndex + 2 < static_cast<int>(points.size()) &&
				std::abs(points[m_movingEdgePointIndex + 2].Y - oldEdgeEnd.Y) < 0.000001)
			{
				points.insert(points.begin() + m_movingEdgePointIndex + 2, oldEdgeEnd);
			}

			if (m_movingEdgePointIndex - 1 >= 0 &&
				std::abs(points[m_movingEdgePointIndex - 1].Y - oldEdgeStart.Y) < 0.000001)
			{
				points.insert(points.begin() + m_movingEdgePointIndex, oldEdgeStart);
			}
		}
		break;
	case MouseState::MovingVerticalEdge:
		{
			double diff = m_editEndMovingEdge - m_editStartMovingEdge;

			VFrame30::SchemaPoint oldEdgeStart = points[m_movingEdgePointIndex];
			VFrame30::SchemaPoint oldEdgeEnd = points[m_movingEdgePointIndex + 1];

			VFrame30::SchemaPoint op = points[m_movingEdgePointIndex];
			op.X += diff;

			points[m_movingEdgePointIndex] = op;
			//
			op = points[m_movingEdgePointIndex + 1];
			op.X += diff;

			points[m_movingEdgePointIndex + 1] = op;

			rullerPoint.setX(op.X);

			// ���� �� �������� ���� ��� ������������ ����� �� �������� �����,
			// ��� �� ����� �� ������ �� ��������� �������� �������
			//
			if (m_movingEdgePointIndex + 2 < static_cast<int>(points.size()) &&
				std::abs(points[m_movingEdgePointIndex + 2].X - oldEdgeEnd.X) < 0.000001)
			{
				points.insert(points.begin() + m_movingEdgePointIndex + 2, oldEdgeEnd);
			}

			if (m_movingEdgePointIndex - 1 >= 0 &&
				std::abs(points[m_movingEdgePointIndex - 1].X - oldEdgeStart.X) < 0.000001)
			{
				points.insert(points.begin() + m_movingEdgePointIndex, oldEdgeStart);
			}
		}
		break;
	case MouseState::MovingConnectionLinePoint:
		{
			double diffX = m_editEndMovingEdgeX - points[m_movingEdgePointIndex].X;
			double diffY = m_editEndMovingEdgeY - points[m_movingEdgePointIndex].Y;

			// Shift the previouse point
			//
			if (m_movingEdgePointIndex - 1 >= 0)
			{
				int index = m_movingEdgePointIndex;
				bool sameDirrection = true;
				bool wasVert = true;
				bool wasHorz = true;
				VFrame30::SchemaPoint curPoint = points[index];

				while (index > 0 && sameDirrection == true)
				{
					VFrame30::SchemaPoint prevPoint = points[index - 1];

					if (std::abs(prevPoint.X - curPoint.X) < std::abs(prevPoint.Y - curPoint.Y))
					{
						if (wasVert == true)
						{
							// The line is vertical
							//
							prevPoint.X += diffX;
							wasHorz = false;
							wasVert = true;
						}
						else
						{
							sameDirrection = false;
						}
					}
					else
					{
						if (wasHorz == true)
						{
							// The line is horizontal
							//
							prevPoint.Y += diffY;
							wasHorz = true;
							wasVert = false;
						}
						else
						{
							sameDirrection = false;
						}
					}

					if (sameDirrection == true)
					{
						curPoint = points[index - 1];
						points[index - 1] = prevPoint;
					}

					index--;
				}
			}

			// Shift the next point
			//
			if (m_movingEdgePointIndex + 1 < static_cast<int>(points.size()))
			{
				int index = m_movingEdgePointIndex;
				bool sameDirrection = true;
				bool wasVert = true;
				bool wasHorz = true;
				VFrame30::SchemaPoint curPoint = points[index];

				while (index + 1 < static_cast<int>(points.size()) && sameDirrection == true)
				{
					VFrame30::SchemaPoint nextPoint = points[index + 1];

					if (std::abs(nextPoint.X - curPoint.X) < std::abs(nextPoint.Y - curPoint.Y))
					{
						if (wasVert == true)
						{
							// The line is vertical
							//
							nextPoint.X += diffX;
							wasHorz = false;
							wasVert = true;
						}
						else
						{
							sameDirrection = false;
						}
					}
					else
					{
						if (wasHorz == true)
						{
							// The line is horizontal
							//
							nextPoint.Y += diffY;
							wasHorz = true;
							wasVert = false;
						}
						else
						{
							sameDirrection = false;
						}
					}

					if (sameDirrection == true)
					{
						curPoint = points[index + 1];
						points[index + 1] = nextPoint;
					}

					index++;
				}
			}

			// Shift the moving point
			//
			VFrame30::SchemaPoint pt = points[m_movingEdgePointIndex];

			pt.X += diffX;
			pt.Y += diffY;

			points[m_movingEdgePointIndex] = pt;

			rullerPoint.setX(pt.X);
			rullerPoint.setY(pt.Y);
		}
		break;

		default:
			void();
	}

	// Set calculated pos to SchemaItem
	//
	pointsList.assign(points.begin(), points.end());
	itemPos->SetPointList(pointsList);
	itemPos->RemoveSamePoints();

	// SavePoints to View, so later they will be used in MouseUp action to set new position
	//
	m_movingVertexPoints = itemPos->GetPointList();

	// Draw item outline
	//
	VFrame30::SchemaItem::DrawOutline(drawParam, m_selectedItems);

	// Draw rullers
	//
	QPainter* p = drawParam->painter();

	QColor outlineColor(Qt::blue);
	outlineColor.setAlphaF(0.5);

	QPen outlinePen(outlineColor);
	outlinePen.setWidth(0);

	QPainter::RenderHints oldrenderhints = p->renderHints();
	p->setRenderHint(QPainter::Antialiasing, false);

	p->setPen(outlinePen);

	switch (mouseState())
	{
	case MouseState::MovingHorizontalEdge:
		p->drawLine(QPointF(0.0, rullerPoint.y()), QPointF(schema()->docWidth(), rullerPoint.y()));
		break;
	case MouseState::MovingVerticalEdge:
		p->drawLine(QPointF(rullerPoint.x(), 0.0), QPointF(rullerPoint.x(), schema()->docHeight()));
		break;
	case MouseState::MovingConnectionLinePoint:
		p->drawLine(QPointF(rullerPoint.x(), 0.0), QPointF(rullerPoint.x(), schema()->docHeight()));
		p->drawLine(QPointF(0.0, rullerPoint.y()), QPointF(schema()->docWidth(), rullerPoint.y()));
		break;
	default:
		assert(false);
		break;
	}
	p->setRenderHints(oldrenderhints);


	// Restore ald position
	//
	si->setPointList(oldPos);

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
			const std::shared_ptr<VFrame30::SchemaItem>& item = *vi;

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
				item->IsIntersectRect(clipX, clipY, clipWidth, clipHeight) == true)
			{
				// Draw item issue
				//
				item->drawCompareAction(drawParam, color);
			}
		}
	}

}

void EditSchemaView::drawGrid(QPainter* p)
{
	assert(p);

	if (m_mouseSelectionStartPoint.isNull() == false &&
		m_mouseSelectionEndPoint.isNull() == false)
	{
		// Don't draw grid if selection now,
		// just speed optimization
		//
		return;
	}

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
		// ��������� ����, ���� ����� ��������� ����� ��� 2 �� ���� � �����
		//
		while (gridSize * scale < 2.6 / 25.4)
		{
			gridSize *= 2;
		}
	}

	// Calculate points count
	//
	int horzGridCount = (int)(frameWidth / gridSize);
	int vertGridCount = (int)(frameHeight / gridSize);

	// Drawing grid
	//
	p->setPen(QColor(0x00, 0x00, 0x80, 0xB4));
	QPointF pt;

	QRegion visiblePart = visibleRegion();

	double dpiX = unit == VFrame30::SchemaUnit::Display ? 1.0 : p->device()->physicalDpiX();
	double dpiY = unit == VFrame30::SchemaUnit::Display ? 1.0 : p->device()->physicalDpiY();

	for (int v = 0; v < vertGridCount; v++)
	{
		pt.setY(static_cast<double>(v + 1) * gridSize * dpiY * scale);

		for (int h = 0; h < horzGridCount; h++)
		{
			pt.setX(static_cast<double>(h + 1) * gridSize * dpiX * scale);

			if (visiblePart.contains(pt.toPoint()) == false)
			{
				continue;
			}

			p->drawPoint(pt);
		}
	}

	return;
}

SchemaItemAction EditSchemaView::getPossibleAction(VFrame30::SchemaItem* schemaItem, QPointF point, int* outMovingEdgePointIndex)
{
	// Params checks
	//
	if (schemaItem == nullptr)
	{
		assert(schemaItem != nullptr);
		return SchemaItemAction::NoAction;
	}

	if (schemaItem->itemUnit() != schema()->unit())
	{
		assert(schemaItem->itemUnit() == schema()->unit());
		return SchemaItemAction::NoAction;
	}

	if (outMovingEdgePointIndex != nullptr)
	{
		*outMovingEdgePointIndex = -1;
	}

	// --
	//
	float controlBarSize = ControlBar(schemaItem->itemUnit(), zoom());
	bool ctrlIsPressed = QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);

	// SchemaItem position and point are the same units
	//
	if (dynamic_cast<VFrame30::IPosRect*>(schemaItem) != nullptr)
	{
		VFrame30::IPosRect* itemPos = dynamic_cast<VFrame30::IPosRect*>(schemaItem) ;

		// If inside the rect then SchemaItemAction.MoveItem
		//
		if (schemaItem->IsIntersectPoint(point.x(), point.y()) == true)
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

		// ���� ������ �� �����, �� SchemaItemAction.MoveItem
		//
		if (schemaItem->IsIntersectPoint(point.x(), point.y()) == true)
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
		// ��������������, �� ������� ����� ��������� � �������� �������
		//
		QRectF controlRectangles[2];

		controlRectangles[0] = QRectF(x1 - controlBarSize / 2, y1 - controlBarSize / 2, controlBarSize, controlBarSize);
		controlRectangles[1] =  QRectF(x2 - controlBarSize / 2, y2 - controlBarSize/ 2, controlBarSize, controlBarSize);

		if (controlRectangles[0].contains(point) == true)
		{
			return SchemaItemAction::MoveStartLinePoint;
		}

		if (controlRectangles[1].contains(point) == true)
		{
			return SchemaItemAction::MoveEndLinePoint;
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
				x1 -= controlBarSize / 4;
				x2 += controlBarSize / 4;

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
				y1 -= controlBarSize / 4;
				y2 += controlBarSize / 4;

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


QUuid EditSchemaView::activeLayerGuid() const
{
	if (m_activeLayer >= static_cast<int>(schema()->Layers.size()))
	{
		assert(m_activeLayer < static_cast<int>(schema()->Layers.size()));
		return QUuid();
	}

	return schema()->Layers[m_activeLayer]->guid();
}

std::shared_ptr<VFrame30::SchemaLayer> EditSchemaView::activeLayer()
{
	if (m_activeLayer >= static_cast<int>(schema()->Layers.size()))
	{
		assert(m_activeLayer < static_cast<int>(schema()->Layers.size()));
		return std::make_shared<VFrame30::SchemaLayer>("Error", false);
	}

	return schema()->Layers[m_activeLayer];
}

void EditSchemaView::setActiveLayer(std::shared_ptr<VFrame30::SchemaLayer> layer)
{
	for (int i = 0; i < static_cast<int>(schema()->Layers.size()); i++)
	{
		if (schema()->Layers[i] == layer)
		{
			m_activeLayer = i;
			return;
		}
	}

	// Layer was not found
	//
	assert(false);
	return;
}

MouseState EditSchemaView::mouseState() const
{
	return m_mouseState;
}

void EditSchemaView::setMouseState(MouseState state)
{
	m_mouseState = state;
}

const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& EditSchemaView::selectedItems() const
{
	return m_selectedItems;
}

void EditSchemaView::setSelectedItems(const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items)
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
	m_selectedItems = items;

	emit selectionChanged();
}

void EditSchemaView::setSelectedItems(const std::list<std::shared_ptr<VFrame30::SchemaItem>>& items)
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
}

void EditSchemaView::setSelectedItem(const std::shared_ptr<VFrame30::SchemaItem>& item)
{
	if (m_selectedItems.size() == 1 && item == m_selectedItems.back())
	{
		return;
	}

	m_selectedItems.clear();
	m_selectedItems.push_back(item);

	emit selectionChanged();
}

void EditSchemaView::addSelection(const std::shared_ptr<VFrame30::SchemaItem>& item, bool emitSectionChanged)
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
}

bool EditSchemaView::removeFromSelection(const std::shared_ptr<VFrame30::SchemaItem>& item, bool emitSectionChanged)
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

bool EditSchemaView::isItemSelected(const std::shared_ptr<VFrame30::SchemaItem>& item)
{
	auto findResult = std::find(m_selectedItems.begin(), m_selectedItems.end(), item);
	return findResult != m_selectedItems.end();
}


//
//
// EditSchemaWidget
//
//


EditSchemaWidget::~EditSchemaWidget()
{
}

void EditSchemaWidget::createActions()
{
	// Escape Button Pressed
	//
	m_escapeAction = new QAction(tr("Escape"), this);
	m_escapeAction->setEnabled(true);
	m_escapeAction->setMenuRole(QAction::NoRole);
	m_escapeAction->setShortcut(QKeySequence(Qt::Key_Escape));
	connect(m_escapeAction, &QAction::triggered, this, &EditSchemaWidget::escapeKey);
	addAction(m_escapeAction);

	// F2 Button Pressed
	//
	m_f2Action = new QAction(tr("Edit AppSignalID(s)"), this);
	m_f2Action->setEnabled(true);
	m_f2Action->setMenuRole(QAction::NoRole);
	m_f2Action->setShortcut(QKeySequence(Qt::Key_F2));
	connect(m_f2Action, &QAction::triggered, this, &EditSchemaWidget::f2Key);
	addAction(m_f2Action);

	// Info Mode Action
	//
	m_infoModeAction = new QAction(tr("Info Mode"), this);
	m_infoModeAction->setEnabled(true);
	m_infoModeAction->setCheckable(true);
	m_infoModeAction->setChecked(theSettings.isInfoMode());
	m_infoModeAction->setMenuRole(QAction::NoRole);
	m_infoModeAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));
	connect(m_infoModeAction, &QAction::toggled, this, [this](bool checked)
		{
			theSettings.setInfoMode(checked);
			editSchemaView()->update();
		});
	addAction(m_infoModeAction);

	//
	// File
	//
	m_fileAction = new QAction(tr("File"), this);
	m_fileAction->setEnabled(true);

	m_fileCheckInAction = new QAction(tr("Check In"), this);
	m_fileCheckInAction->setStatusTip(tr("Check In changes..."));
	m_fileCheckInAction->setEnabled(false);
	connect(m_fileCheckInAction, &QAction::triggered, this, &EditSchemaWidget::checkInFile);

	m_fileCheckOutAction = new QAction(tr("Check Out"), this);
	m_fileCheckOutAction->setStatusTip(tr("Check Out for edit..."));
	m_fileCheckOutAction->setEnabled(false);
	connect(m_fileCheckOutAction, &QAction::triggered, this, &EditSchemaWidget::checkOutFile);

	m_fileUndoChangesAction = new QAction(tr("Undo Changes..."), this);
	m_fileUndoChangesAction->setStatusTip(tr("Undo Pending Changes..."));
	m_fileUndoChangesAction->setEnabled(false);
	connect(m_fileUndoChangesAction, &QAction::triggered, this, &EditSchemaWidget::undoChangesFile);

	m_fileSeparatorAction0 = new QAction(this);
	m_fileSeparatorAction0->setSeparator(true);

	m_fileSaveAction = new QAction(tr("Save"), this);
	m_fileSaveAction->setStatusTip(tr("Save current changes..."));
	m_fileSaveAction->setEnabled(false);
	m_fileSaveAction->setShortcuts(QKeySequence::Save);
	connect(m_fileSaveAction, &QAction::triggered, this, &EditSchemaWidget::saveWorkcopy);
	addAction(m_fileSaveAction);

	m_fileExportToPdfAction = new QAction(tr("Export to PDF"), this);
	m_fileExportToPdfAction->setStatusTip(tr("Export schema to PDF..."));
	m_fileExportToPdfAction->setEnabled(true);
	connect(m_fileExportToPdfAction, &QAction::triggered, this, &EditSchemaWidget::exportToPdf);
	addAction(m_fileExportToPdfAction);

	m_fileSeparatorAction1 = new QAction(this);
	m_fileSeparatorAction1->setSeparator(true);

	m_fileExportAction = new QAction(tr("Export File..."), this);
	m_fileExportAction->setStatusTip(tr("Export file to disk"));
	m_fileExportAction->setEnabled(true);
	connect(m_fileExportAction, &QAction::triggered, this, &EditSchemaWidget::getCurrentWorkcopy);

	m_fileImportAction = new QAction(tr("Import file..."), this);
	m_fileImportAction->setStatusTip(tr("Import file from disk to project"));
	m_fileImportAction->setEnabled(false);
	connect(m_fileImportAction, &QAction::triggered, this, &EditSchemaWidget::setCurrentWorkcopy);

	m_fileSeparatorAction2 = new QAction(this);
	m_fileSeparatorAction2->setSeparator(true);

	m_filePropertiesAction = new QAction(tr("Properties..."), this);
	m_filePropertiesAction->setStatusTip(tr("Edit schema properties"));
	m_filePropertiesAction->setEnabled(true);
	connect(m_filePropertiesAction, &QAction::triggered, this, &EditSchemaWidget::schemaProperties);

	m_fileSeparatorAction3 = new QAction(this);
	m_fileSeparatorAction3->setSeparator(true);

	m_fileCloseAction = new QAction(tr("Close"), this);
	m_fileCloseAction->setStatusTip(tr("Close file"));
	m_fileCloseAction->setEnabled(true);
	m_fileCloseAction->setShortcuts(QKeySequence::Close);
	connect(m_fileCloseAction, &QAction::triggered, [this](bool) { emit closeTab(this); });
	addAction(m_fileCloseAction);

	//
	// Add Item
	//
	m_addAction = new QAction(tr("Add Item"), this);
	m_addAction->setEnabled(true);

	m_addLineAction = new QAction(tr("Line"), this);
	m_addLineAction->setEnabled(true);
	m_addLineAction->setIcon(QIcon(":/Images/Images/SchemaLine.svg"));
	connect(m_addLineAction, &QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::SchemaItemLine>(schema()->unit()));
			});

	m_addPathAction = new QAction(tr("Path"), this);
	m_addPathAction->setEnabled(true);
	m_addPathAction->setIcon(QIcon(":/Images/Images/SchemaPath.svg"));
	connect(m_addPathAction, &QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::SchemaItemPath>(schema()->unit()));
			});

	m_addRectAction = new QAction(tr("Rect"), this);
	m_addRectAction->setEnabled(true);
	m_addRectAction->setIcon(QIcon(":/Images/Images/SchemaRect.svg"));
	connect(m_addRectAction, &QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::SchemaItemRect>(schema()->unit()));
			});

	m_addTextAction = new QAction(tr("Text"), this);
	m_addTextAction->setEnabled(true);
	m_addTextAction->setIcon(QIcon(":/Images/Images/SchemaText.svg"));
	connect(m_addTextAction, &QAction::triggered,
			[this](bool)
			{
				auto text = std::make_shared<VFrame30::SchemaItemRect>(schema()->unit());
				text->setText(QLatin1String("Text"));
				text->setFill(false);
				text->setDrawRect(false);
				addItem(text);
			});

	m_addSeparatorAction0 = new QAction(this);
	m_addSeparatorAction0->setSeparator(true);

	m_addInputSignalAction = new QAction(tr("Input"), this);
	m_addInputSignalAction->setEnabled(true);
	m_addInputSignalAction->setIcon(QIcon(":/Images/Images/SchemaInputSignal.svg"));
	connect(m_addInputSignalAction, &QAction::triggered,
			[this](bool)
			{
				auto item = std::make_shared<VFrame30::SchemaItemInput>(schema()->unit());
				addItem(item);
			});

	m_addOutputSignalAction = new QAction(tr("Output"), this);
	m_addOutputSignalAction->setEnabled(true);
	m_addOutputSignalAction->setIcon(QIcon(":/Images/Images/SchemaOutputSignal.svg"));
	connect(m_addOutputSignalAction, &QAction::triggered,
			[this](bool)
			{
				auto item = std::make_shared<VFrame30::SchemaItemOutput>(schema()->unit());
				addItem(item);
			});

	m_addInOutSignalAction = new QAction(tr("In/Out"), this);
	m_addInOutSignalAction->setEnabled(true);
	m_addInOutSignalAction->setIcon(QIcon(":/Images/Images/SchemaInOutSignal.svg"));
	connect(m_addInOutSignalAction, &QAction::triggered,
			[this](bool)
			{
				auto item = std::make_shared<VFrame30::SchemaItemInOut>(schema()->unit());
				addItem(item);
			});

	m_addConstantAction = new QAction(tr("Constant"), this);
	m_addConstantAction->setEnabled(true);
	m_addConstantAction->setIcon(QIcon(":/Images/Images/SchemaConstant.svg"));
	connect(m_addConstantAction, &QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::SchemaItemConst>(schema()->unit()));
			});

	m_addTerminatorAction = new QAction(tr("Terminator"), this);
	m_addTerminatorAction->setEnabled(true);
	m_addTerminatorAction->setIcon(QIcon(":/Images/Images/SchemaTerminator.svg"));
	connect(m_addTerminatorAction, &QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::SchemaItemTerminator>(schema()->unit()));
			});

	m_addFblElementAction = new QAction(tr("App Functional Block"), this);
	m_addFblElementAction->setEnabled(true);
	m_addFblElementAction->setIcon(QIcon(":/Images/Images/SchemaFblElement.svg"));
	connect(m_addFblElementAction, &QAction::triggered, this, &EditSchemaWidget::addFblElement);

	m_addLinkAction = new QAction(tr("Link"), this);
	m_addLinkAction->setEnabled(true);
	m_addLinkAction->setIcon(QIcon(":/Images/Images/SchemaLink.svg"));
	connect(m_addLinkAction, &QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::SchemaItemLink>(schema()->unit()));
			});

	m_addTransmitter = new QAction(tr("Transmitter"), this);
	m_addTransmitter->setEnabled(true);
	m_addTransmitter->setIcon(QIcon(":/Images/Images/SchemaTransmitter.svg"));
	connect(m_addTransmitter, &QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::SchemaItemTransmitter>(schema()->unit()));
			});

	m_addReceiver = new QAction(tr("Receiver"), this);
	m_addReceiver->setEnabled(true);
	m_addReceiver->setIcon(QIcon(":/Images/Images/SchemaReceiver.svg"));
	connect(m_addReceiver, &QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::SchemaItemReceiver>(schema()->unit()));
			});

	m_addUfbAction = new QAction(tr("User Functional Block"), this);
	m_addUfbAction->setEnabled(true);
	m_addUfbAction->setIcon(QIcon(":/Images/Images/SchemaUfbElement.svg"));
	connect(m_addUfbAction, &QAction::triggered, this, &EditSchemaWidget::addUfbElement);

	//
	// Edit
	//
	m_editAction = new QAction(tr("Edit"), this);
	m_editAction->setEnabled(true);

	// Edit->Undo
	//
	m_undoAction = new QAction(tr("Undo"), this);
	m_undoAction->setEnabled(false);
	m_undoAction->setShortcuts(QKeySequence::Undo);
	connect(m_undoAction, &QAction::triggered, this, &EditSchemaWidget::undo);
	addAction(m_undoAction);

	// Edit->Redo
	//
	m_redoAction = new QAction(tr("Redo"), this);
	m_redoAction->setEnabled(false);
	m_redoAction->setShortcuts(QKeySequence::Redo);
	connect(m_redoAction, &QAction::triggered, this, &EditSchemaWidget::redo);
	addAction(m_redoAction);

	// ------------------------------------
	//
	m_editSeparatorAction0 = new QAction(this);
	m_editSeparatorAction0->setSeparator(true);

	// Edit->Select All
	//
	m_selectAllAction = new QAction(tr("Select All"), this);
	m_selectAllAction->setEnabled(true);
	m_selectAllAction->setShortcuts(QKeySequence::SelectAll);
	connect(m_selectAllAction, &QAction::triggered, this, &EditSchemaWidget::selectAll);
	addAction(m_selectAllAction);

	// ------------------------------------
	//
	m_editSeparatorAction1 = new QAction(this);
	m_editSeparatorAction1->setSeparator(true);

	// Edit->Cut
	//
	m_editCutAction = new QAction(tr("Cut"), this);
	m_editCutAction->setEnabled(false);
	m_editCutAction->setShortcuts(QKeySequence::Cut);
	connect(m_editCutAction, &QAction::triggered, this, &EditSchemaWidget::editCut);
	addAction(m_editCutAction);

	// Edit->Copy
	//
	m_editCopyAction = new QAction(tr("Copy"), this);
	m_editCopyAction->setEnabled(false);
	m_editCopyAction->setShortcuts(QKeySequence::Copy);
	connect(m_editCopyAction, &QAction::triggered, this, &EditSchemaWidget::editCopy);
	addAction(m_editCopyAction);

	// Edit->Paste
	//
	m_editPasteAction = new QAction(tr("Paste"), this);
	m_editPasteAction->setEnabled(false);
	m_editPasteAction->setShortcuts(QKeySequence::Paste);
	connect(m_editPasteAction, &QAction::triggered, this, &EditSchemaWidget::editPaste);
	addAction(m_editPasteAction);

	// ------------------------------------
	//
	m_editSeparatorAction2 = new QAction(this);
	m_editSeparatorAction2->setSeparator(true);

	// Edit->Delete
	//
	m_deleteAction = new QAction(tr("Delete"), this);
	m_deleteAction->setEnabled(false);
	m_deleteAction->setMenuRole(QAction::NoRole);
	m_deleteAction->setShortcut(QKeySequence(Qt::Key_Delete));
	connect(m_deleteAction, &QAction::triggered, this, &EditSchemaWidget::deleteKey);
	addAction(m_deleteAction);

	// ------------------------------------
	//
	m_editSeparatorAction3 = new QAction(this);
	m_editSeparatorAction3->setSeparator(true);

	// Edit->Properties
	//
	m_propertiesAction = new QAction(tr("Properties..."), this);
	m_propertiesAction->setEnabled(true);
	m_propertiesAction->setMenuRole(QAction::NoRole);
	m_propertiesAction->setShortcut(QKeySequence(tr("Alt+Return")));
	// Shortcuts Alt+Return and Alt+Numeric Enter are different,
	// Look for real call of EditSchemaWidget::properties in keyPressEvent!!!!
	//
	connect(m_propertiesAction, &QAction::triggered, this, &EditSchemaWidget::properties);
	addAction(m_propertiesAction);

	//
	// Size And Pos (align)
	//
	m_sizeAndPosAction = new QAction(tr("Size/Pos"), this);
	m_sizeAndPosAction->setEnabled(true);

	// Size/Pos->Same Width
	//
	m_sameWidthAction = new QAction(tr("Same Width"), this);
	m_sameWidthAction->setEnabled(false);
	connect(m_sameWidthAction, &QAction::triggered, this, &EditSchemaWidget::sameWidth);
	addAction(m_sameWidthAction);

	// Size/Pos->Same Height
	//
	m_sameHeightAction = new QAction(tr("Same Height"), this);
	m_sameHeightAction->setEnabled(false);
	connect(m_sameHeightAction, &QAction::triggered, this, &EditSchemaWidget::sameHeight);
	addAction(m_sameHeightAction);

	// Size/Pos->Same Size
	//
	m_sameSizeAction = new QAction(tr("Same Size"), this);
	m_sameSizeAction->setEnabled(false);
	connect(m_sameSizeAction, &QAction::triggered, this, &EditSchemaWidget::sameSize);
	addAction(m_sameSizeAction);

	// ------------------------------------
	//
	m_sizeAndPosSeparatorAction0 = new QAction(this);
	m_sizeAndPosSeparatorAction0->setSeparator(true);

	// Size/Pos->Align Left
	//
	m_alignLeftAction = new QAction(tr("Align Left"), this);
	m_alignLeftAction->setEnabled(false);
	connect(m_alignLeftAction, &QAction::triggered, this, &EditSchemaWidget::alignLeft);
	addAction(m_alignLeftAction);

	// Size/Pos->Align Right
	//
	m_alignRightAction = new QAction(tr("Align Right"), this);
	m_alignRightAction->setEnabled(false);
	connect(m_alignRightAction, &QAction::triggered, this, &EditSchemaWidget::alignRight);
	addAction(m_alignRightAction);

	// Size/Pos->Align Top
	//
	m_alignTopAction = new QAction(tr("Align Top"), this);
	m_alignTopAction->setEnabled(false);
	connect(m_alignTopAction, &QAction::triggered, this, &EditSchemaWidget::alignTop);
	addAction(m_alignTopAction);

	// Size/Pos->Align Bottom
	//
	m_alignBottomAction = new QAction(tr("Align Bottom"), this);
	m_alignBottomAction->setEnabled(false);
	connect(m_alignBottomAction, &QAction::triggered, this, &EditSchemaWidget::alignBottom);
	addAction(m_alignBottomAction);

	//
	// Items Order
	//
	m_orderAction = new QAction(tr("Order"), this);
	m_orderAction->setEnabled(true);

	// Items Order->Bring to Front
	//
	m_bringToFrontAction = new QAction(tr("Bring to Front"), this);
	m_bringToFrontAction->setEnabled(false);
	m_bringToFrontAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Home));
	connect(m_bringToFrontAction, &QAction::triggered, this, &EditSchemaWidget::bringToFront);
	addAction(m_bringToFrontAction);

	// Items Order->Bring Forward
	//
	m_bringForwardAction = new QAction(tr("Bring Forward"), this);
	m_bringForwardAction->setEnabled(false);
	m_bringForwardAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_PageUp));
	connect(m_bringForwardAction, &QAction::triggered, this, &EditSchemaWidget::bringForward);
	addAction(m_bringForwardAction);

	// Items Order->Send to Back
	//
	m_sendToBackAction = new QAction(tr("Send to Back"), this);
	m_sendToBackAction->setEnabled(false);
	m_sendToBackAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_End));
	connect(m_sendToBackAction, &QAction::triggered, this, &EditSchemaWidget::sendToBack);
	addAction(m_sendToBackAction);

	// Items Order->Send Backward
	//
	m_sendBackwardAction = new QAction(tr("Send Backward"), this);
	m_sendBackwardAction->setEnabled(false);
	m_sendBackwardAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_PageDown));
	connect(m_sendBackwardAction, &QAction::triggered, this, &EditSchemaWidget::sendBackward);
	addAction(m_sendBackwardAction);

	//
	// View
	//
	m_viewAction = new QAction(tr("View"), this);
	m_viewAction->setEnabled(true);

	// View->ZoomIn, creating of these actions was moved to VFrame30::BaseSchemaWidget
	//
	m_zoomInAction = new QAction(tr("Zoom In"), this);
	m_zoomInAction->setEnabled(true);
	m_zoomInAction->setShortcut(QKeySequence::ZoomIn);
	connect(m_zoomInAction, &QAction::triggered, this, &VFrame30::BaseSchemaWidget::zoomIn);
	addAction(m_zoomInAction);

	// View->ZoomOut
	//
	m_zoomOutAction = new QAction(tr("Zoom Out"), this);
	m_zoomOutAction->setEnabled(true);
	m_zoomOutAction->setShortcut(QKeySequence::ZoomOut);
	connect(m_zoomOutAction, &QAction::triggered, this, &VFrame30::BaseSchemaWidget::zoomOut);
	addAction(m_zoomOutAction);

	// View->Zoom100
	//
	m_zoom100Action = new QAction(tr("Zoom 100%"), this);
	m_zoom100Action->setEnabled(true);
	m_zoom100Action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Asterisk));
	connect(m_zoom100Action, &QAction::triggered, this, &VFrame30::BaseSchemaWidget::zoom100);
	addAction(m_zoom100Action);

	// ------------------------------------
	//
	m_viewSeparatorAction0 = new QAction(this);
	m_viewSeparatorAction0->setSeparator(true);

	// View->SnapToGrid
	//
	m_snapToGridAction = new QAction(tr("Snap To Grid"), this);
	m_snapToGridAction->setEnabled(true);
	//connect(m_snapToGridAction, &QAction::triggered, this, &EditSchemaWidget::zoom100);


	// High Level Menu
	//
	m_separatorAction0 = new QAction(this);
	m_separatorAction0->setSeparator(true);


	// Edit->Properties
	//
	m_layersAction = new QAction(tr("Layers..."), this);
	m_layersAction->setEnabled(true);
	//m_layersAction->setMenuRole(QAction::NoRole);
	//m_layersAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Enter));
	connect(m_layersAction, &QAction::triggered, this, &EditSchemaWidget::layers);
	addAction(m_layersAction);

	m_compareDiffAction = new QAction(tr("Item Diffs..."), this);
	m_compareDiffAction->setEnabled(true);
	connect(m_compareDiffAction, &QAction::triggered, this, &EditSchemaWidget::compareSchemaItem);
	//addAction(m_compareDiffAction);

	// Comment
	//
	m_toggleCommentAction = new QAction(tr("Comment/Uncomment"), this);
	m_toggleCommentAction->setEnabled(false);
	m_toggleCommentAction->setShortcut(Qt::CTRL + Qt::Key_Slash);
	connect(m_toggleCommentAction, &QAction::triggered, this, &EditSchemaWidget::toggleComment);
	addAction(m_toggleCommentAction);

	// Lock/Unlock
	//
	m_lockAction = new QAction(tr("Lock/Unlock"), this);
	m_lockAction->setEnabled(false);
	m_lockAction->setShortcut(Qt::CTRL + Qt::Key_L);
	connect(m_lockAction, &QAction::triggered, this, &EditSchemaWidget::toggleLock);
	addAction(m_lockAction);

	// Find
	//
	m_findAction = new QAction(tr("Find..."), this);
	m_findAction->setEnabled(true);
	m_findAction->setShortcut(QKeySequence::Find);
	connect(m_findAction, &QAction::triggered, this, &EditSchemaWidget::find);
	addAction(m_findAction);

	m_findNextAction = new QAction(tr("Find Next"), this);
	m_findNextAction->setEnabled(true);
	m_findNextAction->setShortcut(QKeySequence::FindNext);
	connect(m_findNextAction, &QAction::triggered, this, &EditSchemaWidget::findNext);
	addAction(m_findNextAction);

	m_findPrevAction = new QAction(tr("Find Previous"), this);
	m_findPrevAction->setEnabled(true);
	m_findPrevAction->setShortcut(QKeySequence::FindPrevious);
	connect(m_findPrevAction, &QAction::triggered, this, &EditSchemaWidget::findPrev);
	addAction(m_findPrevAction);

	//
	// Create Sub Menus
	//
	m_fileMenu = new QMenu(this);
	m_fileAction->setMenu(m_fileMenu);
		m_fileMenu->addAction(m_fileCheckOutAction);
		m_fileMenu->addAction(m_fileCheckInAction);
		m_fileMenu->addAction(m_fileUndoChangesAction);
		m_fileMenu->addAction(m_fileSeparatorAction0);
		m_fileMenu->addAction(m_fileSaveAction);
		m_fileMenu->addAction(m_fileExportToPdfAction);
		m_fileMenu->addAction(m_fileSeparatorAction1);
		m_fileMenu->addAction(m_fileExportAction);
		m_fileMenu->addAction(m_fileImportAction);
		m_fileMenu->addAction(m_fileSeparatorAction2);
		m_fileMenu->addAction(m_filePropertiesAction);
		m_fileMenu->addAction(m_fileSeparatorAction3);
		m_fileMenu->addAction(m_fileCloseAction);

	m_addMenu = new QMenu(this);
	m_addAction->setMenu(m_addMenu);
		m_addMenu->addAction(m_addLineAction);
		m_addMenu->addAction(m_addRectAction);
		m_addMenu->addAction(m_addPathAction);
		m_addMenu->addAction(m_addTextAction);

		m_addMenu->addAction(m_addSeparatorAction0);

		if (isLogicSchema() == true)
		{
			m_addMenu->addAction(m_addLinkAction);
			m_addMenu->addAction(m_addInputSignalAction);
			m_addMenu->addAction(m_addOutputSignalAction);
			m_addMenu->addAction(m_addInOutSignalAction);
			m_addMenu->addAction(m_addConstantAction);
			m_addMenu->addAction(m_addTerminatorAction);
			m_addMenu->addAction(m_addFblElementAction);
			m_addMenu->addAction(m_addTransmitter);
			m_addMenu->addAction(m_addReceiver);
			m_addMenu->addAction(m_addUfbAction);
		}

		if (isUfbSchema() == true)
		{
			m_addMenu->addAction(m_addLinkAction);
			m_addMenu->addAction(m_addInputSignalAction);
			m_addMenu->addAction(m_addOutputSignalAction);
			m_addMenu->addAction(m_addConstantAction);
			m_addMenu->addAction(m_addTerminatorAction);
			m_addMenu->addAction(m_addFblElementAction);
		}

	m_editMenu = new QMenu(this);
	m_editAction->setMenu(m_editMenu);
		m_editMenu->addAction(m_undoAction);
		m_editMenu->addAction(m_redoAction);
		m_editMenu->addAction(m_editSeparatorAction0);
		m_editMenu->addAction(m_selectAllAction);
		m_editMenu->addAction(m_editSeparatorAction1);
		m_editMenu->addAction(m_editCutAction);
		m_editMenu->addAction(m_editCopyAction);
		m_editMenu->addAction(m_editPasteAction);
		m_editMenu->addAction(m_editSeparatorAction2);
		m_editMenu->addAction(m_deleteAction);

	m_sizeAndPosMenu = new QMenu(this);
	m_sizeAndPosAction->setMenu(m_sizeAndPosMenu);
		m_sizeAndPosMenu->addAction(m_sameWidthAction);
		m_sizeAndPosMenu->addAction(m_sameHeightAction);
		m_sizeAndPosMenu->addAction(m_sameSizeAction);
		m_sizeAndPosMenu->addAction(m_sizeAndPosSeparatorAction0);
		m_sizeAndPosMenu->addAction(m_alignLeftAction);
		m_sizeAndPosMenu->addAction(m_alignRightAction);
		m_sizeAndPosMenu->addAction(m_alignTopAction);
		m_sizeAndPosMenu->addAction(m_alignBottomAction);

	m_orderMenu = new QMenu(this);
	m_orderAction->setMenu(m_orderMenu);
		m_orderMenu->addAction(m_bringToFrontAction);
		m_orderMenu->addAction(m_bringForwardAction);
		m_orderMenu->addAction(m_sendBackwardAction);
		m_orderMenu->addAction(m_sendToBackAction);

	m_viewMenu = new QMenu(this);
	m_viewAction->setMenu(m_viewMenu);
		m_viewMenu->addAction(m_zoomInAction);
		m_viewMenu->addAction(m_zoomOutAction);
		m_viewMenu->addAction(m_zoom100Action);
		m_viewMenu->addAction(m_viewSeparatorAction0);
		m_viewMenu->addAction(m_snapToGridAction);

	return;
}

void EditSchemaWidget::keyPressEvent(QKeyEvent* e)
{
	switch (e->key())
	{
		case Qt::Key_Left:
			onLeftKey();
			return;
		case Qt::Key_Right:
			onRightKey();
			return;
		case Qt::Key_Up:
			onUpKey();
			return;
		case Qt::Key_Down:
			onDownKey();
			return;
	}

	BaseSchemaWidget::keyPressEvent(e);

	// Show properties dialog
	//
	if ((e->modifiers().testFlag(Qt::AltModifier) == true &&		// Alt + numeric keypad Enter
		e->modifiers().testFlag(Qt::KeypadModifier) == true &&
		e->key() == Qt::Key_Enter) ||
		(e->modifiers().testFlag(Qt::AltModifier) == true &&		// Alt + Enter
		e->key() == Qt::Key_Return))
	{
		properties();
	}

	return;
}

// Set corresponding to the current situation and user actions context menu
//
void EditSchemaWidget::setCorrespondingContextMenu()
{
	setContextMenuPolicy(Qt::CustomContextMenu);
	return;
}

void EditSchemaWidget::mousePressEvent(QMouseEvent* event)
{
	BaseSchemaWidget::mousePressEvent(event);

	if (event->isAccepted() == true)
	{
		return;
	}

	if (event->button() == Qt::LeftButton)
	{
		for (auto msa = m_mouseLeftDownStateAction.begin(); msa != m_mouseLeftDownStateAction.end(); ++msa)
		{
			if (msa->mouseState == mouseState())
			{
				msa->action(event);
				setMouseCursor(event->pos());

				event->accept();
				return;
			}
		}

		// DefaultAction
		//
		setMouseCursor(event->pos());

		event->accept();
		return;
	}

	if (event->button() == Qt::RightButton)
	{
		for (auto msa = m_mouseRightDownStateAction.begin(); msa != m_mouseRightDownStateAction.end(); ++msa)
		{
			if (msa->mouseState == mouseState())
			{
				msa->action(event);
				setMouseCursor(event->pos());

				event->accept();
				return;
			}
		}

		if (mouseState() != MouseState::None)
		{
			event->accept();
			resetAction();
		}
		else
		{
			event->ignore();
		}

		return;
	}

	event->ignore();
	return;
}

void EditSchemaWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		for (auto msa = m_mouseLeftUpStateAction.begin(); msa != m_mouseLeftUpStateAction.end(); ++msa)
		{
			if (msa->mouseState == mouseState())
			{
				msa->action(event);
				setMouseCursor(event->pos());

				event->accept();
				return;
			}
		}

		// DefaultAction
		//
		setMouseCursor(event->pos());

		event->accept();
		return;
	}

	if (event->button() == Qt::RightButton)
	{
		for (auto msa = m_mouseRightUpStateAction.begin(); msa != m_mouseRightUpStateAction.end(); ++msa)
		{
			if (msa->mouseState == mouseState())
			{
				msa->action(event);
				setMouseCursor(event->pos());

				event->accept();
				return;
			}
		}

		// DefaultAction
		//
		//setMouseCursor(event->pos());

		//event->accept();
		//return;
	}


	setMouseCursor(event->pos());

	//unsetCursor();
	event->ignore();
}

void EditSchemaWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
	setMouseState(MouseState::None);

	if (selectedItems().empty() == false)
	{
		properties();
	}

	event->accept();
	return;
}

void EditSchemaWidget::mouseMoveEvent(QMouseEvent* event)
{
	BaseSchemaWidget::mouseMoveEvent(event);
	if (event->isAccepted() == true)
	{
		return;
	}

	for (const MouseStateAction& msa : m_mouseMoveStateAction)
	{
		if (msa.mouseState == mouseState())
		{
			msa.action(event);

			setMouseCursor(event->pos());
			event->accept();
			return;
		}
	}

	// Default Action
	//
	setMouseCursor(event->pos());

	event->ignore();
	return;
}

void EditSchemaWidget::mouseLeftDown_None(QMouseEvent* me)
{
	bool ctrlIsPressed = me->modifiers().testFlag(Qt::ControlModifier);
	bool shiftIsPressed = me->modifiers().testFlag(Qt::ShiftModifier);

	if (shiftIsPressed == false)
	{
		QPointF docPoint = widgetPointToDocument(me->pos(), false);

		// ���� ������� ���� ������, � ���� �� ��������� �������� ����� ������� ��� ��������� �����, ������ � �.�.
		//
		if (selectedItems().size() == 1)
		{
			// ��������� ����������� �� ����� �� ����� �������� ����� ��������� �������� ������ �������
			//
			int movingEdgePointIndex = 0;
			auto selectedItem = selectedItems()[0];

			SchemaItemAction possibleAction = editSchemaView()->getPossibleAction(selectedItem.get(), docPoint, &movingEdgePointIndex);

			if (dynamic_cast<VFrame30::IPosRect*>(selectedItem.get()) != nullptr)
			{
				auto findResult = std::find_if(std::begin(m_sizeActionToMouseCursor), std::end(m_sizeActionToMouseCursor),
					[&possibleAction](const SizeActionToMouseCursor& item)
					{
						return item.action == possibleAction;
					}
					);

				if (findResult != std::end(m_sizeActionToMouseCursor))
				{
					// ������ �������� ����� Xin � Yin ����������� � �����, ��������� ������ ���� ��� ����������� ������� �������� ��� �����
					//
					docPoint = widgetPointToDocument(me->pos(), snapToGrid());

					editSchemaView()->m_editStartDocPt = docPoint;
					editSchemaView()->m_editEndDocPt = docPoint;

					setMouseState(findResult->mouseState);

					setMouseCursor(me->pos());
					editSchemaView()->update();
					return;
				}
			}

			// ��������� �� ��������� ������ �����
			//
			if (dynamic_cast<VFrame30::IPosLine*>(selectedItem.get()) != nullptr)
			{
				if (possibleAction == SchemaItemAction::MoveStartLinePoint)
				{
					// �������� ����� Xin � Yin ����������� � �����, ��������� ������ ���� ��� ����������� ������� �������� ��� �����
					//
					docPoint = widgetPointToDocument(me->pos(), snapToGrid());

					editSchemaView()->m_editStartDocPt = docPoint;
					editSchemaView()->m_editEndDocPt = docPoint;

					setMouseState(MouseState::MovingStartLinePoint);

					setMouseCursor(me->pos());
					editSchemaView()->update();

					return;
				}

				if (possibleAction == SchemaItemAction::MoveEndLinePoint)
				{
					// �������� ����� Xin � Yin ����������� � �����, ��������� ������ ���� ��� ����������� ������� �������� ��� �����
					//
					docPoint = widgetPointToDocument(me->pos(), snapToGrid());

					editSchemaView()->m_editStartDocPt = docPoint;
					editSchemaView()->m_editEndDocPt = docPoint;

					setMouseState(MouseState::MovingEndLinePoint);

					setMouseCursor(me->pos());
					editSchemaView()->update();

					return;
				}
			}

			// ��������� �� ��������� ������ � ����� ����������
			//
			if (dynamic_cast<VFrame30::IPosConnection*>(selectedItem.get()) != nullptr)
			{
				if (possibleAction == SchemaItemAction::MoveHorizontalEdge)
				{
					assert(movingEdgePointIndex != -1);

					// �������� ����� Xin � Yin ����������� � �����, ��������� ������ ���� ��� ����������� ������� �������� ��� �����
					//
					docPoint = widgetPointToDocument(me->pos(), snapToGrid());

					editSchemaView()->m_editStartMovingEdge = docPoint.y();
					editSchemaView()->m_editEndMovingEdge = docPoint.y();

					editSchemaView()->m_movingEdgePointIndex = movingEdgePointIndex;

					setMouseState(MouseState::MovingHorizontalEdge);

					setMouseCursor(me->pos());
					editSchemaView()->update();

					return;
				}

				if (possibleAction == SchemaItemAction::MoveVerticalEdge)
				{
					assert(movingEdgePointIndex != -1);

					// �������� ����� Xin � Yin ����������� � �����, ��������� ������ ���� ��� ����������� ������� �������� ��� �����
					//
					docPoint = widgetPointToDocument(me->pos(), snapToGrid());

					editSchemaView()->m_editStartMovingEdge = docPoint.x();
					editSchemaView()->m_editEndMovingEdge = docPoint.x();

					editSchemaView()->m_movingEdgePointIndex = movingEdgePointIndex;

					setMouseState(MouseState::MovingVerticalEdge);

					setMouseCursor(me->pos());
					editSchemaView()->update();

					return;
				}

				if (possibleAction == SchemaItemAction::MoveConnectionLinePoint)
				{
					assert(movingEdgePointIndex != -1);

					//if (movingEdgePointIndex == )

					// �������� ����� Xin � Yin ����������� � �����, ��������� ������ ���� ��� ����������� ������� �������� ��� �����
					//
					docPoint = widgetPointToDocument(me->pos(), snapToGrid());

					editSchemaView()->m_editStartMovingEdgeX = docPoint.x();
					editSchemaView()->m_editStartMovingEdgeY = docPoint.y();

					editSchemaView()->m_editEndMovingEdgeX = docPoint.x();
					editSchemaView()->m_editEndMovingEdgeY = docPoint.y();

					editSchemaView()->m_movingEdgePointIndex = movingEdgePointIndex;

					setMouseState(MouseState::MovingConnectionLinePoint);

					setMouseCursor(me->pos());
					editSchemaView()->update();

					return;
				}
			}
		}

		// ��������� ���������� ��������, �� ����������� ���������� ������� �� �����������
		//
		for (auto si = selectedItems().begin(); si != selectedItems().end(); ++si)
		{
			int movingEdgePointIndex = 0;
			SchemaItemAction possibleAction = editSchemaView()->getPossibleAction(si->get(), docPoint, &movingEdgePointIndex);

			if (possibleAction == SchemaItemAction::MoveItem)
			{
				// ������ �������� ����� Xin � Yin ����������� � �����, ��������� ������ ���� ��� ����������� ������� �������� ��� �����
				//
				docPoint = widgetPointToDocument(me->pos(), snapToGrid());

				editSchemaView()->m_editStartDocPt = docPoint;
				editSchemaView()->m_editEndDocPt = docPoint;

				setMouseState(MouseState::Moving);

				setMouseCursor(me->pos());
				editSchemaView()->update();
				return;
			}
		}

		// If mouse is over pin, star drawing SchmeItemLink
		//
		{
			std::vector<VFrame30::AfbPin> itemPins;
			itemPins.reserve(64);

			double gridSize = schema()->gridSize();
			double pinGridStep = static_cast<double>(schema()->pinGridStep());

			for (std::shared_ptr<VFrame30::SchemaItem> item : activeLayer()->Items)
			{
				VFrame30::FblItemRect* fbRect = dynamic_cast<VFrame30::FblItemRect*>(item.get());
				VFrame30::SchemaItemLink* link = dynamic_cast<VFrame30::SchemaItemLink*>(item.get());

				if (fbRect != nullptr &&
					std::find(selectedItems().begin(), selectedItems().end(), item) == selectedItems().end())	// Item is not selected, as in this case it can be resized or moved by control bars
				{
					const std::vector<VFrame30::AfbPin>& inputs = fbRect->inputs();
					const std::vector<VFrame30::AfbPin>& outputs = fbRect->outputs();

					itemPins.clear();
					itemPins.insert(itemPins.end(), inputs.begin(), inputs.end());
					itemPins.insert(itemPins.end(), outputs.begin(), outputs.end());

					for (const VFrame30::AfbPin& pin : itemPins)
					{
						QRectF pinArea = {pin.x() - gridSize * pinGridStep / 2, pin.y() - gridSize * pinGridStep / 2,
										 gridSize * pinGridStep, gridSize * pinGridStep};

						if (pinArea.contains(docPoint) == true)
						{
							addItem(std::make_shared<VFrame30::SchemaItemLink>(schema()->unit()));

							mouseLeftDown_AddSchemaPosConnectionStartPoint(me);

							return;
						}
					}

					continue;
				}

				if (link != nullptr)
				{
					continue;
				}
			}
		}

		// ���� ���� �� ����������� ������� ����� ���������� (���������� ��� ������ ������)
		// �� ������� � ����� ����������� �������
		//
		auto itemUnderPoint = editSchemaView()->activeLayer()->getItemUnderPoint(docPoint);

		if (itemUnderPoint != nullptr &&
			(itemUnderPoint->isLocked() == false || (itemUnderPoint->isLocked() == true && ctrlIsPressed == true)))
		{
			if (std::find(selectedItems().begin(), selectedItems().end(), itemUnderPoint) != selectedItems().end())
			{
				// ������� � ����� ����������� ���� ���������� ���������
				// SelectedItems ��� ��������
				//
			}
			else
			{
				// ������� � ����� ����������� ������ ��������
				//
				editSchemaView()->clearSelection();
				editSchemaView()->setSelectedItem(itemUnderPoint);
			}

			// �������� ����� Xin � Yin ����������� � �����, ��������� ������ ���� ��� ����������� ������� �������� ��� �����
			//
			docPoint = widgetPointToDocument(me->pos(), snapToGrid());

			editSchemaView()->m_editStartDocPt = docPoint;
			editSchemaView()->m_editEndDocPt = docPoint;

			setMouseState(MouseState::Moving);

			setMouseCursor(me->pos());
			editSchemaView()->update();
			return;
		}
	}

	// ���� ���� ������ ������ ��������� �� �� ���������� � ��������� � ����� ����������� ��������
	//
	if (me->modifiers().testFlag(Qt::ShiftModifier) == false)
	{
		editSchemaView()->clearSelection();
	}

	// Selection item or area
	//
//	editSchemaView()->m_rubberBand->show();
//	editSchemaView()->m_rubberBand->setGeometry(QRect(me->pos(), QSize()));

	editSchemaView()->m_mouseSelectionStartPoint = widgetPointToDocument(me->pos(), false);
	editSchemaView()->m_mouseSelectionEndPoint = editSchemaView()->m_mouseSelectionStartPoint;

	setMouseState(MouseState::Selection);

	editSchemaView()->update();

	setMouseCursor(me->pos());

	return;
}

void EditSchemaWidget::mouseLeftDown_AddSchemaPosLineStartPoint(QMouseEvent* event)
{
	if (editSchemaView()->m_newItem == nullptr)
	{
		assert(editSchemaView()->m_newItem != nullptr);
		resetAction();
		return;
	}

	VFrame30::IPosLine* itemPos = dynamic_cast<VFrame30::IPosLine*>(editSchemaView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		resetAction();
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	itemPos->setStartXDocPt(docPoint.x());
	itemPos->setStartYDocPt(docPoint.y());

	setMouseState(MouseState::AddSchemaPosLineEndPoint);

	return;
}

void EditSchemaWidget::mouseLeftDown_AddSchemaPosRectStartPoint(QMouseEvent* event)
{
	if (editSchemaView()->m_newItem == nullptr)
	{
		assert(editSchemaView()->m_newItem != nullptr);

		resetAction();
		return;
	}

	VFrame30::IPosRect* itemPos = dynamic_cast<VFrame30::IPosRect*>(editSchemaView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);

		resetAction();
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	itemPos->setLeftDocPt(docPoint.x());
	itemPos->setTopDocPt(docPoint.y());

	double minWidth = itemPos->minimumPossibleWidthDocPt(schema()->gridSize(), schema()->pinGridStep());
	double minHeight = itemPos->minimumPossibleHeightDocPt(schema()->gridSize(), schema()->pinGridStep());

	itemPos->setWidthDocPt(minWidth);
	itemPos->setHeightDocPt(minHeight);

	editSchemaView()->m_addRectStartPoint = docPoint;
	editSchemaView()->m_addRectEndPoint.setX(itemPos->leftDocPt() + itemPos->widthDocPt());
	editSchemaView()->m_addRectEndPoint.setX(itemPos->topDocPt() + itemPos->heightDocPt());

	setMouseState(MouseState::AddSchemaPosRectEndPoint);

	editSchemaView()->update();

	return;
}

void EditSchemaWidget::mouseLeftDown_AddSchemaPosConnectionStartPoint(QMouseEvent* event)
{
	if (editSchemaView()->m_newItem == nullptr)
	{
		assert(editSchemaView()->m_newItem != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	VFrame30::IPosConnection* itemPos = dynamic_cast<VFrame30::IPosConnection*>(editSchemaView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	// magnet point to pin
	//
	docPoint = magnetPointToPin(docPoint);

	itemPos->DeleteAllPoints();

	itemPos->AddPoint(docPoint.x(), docPoint.y());		        // ����� ����������� ��� �����
	itemPos->AddExtensionPoint(docPoint.x(), docPoint.y());

	// ��������� ��� ���������� ���������� ����
	//
	if (dynamic_cast<VFrame30::FblItem*>(editSchemaView()->m_newItem.get()) != nullptr)
	{
		// ??
		//VFrame30Ext.IFblItem fblItem = schemaView.newItem as VFrame30Ext.IFblItem;
	}

	setMouseState(MouseState::AddSchemaPosConnectionNextPoint);

	return;
}

void EditSchemaWidget::mouseLeftUp_Selection(QMouseEvent* me)
{
	// ���������� ����� �������� ������ � ���������� �������,
	// �������� �� � selectedItem
	//
	bool shiftIsPressed = me->modifiers().testFlag(Qt::ShiftModifier);

	// ����� ���� ����� ��� ���� ��� �����, �� � �������� ��������� ��� ��������,
	// ������� � ��������� ��� �������� �� ���������
	//
	if (shiftIsPressed == false)
	{
		editSchemaView()->clearSelection();
	}

	editSchemaView()->m_mouseSelectionEndPoint = widgetPointToDocument(me->pos(), false);

	// ��������� ���������� ��������� ��� �����
	//
	QRectF pageSelectionArea = QRectF(editSchemaView()->m_mouseSelectionStartPoint, editSchemaView()->m_mouseSelectionEndPoint)
							   .normalized();

	// ����� ��������� ������ pageSelectionArea
	//
	auto activeLayer = editSchemaView()->activeLayer();

	// ���� ���� ��� �������� �����, �� ��������� �������� ������ �������� ��������
	//
	if (editSchemaView()->m_mouseSelectionStartPoint == editSchemaView()->m_mouseSelectionEndPoint)
	{
		auto item = activeLayer->getItemUnderPoint(pageSelectionArea.topLeft());

		if (item != nullptr)
		{
			// ���� ����� ������� ��� ���� � ������, �� ������� ��� �� ������ ����������
			//
			bool wasDeleted = editSchemaView()->removeFromSelection(item);

			if (wasDeleted == false)
			{
				// This item was not selected, so just select it
				//
				editSchemaView()->addSelection(item);
			}
		}
	}
	else
	{
		// ������������� ������ ��������� , ������������ (��� �������������)
		// ��� �������� ��������� � �������������
		//
		auto itemsInRect = activeLayer->getItemListInRectangle(pageSelectionArea);

		for (auto item = itemsInRect.begin(); item != itemsInRect.end(); ++item)
		{
			// ���� ����� ������� ��� ���� � ������, �� ������� ��� �� ������ ����������
			//
			auto findResult = std::find(selectedItems().begin(), selectedItems().end(), *item);

			if (findResult != selectedItems().end())
			{
				editSchemaView()->removeFromSelection(*item, false);	// do emit SectionChanged manually, after the loop
			}
			else
			{
				editSchemaView()->addSelection(*item, false);			// do emit SectionChanged manually, after the loopB
			}
		}

		emit selectionChanged();
	}

	// --
	//
	editSchemaView()->m_mouseSelectionStartPoint = QPoint();
	editSchemaView()->m_mouseSelectionEndPoint = QPoint();

	resetAction();

	return;
}

void EditSchemaWidget::mouseLeftUp_Moving(QMouseEvent* event)
{
	if (selectedItems().empty() == true)
	{
		assert(selectedItems().empty() != true);
		return;
	}

	const auto& selected = selectedItems();

	QPointF mouseMovingStartPointIn = editSchemaView()->m_editStartDocPt;
	QPointF mouseMovingEndPointIn = widgetPointToDocument(event->pos(), snapToGrid());

	editSchemaView()->m_editEndDocPt = mouseMovingEndPointIn;

	float xdif = mouseMovingEndPointIn.x() - mouseMovingStartPointIn.x();
	float ydif = mouseMovingEndPointIn.y() - mouseMovingStartPointIn.y();

	if (std::abs(xdif) < 0.0000001 && std::abs(ydif) < 0.0000001)
	{
		// SchemaItem's have not changed positions
		//
		resetAction();
		return;
	}

	bool ctrlIsPressed = event->modifiers().testFlag(Qt::ControlModifier);

	if (ctrlIsPressed == false)
	{
		// Move items
		//
		std::vector<std::shared_ptr<VFrame30::SchemaItem>> itemsForMove;
		itemsForMove.reserve(selected.size());

		for (auto& item : selected)
		{
			if (item->isLocked() == false)
			{
				itemsForMove.push_back(item);
			}
		}

		if (itemsForMove.empty() == false)
		{
			m_editEngine->runMoveItem(xdif, ydif, itemsForMove, snapToGrid());
		}
	}
	else
	{
		// Copy SchemaItems and move copied items
		//
		std::vector<std::shared_ptr<VFrame30::SchemaItem>> newItems;

		DbController* dbc = db();
		auto s = schema();

		std::for_each(selected.begin(), selected.end(),
			[xdif, ydif, &newItems, dbc, s](const std::shared_ptr<VFrame30::SchemaItem>& si)
			{
				QByteArray data;

				bool result = si->Save(data);

				if (result == false || data.isEmpty() == true)
				{
					assert(result == true);
					assert(data.isEmpty() == false);
					return;
				}

				std::shared_ptr<VFrame30::SchemaItem> newItem = VFrame30::SchemaItem::Create(data);

				if (newItem == nullptr)
				{
					assert(newItem != nullptr);
					return;
				}

				newItem->setNewGuid();

				if (newItem->isFblItemRect() == true)
				{
					VFrame30::FblItemRect* fblItemRect = newItem->toFblItemRect();
					assert(fblItemRect);

					int counterValue = 0;
					bool nextValRes = dbc->nextCounterValue(&counterValue);
					if (nextValRes == false)
					{
						return;
					}

					fblItemRect->setLabel(s->schemaId() + "_" + QString::number(counterValue));
				}

				newItem->MoveItem(xdif, ydif);

				newItems.push_back(newItem);
				return;
			});

		m_editEngine->runAddItem(newItems, editSchemaView()->activeLayer());
	}

	resetAction();
	return;
}

void EditSchemaWidget::mouseLeftUp_SizingRect(QMouseEvent* event)
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

	if (editSchemaView()->m_editStartDocPt.isNull() == true ||
		editSchemaView()->m_editEndDocPt.isNull() == true)
	{
		assert(editSchemaView()->m_editStartDocPt.isNull() == false);
		assert(editSchemaView()->m_editEndDocPt.isNull() == false);
		return;
	}

	if (selectedItems().size() != 1)
	{
		assert(selectedItems().size() == 1);
		return;
	}

	QPointF mouseSizingStartPointDocPt = editSchemaView()->m_editStartDocPt;
	QPointF mouseSizingEndPointDocPt = widgetPointToDocument(event->pos(), snapToGrid());

	auto si = selectedItems().front();
	VFrame30::IPosRect* itemPos = dynamic_cast<VFrame30::IPosRect*>(selectedItems().front().get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		return;
	}

	double xdif = mouseSizingEndPointDocPt.x() - mouseSizingStartPointDocPt.x();
	double ydif = mouseSizingEndPointDocPt.y() - mouseSizingStartPointDocPt.y();

	// set new pos
	//
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

	// --
	//
	std::vector<VFrame30::SchemaPoint> itemPoints;

	itemPoints.push_back(VFrame30::SchemaPoint(std::min(x1, x2), std::min(y1, y2)));
	itemPoints.push_back(VFrame30::SchemaPoint(std::min(x1, x2) + std::abs(x2 - x1), std::min(y1, y2) + std::abs(y2 - y1)));

	m_editEngine->runSetPoints(itemPoints, si);

	resetAction();
	return;
}

void EditSchemaWidget::mouseLeftUp_MovingLinePoint(QMouseEvent* event)
{
	if (mouseState() != MouseState::MovingStartLinePoint &&
		mouseState() != MouseState::MovingEndLinePoint)
	{
		return;
	}

	if (selectedItems().size() != 1)
	{
		assert(selectedItems().size() == 1);
		return;
	}

	auto si = selectedItems().front();
	VFrame30::IPosLine* itemPos = dynamic_cast<VFrame30::IPosLine*>(selectedItems().front().get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		return;
	}

	std::vector<VFrame30::SchemaPoint> points(2);

	QPointF spt = editSchemaView()->m_editStartDocPt;
	QPointF ept = widgetPointToDocument(event->pos(), snapToGrid());

	double xdif = ept.x() - spt.x();
	double ydif = ept.y() - spt.y();

	if (std::abs(xdif) < 0.000001 && std::abs(ydif) < 0.000001)
	{
		// There is no real moving
		//
		resetAction();
		return;
	}

	if (mouseState() == MouseState::MovingStartLinePoint)
	{
		points[0] = static_cast<VFrame30::SchemaPoint>(QPointF(itemPos->startXDocPt() + xdif,itemPos->startYDocPt() + ydif));
		points[1] = static_cast<VFrame30::SchemaPoint>(QPointF(itemPos->endXDocPt(), itemPos->endYDocPt()));
	}

	if (mouseState() == MouseState::MovingEndLinePoint)
	{
		points[0] = static_cast<VFrame30::SchemaPoint>(QPointF(itemPos->startXDocPt(),itemPos->startYDocPt()));
		points[1] = static_cast<VFrame30::SchemaPoint>(QPointF(itemPos->endXDocPt() + xdif, itemPos->endYDocPt() + ydif));
	}

	m_editEngine->runSetPoints(points, si);

	//--
	//
	resetAction();
	return;
}

void EditSchemaWidget::mouseLeftUp_AddSchemaPosLineEndPoint(QMouseEvent* event)
{
	assert(editSchemaView()->m_newItem != nullptr);

	VFrame30::IPosLine* itemPos = dynamic_cast<VFrame30::IPosLine*>(editSchemaView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		editSchemaView()->m_newItem.reset();
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	itemPos->setEndXDocPt(docPoint.x());
	itemPos->setEndYDocPt(docPoint.y());

	if (std::abs(itemPos->startXDocPt() - itemPos->endXDocPt()) < 0.000001 &&
		std::abs(itemPos->startYDocPt() - itemPos->endYDocPt()) < 0.000001)
	{
		// The line is empty
		//
		update();
	}
	else
	{
		// Add item to the active layer
		//
		m_editEngine->runAddItem(editSchemaView()->m_newItem, editSchemaView()->activeLayer());
	}

	resetAction();

	return;
}

void EditSchemaWidget::mouseLeftUp_AddSchemaPosRectEndPoint(QMouseEvent* event)
{
	assert(editSchemaView()->m_newItem != nullptr);

	VFrame30::IPosRect* itemPos = dynamic_cast<VFrame30::IPosRect*>(editSchemaView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		editSchemaView()->m_newItem.reset();
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	editSchemaView()->m_addRectEndPoint = docPoint;

	QPointF sp = editSchemaView()->m_addRectStartPoint;
	QPointF ep = editSchemaView()->m_addRectEndPoint;

	itemPos->setLeftDocPt(sp.x());
	itemPos->setTopDocPt(sp.y());
	itemPos->setWidthDocPt(ep.x() - sp.x());
	itemPos->setHeightDocPt(ep.y() - sp.y());

	double minWidth = itemPos->minimumPossibleWidthDocPt(schema()->gridSize(), schema()->pinGridStep());
	double minHeight = itemPos->minimumPossibleHeightDocPt(schema()->gridSize(), schema()->pinGridStep());

	if (itemPos->widthDocPt() < minWidth)
	{
		itemPos->setWidthDocPt(minWidth);
	}
	if (itemPos->heightDocPt() < minHeight)
	{
		itemPos->setHeightDocPt(minHeight);
	}

	if (itemPos->widthDocPt() < 0.000001 && itemPos->heightDocPt() < 0.000001)
	{
		// The rect is empty
		//
		update();
	}
	else
	{
		// �������� ������� � �������� ����
		//
		m_editEngine->runAddItem(editSchemaView()->m_newItem, editSchemaView()->activeLayer());
	}

	resetAction();

	return;
}

void EditSchemaWidget::mouseLeftUp_AddSchemaPosConnectionNextPoint(QMouseEvent* e)
{
	assert(editSchemaView()->m_newItem != nullptr);

	VFrame30::IPosConnection* itemPos = dynamic_cast<VFrame30::IPosConnection*>(editSchemaView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		editSchemaView()->m_newItem.reset();
		return;
	}

	// Add the last point, where cursor is now
	//
	mouseRightDown_AddSchemaPosConnectionNextPoint(e);

	itemPos->RemoveSamePoints();
	itemPos->DeleteAllExtensionPoints();

	if (itemPos->GetPointList().size() >= 2)
	{
		// ���� ����� �������� ��� ���������� �� ����� �� ��������, �� ��������� ��� ��� (��� ���) �����
		//
		auto points = itemPos->GetPointList();

		VFrame30::SchemaPoint startPoint = points.front();
		VFrame30::SchemaPoint endPoint = points.back();

		// ����� ��������� ������� ����� �� ������ startPoint, endPoint
		//
		QUuid startItemGuid = QUuid();			// ����� ���������� �� �������� ����� ������� ����� ����������� � ������ ����� ����� �����
		bool startPointAddedToOther = false;	// ����� ������� ��� ����������� � ������������� (�������� ����� ����� �� ����� ���������)
		bool endPointAddedToOther = false;		// ����� ������� ��� ����������� � ������������� (�������� ����� ����� �� ����� ���������)

		std::shared_ptr<VFrame30::SchemaItem> linkUnderStartPoint = activeLayer()->getItemUnderPoint(QPointF(startPoint.X, startPoint.Y), editSchemaView()->m_newItem->metaObject()->className());
		std::shared_ptr<VFrame30::SchemaItem> linkUnderEndPoint = activeLayer()->getItemUnderPoint(QPointF(endPoint.X, endPoint.Y), editSchemaView()->m_newItem->metaObject()->className());

		std::shared_ptr<VFrame30::SchemaItem> fblRectUnderStartPoint =
			activeLayer()->findPinUnderPoint(startPoint, schema()->gridSize(), schema()->pinGridStep());

		std::shared_ptr<VFrame30::SchemaItem> fblRectUnderEndPoint =
			activeLayer()->findPinUnderPoint(endPoint, schema()->gridSize(), schema()->pinGridStep());

		if (linkUnderStartPoint != nullptr &&
			fblRectUnderStartPoint.get() == nullptr)	// Start point is not on BlItemRect pin
		{
			assert(linkUnderStartPoint->metaObject()->className() == editSchemaView()->m_newItem->metaObject()->className());

			// ��� ����� �� ��������, ���� ����� schemaItemStartPoint ����� �� ������ ��� ��������� ����� schemaItemStartPoint,
			// �� ���������� schemaItemStartPoint � ����� �������
			//
			assert(dynamic_cast<VFrame30::IPosConnection*>(linkUnderStartPoint.get()) != nullptr);

			VFrame30::IPosConnection* existingItemPos = dynamic_cast<VFrame30::IPosConnection*>(linkUnderStartPoint.get());

			auto existingItemPoints = existingItemPos->GetPointList();

			if (std::abs(existingItemPoints.front().X - startPoint.X) < 0.000001 &&
				std::abs(existingItemPoints.front().Y - startPoint.Y) < 0.000001)
			{
				// ��������� ����� ����� ����� ����� �� ��������� ����� ������ �����
				//
				startItemGuid = linkUnderStartPoint->guid();		// ���������, ��� �� ����� �� ���������� � ���� �� ����� � ��������� �����, ��� �� �� ���������� ������
				startPointAddedToOther = true;

				// --
				points.reverse();
				existingItemPoints.pop_front();
				points.insert(points.end(), existingItemPoints.begin(), existingItemPoints.end());
				points.reverse();								// ���� ����� ����������� �� ��������� �����, �� ���� Recerse ����� �����

				std::vector<VFrame30::SchemaPoint> newPoints(points.begin(), points.end());
				newPoints = removeUnwantedPoints(newPoints);

				m_editEngine->runSetPoints(newPoints, linkUnderStartPoint);
			}
			else
			{
				if (std::abs(existingItemPoints.back().X - startPoint.X) < 0.000001 &&
					std::abs(existingItemPoints.back().Y - startPoint.Y) < 0.000001)
				{
					// ������ ����� ����� ����� ����� �� ��������� ����� ������������ �����
					//
					startItemGuid = linkUnderStartPoint->guid();	// ���������, ��� �� ����� �� ���������� � ���� �� ����� � ��������� �����, ��� �� �� ���������� ������
					startPointAddedToOther = true;

					// --
					points.pop_front();

					existingItemPoints.insert(existingItemPoints.end(), points.begin(), points.end());
					points.clear();
					points.assign(existingItemPoints.begin(), existingItemPoints.end());

					std::vector<VFrame30::SchemaPoint> newPoints(points.begin(), points.end());
					newPoints = removeUnwantedPoints(newPoints);

					m_editEngine->runSetPoints(newPoints, linkUnderStartPoint);
				}
			}
		}

		// --
		//
		if (linkUnderEndPoint != nullptr &&
			fblRectUnderEndPoint.get() == nullptr &&				// End point is not on BlItemRect pin
			linkUnderEndPoint->guid() != startItemGuid)				// ������� �� ������ ���� � ���������� ������� ����������� � ���� �� �����
		{
			// ������ ���� ����� �� ��� ��������
			//
			assert(linkUnderEndPoint->metaObject()->className() == editSchemaView()->m_newItem->metaObject()->className());

			// ��� ����� �� ��������, ���� ����� schemaItemEndPoint ����� �� ������ ��� ��������� ����� schemaItemEndPoint,
			// �� ���������� schemaItemEndPoint � ����� �������
			//
			assert(dynamic_cast<VFrame30::IPosConnection*>(linkUnderEndPoint.get()) != nullptr);
			VFrame30::IPosConnection* existingItemPos = dynamic_cast<VFrame30::IPosConnection*>(linkUnderEndPoint.get());

			auto existingItemPoints = existingItemPos->GetPointList();

			if (std::abs(existingItemPoints.front().X - endPoint.X) < 0.000001 &&
				std::abs(existingItemPoints.front().Y - endPoint.Y) < 0.000001)
			{
				// ��������� ����� ����� ����� ����� �� ������ ����� ������ �����
				//
				endPointAddedToOther = true;

				if (startPointAddedToOther == true)	// ����� ����� ��� ���� ��������� ��������� schemaItemStartPoint
				{
					// ����������� ���� �������� - schemaItemStartPoint � schemaItemEndPoint
					//
					existingItemPoints.pop_front();
					points.insert(points.end(), existingItemPoints.begin(), existingItemPoints.end());

					m_editEngine->runDeleteItem(linkUnderStartPoint, activeLayer());

					std::vector<VFrame30::SchemaPoint> newPoints(points.begin(), points.end());
					newPoints = removeUnwantedPoints(newPoints);

					m_editEngine->runSetPoints(newPoints, linkUnderEndPoint);
				}
				else
				{
					// � �������� schemaItemEndPoint �������� points
					//
					existingItemPoints.pop_front();

					points.insert(points.end(), existingItemPoints.begin(), existingItemPoints.end());

					std::vector<VFrame30::SchemaPoint> newPoints(points.begin(), points.end());
					newPoints = removeUnwantedPoints(newPoints);

					m_editEngine->runSetPoints(newPoints, linkUnderEndPoint);
				}
			}
			else
			{
				if (std::abs(existingItemPoints.back().X - endPoint.X) < 0.000001 &&
					std::abs(existingItemPoints.back().Y - endPoint.Y) < 0.000001)
				{
					// ��������� ����� ����� ����� ����� �� ��������� ����� ������������ �����
					//
					endPointAddedToOther = true;

					if (startPointAddedToOther == true)	// ����� ����� ��� ���� ��������� ��������� schemaItemStartPoint
					{
						// ����������� ���� �������� - schemaItemStartPoint � schemaItemEndPoint
						//
						existingItemPoints.reverse();
						existingItemPoints.pop_front();
						points.insert(points.end(), existingItemPoints.begin(), existingItemPoints.end());

						m_editEngine->runDeleteItem(linkUnderStartPoint, activeLayer());

						std::vector<VFrame30::SchemaPoint> newPoints(points.begin(), points.end());
						newPoints = removeUnwantedPoints(newPoints);

						m_editEngine->runSetPoints(newPoints, linkUnderEndPoint);
					}
					else
					{
						// � �������� schemaItemEndPoint �������� points
						//
						existingItemPoints.reverse();
						existingItemPoints.pop_front();
						points.insert(points.end(), existingItemPoints.begin(), existingItemPoints.end());

						std::vector<VFrame30::SchemaPoint> newPoints(points.begin(), points.end());
						newPoints = removeUnwantedPoints(newPoints);

						m_editEngine->runSetPoints(newPoints, linkUnderEndPoint);
					}
				}
			}
		}

		if (startPointAddedToOther == false &&
			endPointAddedToOther == false)
		{
			if (itemPos->GetPointList().size() > 2 ||
					(itemPos->GetPointList().size() == 2 &&
					itemPos->GetPointList().front() != itemPos->GetPointList().back()))
			{
				const std::list<VFrame30::SchemaPoint>& pointList = itemPos->GetPointList();

				std::list<VFrame30::SchemaPoint> newPoints = removeUnwantedPoints(pointList);

				itemPos->SetPointList(newPoints);
				assert(itemPos->GetPointList().size() >= 2);

				m_editEngine->runAddItem(editSchemaView()->m_newItem, activeLayer());
			}
		}
	}

	resetAction();

	return;
}

void EditSchemaWidget::mouseLeftUp_MovingEdgeOrVertex(QMouseEvent*)
{
	if (mouseState() != MouseState::MovingHorizontalEdge &&
		mouseState() != MouseState::MovingVerticalEdge &&
		mouseState() != MouseState::MovingConnectionLinePoint)
	{
		assert(false);
		return;
	}

	if (selectedItems().size() != 1)
	{
		assert(selectedItems().size() == 1);
		resetAction();
		return;
	}

	auto si = selectedItems().front();
	assert(si != nullptr);

	VFrame30::IPosConnection* itemPos = dynamic_cast<VFrame30::IPosConnection*>(si.get());
	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		resetAction();
		return;
	}

	if ((mouseState() == MouseState::MovingHorizontalEdge || mouseState() == MouseState::MovingVerticalEdge) &&
		std::abs(editSchemaView()->m_editEndMovingEdge - editSchemaView()->m_editStartMovingEdge) < 0.000001)
	{
		// ��������� ��������� ������, ������ � �� ���� ��������� �������
		//
		resetAction();
		return;
	}

	if (mouseState() == MouseState::MovingConnectionLinePoint &&
		std::abs(editSchemaView()->m_editEndMovingEdgeX - editSchemaView()->m_editStartMovingEdgeX) < 0.000001 &&
		std::abs(editSchemaView()->m_editEndMovingEdgeY - editSchemaView()->m_editStartMovingEdgeY) < 0.000001)
	{
		// ��������� ��������� ������, ������ � �� ���� ��������� �������
		//
		resetAction();
		return;
	}

	std::vector<VFrame30::SchemaPoint> setPoints(editSchemaView()->m_movingVertexPoints.begin(), editSchemaView()->m_movingVertexPoints.end());
	m_editEngine->runSetPoints(setPoints, si);

	resetAction();
	return;
}

void EditSchemaWidget::mouseMove_Scrolling(QMouseEvent*)
{
	// To Do
	assert(false);
	return;
}

void EditSchemaWidget::mouseMove_Selection(QMouseEvent* me)
{
	// ��������� ����������� ���������.
	//
	editSchemaView()->m_mouseSelectionEndPoint = widgetPointToDocument(me->pos(), false);
	editSchemaView()->update();

//	QRect selectionRect = QRect(editSchemaView()->m_rubberBand->pos(), me->pos()).normalized();
//	editSchemaView()->m_rubberBand->setGeometry(selectionRect);

	return;
}

void EditSchemaWidget::mouseMove_Moving(QMouseEvent* me)
{
	if (selectedItems().empty() == true)
	{
		assert(selectedItems().empty() == false);
		setMouseState(MouseState::None);
		return;
	}

	editSchemaView()->m_editEndDocPt = widgetPointToDocument(me->pos(), snapToGrid());

	editSchemaView()->update();
	return;
}

void EditSchemaWidget::mouseMove_SizingRect(QMouseEvent* me)
{
	if (selectedItems().size() != 1)
	{
		assert(selectedItems().size() == 1);
		setMouseState(MouseState::None);
		return;
	}

	auto si = selectedItems().front();

	if (dynamic_cast<VFrame30::IPosRect*>(si.get()) == nullptr)
	{
		assert(dynamic_cast<VFrame30::IPosRect*>(si.get()) != nullptr);
		setMouseState(MouseState::None);
		return;
	}

	editSchemaView()->m_editEndDocPt = widgetPointToDocument(me->pos(), snapToGrid());

	editSchemaView()->update();
	return;
}

void EditSchemaWidget::mouseMove_MovingLinePoint(QMouseEvent* me)
{
	if (selectedItems().size() != 1)
	{
		assert(selectedItems().size() == 1);
		setMouseState(MouseState::None);
		return;
	}

	auto si = selectedItems().front();

	if (dynamic_cast<VFrame30::IPosLine*>(si.get()) == nullptr)
	{
		assert(dynamic_cast<VFrame30::IPosLine*>(si.get()) != nullptr);
		setMouseState(MouseState::None);
		return;
	}

	editSchemaView()->m_editEndDocPt = widgetPointToDocument(me->pos(), snapToGrid());
	editSchemaView()->update();
	return;
}

void EditSchemaWidget::mouseMove_AddSchemaPosLineEndPoint(QMouseEvent* event)
{
	if (editSchemaView()->m_newItem == nullptr)
	{
		assert(editSchemaView()->m_newItem != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	VFrame30::IPosLine* itemPos = dynamic_cast<VFrame30::IPosLine*>(editSchemaView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	itemPos->setEndXDocPt(docPoint.x());
	itemPos->setEndYDocPt(docPoint.y());

	editSchemaView()->update();
	return;
}

void EditSchemaWidget::mouseMove_AddSchemaPosRectEndPoint(QMouseEvent* event)
{
	if (editSchemaView()->m_newItem == nullptr)
	{
		assert(editSchemaView()->m_newItem != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	VFrame30::IPosRect* itemPos = dynamic_cast<VFrame30::IPosRect*>(editSchemaView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	editSchemaView()->m_addRectEndPoint = docPoint;

	QPointF sp = editSchemaView()->m_addRectStartPoint;
	QPointF ep = editSchemaView()->m_addRectEndPoint;

	itemPos->setLeftDocPt(sp.x());
	itemPos->setTopDocPt(sp.y());


	itemPos->setWidthDocPt(ep.x() - sp.x());
	itemPos->setHeightDocPt(ep.y() - sp.y());

	double minWidth = itemPos->minimumPossibleWidthDocPt(schema()->gridSize(), schema()->pinGridStep());
	double minHeight = itemPos->minimumPossibleHeightDocPt(schema()->gridSize(), schema()->pinGridStep());

	if (itemPos->widthDocPt() < minWidth)
	{
		itemPos->setWidthDocPt(minWidth);
	}
	if (itemPos->heightDocPt() < minHeight)
	{
		itemPos->setHeightDocPt(minHeight);
	}

	editSchemaView()->m_addRectEndPoint.setX(itemPos->leftDocPt() + itemPos->widthDocPt());
	editSchemaView()->m_addRectEndPoint.setY(itemPos->topDocPt() + itemPos->heightDocPt());

	// --
	//
	editSchemaView()->update();

	return;
}

void EditSchemaWidget::mouseMove_AddSchemaPosConnectionNextPoint(QMouseEvent* event)
{
	double gridSize = schema()->gridSize();

	if (editSchemaView()->m_newItem == nullptr)
	{
		assert(editSchemaView()->m_newItem != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	VFrame30::IPosConnection* itemPos = dynamic_cast<VFrame30::IPosConnection*>(editSchemaView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	// magnet point to pin
	//
	docPoint = magnetPointToPin(docPoint);

	// --
	//
	auto points = itemPos->GetPointList();
	auto extPoints = itemPos->GetExtensionPoints();

	if (points.empty() == true || extPoints.empty() == true)
	{
		assert(points.size() > 0);
		assert(extPoints.size() > 0);
		return;
	}

	if (points.size() + extPoints.size() < 2)
	{
		assert(points.size() + extPoints.size() >= 2);
		return;
	}

	VFrame30::SchemaPoint ptBase = points.back();

	// Add extra points
	//
	double horzDistance = std::abs(ptBase.X - docPoint.x()) * (ptBase.X - docPoint.x() > 0.0 ? -1.0 : 1.0);
	double midPoint = 0.0;

	if (std::abs(ptBase.X - docPoint.x()) < gridSize * 1.0)
	{
		midPoint = ptBase.X;
	}
	else
	{
		midPoint = ptBase.X + horzDistance / 2;
	}

	QPointF onePoint(midPoint, ptBase.Y);
	onePoint = snapToGrid(onePoint);

	itemPos->DeleteAllExtensionPoints();

	// if onePoint on previous line, then move it to base
	//
	if (points.size() > 1)
	{
		VFrame30::SchemaPoint lastLinkPt1 = *std::prev(points.end(), 2);
		VFrame30::SchemaPoint lastLinkPt2 = points.back();

		if (std::abs(lastLinkPt1.Y - lastLinkPt2.Y) < 0.0000001 &&						// prev line is horizontal
			std::abs(lastLinkPt1.Y - onePoint.y()) < 0.0000001 &&
			((lastLinkPt2.X - lastLinkPt1.X > 0 && ptBase.X - onePoint.x() > 0) ||		// new line on the sime side
			 (lastLinkPt2.X - lastLinkPt1.X < 0 && ptBase.X - onePoint.x() < 0)
			))
		{
			onePoint.setX(ptBase.X);
			onePoint.setY(ptBase.Y);
		}
	}

	QPointF twoPoint(onePoint.x(), docPoint.y());

	if (onePoint != ptBase)
	{
		itemPos->AddExtensionPoint(onePoint.x(), onePoint.y());
	}
	itemPos->AddExtensionPoint(twoPoint.x(), twoPoint.y());
	itemPos->AddExtensionPoint(docPoint.x(), docPoint.y());


	editSchemaView()->update();

	return;
}

void EditSchemaWidget::mouseMove_MovingEdgesOrVertex(QMouseEvent* event)
{
	if (mouseState() != MouseState::MovingHorizontalEdge &&
		mouseState() != MouseState::MovingVerticalEdge &&
		mouseState() != MouseState::MovingConnectionLinePoint)
	{
		assert(false);
		resetAction();
		return;
	}

	if (selectedItems().size() != 1)
	{
		assert(selectedItems().size() == 1);
		resetAction();
		return;
	}

	auto si = selectedItems().front();
	assert(si != nullptr);

	VFrame30::IPosConnection* itemPos = dynamic_cast<VFrame30::IPosConnection*>(si.get());
	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		resetAction();
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	// --
	//
	switch (mouseState())
	{
	case MouseState::MovingHorizontalEdge:
		editSchemaView()->m_editEndMovingEdge = docPoint.y();
		break;
	case MouseState::MovingVerticalEdge:
		editSchemaView()->m_editEndMovingEdge = docPoint.x();
		break;
	case MouseState::MovingConnectionLinePoint:
		// magnet point to pin
		//
		docPoint = magnetPointToPin(docPoint);

		editSchemaView()->m_editEndMovingEdgeX = docPoint.x();
		editSchemaView()->m_editEndMovingEdgeY = docPoint.y();
		break;
	default:
		assert(false);
	}

	editSchemaView()->update();

	return;
}

void EditSchemaWidget::mouseRightDown_None(QMouseEvent*)
{
	// CURRENTLY THIS ACTION IS DISABLED IN CONSTRUCTOR, ADD IT TO THE RightClickPress array
	//
	// To Do from old project
	//
	return;
}

void EditSchemaWidget::mouseRightDown_AddSchemaPosConnectionNextPoint(QMouseEvent* event)
{
	if (editSchemaView()->m_newItem == nullptr)
	{
		assert(editSchemaView()->m_newItem != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	VFrame30::IPosConnection* itemPos = dynamic_cast<VFrame30::IPosConnection*>(editSchemaView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	const std::list<VFrame30::SchemaPoint>& extPoints = itemPos->GetExtensionPoints();

	if (extPoints.empty() == true)
	{
		assert(extPoints.size() > 0);
		return;
	}

	for (VFrame30::SchemaPoint p : extPoints)
	{
		itemPos->AddPoint(p.X, p.Y);
	}

	VFrame30::SchemaPoint lastExtPt = extPoints.back();	// Cache point before deleteing, as it can be removed from REFERENCED list
	itemPos->DeleteAllExtensionPoints();

	itemPos->AddExtensionPoint(lastExtPt.X, lastExtPt.Y);

	// --
	//
	editSchemaView()->update();

	return;
}

void EditSchemaWidget::mouseRightUp_None(QMouseEvent* event)
{
	QPointF docPoint = widgetPointToDocument(event->pos(), false);

	auto item = editSchemaView()->activeLayer()->getItemUnderPoint(docPoint);

	if (item == nullptr)
	{
		editSchemaView()->clearSelection();
		resetAction();
	}
	else
	{
		bool itemIsAlreadySelected = editSchemaView()->isItemSelected(item);

		if (itemIsAlreadySelected == false)
		{
			editSchemaView()->setSelectedItem(item);
			resetAction();
		}
	}

	return;
}

DbController* EditSchemaWidget::dbcontroller()
{
	return m_dbcontroller;
}

DbController* EditSchemaWidget::db()
{
	return m_dbcontroller;
}

EditSchemaView* EditSchemaWidget::editSchemaView()
{
	EditSchemaView* sw = dynamic_cast<EditSchemaView*>(schemaView());
	assert(sw != nullptr);
	return sw;
}

const EditSchemaView* EditSchemaWidget::editSchemaView() const
{
	const EditSchemaView* sw = dynamic_cast<const EditSchemaView*>(schemaView());
	assert(sw != nullptr);
	return sw;
}

bool EditSchemaWidget::isLogicSchema() const
{
	return schema()->isLogicSchema();
}

bool EditSchemaWidget::isUfbSchema() const
{
	return schema()->isUfbSchema();
}

std::shared_ptr<VFrame30::LogicSchema> EditSchemaWidget::logicSchema()
{
	std::shared_ptr<VFrame30::LogicSchema> logicSchema = std::dynamic_pointer_cast<VFrame30::LogicSchema>(schema());
	return logicSchema;
}

const std::shared_ptr<VFrame30::LogicSchema> EditSchemaWidget::logicSchema() const
{
	const std::shared_ptr<VFrame30::LogicSchema> logicSchema = std::dynamic_pointer_cast<VFrame30::LogicSchema>(schema());
	return logicSchema;
}

const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& EditSchemaWidget::selectedItems() const
{
	return editSchemaView()->m_selectedItems;
}

std::shared_ptr<VFrame30::SchemaLayer> EditSchemaWidget::activeLayer()
{
	return editSchemaView()->activeLayer();
}


QPointF EditSchemaWidget::widgetPointToDocument(const QPoint& widgetPoint, bool snapToGrid) const
{
	QPointF result = BaseSchemaWidget::widgetPointToDocument(widgetPoint);

	if (snapToGrid == true)
	{
		QPointF snapped = this->snapToGrid(result);
		return snapped;
	}

	return result;
}

QPointF EditSchemaWidget::snapToGrid(QPointF pt) const
{
	double gridSize = schema()->gridSize();

	QPointF result = CUtils::snapToGrid(pt, gridSize);
	return result;
}

bool EditSchemaWidget::updateAfbsForSchema()
{
	// Update Afb list
	//
	std::vector<std::shared_ptr<Afb::AfbElement>> afbs;

	bool ok = loadAfbsDescriptions(&afbs);
	if (ok == false)
	{
		return false;
	}

	QString errorMessage;
	int updatedItemCount = 0;
	ok = schema()->updateAllSchemaItemFbs(afbs, &updatedItemCount, &errorMessage);

	if (ok == false)
	{
		QMessageBox::critical(this, qApp->applicationName(), tr("Update AFB schema items error: ") + errorMessage);
		return false;
	}

	if (updatedItemCount != 0)
	{
		setModified();

		QMessageBox msgBox(this);
		msgBox.setWindowTitle(qApp->applicationName());
		msgBox.setText(tr("%1 AFB(s) are updated according to the latest AFB description.").arg(updatedItemCount));
		msgBox.setInformativeText("Please, check iput/output pins and parameters.\nClose schema without saving to discard changes.");
		msgBox.exec();
	}

	return true;
}

bool EditSchemaWidget::updateUfbsForSchema()
{
	// Get Ufb list
	//
	std::vector<std::shared_ptr<VFrame30::UfbSchema>> ufbs;

	bool ok = loadUfbSchemas(&ufbs);

	if (ok == false)
	{
		return false;
	}

	// Update
	//
	QString errorMessage;
	int updatedItemCount = 0;

	ok = schema()->updateAllSchemaItemUfb(ufbs, &updatedItemCount, &errorMessage);

	if (ok == false)
	{
		QMessageBox::critical(this, qApp->applicationName(), tr("Update UFB schema items error: ") + errorMessage);
		return false;
	}

	if (updatedItemCount != 0)
	{
		setModified();

		QMessageBox msgBox(this);
		msgBox.setWindowTitle(qApp->applicationName());
		msgBox.setText(tr("%1 UFB(s) are updated according to the latest UFB schemas.").arg(updatedItemCount));
		msgBox.setInformativeText("Please, check iput/output pins and parameters.\nClose schema without saving to discard changes.");
		msgBox.exec();
	}

	return true;
}


void EditSchemaWidget::addItem(std::shared_ptr<VFrame30::SchemaItem> newItem)
{
	if (newItem == nullptr)
	{
		assert(newItem != nullptr);
		return;
	}

	editSchemaView()->m_newItem = newItem;

	// If items is FblItemRect, set label to it
	//
	if (newItem->isFblItemRect() == true)
	{
		VFrame30::FblItemRect* fblItemRect = newItem->toFblItemRect();
		assert(fblItemRect);

		int counterValue = 0;
		bool nextValRes = db()->nextCounterValue(&counterValue);
		if (nextValRes == false)
		{
			return;
		}

		fblItemRect->setLabel(schema()->schemaId() + "_" + QString::number(counterValue));
	}

	// --
	//
	bool posInterfaceFound = false;

	// ����������� ������� � ������������� ISchemaPosLine
	//
	if (dynamic_cast<VFrame30::IPosLine*>(newItem.get()) != nullptr)
	{
		posInterfaceFound = true;
		setMouseState(MouseState::AddSchemaPosLineStartPoint);
	}

	// ����������� ������� � ������������� ISchemaPosRect
	//
	if (dynamic_cast<VFrame30::IPosRect*>(newItem.get()) != nullptr)
	{
		posInterfaceFound = true;
		setMouseState(MouseState::AddSchemaPosRectStartPoint);

		VFrame30::IPosRect* itemPos = dynamic_cast<VFrame30::IPosRect*>(newItem.get());
		itemPos->minimumPossibleHeightDocPt(schema()->gridSize(), schema()->pinGridStep());		// chachedGridSize and pinStep will be initialized here
	}

	// ����������� ������� � ������������� ISchemaPosConnection
	//
	if (dynamic_cast<VFrame30::IPosConnection*>(newItem.get()) != nullptr)
	{
		posInterfaceFound = true;
		setMouseState(MouseState::AddSchemaPosConnectionStartPoint);
	}

	if (posInterfaceFound == true)
	{
		// �������� �������� ������, ������ �������� ��������� �����
		//
		setMouseCursor(mapFromGlobal(QCursor::pos()));
	}
	else
	{
		setMouseState(MouseState::None);
		assert(posInterfaceFound != false);
	}

	return;
}

void EditSchemaWidget::setMouseCursor(QPoint mousePos)
{
	setCursor(QCursor(Qt::CursorShape::ArrowCursor));

	for (size_t i = 0; i < sizeof(m_mouseStateCursor) / sizeof(m_mouseStateCursor[0]); i++)
	{
		if (mouseState() == m_mouseStateCursor[i].mouseState)
		{
			QCursor cursor(m_mouseStateCursor[i].cursorShape);
			setCursor(cursor);

			return;
		}
	}

	// ���� ������ ��� �� �����, �� �� ������� ������� �������� � �� ������� ������ � mouseStateToCursor!!!!!!!
	//
	int movingEdgePointIndex = -1;

	// ������� ������ ��������� �������
	//
	if (mouseState() == MouseState::None)
	{
		// ���������� ��� ����� ��������� �����, � ��� � ��� ����� �������
		//
		// Convert pixels to document points
		//
		QPointF docPos = widgetPointToDocument(mousePos, false);

		if (selectedItems().empty() == true)
		{
			auto itemUnderPoint = editSchemaView()->activeLayer()->getItemUnderPoint(docPos);

			// ���� ������� �� �������, �� ��� ����� ������ ����������
			//
			if (itemUnderPoint != nullptr &&
				editSchemaView()->getPossibleAction(itemUnderPoint.get(), docPos, &movingEdgePointIndex) == SchemaItemAction::MoveItem)
			{
				setCursor(Qt::SizeAllCursor);
				return;
			}
		}

		for (auto si = editSchemaView()->selectedItems().begin(); si != editSchemaView()->selectedItems().end(); ++si)
		{
			SchemaItemAction possibleAction = editSchemaView()->getPossibleAction(si->get(), docPos, &movingEdgePointIndex);

			if (possibleAction != SchemaItemAction::NoAction)
			{
				// Changing size, is possible for only one selected object
				//
				if (editSchemaView()->selectedItems().size() == 1)
				{
					auto findResult = std::find_if(std::begin(m_sizeActionToMouseCursor), std::end(m_sizeActionToMouseCursor),
						[&possibleAction](const SizeActionToMouseCursor& c) -> bool
						{
							return c.action == possibleAction;
						}
						);

					if (findResult != std::end(m_sizeActionToMouseCursor))
					{
						//qDebug() << Q_FUNC_INFO << static_cast<int>(findResult->cursorShape);
						setCursor(findResult->cursorShape);
						return;
					}
				}

				// --
				//
				switch (possibleAction)
				{
				case SchemaItemAction::MoveItem:
					setCursor(Qt::SizeAllCursor);
					return;
				case SchemaItemAction::MoveStartLinePoint:
					setCursor(Qt::SizeAllCursor);
					return;
				case SchemaItemAction::MoveEndLinePoint:
					setCursor(Qt::SizeAllCursor);
					return;
				case SchemaItemAction::MoveHorizontalEdge:
					setCursor(Qt::SplitVCursor);
					return;
				case SchemaItemAction::MoveVerticalEdge:
					setCursor(Qt::SplitHCursor);
					return;
				case SchemaItemAction::MoveConnectionLinePoint:
					setCursor(Qt::SizeAllCursor);
					return;
				default:
					void();
				}
			}
		}

		QCursor cursor(Qt::ArrowCursor);
		setCursor(cursor);
		return;
	}

	// ���������� ������� ��� �������� ������ �������� �� �����
	//
	if (dynamic_cast<VFrame30::IPosLine*>(editSchemaView()->m_newItem.get()) != nullptr)
	{
		QCursor cursor(Qt::CursorShape::CrossCursor);
		setCursor(cursor);
		return;
	}

	if (dynamic_cast<VFrame30::IPosRect*>(editSchemaView()->m_newItem.get()) != nullptr)
	{
		QCursor cursor(Qt::CursorShape::CrossCursor);
		setCursor(cursor);
		return;
	}

	if (dynamic_cast<VFrame30::IPosConnection*>(editSchemaView()->m_newItem.get()) != nullptr)
	{
		QCursor cursor(Qt::CursorShape::CrossCursor);
		setCursor(cursor);
		return;
	}

	return;
}

QPointF EditSchemaWidget::magnetPointToPin(QPointF docPoint)
{
	double gridSize = schema()->gridSize();
	double pinGridStep = static_cast<double>(schema()->pinGridStep());

	// Find "magnet" points, it can be any pin or link
	// Detect if docPoint closer then any pin to gridSize * pinGridStep / 2
	// If so, stick docPoint to this pin
	//
	std::vector<VFrame30::AfbPin> itemPins;
	itemPins.reserve(64);

	for (const std::shared_ptr<VFrame30::SchemaItem>& item : activeLayer()->Items)
	{
		VFrame30::FblItemRect* fblItemRect = dynamic_cast<VFrame30::FblItemRect*>(item.get());

		if (fblItemRect != nullptr)
		{
			fblItemRect->SetConnectionsPos(schema()->gridSize(), schema()->pinGridStep());

			const std::vector<VFrame30::AfbPin>& inputs = fblItemRect->inputs();
			const std::vector<VFrame30::AfbPin>& outputs = fblItemRect->outputs();

			itemPins.clear();
			itemPins.insert(itemPins.end(), inputs.begin(), inputs.end());
			itemPins.insert(itemPins.end(), outputs.begin(), outputs.end());

			for (const VFrame30::AfbPin& pin : itemPins)
			{
				QRectF pinArea = {pin.x() - gridSize * pinGridStep / 2, pin.y() - gridSize * pinGridStep / 2,
								 gridSize * pinGridStep, gridSize * pinGridStep};

				if (pinArea.contains(docPoint) == true)
				{
					return QPointF(pin.x(), pin.y());
				}
			}

			continue;
		}

		// To do, magnet to link
		//
	}

	return docPoint;
}

std::vector<VFrame30::SchemaPoint> EditSchemaWidget::removeUnwantedPoints(const std::vector<VFrame30::SchemaPoint>& source) const
{
	std::vector<VFrame30::SchemaPoint> result = source;

	int sameXPosCount = 0;			// Pairs of points amount by X coordinate
	int sameYPosCount = 0;			// Pairs of points amount by Y coordinate

	size_t currentPointIndex = 0;	// Index of current point to process

	// In cycle we are processing current point with previous point
	//
	for (currentPointIndex = 1; currentPointIndex < result.size(); currentPointIndex++)
	{
		VFrame30::SchemaPoint curPoint = result.at(currentPointIndex);
		VFrame30::SchemaPoint prevPoint = result.at(currentPointIndex - 1);

		if (std::abs(curPoint.X - prevPoint.X) < 0.0000001)
		{
			sameXPosCount ++;
		}
		else
		{
			// Remove points only if we have more than one pair with same
			// X coordinates
			//
			if (sameXPosCount > 1)
			{
				assert(currentPointIndex > 0);
				assert(currentPointIndex <= result.size());

				size_t startIndex = currentPointIndex - sameXPosCount;
				size_t lastIndex = currentPointIndex - 1;

				result.erase(result.begin() + startIndex, result.begin() + lastIndex);

				currentPointIndex = currentPointIndex - sameXPosCount-1;
				sameYPosCount = 0;
			}

			sameXPosCount = 0;
		}

		if (std::abs(curPoint.Y - prevPoint.Y) < 0.0000001)
		{
			sameYPosCount++;
		}
		else
		{
			// Remove points only if we have more than one pair with same
			// X coordinates
			//
			if (sameYPosCount > 1)
			{
				assert(currentPointIndex > 0);
				assert(currentPointIndex <= result.size());

				size_t startIndex = currentPointIndex - sameYPosCount;
				size_t lastIndex = currentPointIndex - 1;

				result.erase(result.begin() + startIndex, result.begin() + lastIndex);

				currentPointIndex = currentPointIndex - sameYPosCount-1;
				sameXPosCount = 0;
			}

			sameYPosCount = 0;
		}
	}

	// If some pairs with same coordinate values are placed at the end of
	// the line, we must remove them!
	//

	if (sameYPosCount > 1)
	{
		assert(currentPointIndex == result.size());

		size_t beginIndex = currentPointIndex - sameYPosCount;
		size_t lastIndex = currentPointIndex - 1;

		result.erase(result.begin() + beginIndex, result.begin() + lastIndex);
	}

	if (sameXPosCount > 1)
	{
		assert(currentPointIndex == result.size());

		size_t beginIndex = currentPointIndex - sameXPosCount;
		size_t lastIndex = currentPointIndex - 1;

		result.erase(result.begin() + beginIndex, result.begin() + lastIndex);
	}

	// Check points before return
	//

	for (currentPointIndex = 1; currentPointIndex < result.size(); currentPointIndex++)
	{
		VFrame30::SchemaPoint curPoint = result.at(currentPointIndex);
		VFrame30::SchemaPoint prevPoint = result.at(currentPointIndex - 1);

		// Points must be connected by X or Y axis. In other way - exception must be rised
		//

		assert((curPoint.X == prevPoint.X) ||
			   (curPoint.Y == prevPoint.Y));
	}

	return result;
}

std::list<VFrame30::SchemaPoint> EditSchemaWidget::removeUnwantedPoints(const std::list<VFrame30::SchemaPoint>& source) const
{
	std::vector<VFrame30::SchemaPoint> sourceVector(source.begin(), source.end());
	sourceVector = removeUnwantedPoints(sourceVector);

	std::list<VFrame30::SchemaPoint> result(sourceVector.begin(), sourceVector.end());
	return result;
}

bool EditSchemaWidget::loadAfbsDescriptions(std::vector<std::shared_ptr<Afb::AfbElement>>* out)
{
	if (out == nullptr)
	{
		assert(out);
		return false;
	}

	std::vector<DbFileInfo> fileList;

	bool result = db()->getFileList(&fileList, db()->afblFileId(), "afb", true, this);
	if (result == false)
	{
		return false;
	}

	if (fileList.empty() == true)
	{
		return true;
	}

	// Read all Afb's and refresh it in schema
	//
	std::vector<std::shared_ptr<DbFile>> files;
	result = db()->getLatestVersion(fileList, &files, this);
	if (result == false)
	{
		return false;
	}

	std::vector<std::shared_ptr<Afb::AfbElement>> elements;
	elements.reserve(files.size());

	for (std::shared_ptr<DbFile> f : files)
	{
		if (f->deleted() == true ||
			f->action() == VcsItemAction::Deleted)
		{
			continue;
		}

		std::shared_ptr<Afb::AfbElement> afb = std::make_shared<Afb::AfbElement>();

		QString errorMsg;
		result = afb->loadFromXml(f->data(), errorMsg);
		if (result == true)
		{
			elements.push_back(afb);
		}
		else
		{
			QString errorMsg = tr("Error parsing AFB %1 description. These items will not be updated.").arg(f->fileName());
			QMessageBox::critical(this, qApp->applicationName(), errorMsg);
			continue;
		}
	}

	std::swap(*out, elements);

	return true;
}

bool EditSchemaWidget::loadUfbSchemas(std::vector<std::shared_ptr<VFrame30::UfbSchema>>* out)
{
	if (out == nullptr)
	{
		assert(out);
		return false;
	}

	out->clear();

	// Get User Functional Block List
	//
	std::vector<DbFileInfo> fileList;

	bool ok = db()->getFileList(&fileList, db()->ufblFileId(), QString(".") + UfbFileExtension, true, this);
	if (ok == false)
	{
		return false;
	}

	// Get UFBs latest version from the DB
	//
	std::vector<std::shared_ptr<DbFile>> files;

	ok = db()->getLatestVersion(fileList, &files, this);

	if (ok == false)
	{
		return false;
	}

	// Parse files, create actual UFBs
	//
	std::vector<std::shared_ptr<VFrame30::UfbSchema>> ufbs;
	ufbs.reserve(files.size());

	for (const std::shared_ptr<DbFile>& f : files)
	{
		if (f->deleted() == true ||
			f->action() == VcsItemAction::Deleted)
		{
			continue;
		}

		std::shared_ptr<VFrame30::Schema> s = VFrame30::Schema::Create(f->data());

		if (s == nullptr)
		{
			assert(s);
			continue;
		}

		if (s->isUfbSchema() == false)
		{
			assert(s->isUfbSchema() == true);
			continue;
		}

		std::shared_ptr<VFrame30::UfbSchema> u =  std::dynamic_pointer_cast<VFrame30::UfbSchema>(s);
		if (u == nullptr)
		{
			assert(u);
			continue;
		}

		ufbs.push_back(u);

		qDebug() << u->schemaId() << " " << u->version();
	}

	std::swap(ufbs, *out);

	return true;
}

void EditSchemaWidget::resetAction()
{
	setMouseState(MouseState::None);
	editSchemaView()->m_newItem.reset();

	editSchemaView()->m_mouseSelectionStartPoint = QPoint();
	editSchemaView()->m_mouseSelectionEndPoint = QPoint();
	editSchemaView()->m_editStartDocPt = QPointF();
	editSchemaView()->m_editEndDocPt = QPointF();

	editSchemaView()->m_movingVertexPoints.clear();

	setMouseCursor(mapFromGlobal(QCursor::pos()));

	editSchemaView()->update();

	return;
}

void EditSchemaWidget::clearSelection()
{
	editSchemaView()->clearSelection();
}

void EditSchemaWidget::contextMenu(const QPoint& pos)
{
	if (mouseState() == MouseState::AddSchemaPosConnectionNextPoint)
	{
		// Don't show context menu, because it's right click for adding next point to connection line
		//
		return;
	}

	// If there is some action, don't show context menu
	//
	if (mouseState() != MouseState::None)
	{
		resetAction();
		setMouseCursor(pos);

		return;
	}

	// Disable some actions in ReadOnly mode
	//
	m_addAction->setDisabled(readOnly());

	// Version Control enable/disable items
	//
	m_fileSaveAction->setEnabled(readOnly() == false && modified() == true);
	m_fileCheckInAction->setEnabled(readOnly() == false && fileInfo().state() == VcsState::CheckedOut);
	m_fileCheckOutAction->setEnabled(readOnly() == true && fileInfo().state() == VcsState::CheckedIn);
	m_fileUndoChangesAction->setEnabled(readOnly() == false && fileInfo().state() == VcsState::CheckedOut);
	m_fileExportAction->setEnabled(true);
	m_fileImportAction->setEnabled(readOnly() == false && fileInfo().state() == VcsState::CheckedOut);

	m_propertiesAction->setDisabled(editSchemaView()->selectedItems().empty());

	// Compose menu
	//
	QMenu menu(this);

	QList<QAction*> actions;

	actions << m_fileAction;
	actions << m_addAction;
	actions << m_editAction;
	actions << m_orderAction;
	actions << m_sizeAndPosAction;

	// Signal properties
	//
	if (isLogicSchema() == true)
	{
		QSet<QString> signalStrIds;		// QSet for unique strIds

		if (selectedItems().empty() == false)
		{
			auto& selected = selectedItems();

			for (auto item : selected)
			{
				if (dynamic_cast<VFrame30::SchemaItemSignal*>(item.get()) != nullptr)
				{
					auto itemSignal = dynamic_cast<VFrame30::SchemaItemSignal*>(item.get());
					assert(itemSignal);

					const QStringList& signalStrIdList = itemSignal->appSignalIdList();

					for (const QString& s : signalStrIdList)
					{
						if (s.isEmpty() == false)
						{
							signalStrIds << s;
						}
					}
				}

				if (dynamic_cast<VFrame30::SchemaItemReceiver*>(item.get()) != nullptr)
				{
					VFrame30::SchemaItemReceiver* itemReceiver = dynamic_cast<VFrame30::SchemaItemReceiver*>(item.get());
					assert(itemReceiver);

					const QString& appSignal = itemReceiver->appSignalId();

					if (appSignal.isEmpty() == false)
					{
						signalStrIds << appSignal;
					}
				}
			}

			if (signalStrIds.empty() == false)
			{
				QAction* signalSeparator = new QAction(tr("Signals"), &menu);
				signalSeparator->setSeparator(true);
				actions << signalSeparator;

				for (QString s : signalStrIds)
				{
					QAction* signalAction = new QAction(s, &menu);
					connect(signalAction, &QAction::triggered,
							[s, this](bool)
							{
								QStringList sl;
								sl << s;
								this->signalsProperties(sl);
							});

					actions << signalAction;
				}

				if (signalStrIds.size() > 1)
				{
					QAction* allSignals = new QAction(tr("All Signals %1 Properties...").arg(signalStrIds.size()), &menu);
					connect(allSignals, &QAction::triggered,
							[signalStrIds, this](bool)
							{
								QStringList sl;
								for (auto s : signalStrIds)
								{
									sl << s;
								}

								this->signalsProperties(sl);
							});

					actions << allSignals;
				}
			}
		}
	}

	// Add new Application Logic signal
	//
	if (isLogicSchema() == true)
	{
		if (selectedItems().size() == 1)
		{
			std::shared_ptr<VFrame30::SchemaItem> selected = selectedItems().front();

			auto itemSignal = dynamic_cast<VFrame30::SchemaItemSignal*>(selected.get());

			if (itemSignal != nullptr)
			{
				QAction* addSignal = new QAction(tr("Add New App Signal..."), &menu);

				connect(addSignal, &QAction::triggered,
					[this, selected](bool)
					{
						this->addNewAppSignal(selected);
					});

				actions << addSignal;

			}
			else
			{
				// it is not VFrame30::SchemaItemSignal
				//
			}
		}
	}

	// --
	//
	QAction* separatorCommentFind = new QAction(&menu);
	separatorCommentFind->setSeparator(true);

	actions << separatorCommentFind;
	actions << m_toggleCommentAction;
	actions << m_lockAction;
	actions << m_findAction;

	// Layer, Item property etc
	//
	actions << m_separatorAction0;
	actions << m_layersAction;

	if (compareWidget() == true)
	{
		actions << m_compareDiffAction;
	}

	actions << m_propertiesAction;

	menu.exec(actions, mapToGlobal(pos), 0, this);
	return;
}

void EditSchemaWidget::exportToPdf()
{
	assert(schema());

	QString fileName = QFileDialog::getSaveFileName(
		this, "Export schema to PDF", schema()->schemaId() + ".pdf", "PDF (*.pdf);;All files (*.*)");

	if (fileName.isEmpty())
	{
		return;
	}

	qDebug() << "Export schema " << schema()->caption() << " " << schema()->schemaId() << " to PDF, " << fileName;

	editSchemaView()->exportToPdf(fileName);

	return;
}

void EditSchemaWidget::signalsProperties(QStringList strIds)
{
	if (isLogicSchema() == false)
	{
		assert(isLogicSchema() == false);
		return;
	}

	if (strIds.isEmpty() == true)
	{
		return;
	}

	std::vector<std::pair<QString, QString>> result = ::editApplicationSignals(strIds, db(), this);

	std::map<QString, QString> newIdsMap;
	for (auto& p : result)
	{
		if (p.first != p.second)
		{
			newIdsMap[p.first] = p.second;
		}
	}

	// in editApplicationSignals AppSignalIds could be changed,
	// update them in selected items, apparently this function was called for selected items.
	//

	// !!! Make a copy of selectted items!!!!!
	// As in this loop runSetProperty is called and selection vector is changed,
	// and this loop will crash if it is not a copy
	//
	std::vector<std::shared_ptr<VFrame30::SchemaItem>> selected = selectedItems();

	for (auto item : selected)
	{
		if (dynamic_cast<VFrame30::SchemaItemSignal*>(item.get()) != nullptr)
		{
			auto itemSignal = dynamic_cast<VFrame30::SchemaItemSignal*>(item.get());
			assert(itemSignal);

			QStringList signalStrIdList = itemSignal->appSignalIdList();
			bool itemsSignalsWereChanged = false;

			for (QString& appSignalId : signalStrIdList)
			{
				auto foundInChanged = newIdsMap.find(appSignalId);

				if (foundInChanged != newIdsMap.end())
				{
					// AppSignalIdWasChanged
					//
					appSignalId = foundInChanged->second;		// appSignalId is a reference
					itemsSignalsWereChanged = true;
				}
			}

			if (itemsSignalsWereChanged == true)
			{
				QString oneStringIds;
				for (const QString& s : signalStrIdList)
				{
					oneStringIds += s + QChar::LineFeed;
				}

				std::shared_ptr<VFrame30::SchemaItem> itemPtrCopy(item);

				m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalIDs, QVariant(oneStringIds), itemPtrCopy);
			}

			continue;
		}

		if (dynamic_cast<VFrame30::SchemaItemReceiver*>(item.get()) != nullptr)
		{
			auto itemReceiver = dynamic_cast<VFrame30::SchemaItemReceiver*>(item.get());
			assert(itemReceiver);

			QString appSignalId = itemReceiver->appSignalId();

			auto foundInChanged = newIdsMap.find(appSignalId);

			if (foundInChanged != newIdsMap.end())
			{
				// AppSignalIdWasChanged
				//
				m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalId, QVariant(foundInChanged->second), item);
			}
			continue;
		}
	}

	return;
}

void EditSchemaWidget::addNewAppSignal(std::shared_ptr<VFrame30::SchemaItem> schemaItem)
{
	if (isLogicSchema() == false ||
		schemaItem == nullptr ||
		dynamic_cast<VFrame30::SchemaItemSignal*>(schemaItem.get()) == nullptr)
	{
		assert(isLogicSchema() == false);
		assert(schemaItem);
		assert(dynamic_cast<VFrame30::SchemaItemSignal*>(schemaItem.get()) != nullptr);
		return;
	}

	QStringList equipmentIdList = logicSchema()->equipmentIdList();
	if (equipmentIdList.isEmpty() == true)
	{
		QMessageBox::critical(this, qAppName(), tr("Cannot create Application Signal as schema property EquipmentIDs is empty."));
		return;
	}

	qDebug() << equipmentIdList;

	int counterValue = 0;
	bool nextValRes = db()->nextCounterValue(&counterValue);
	if (nextValRes == false)
	{
		return;
	}

	QStringList signalsIds = SignalsTabPage::createSignal(db(),
														  equipmentIdList,
														  counterValue,
														  schema()->schemaId(),
														  schema()->caption(),
														  this);

	if (signalsIds.isEmpty() == false)
	{
		// Set value
		//
		QString oneStringIds;
		for (QString s : signalsIds)
		{
			oneStringIds += s + QChar::LineFeed;
		}

		m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalIDs, QVariant(oneStringIds), schemaItem);
	}

	return;
}

void EditSchemaWidget::escapeKey()
{
	if (mouseState() != MouseState::None)
	{
		resetAction();
	}
	else
	{
		editSchemaView()->clearSelection();
	}

	editSchemaView()->update();
	return;
}

void EditSchemaWidget::f2Key()
{
	if (mouseState() != MouseState::None)
	{
		return;
	}

	const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& selected = selectedItems();

	if (selected.size() != 1)
	{
		return;
	}

	std::shared_ptr<VFrame30::SchemaItem> item = selected.at(0);
	assert(item);

	VFrame30::SchemaItemSignal* itemSignal = dynamic_cast<VFrame30::SchemaItemSignal*>(item.get());
	VFrame30::SchemaItemReceiver* itemReceiver = dynamic_cast<VFrame30::SchemaItemReceiver*>(item.get());
	VFrame30::SchemaItemTransmitter* itemTransmitter = dynamic_cast<VFrame30::SchemaItemTransmitter*>(item.get());
	VFrame30::SchemaItemRect* itemRect = dynamic_cast<VFrame30::SchemaItemRect*>(item.get());

	if (itemRect != nullptr)
	{
		QString text = itemRect->text();

		// Show input dialog
		//
		QInputDialog inputDialog(this);

		inputDialog.setInputMode(QInputDialog::InputMode::TextInput);
		inputDialog.setWindowTitle("Set text");
		inputDialog.setLabelText(tr("Text:"));
		inputDialog.setTextEchoMode(QLineEdit::Normal);
		inputDialog.resize(400, inputDialog.height());
		inputDialog.setTextValue(text);

		int inputDialogRecult = inputDialog.exec();
		QString newValue = inputDialog.textValue();

		if (inputDialogRecult == QDialog::Accepted &&
			newValue.isNull() == false &&
			text != newValue)
		{
			m_editEngine->runSetProperty(VFrame30::PropertyNames::text, QVariant(newValue), item);
			editSchemaView()->update();
		}

		return;
	}

	if (itemSignal != nullptr || itemReceiver != nullptr)
	{
		QString appSignalId;

		if (itemSignal != nullptr)
		{
			appSignalId = itemSignal->appSignalIds();
		}

		if (itemReceiver != nullptr)
		{
			appSignalId = itemReceiver->appSignalId();
		}

		// Show input dialog
		//
		QInputDialog inputDialog(this);

		inputDialog.setInputMode(QInputDialog::InputMode::TextInput);
		inputDialog.setWindowTitle("Set AppSignalID");
		inputDialog.setLabelText(tr("AppSignalID:"));
		inputDialog.setTextEchoMode(QLineEdit::Normal);
		inputDialog.resize(400, inputDialog.height());
		inputDialog.setTextValue(appSignalId);

		int inputDialogRecult = inputDialog.exec();
		QString newValue = inputDialog.textValue();

		if (inputDialogRecult == QDialog::Accepted &&
			newValue.isNull() == false &&
			appSignalId != newValue)
		{
			// Set value
			//
			if (itemSignal != nullptr)
			{
				m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalIDs, QVariant(newValue), item);
			}

			if (itemReceiver != nullptr)
			{
				m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalId, QVariant(newValue), item);
			}

			editSchemaView()->update();
		}

		return;
	}

	if (itemTransmitter != nullptr)
	{
		QString connectionId = itemTransmitter->connectionId();

		// Show input dialog
		//
		QInputDialog inputDialog(this);

		inputDialog.setInputMode(QInputDialog::InputMode::TextInput);
		inputDialog.setWindowTitle("Set ConnectionID");
		inputDialog.setLabelText(tr("ConnectionID:"));
		inputDialog.setTextEchoMode(QLineEdit::Normal);
		inputDialog.resize(400, inputDialog.height());

		inputDialog.setTextValue(connectionId);

		int inputDialogRecult = inputDialog.exec();
		QString newValue = inputDialog.textValue();

		if (inputDialogRecult == QDialog::Accepted &&
			newValue.isNull() == false &&
			connectionId != newValue)
		{
			// Set value
			//
			m_editEngine->runSetProperty(VFrame30::PropertyNames::connectionId, QVariant(newValue), item);

			editSchemaView()->update();
		}

		return;
	}

	return;
}


void EditSchemaWidget::deleteKey()
{
	auto selection = editSchemaView()->selectedItems();

	if (mouseState() == MouseState::None &&
		selection.empty() == false)
	{
		m_editEngine->runDeleteItem(selection, activeLayer());
	}

	return;
}

void EditSchemaWidget::undo()
{
	m_editEngine->undo(1);

	if (m_schemaPropertiesDialog != nullptr && m_schemaPropertiesDialog->isVisible())
	{
		m_schemaPropertiesDialog->setSchema(schema());
	}
}

void EditSchemaWidget::redo()
{
	m_editEngine->redo(1);

	if (m_schemaPropertiesDialog != nullptr && m_schemaPropertiesDialog->isVisible())
	{
		m_schemaPropertiesDialog->setSchema(schema());
	}
}

void EditSchemaWidget::editEngineStateChanged(bool canUndo, bool canRedo)
{
	if (m_undoAction == nullptr ||
		m_redoAction == nullptr)
	{
		assert(m_undoAction);
		assert(m_redoAction);
		return;
	}

	m_undoAction->setEnabled(canUndo);
	m_redoAction->setEnabled(canRedo);

	return;
}

void EditSchemaWidget::modifiedChangedSlot(bool modified)
{
	m_fileSaveAction->setEnabled(modified);
	emit modifiedChanged(modified);
	return;
}

void EditSchemaWidget::selectAll()
{
	editSchemaView()->clearSelection();

	std::vector<std::shared_ptr<VFrame30::SchemaItem>> items;
	items.assign(editSchemaView()->activeLayer()->Items.begin(), editSchemaView()->activeLayer()->Items.end());

	editSchemaView()->setSelectedItems(items);

	editSchemaView()->update();
	return;
}

void EditSchemaWidget::selectItem(std::shared_ptr<VFrame30::SchemaItem> item)
{
	editSchemaView()->clearSelection();

	std::vector<std::shared_ptr<VFrame30::SchemaItem>> items;
	items.push_back(item);
	editSchemaView()->setSelectedItems(items);

	editSchemaView()->update();
	return;
}

void EditSchemaWidget::editCut()
{
	// Cut schema item(s) to clipboard
	//
	if (selectedItems().empty() == true)
	{
		return;
	}

	const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& selected = selectedItems();

	// Save to protobuf message
	//
	::Proto::EnvelopeSet message;
	for (std::shared_ptr<VFrame30::SchemaItem> si : selected)
	{
		::Proto::Envelope* protoSchemaItem = message.add_items();
		si->Save(protoSchemaItem);
	}

	std::string dataString;
	bool ok = message.SerializeToString(&dataString);

	if (ok == false)
	{
		assert(ok);
		return;
	}

	// Delete items, it a cut operation
	//
	m_editEngine->runDeleteItem(selected, editSchemaView()->activeLayer());

	// Set data to clipboard
	//
	QByteArray ba(dataString.data(), static_cast<int>(dataString.size()));

	if (ba.isEmpty() == false)
	{
		QClipboard* clipboard = QApplication::clipboard();

		QMimeData* mime = new QMimeData();
		mime->setData(SchemaItemClipboardData::mimeType, ba);

		clipboard->clear();
		clipboard->setMimeData(mime);
	}

	return;
}


void EditSchemaWidget::editCopy()
{
	// Copy schema item(s) to clipboard
	//
	if (selectedItems().empty() == true)
	{
		return;
	}

	const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& selected = selectedItems();

	// Save to protobuf message
	//
	::Proto::EnvelopeSet message;
	message.mutable_items()->Reserve(static_cast<int>(selected.size()));
	for (std::shared_ptr<VFrame30::SchemaItem> si : selected)
	{
		::Proto::Envelope* protoSchemaItem = message.add_items();
		si->Save(protoSchemaItem);
	}

	std::string dataString;
	bool ok = message.SerializeToString(&dataString);

	if (ok == false)
	{
		assert(ok);
		return;
	}

	// Set data to clipboard
	//
	QByteArray ba(dataString.data(), static_cast<int>(dataString.size()));

	if (ba.isEmpty() == false)
	{
		QClipboard* clipboard = QApplication::clipboard();

		QMimeData* mime = new QMimeData();
		mime->setData(SchemaItemClipboardData::mimeType, ba);

		clipboard->clear();
		clipboard->setMimeData(mime);
	}

	return;
}

void EditSchemaWidget::editPaste()
{
	const QClipboard* clipboard = QApplication::clipboard();
	const QMimeData* mimeData = clipboard->mimeData();

	if (mimeData == nullptr)
	{
		return;
	}

	// Paste schema items
	//
	if (mimeData->hasFormat(SchemaItemClipboardData::mimeType) == true)
	{
		QByteArray cbData = mimeData->data(SchemaItemClipboardData::mimeType);

		::Proto::EnvelopeSet message;
		bool ok = message.ParseFromArray(cbData.constData(), cbData.size());

		if (ok == false)
		{
			QMessageBox::critical(this, qApp->applicationName(),  tr("Clipboard has Schema Items, but it seems that data corrupted or data has incompatible format."));
			return;
		}

		std::list<std::shared_ptr<VFrame30::SchemaItem>> itemList;

		bool schemaItemAfbIsPresent = false;
		bool schemaItemUfbIsPresent = false;

		for (int i = 0; i < message.items_size(); i++)
		{
			const ::Proto::Envelope& schemaItemMessage = message.items(i);

			std::shared_ptr<VFrame30::SchemaItem> schemaItem = VFrame30::SchemaItem::Create(schemaItemMessage);

			if (schemaItem != nullptr)
			{
				schemaItem->setNewGuid();
				itemList.push_back(schemaItem);
			}

			if (schemaItem->isSchemaItemAfb() == true)
			{
				schemaItemAfbIsPresent = true;
			}

			if (schemaItem->isType<VFrame30::UfbSchema>() == true)
			{
				schemaItemUfbIsPresent = true;
			}

			if (schemaItem->isFblItemRect() == true)
			{
				// If items is FblItemRect set label to it
				//
				VFrame30::FblItemRect* fblItemRect = schemaItem->toFblItemRect();
				assert(fblItemRect);

				int counterValue = 0;
				bool nextValRes = db()->nextCounterValue(&counterValue);
				if (nextValRes == false)
				{
					return;
				}

				fblItemRect->setLabel(schema()->schemaId() + "_" + QString::number(counterValue));
			}
		}

		if (itemList.empty() == false)
		{
			m_editEngine->runAddItem(itemList, editSchemaView()->activeLayer());
		}

		// If new itesm has differeten afb/ufb description version
		// then they will be updated to the current version
		//
		if (schemaItemAfbIsPresent == true)
		{
			updateAfbsForSchema();
		}

		if (schemaItemUfbIsPresent == true)
		{
			updateUfbsForSchema();
		}

		return;
	}

	// Specific pastes
	//
	const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& selected = editSchemaView()->selectedItems();

	if (selected.empty() == true ||
		mimeData->hasText() == false)
	{
		return;
	}

	// Paste text to SchemaItemConst
	//
	bool allItemsAreConsts = true;

	bool okInteger = false;
	bool okFloat = false;

	int constInt = mimeData->text().toInt(&okInteger);
	double constFloat = mimeData->text().toDouble(&okFloat);

	std::vector<std::shared_ptr<VFrame30::SchemaItem>> constIntItems;
	constIntItems.reserve(selected.size());

	std::vector<std::shared_ptr<VFrame30::SchemaItem>> constFloatItems;
	constFloatItems.reserve(selected.size());

	for (std::shared_ptr<VFrame30::SchemaItem> item : selected)
	{
		VFrame30::SchemaItemConst* constItem = dynamic_cast<VFrame30::SchemaItemConst*>(item.get());

		if (constItem == nullptr)
		{
			allItemsAreConsts = false;
			break;
		}

		switch (constItem->type())
		{
		case VFrame30::SchemaItemConst::ConstType::IntegerlType:
			if (okInteger == true)
			{
				constIntItems.push_back(item);
			}
			break;

		case VFrame30::SchemaItemConst::ConstType::FloatType:
			if (okFloat == true)
			{
				constFloatItems.push_back(item);
			}
			break;

		default:
			assert(false);
			allItemsAreConsts = false;
		}
	}

	if (allItemsAreConsts == true)
	{
		if (okInteger == true && constIntItems.empty() == false)
		{
			m_editEngine->runSetProperty(VFrame30::PropertyNames::valueInteger, QVariant(constInt), constIntItems);
		}

		if (okFloat == true && constFloatItems.empty() == false)
		{
			m_editEngine->runSetProperty(VFrame30::PropertyNames::valueFloat, QVariant(constFloat), constFloatItems);
		}
	}

	// Paste text to SchemaItemRect
	//
	bool allItemsAreRects = true;
	for (std::shared_ptr<VFrame30::SchemaItem> item : selected)
	{
		if (dynamic_cast<VFrame30::SchemaItemRect*>(item.get()) == nullptr)
		{
			allItemsAreRects = false;
			break;
		}
	}

	if (allItemsAreRects == true)
	{
		m_editEngine->runSetProperty(VFrame30::PropertyNames::text, QVariant(mimeData->text()), selected);
	}

	// Paste appSignalID to SchemaItemSignal
	//
	bool allItemsAreSignals = true;
	for (std::shared_ptr<VFrame30::SchemaItem> item : selected)
	{
		if (dynamic_cast<VFrame30::SchemaItemSignal*>(item.get()) == nullptr)
		{
			allItemsAreSignals = false;
			break;
		}
	}

	if (allItemsAreSignals == true &&
		mimeData->text().startsWith('#') == true)
	{
		m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalIDs, QVariant(mimeData->text()), selected);
	}

	// Paste appSignalID to VFrame30::SchemaItemReceiver
	//
	bool allItemsAreReceivers = true;
	for (std::shared_ptr<VFrame30::SchemaItem> item : selected)
	{
		if (dynamic_cast<VFrame30::SchemaItemReceiver*>(item.get()) == nullptr)
		{
			allItemsAreReceivers = false;
			break;
		}
	}

	if (allItemsAreReceivers == true &&
		mimeData->text().startsWith('#') == true)
	{
		m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalId, QVariant(mimeData->text()), selected);
	}

	return;
}

void EditSchemaWidget::schemaProperties()
{
	if (m_schemaPropertiesDialog == nullptr)
	{
		m_schemaPropertiesDialog = new SchemaPropertiesDialog(m_editEngine, this);
	}

	m_schemaPropertiesDialog->setSchema(schema());
	m_schemaPropertiesDialog->show();
	return;
}

void EditSchemaWidget::properties()
{
	if (m_itemsPropertiesDialog == nullptr)
	{
		m_itemsPropertiesDialog = new SchemaItemPropertiesDialog(m_editEngine, this);
	}

	m_itemsPropertiesDialog->setObjects(editSchemaView()->selectedItems());
	m_itemsPropertiesDialog->show();
	return;
}

void EditSchemaWidget::layers()
{
	SchemaLayersDialog schemaLayersDialog(editSchemaView(), this);
	if (schemaLayersDialog.exec() == QDialog::Accepted)
	{

	}
	update();
	return;
}

void EditSchemaWidget::compareSchemaItem()
{
	if (editSchemaView()->m_compareWidget == false ||
		editSchemaView()->m_compareSourceSchema == nullptr ||
		editSchemaView()->m_compareTargetSchema == nullptr)
	{
		assert(editSchemaView()->m_compareWidget == true);
		assert(editSchemaView()->m_compareSourceSchema);
		assert(editSchemaView()->m_compareTargetSchema);
		return;
	}

	if (selectedItems().size() != 1)
	{
		assert(selectedItems().size() == 1);
		return;
	}

	std::shared_ptr<VFrame30::SchemaItem> selectedItem = selectedItems().front();
	if (selectedItem == nullptr)
	{
		assert(selectedItem);
		return;
	}

	std::shared_ptr<VFrame30::SchemaItem> sourceItem = editSchemaView()->m_compareSourceSchema->getItemById(selectedItem->guid());
	std::shared_ptr<VFrame30::SchemaItem> targetItem = editSchemaView()->m_compareTargetSchema->getItemById(selectedItem->guid());

	if (sourceItem == nullptr ||
		targetItem == nullptr)
	{
		return;
	}

	DbChangesetObject dbObject;	// Fake object, need to fill only name

	QString title;
	if (selectedItem->isFblItem() == true)
	{
		title = selectedItem->metaObject()->className() + QString(" ") + selectedItem->toFblItemRect()->label();
		title = title.remove("VFrame30::SchemaItem");
	}
	else
	{
		title = selectedItem->metaObject()->className();
		title = title.remove("VFrame30::SchemaItem");
	}
	dbObject.setName(title);

	CompareData cd = CompareData();
	ComparePropertyObjectDialog::showDialog(dbObject, cd, sourceItem, targetItem, this);

	return;
}

void EditSchemaWidget::selectionChanged()
{
	// Properties dialog
	//
	if (m_itemsPropertiesDialog == nullptr)
	{
		m_itemsPropertiesDialog = new SchemaItemPropertiesDialog(m_editEngine, this);
	}

	m_itemsPropertiesDialog->setObjects(editSchemaView()->selectedItems());

	const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& selected = editSchemaView()->selectedItems();

	// Edit Menu
	//
	int selectionCount = static_cast<int>(editSchemaView()->selectedItems().size());
	int lockedCound = 0;

	for (std::shared_ptr<VFrame30::SchemaItem> si : selected)
	{
		if (si->isLocked() == true)
		{
			lockedCound ++;
		}
	}

	m_deleteAction->setEnabled((selectionCount - lockedCound) > 0 && readOnly() == false);
	m_editCutAction->setEnabled((selectionCount - lockedCound) > 0 && readOnly() == false);
	m_editCopyAction->setEnabled(selectionCount > 0);

	// Allign
	//
	bool allowAlign = selectedItems().size() >= 2 && readOnly() == false;

	m_alignLeftAction->setEnabled(allowAlign);
	m_alignRightAction->setEnabled(allowAlign);
	m_alignTopAction->setEnabled(allowAlign);
	m_alignBottomAction->setEnabled(allowAlign);

	// Size
	//
	std::vector<std::shared_ptr<VFrame30::SchemaItem>> selectedFiltered;

	for (std::shared_ptr<VFrame30::SchemaItem> item : selected)
	{
		if (dynamic_cast<VFrame30::PosLineImpl*>(item.get()) != nullptr ||
			dynamic_cast<VFrame30::PosRectImpl*>(item.get()) != nullptr)
		{
			selectedFiltered.push_back(item);
		}
	}

	bool allowSize = selectedFiltered.size() >= 2 && readOnly() == false;

	m_sameWidthAction->setEnabled(allowSize);
	m_sameHeightAction->setEnabled(allowSize);
	m_sameSizeAction->setEnabled(allowSize);

	// Order
	//
	bool allowSetOrder = selectedItems().empty() == false  && readOnly() == false;
	m_bringToFrontAction->setEnabled(allowSetOrder);
	m_bringForwardAction->setEnabled(allowSetOrder);
	m_sendToBackAction->setEnabled(allowSetOrder);
	m_sendBackwardAction->setEnabled(allowSetOrder);

	// Comment Action
	//
	bool hasFblItems = false;
	for (auto& selItem : selected)
	{
		if (selItem->isFblItem() == true)
		{
			hasFblItems = true;
			break;
		}
	}

	m_toggleCommentAction->setEnabled(hasFblItems && readOnly() == false);

	// Lock action
	//
	m_lockAction->setEnabled(selected.empty() == false && readOnly() == false);

	// Compare SchemaItem
	//
	m_compareDiffAction->setEnabled(selected.size() == 1 && isCompareWidget() == true);

	// --
	//
	clipboardDataChanged();

	return;
}

void EditSchemaWidget::clipboardDataChanged()
{
	if (readOnly() == true)
	{
		m_editPasteAction->setEnabled(false);
		return;
	}

	const QClipboard* clipboard = QApplication::clipboard();
	const QMimeData* mimeData = clipboard->mimeData();

	if (mimeData == nullptr)
	{
		m_editPasteAction->setEnabled(false);
		return;
	}

	// if same SchemaItems in the clipboard
	//
	QStringList hasFormats = mimeData->formats();
	for (auto f : hasFormats)
	{
		if (f == SchemaItemClipboardData::mimeType)
		{
			m_editPasteAction->setEnabled(true);
			return;
		}
	}

	// Specific items cases
	//
	const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& selected = editSchemaView()->selectedItems();

	if (selected.empty() == true)
	{
		m_editPasteAction->setEnabled(false);
		return;
	}

	// All Items are SchemaItemConsts
	//
	bool allItemsAreConsts = true;

	bool okInteger = false;
	bool okFloat = false;

	mimeData->text().toInt(&okInteger);
	mimeData->text().toDouble(&okFloat);

	std::vector<std::shared_ptr<VFrame30::SchemaItem>> constIntItems;
	constIntItems.reserve(selected.size());

	std::vector<std::shared_ptr<VFrame30::SchemaItem>> constFloatItems;
	constFloatItems.reserve(selected.size());

	for (std::shared_ptr<VFrame30::SchemaItem> item : selected)
	{
		VFrame30::SchemaItemConst* constItem = dynamic_cast<VFrame30::SchemaItemConst*>(item.get());

		if (constItem == nullptr)
		{
			allItemsAreConsts = false;
			break;
		}

		switch (constItem->type())
		{
		case VFrame30::SchemaItemConst::ConstType::IntegerlType:
			if (okInteger == true)
			{
				constIntItems.push_back(item);
			}
			break;

		case VFrame30::SchemaItemConst::ConstType::FloatType:
			if (okFloat == true)
			{
				constFloatItems.push_back(item);
			}
			break;

		default:
			assert(false);
			allItemsAreConsts = false;
		}
	}

	if (allItemsAreConsts == true)
	{
		if (okInteger == true && constIntItems.empty() == false)
		{
			m_editPasteAction->setEnabled(true);
			return;
		}

		if (okFloat == true && constFloatItems.empty() == false)
		{
			m_editPasteAction->setEnabled(true);
			return;
		}
	}

	// if SchemaItemRect is selected and Text is in the clipboard
	//
	bool allItemsAreRects = true;
	for (std::shared_ptr<VFrame30::SchemaItem> item : selected)
	{
		if (dynamic_cast<VFrame30::SchemaItemRect*>(item.get()) == nullptr)
		{
			allItemsAreRects = false;
			break;
		}
	}

	if (allItemsAreRects == true &&
		mimeData->hasText() == true)
	{
		m_editPasteAction->setEnabled(true);
		return;
	}

	// if Any SchemaItemSignal is selected and AppSignalID is in the clipboard
	//
	bool allItemsAreSignals = true;
	for (std::shared_ptr<VFrame30::SchemaItem> item : selected)
	{
		if (dynamic_cast<VFrame30::SchemaItemSignal*>(item.get()) == nullptr)
		{
			allItemsAreSignals = false;
			break;
		}
	}

	if (allItemsAreSignals == true &&
		mimeData->hasText() == true &&
		mimeData->text().startsWith('#') == true)
	{
		m_editPasteAction->setEnabled(true);
		return;
	}

	// if Any SchemaItemReceiver is selected and AppSignalID is in the clipboard
	//
	bool allItemsAreReceivers = true;
	for (std::shared_ptr<VFrame30::SchemaItem> item : selected)
	{
		if (dynamic_cast<VFrame30::SchemaItemReceiver*>(item.get()) == nullptr)
		{
			allItemsAreReceivers = false;
			break;
		}
	}

	if (allItemsAreReceivers == true &&
		mimeData->hasText() == true &&
		mimeData->text().startsWith('#') == true)
	{
		m_editPasteAction->setEnabled(true);
		return;
	}

	// --
	//
	m_editPasteAction->setEnabled(false);

	return;
}

void EditSchemaWidget::addFblElement()
{
	// Get Afb descriptions
	//
	std::vector<std::shared_ptr<Afb::AfbElement>> afbs;
	bool ok = loadAfbsDescriptions(&afbs);

	if (ok == false)
	{
		return;
	}

	// --
	//
	ChooseAfbDialog* dialog = new ChooseAfbDialog(afbs, this);

	if (dialog->exec() == QDialog::Accepted)
	{
		int index = dialog->index();

		if (index < 0 || static_cast<size_t>(index) >= afbs.size())
		{
			assert(false);
			return;
		}

		std::shared_ptr<Afb::AfbElement> afb = afbs[index];

		QString errorMsg;
		addItem(std::make_shared<VFrame30::SchemaItemAfb>(schema()->unit(), *(afb.get()), &errorMsg));

		if (errorMsg.isEmpty() == false)
		{
			QMessageBox::critical(this, QObject::tr("Error"), errorMsg);
		}
	}

	return;
}

void EditSchemaWidget::addUfbElement()
{
	// Get Schemas
	//
	std::vector<std::shared_ptr<VFrame30::UfbSchema>> ufbs;

	bool ok = loadUfbSchemas(&ufbs);

	if (ok == false)
	{
		return;
	}

	// Choose User Functional Block
	//
	ChooseUfbDialog dialog(ufbs, this);

	if (dialog.exec() == QDialog::Accepted)
	{
		std::shared_ptr<VFrame30::UfbSchema> ufb = dialog.result();
		assert(ufb);

		qDebug() << "UserFunctionalBlock selected " << ufb->caption();

		QString errorMsg;
		addItem(std::make_shared<VFrame30::SchemaItemUfb>(schema()->unit(), ufb.get(), &errorMsg));

		if (errorMsg.isEmpty() == false)
		{
			QMessageBox::critical(this, QObject::tr("Error"), errorMsg);
		}
	}

	return;
}

void EditSchemaWidget::onLeftKey()
{
	if (selectedItems().empty() == true)
	{
		return;
	}

	const auto& selected = selectedItems();

	std::vector<std::shared_ptr<VFrame30::SchemaItem>> itemsForMove;
	itemsForMove.reserve(selected.size());

	for (auto& item : selected)
	{
		if (item->isLocked() == false)
		{
			itemsForMove.push_back(item);
		}
	}

	if (itemsForMove.empty() == false)
	{
		double dif = -schemaView()->schema()->gridSize();

		m_editEngine->runMoveItem(dif, 0, itemsForMove, snapToGrid());
	}

	return;
}

void EditSchemaWidget::onRightKey()
{
	if (selectedItems().empty() == true)
	{
		return;
	}

	const auto& selected = selectedItems();

	std::vector<std::shared_ptr<VFrame30::SchemaItem>> itemsForMove;
	itemsForMove.reserve(selected.size());

	for (auto& item : selected)
	{
		if (item->isLocked() == false)
		{
			itemsForMove.push_back(item);
		}
	}

	if (itemsForMove.empty() == false)
	{
		double dif = schemaView()->schema()->gridSize();

		m_editEngine->runMoveItem(dif, 0, itemsForMove, snapToGrid());
	}

	return;
}

void EditSchemaWidget::onUpKey()
{
	if (selectedItems().empty() == true)
	{
		return;
	}

	const auto& selected = selectedItems();

	std::vector<std::shared_ptr<VFrame30::SchemaItem>> itemsForMove;
	itemsForMove.reserve(selected.size());

	for (auto& item : selected)
	{
		if (item->isLocked() == false)
		{
			itemsForMove.push_back(item);
		}
	}

	if (itemsForMove.empty() == false)
	{
		double dif = -schemaView()->schema()->gridSize();

		m_editEngine->runMoveItem(0, dif, itemsForMove, snapToGrid());
	}

	return;
}

void EditSchemaWidget::onDownKey()
{
	if (selectedItems().empty() == true)
	{
		return;
	}

	const auto& selected = selectedItems();

	std::vector<std::shared_ptr<VFrame30::SchemaItem>> itemsForMove;
	itemsForMove.reserve(selected.size());

	for (auto& item : selected)
	{
		if (item->isLocked() == false)
		{
			itemsForMove.push_back(item);
		}
	}

	if (itemsForMove.empty() == false)
	{
		double dif = schemaView()->schema()->gridSize();

		m_editEngine->runMoveItem(0, dif, itemsForMove, snapToGrid());
	}

	return;
}

void EditSchemaWidget::sameWidth()
{
	const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& selected = selectedItems();

	// Same Width/Height works only for usual lines and rectangles,filter connection line
	//
	std::vector<std::shared_ptr<VFrame30::SchemaItem>> selectedFiltered;

	for (auto item : selected)
	{
		if (dynamic_cast<VFrame30::PosLineImpl*>(item.get()) != nullptr ||
			dynamic_cast<VFrame30::PosRectImpl*>(item.get()) != nullptr)
		{
			selectedFiltered.push_back(item);
		}
	}

	if (selectedFiltered.size() < 2)
	{
		assert(selectedFiltered.size() >= 2);
		return;
	}

	std::shared_ptr<VFrame30::SchemaItem> firstItem = selectedFiltered.at(0);

	// find the most left and most right points
	//
	std::vector<VFrame30::SchemaPoint> firstItemPoints = firstItem->getPointList();

	if (firstItemPoints.empty() == true)
	{
		assert(firstItemPoints.empty() == false);
		return;
	}

	VFrame30::SchemaPoint leftPoint = firstItemPoints[0];
	VFrame30::SchemaPoint rightPoint = firstItemPoints[0];

	for (const VFrame30::SchemaPoint& pt : firstItemPoints)
	{
		if (pt.X < leftPoint.X)
		{
			leftPoint = pt;
		}

		if (pt.X > rightPoint.X)
		{
			rightPoint = pt;
		}
	}

	double width = rightPoint.X - leftPoint.X;

	// get new points for all items
	//
	std::vector<std::vector<VFrame30::SchemaPoint>> newPoints;
	for (std::shared_ptr<VFrame30::SchemaItem> item : selectedFiltered)
	{
		std::vector<VFrame30::SchemaPoint> points = item->getPointList();
		assert(points.empty() == false);

		if (dynamic_cast<VFrame30::PosLineImpl*>(item.get()) != nullptr)
		{
			assert(points.size() == 2);

			if (points[0].X < points[1].X)
			{
				points[1].X = points[0].X + width;
			}
			else
			{
				points[0].X = points[1].X + width;
			}
		}

		if (dynamic_cast<VFrame30::PosRectImpl*>(item.get()) != nullptr)
		{
			assert(points.size() == 2);

			if (points.size() == 2)
			{
				points[1].X = points[0].X + width;
			}
		}

		newPoints.push_back(points);
	}

	m_editEngine->runSetPoints(newPoints, selectedFiltered);

	return;
}

void EditSchemaWidget::sameHeight()
{
	const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& selected = selectedItems();

	// Same Width/Height works only for usual lines and rectangles,filter connection line
	//
	std::vector<std::shared_ptr<VFrame30::SchemaItem>> selectedFiltered;

	for (auto item : selected)
	{
		if (dynamic_cast<VFrame30::PosLineImpl*>(item.get()) != nullptr ||
			dynamic_cast<VFrame30::PosRectImpl*>(item.get()) != nullptr)
		{
			selectedFiltered.push_back(item);
		}
	}

	if (selectedFiltered.size() < 2)
	{
		assert(selectedFiltered.size() >= 2);
		return;
	}

	std::shared_ptr<VFrame30::SchemaItem> firstItem = selectedFiltered.at(0);

	// find the top and bottom points
	//
	std::vector<VFrame30::SchemaPoint> firstItemPoints = firstItem->getPointList();

	if (firstItemPoints.empty() == true)
	{
		assert(firstItemPoints.empty() == false);
		return;
	}

	VFrame30::SchemaPoint topPoint = firstItemPoints[0];
	VFrame30::SchemaPoint bottomPoint = firstItemPoints[0];

	for (const VFrame30::SchemaPoint& pt : firstItemPoints)
	{
		if (pt.Y < topPoint.Y)
		{
			topPoint = pt;
		}

		if (pt.Y > bottomPoint.Y)
		{
			bottomPoint = pt;
		}
	}

	double height = bottomPoint.Y - topPoint.Y;

	// get new points for all items
	//
	std::vector<std::vector<VFrame30::SchemaPoint>> newPoints;
	for (std::shared_ptr<VFrame30::SchemaItem> item : selectedFiltered)
	{
		std::vector<VFrame30::SchemaPoint> points = item->getPointList();
		assert(points.empty() == false);

		if (dynamic_cast<VFrame30::PosLineImpl*>(item.get()) != nullptr)
		{
			assert(points.size() == 2);

			if (points[0].Y < points[1].Y)
			{
				points[1].Y = points[0].Y + height;
			}
			else
			{
				points[0].Y = points[1].Y + height;
			}
		}

		if (dynamic_cast<VFrame30::PosRectImpl*>(item.get()) != nullptr)
		{
			assert(points.size() == 2);

			if (points.size() == 2)
			{
				points[1].Y = points[0].Y + height;
			}
		}

		newPoints.push_back(points);
	}

	m_editEngine->runSetPoints(newPoints, selectedFiltered);

	return;
}

void EditSchemaWidget::sameSize()
{
	const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& selected = selectedItems();

	// Same Width/Height works only for usual lines and rectangles,filter connection line
	//
	std::vector<std::shared_ptr<VFrame30::SchemaItem>> selectedFiltered;

	for (auto item : selected)
	{
		if (dynamic_cast<VFrame30::PosLineImpl*>(item.get()) != nullptr ||
			dynamic_cast<VFrame30::PosRectImpl*>(item.get()) != nullptr)
		{
			selectedFiltered.push_back(item);
		}
	}

	if (selectedFiltered.size() < 2)
	{
		assert(selectedFiltered.size() >= 2);
		return;
	}

	std::shared_ptr<VFrame30::SchemaItem> firstItem = selectedFiltered.at(0);

	// find the most left and most right points
	//
	std::vector<VFrame30::SchemaPoint> firstItemPoints = firstItem->getPointList();

	if (firstItemPoints.empty() == true)
	{
		assert(firstItemPoints.empty() == false);
		return;
	}

	VFrame30::SchemaPoint leftPoint = firstItemPoints[0];
	VFrame30::SchemaPoint rightPoint = firstItemPoints[0];

	VFrame30::SchemaPoint topPoint = firstItemPoints[0];
	VFrame30::SchemaPoint bottomPoint = firstItemPoints[0];

	for (const VFrame30::SchemaPoint& pt : firstItemPoints)
	{
		if (pt.X < leftPoint.X)
		{
			leftPoint = pt;
		}

		if (pt.X > rightPoint.X)
		{
			rightPoint = pt;
		}

		if (pt.Y < topPoint.Y)
		{
			topPoint = pt;
		}

		if (pt.Y > bottomPoint.Y)
		{
			bottomPoint = pt;
		}
	}

	double width = rightPoint.X - leftPoint.X;
	double height = bottomPoint.Y - topPoint.Y;

	// get new points for all items
	//
	std::vector<std::vector<VFrame30::SchemaPoint>> newPoints;
	for (std::shared_ptr<VFrame30::SchemaItem> item : selectedFiltered)
	{
		std::vector<VFrame30::SchemaPoint> points = item->getPointList();
		assert(points.empty() == false);

		if (dynamic_cast<VFrame30::PosLineImpl*>(item.get()) != nullptr)
		{
			assert(points.size() == 2);

			if (points[0].X < points[1].X)
			{
				points[1].X = points[0].X + width;
			}
			else
			{
				points[0].X = points[1].X + width;
			}

			if (points[0].Y < points[1].Y)
			{
				points[1].Y = points[0].Y + height;
			}
			else
			{
				points[0].Y = points[1].Y + height;
			}
		}

		if (dynamic_cast<VFrame30::PosRectImpl*>(item.get()) != nullptr)
		{
			assert(points.size() == 2);

			if (points.size() == 2)
			{
				points[1].X = points[0].X + width;
			}

			if (points.size() == 2)
			{
				points[1].Y = points[0].Y + height;
			}
		}

		// --
		//
		newPoints.push_back(points);
	}

	m_editEngine->runSetPoints(newPoints, selectedFiltered);

	return;
}

void EditSchemaWidget::alignLeft()
{
	const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& selected = selectedItems();

	if (selected.size() < 2)
	{
		assert(selected.size() >= 2);
		return;
	}

	std::shared_ptr<VFrame30::SchemaItem> firstItem = selected.at(0);

	// find the most left point
	//
	std::vector<VFrame30::SchemaPoint> firstItemPoints = firstItem->getPointList();

	if (firstItemPoints.empty() == true)
	{
		assert(firstItemPoints.empty() == false);
		return;
	}

	VFrame30::SchemaPoint firstItemMostLeftPoint = firstItemPoints[0];
	for (const VFrame30::SchemaPoint& pt : firstItemPoints)
	{
		if (pt.X < firstItemMostLeftPoint.X)
		{
			firstItemMostLeftPoint = pt;
		}
	}

	// get new points for all items
	//
	std::vector<std::vector<VFrame30::SchemaPoint>> newPoints;
	for (std::shared_ptr<VFrame30::SchemaItem> item : selected)
	{
		std::vector<VFrame30::SchemaPoint> points = item->getPointList();
		assert(points.empty() == false);

		// find the most left point for the item
		//
		VFrame30::SchemaPoint itemMostLeftPoint = points[0];
		for (const VFrame30::SchemaPoint& pt : points)
		{
			if (pt.X < itemMostLeftPoint.X)
			{
				itemMostLeftPoint = pt;
			}
		}

		double diff = firstItemMostLeftPoint.X - itemMostLeftPoint.X;

		for (VFrame30::SchemaPoint& pt : points)
		{
			pt.X += diff;
		}

		newPoints.push_back(points);
	}

	m_editEngine->runSetPoints(newPoints, selected);

	return;
}

void EditSchemaWidget::alignRight()
{
	const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& selected = selectedItems();

	if (selected.size() < 2)
	{
		assert(selected.size() >= 2);
		return;
	}

	std::shared_ptr<VFrame30::SchemaItem> firstItem = selected.at(0);

	// find the most right point
	//
	std::vector<VFrame30::SchemaPoint> firstItemPoints = firstItem->getPointList();

	if (firstItemPoints.empty() == true)
	{
		assert(firstItemPoints.empty() == false);
		return;
	}

	VFrame30::SchemaPoint firstItemMostRightPoint = firstItemPoints[0];
	for (const VFrame30::SchemaPoint& pt : firstItemPoints)
	{
		if (pt.X > firstItemMostRightPoint.X)
		{
			firstItemMostRightPoint = pt;
		}
	}

	// get new points for all items
	//
	std::vector<std::vector<VFrame30::SchemaPoint>> newPoints;
	for (std::shared_ptr<VFrame30::SchemaItem> item : selected)
	{
		std::vector<VFrame30::SchemaPoint> points = item->getPointList();
		assert(points.empty() == false);

		// find the most right point for the item
		//
		VFrame30::SchemaPoint itemMostRightPoint = points[0];
		for (const VFrame30::SchemaPoint& pt : points)
		{
			if (pt.X > itemMostRightPoint.X)
			{
				itemMostRightPoint = pt;
			}
		}

		double diff = itemMostRightPoint.X - firstItemMostRightPoint.X;

		for (VFrame30::SchemaPoint& pt : points)
		{
			pt.X -= diff;
		}

		newPoints.push_back(points);
	}

	m_editEngine->runSetPoints(newPoints, selected);

	return;
}

void EditSchemaWidget::alignTop()
{
	const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& selected = selectedItems();

	if (selected.size() < 2)
	{
		assert(selected.size() >= 2);
		return;
	}

	std::shared_ptr<VFrame30::SchemaItem> firstItem = selected.at(0);

	// find the most top point
	//
	std::vector<VFrame30::SchemaPoint> firstItemPoints = firstItem->getPointList();

	if (firstItemPoints.empty() == true)
	{
		assert(firstItemPoints.empty() == false);
		return;
	}

	VFrame30::SchemaPoint firstItemMostTopPoint = firstItemPoints[0];
	for (const VFrame30::SchemaPoint& pt : firstItemPoints)
	{
		if (pt.Y < firstItemMostTopPoint.Y)
		{
			firstItemMostTopPoint = pt;
		}
	}

	// get new points for all items
	//
	std::vector<std::vector<VFrame30::SchemaPoint>> newPoints;
	for (std::shared_ptr<VFrame30::SchemaItem> item : selected)
	{
		std::vector<VFrame30::SchemaPoint> points = item->getPointList();
		assert(points.empty() == false);

		// find the most top point for the item
		//
		VFrame30::SchemaPoint itemMostTopPoint = points[0];
		for (const VFrame30::SchemaPoint& pt : points)
		{
			if (pt.Y < itemMostTopPoint.Y)
			{
				itemMostTopPoint = pt;
			}
		}

		double diff = firstItemMostTopPoint.Y - itemMostTopPoint.Y;

		for (VFrame30::SchemaPoint& pt : points)
		{
			pt.Y += diff;
		}

		newPoints.push_back(points);
	}

	m_editEngine->runSetPoints(newPoints, selected);

	return;
}

void EditSchemaWidget::alignBottom()
{
	const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& selected = selectedItems();

	if (selected.size() < 2)
	{
		assert(selected.size() >= 2);
		return;
	}

	std::shared_ptr<VFrame30::SchemaItem> firstItem = selected.at(0);

	// find the most bottom point
	//
	std::vector<VFrame30::SchemaPoint> firstItemPoints = firstItem->getPointList();

	if (firstItemPoints.empty() == true)
	{
		assert(firstItemPoints.empty() == false);
		return;
	}

	VFrame30::SchemaPoint firstItemMostBottomPoint = firstItemPoints[0];
	for (const VFrame30::SchemaPoint& pt : firstItemPoints)
	{
		if (pt.Y > firstItemMostBottomPoint.Y)
		{
			firstItemMostBottomPoint = pt;
		}
	}

	// get new points for all items
	//
	std::vector<std::vector<VFrame30::SchemaPoint>> newPoints;
	for (std::shared_ptr<VFrame30::SchemaItem> item : selected)
	{
		std::vector<VFrame30::SchemaPoint> points = item->getPointList();
		assert(points.empty() == false);

		// find the most bottom point for the item
		//
		VFrame30::SchemaPoint itemMostBottomPoint = points[0];
		for (const VFrame30::SchemaPoint& pt : points)
		{
			if (pt.Y > itemMostBottomPoint.Y)
			{
				itemMostBottomPoint = pt;
			}
		}

		double diff = itemMostBottomPoint.Y - firstItemMostBottomPoint.Y;

		for (VFrame30::SchemaPoint& pt : points)
		{
			pt.Y -= diff;
		}

		newPoints.push_back(points);
	}

	m_editEngine->runSetPoints(newPoints, selected);

	return;
}

void EditSchemaWidget::bringToFront()
{
	const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& selected = selectedItems();
	m_editEngine->runSetOrder(EditEngine::SetOrder::BringToFront, selected, activeLayer());
}

void EditSchemaWidget::bringForward()
{
	const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& selected = selectedItems();
	m_editEngine->runSetOrder(EditEngine::SetOrder::BringForward, selected, activeLayer());
}

void EditSchemaWidget::sendToBack()
{
	const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& selected = selectedItems();
	m_editEngine->runSetOrder(EditEngine::SetOrder::SendToBack, selected, activeLayer());
}

void EditSchemaWidget::sendBackward()
{
	const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& selected = selectedItems();
	m_editEngine->runSetOrder(EditEngine::SetOrder::SendBackward, selected, activeLayer());
}

void EditSchemaWidget::toggleComment()
{
	qDebug() << "EditSchemaWidget::toggleComment()";

	if (selectedItems().empty() == true)
	{
		return;
	}

	// Only FblItems can be commented
	//
	bool hasCommented = false;
	bool hasUncommented = false;

	const auto& selected = selectedItems();

	for (auto& selItem : selected)
	{
		if (selItem->isFblItem() == true)
		{
			if (selItem->isCommented() == true)
			{
				hasCommented = true;
			}
			else
			{
				hasUncommented = true;
			}
		}
	}

	if (hasUncommented == true)
	{
		// Comment all
		//
		std::vector<std::shared_ptr<VFrame30::SchemaItem>> items;
		items.reserve(selected.size());

		for (auto& selItem : selected)
		{
			if (selItem->isFblItem() == true)
			{
				items.push_back(selItem);
			}
		}

		m_editEngine->runSetProperty(VFrame30::PropertyNames::commented, QVariant(true), items);
		return;
	}

	if (hasCommented == true)
	{
		// Uncomment all
		//
		std::vector<std::shared_ptr<VFrame30::SchemaItem>> items;
		items.reserve(selected.size());

		for (auto& selItem : selected)
		{
			if (selItem->isFblItem() == true)
			{
				items.push_back(selItem);
			}
		}

		m_editEngine->runSetProperty(VFrame30::PropertyNames::commented, QVariant(false), items);
		return;
	}

	return;
}

void EditSchemaWidget::toggleLock()
{
	qDebug() << "EditSchemaWidget::toggleLock()";

	if (selectedItems().empty() == true)
	{
		return;
	}

	// Only FblItems can be commented
	//
	bool hasLocked = false;
	bool hasUnlocked = false;

	const auto& selected = selectedItems();

	for (auto& selItem : selected)
	{
		if (selItem->isLocked() == true)
		{
			hasLocked = true;
		}
		else
		{
			hasUnlocked = true;
		}
	}

	if (hasUnlocked == true)
	{
		// Lock all
		//
		m_editEngine->runSetProperty(VFrame30::PropertyNames::locked, QVariant(true), selected);
		return;
	}

	if (hasLocked == true)
	{
		// Unlock all
		//
		m_editEngine->runSetProperty(VFrame30::PropertyNames::locked, QVariant(false), selected);
		return;
	}

	return;
}

void EditSchemaWidget::find()
{
	if (m_findDialog == nullptr)
	{
		m_findDialog = new SchemaFindDialog(this);

		connect(m_findDialog, &SchemaFindDialog::findPrev, this, &EditSchemaWidget::findPrev);
		connect(m_findDialog, &SchemaFindDialog::findNext, this, &EditSchemaWidget::findNext);
	}

	m_findDialog->show();
	m_findDialog->raise();
	m_findDialog->activateWindow();

	return;
}

void EditSchemaWidget::findNext()
{
	if (m_findDialog == nullptr)
	{
		find();
		return;
	}

	QString searchText = m_findDialog->findText();

	if (searchText.isEmpty() == true)
	{
		m_findDialog->show();
		m_findDialog->raise();
		m_findDialog->activateWindow();

		m_findDialog->setFocusToEditLine();
		return;
	}

	m_findDialog->updateCompleter();

	// Look for text
	//
	std::shared_ptr<VFrame30::SchemaLayer> layer = activeLayer();
	assert(layer);

	if (layer->Items.empty() == true)
	{
		clearSelection();
		return;
	}

	auto& selected = selectedItems();		// Keep reference!!!!

	// Get start iterator
	//
	auto searchStartIterator = layer->Items.begin();
	if (selected.size() != 1)
	{
		searchStartIterator = layer->Items.begin();
	}
	else
	{
		assert(selected.size() == 1);

		searchStartIterator = std::find(layer->Items.begin(), layer->Items.end(), selected.front());
		if (searchStartIterator == layer->Items.end())
		{
			searchStartIterator = layer->Items.begin();
		}
		else
		{
			searchStartIterator++;
		}
	}

	if (searchStartIterator == layer->Items.end())
	{
		searchStartIterator = layer->Items.begin();
	}

	// Search the text from the selected
	//
	for (auto it = searchStartIterator; it != layer->Items.end(); ++it)
	{
		std::shared_ptr<VFrame30::SchemaItem> item = *it;

		bool found = item->searchText(searchText);

		if (found == true)
		{
			selectItem(item);
			return;
		}
	}

	// Serach text from the beginning to selected
	//
	for (auto it = layer->Items.begin(); it != searchStartIterator; ++it)
	{
		std::shared_ptr<VFrame30::SchemaItem> item = *it;

		bool found = item->searchText(searchText);

		if (found == true)
		{
			selectItem(item);
			return;
		}
	}

	// Text not found
	//
	clearSelection();

	QMessageBox::information(this, qApp->applicationName(), tr("Text <b>%1</b> not found.").arg(searchText));

	m_findDialog->show();
	m_findDialog->raise();
	m_findDialog->activateWindow();
	m_findDialog->setFocusToEditLine();

	return;
}

void EditSchemaWidget::findPrev()
{
	if (m_findDialog == nullptr)
	{
		find();
		return;
	}

	QString searchText = m_findDialog->findText();

	if (searchText.isEmpty() == true)
	{
		m_findDialog->show();
		m_findDialog->raise();
		m_findDialog->activateWindow();

		m_findDialog->setFocusToEditLine();
		return;
	}

	m_findDialog->updateCompleter();

	// Look for text
	//
	std::shared_ptr<VFrame30::SchemaLayer> layer = activeLayer();
	assert(layer);

	if (layer->Items.empty() == true)
	{
		clearSelection();
		return;
	}

	auto& selected = selectedItems();		// Keep reference!!!!

	// Get start iterator
	//
	auto searchStartIterator = layer->Items.rbegin();
	if (selected.size() != 1)
	{
		searchStartIterator = layer->Items.rbegin();
	}
	else
	{
		assert(selected.size() == 1);

		searchStartIterator = std::find(layer->Items.rbegin(), layer->Items.rend(), selected.front());
		if (searchStartIterator == layer->Items.rend())
		{
			searchStartIterator = layer->Items.rbegin();
		}
		else
		{
			searchStartIterator++;
		}
	}

	if (searchStartIterator == layer->Items.rend())
	{
		searchStartIterator = layer->Items.rbegin();
	}

	// Search the text from the selected
	//
	for (auto it = searchStartIterator; it != layer->Items.rend(); ++it)
	{
		std::shared_ptr<VFrame30::SchemaItem> item = *it;

		bool found = item->searchText(searchText);

		if (found == true)
		{
			selectItem(item);
			return;
		}
	}

	// Serach text from the beginning to selected
	//
	for (auto it = layer->Items.rbegin(); it != searchStartIterator; ++it)
	{
		std::shared_ptr<VFrame30::SchemaItem> item = *it;

		bool found = item->searchText(searchText);

		if (found == true)
		{
			selectItem(item);
			return;
		}
	}

	// Text not found
	//
	clearSelection();

	QMessageBox::information(this, qApp->applicationName(), tr("Text <b>%1</b> not found.").arg(searchText));

	m_findDialog->show();
	m_findDialog->raise();
	m_findDialog->activateWindow();
	m_findDialog->setFocusToEditLine();

	return;
}

void EditSchemaWidget::hideWorkDialogs()
{
	if (m_findDialog != nullptr)
	{
		m_findDialog->hide();
	}

	if (m_itemsPropertiesDialog != nullptr)
	{
		m_itemsPropertiesDialog->hide();
	}

	if (m_schemaPropertiesDialog != nullptr)
	{
		m_schemaPropertiesDialog = nullptr;
	}

	return;
}

MouseState EditSchemaWidget::mouseState() const
{
	return editSchemaView()->mouseState();
}

void EditSchemaWidget::setMouseState(MouseState state)
{
	editSchemaView()->setMouseState(state);
	return;
}

const DbFileInfo& EditSchemaWidget::fileInfo() const
{
	return m_fileInfo;
}

void EditSchemaWidget::setFileInfo(const DbFileInfo& fi)
{
	m_fileInfo = fi;
}

bool EditSchemaWidget::snapToGrid() const
{
	return m_snapToGrid;
}

void EditSchemaWidget::setSnapToGrid(bool value)
{
	m_snapToGrid = value;
}

bool EditSchemaWidget::compareWidget() const
{
	return editSchemaView()->m_compareWidget;
}

bool EditSchemaWidget::isCompareWidget() const
{
	return editSchemaView()->m_compareWidget;
}

void EditSchemaWidget::setCompareWidget(bool value, std::shared_ptr<VFrame30::Schema> source, std::shared_ptr<VFrame30::Schema> target)
{
	editSchemaView()->m_compareWidget = value;

	if (value == true)
	{
		assert(source);
		assert(target);

		editSchemaView()->m_compareSourceSchema = source;
		editSchemaView()->m_compareTargetSchema = target;
	}

	return;
}

bool EditSchemaWidget::readOnly() const
{
	assert(m_editEngine);
	return m_editEngine->readOnly();
}

void EditSchemaWidget::setReadOnly(bool value)
{
	assert(m_editEngine);
	m_editEngine->setReadOnly(value);
}

bool EditSchemaWidget::modified() const
{
	assert(m_editEngine);
	return m_editEngine->modified();
}

void EditSchemaWidget::setModified()
{
	assert(m_editEngine);
	m_editEngine->setModified();
}


void EditSchemaWidget::resetModified()
{
	assert(m_editEngine);
	m_editEngine->resetModified();
}

void EditSchemaWidget::resetEditEngine()
{
	assert(m_editEngine);
	m_editEngine->reset();
}

void EditSchemaWidget::setCompareItemActions(const std::map<QUuid, CompareAction>& itemsActions)
{
	editSchemaView()->m_itemsActions = itemsActions;
}

SchemaFindDialog::SchemaFindDialog(QWidget* parent) :
	QDialog(parent)
{
	m_lineEdit = new QLineEdit();

	QCompleter* searchCompleter = new QCompleter(theSettings.buildSearchCompleter(), this);
	searchCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	m_lineEdit->setCompleter(searchCompleter);

	m_prevButton = new QPushButton(tr("Find Previous"));
	//m_prevButton->setShortcut(QKeySequence::FindPrevious);	// Done via Actions, works much faster

	m_nextButton = new QPushButton(tr("Find Next"));
	//m_nextButton->setShortcut(QKeySequence::FindNext);		// Done via Actions, works much faster

	QGridLayout* layout = new QGridLayout();

	layout->addWidget(m_lineEdit, 0, 0, 1, 2);
	layout->addWidget(m_prevButton, 1, 0);
	layout->addWidget(m_nextButton, 1, 1);

	setLayout(layout);

	setMinimumWidth(300);

	QAction* nextAction = new QAction(tr("Find Next"), this);
	nextAction->setShortcut(QKeySequence::FindNext);
	addAction(nextAction);

	QAction* prevAction = new QAction(tr("Find Prev"), this);
	prevAction->setShortcut(QKeySequence::FindPrevious);
	addAction(prevAction);

	connect(nextAction, &QAction::triggered, this, &SchemaFindDialog::findNext);
	connect(prevAction, &QAction::triggered, this, &SchemaFindDialog::findPrev);

	// --
	//
	connect(m_prevButton, &QPushButton::clicked, this, &SchemaFindDialog::findPrev);
	connect(m_nextButton, &QPushButton::clicked, this, &SchemaFindDialog::findNext);

	m_nextButton->setDefault(true);
}

QString SchemaFindDialog::findText() const
{
	assert(m_lineEdit);

	QString text = m_lineEdit->text().trimmed();

	return text;
}

void SchemaFindDialog::setFocusToEditLine()
{
	assert(m_lineEdit);

	m_lineEdit->setFocus();
	m_lineEdit->selectAll();

	return;
}

void SchemaFindDialog::updateCompleter()
{
	// Update completer
	//
	QString searchText = findText();

	if (theSettings.buildSearchCompleter().contains(searchText, Qt::CaseInsensitive) == false)
	{
		theSettings.buildSearchCompleter() << searchText;

		QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_lineEdit->completer()->model());
		assert(completerModel);

		if (completerModel != nullptr)
		{
			completerModel->setStringList(theSettings.buildSearchCompleter());
		}
	}
}

