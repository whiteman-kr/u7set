#include "EditEngine/EditEngine.h"
#include "EditSchemaWidget.h"
#include "SchemaPropertiesDialog.h"
#include "SchemaLayersDialog.h"
#include "SchemaItemPropertiesDialog.h"
#include "./Forms/ChooseAfbDialog.h"
#include "./Forms/ChooseUfbDialog.h"
#include "SignalPropertiesDialog.h"
#include "GlobalMessanger.h"
#include "../lib/Connection.h"
#include "../VFrame30/UfbSchema.h"
#include "../VFrame30/SchemaItemLine.h"
#include "../VFrame30/SchemaItemRect.h"
#include "../VFrame30/SchemaItemFrame.h"
#include "../VFrame30/SchemaItemImage.h"
#include "../VFrame30/SchemaItemPath.h"
#include "../VFrame30/SchemaItemSignal.h"
#include "../VFrame30/SchemaItemAfb.h"
#include "../VFrame30/SchemaItemLink.h"
#include "../VFrame30/SchemaItemConst.h"
#include "../VFrame30/SchemaItemUfb.h"
#include "../VFrame30/SchemaItemTerminator.h"
#include "../VFrame30/SchemaItemValue.h"
#include "../VFrame30/SchemaItemImageValue.h"
#include "../VFrame30/SchemaItemPushButton.h"
#include "../VFrame30/SchemaItemLineEdit.h"
#include "../VFrame30/SchemaItemLoopback.h"
#include "../VFrame30/Session.h"
#include "../VFrame30/DrawParam.h"
#include "../VFrame30/Bus.h"
#include "../lib/LmDescription.h"
#include "SignalsTabPage.h"
#include "Forms/ComparePropertyObjectDialog.h"
#include "Settings.h"
#include "../lib/SignalProperties.h"
#include "DialogInputEx.h"
#include "../lib/QScintillaLexers/LexerJavaScript.h"
#include "../lib/Ui/TextEditCompleter.h"
#include <cfloat>


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


QString EditSchemaWidget::m_lastUsedLoopbackId = "";


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
	m_mouseState(MouseState::None)
{
	// Timer for updates of WRN/ERR count
	//
	m_updateDuringBuildTimer = startTimer(50);
}

EditSchemaView::EditSchemaView(std::shared_ptr<VFrame30::Schema>& schema, QWidget* parent)
	: VFrame30::SchemaView(schema, parent),
	m_activeLayer(0),
	m_mouseState(MouseState::None)
{
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

void EditSchemaView::paintEvent(QPaintEvent* /*pe*/)
{
	// Draw schema
	//

	// VFrame30::SchemaView::paintEvent(pe);
	//
	if (schema().get() != nullptr)
	{
		QPainter p(this);

		VFrame30::CDrawParam drawParam(&p, schema().get(), this, schema()->gridSize(), schema()->pinGridStep());
		drawParam.setControlBarSize(
			schema()->unit() == VFrame30::SchemaUnit::Display ?	10 * (100.0 / zoom()) : mm2in(2.4) * (100.0 / zoom()));

		drawParam.setInfoMode(theSettings.infoMode());
		drawParam.session() = session();

		draw(drawParam);
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

			OutputMessageLevel issue = GlobalMessanger::instance().issueForSchemaItem(item->guid());

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

				item->DrawDebugInfo(drawParam, orderIndexText);
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
	std::vector<std::shared_ptr<VFrame30::SchemaItem>> outlines;
	outlines.push_back(m_newItem);

	VFrame30::SchemaItem::DrawOutline(drawParam, outlines);

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
		// ��������� ����, ���� ����� ��������� ����� ��� 2 �� ���� � �����
		//
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
	p->setPen(QColor(0x00, 0x00, 0x80, 0xB4));
	QPointF pt;

	QRegion visiblePart = visibleRegion();

	double dpiX = unit == VFrame30::SchemaUnit::Display ? 1.0 : p->device()->physicalDpiX();
	double dpiY = unit == VFrame30::SchemaUnit::Display ? 1.0 : p->device()->physicalDpiY();

	std::vector<QPointF> points;
	points.reserve(1024);

	for (int v = 0; v < vertGridCount; v++)
	{
		pt.setY(static_cast<double>(v + 1) * gridSize * dpiY * scale);
		points.clear();

		for (int h = 0; h < horzGridCount; h++)
		{
			pt.setX(static_cast<double>(h + 1) * gridSize * dpiX * scale);

			if (visiblePart.contains(pt.toPoint()) == false)
			{
				continue;
			}

			points.push_back(pt);
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

const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& EditSchemaView::selectedItems() const
{
	return m_selectedItems;
}

std::vector<std::shared_ptr<VFrame30::SchemaItem>> EditSchemaView::selectedNonLockedItems() const
{
	std::vector<std::shared_ptr<VFrame30::SchemaItem>> result;
	result.reserve(m_selectedItems.size());

	for (std::shared_ptr<VFrame30::SchemaItem> si : m_selectedItems)
	{
		if (si->isLocked() == false)
		{
			result.push_back(si);
		}
	}

	return result;
}


void EditSchemaView::setSelectedItems(const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items)
{
	std::vector<std::shared_ptr<VFrame30::SchemaItem>> uniqueItems;
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

	return;
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

	return;
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

	return;
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
EditSchemaWidget::EditSchemaWidget(std::shared_ptr<VFrame30::Schema> schema, const DbFileInfo& fileInfo, DbController* dbController) :
	VFrame30::BaseSchemaWidget(schema, new EditSchemaView(schema)),
	m_fileInfo(fileInfo),
	m_dbcontroller(dbController),
	m_initialSchemaId(schema->schemaId())
{
	assert(schema != nullptr);
	assert(m_dbcontroller);

	createActions();

	// Left Button Down
	//
	m_mouseLeftDownStateAction.push_back(MouseStateAction(MouseState::None, std::bind(&EditSchemaWidget::mouseLeftDown_None, this, std::placeholders::_1)));
	m_mouseLeftDownStateAction.push_back(MouseStateAction(MouseState::AddSchemaPosLineStartPoint, std::bind(&EditSchemaWidget::mouseLeftDown_AddSchemaPosLineStartPoint, this, std::placeholders::_1)));
	m_mouseLeftDownStateAction.push_back(MouseStateAction(MouseState::AddSchemaPosRectStartPoint, std::bind(&EditSchemaWidget::mouseLeftDown_AddSchemaPosRectStartPoint, this, std::placeholders::_1)));
	m_mouseLeftDownStateAction.push_back(MouseStateAction(MouseState::AddSchemaPosConnectionStartPoint, std::bind(&EditSchemaWidget::mouseLeftDown_AddSchemaPosConnectionStartPoint, this, std::placeholders::_1)));

	// Left Button Up
	//
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::Selection, std::bind(&EditSchemaWidget::mouseLeftUp_Selection, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::Moving, std::bind(&EditSchemaWidget::mouseLeftUp_Moving, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingTopLeft, std::bind(&EditSchemaWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingTop, std::bind(&EditSchemaWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingTopRight, std::bind(&EditSchemaWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingRight, std::bind(&EditSchemaWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingBottomRight, std::bind(&EditSchemaWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingBottom, std::bind(&EditSchemaWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingBottomLeft, std::bind(&EditSchemaWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingLeft, std::bind(&EditSchemaWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::MovingStartLinePoint, std::bind(&EditSchemaWidget::mouseLeftUp_MovingLinePoint, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::MovingEndLinePoint, std::bind(&EditSchemaWidget::mouseLeftUp_MovingLinePoint, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::AddSchemaPosLineEndPoint, std::bind(&EditSchemaWidget::mouseLeftUp_AddSchemaPosLineEndPoint, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::AddSchemaPosRectEndPoint, std::bind(&EditSchemaWidget::mouseLeftUp_AddSchemaPosRectEndPoint, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::AddSchemaPosConnectionNextPoint, std::bind(&EditSchemaWidget::mouseLeftUp_AddSchemaPosConnectionNextPoint, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::MovingHorizontalEdge, std::bind(&EditSchemaWidget::mouseLeftUp_MovingEdgeOrVertex, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::MovingVerticalEdge, std::bind(&EditSchemaWidget::mouseLeftUp_MovingEdgeOrVertex, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::MovingConnectionLinePoint, std::bind(&EditSchemaWidget::mouseLeftUp_MovingEdgeOrVertex, this, std::placeholders::_1)));

	// Moouse Mov
	//
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::Scrolling, std::bind(&EditSchemaWidget::mouseMove_Scrolling, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::Selection, std::bind(&EditSchemaWidget::mouseMove_Selection, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::Moving, std::bind(&EditSchemaWidget::mouseMove_Moving, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingTopLeft, std::bind(&EditSchemaWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingTop, std::bind(&EditSchemaWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingTopRight, std::bind(&EditSchemaWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingRight, std::bind(&EditSchemaWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingBottomRight, std::bind(&EditSchemaWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingBottom, std::bind(&EditSchemaWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingBottomLeft, std::bind(&EditSchemaWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingLeft, std::bind(&EditSchemaWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::MovingStartLinePoint, std::bind(&EditSchemaWidget::mouseMove_MovingLinePoint, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::MovingEndLinePoint, std::bind(&EditSchemaWidget::mouseMove_MovingLinePoint, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::AddSchemaPosLineEndPoint, std::bind(&EditSchemaWidget::mouseMove_AddSchemaPosLineEndPoint, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::AddSchemaPosRectEndPoint, std::bind(&EditSchemaWidget::mouseMove_AddSchemaPosRectEndPoint, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::AddSchemaPosConnectionNextPoint, std::bind(&EditSchemaWidget::mouseMove_AddSchemaPosConnectionNextPoint, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::MovingHorizontalEdge, std::bind(&EditSchemaWidget::mouseMove_MovingEdgesOrVertex, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::MovingVerticalEdge, std::bind(&EditSchemaWidget::mouseMove_MovingEdgesOrVertex, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::MovingConnectionLinePoint, std::bind(&EditSchemaWidget::mouseMove_MovingEdgesOrVertex, this, std::placeholders::_1)));

	// Mouse Right Button Down
	//
	//m_mouseRightDownStateAction.push_back(MouseStateAction(MouseState::None, std::bind(&EditSchemaWidget::mouseRightDown_None, this, std::placeholders::_1)));
	m_mouseRightDownStateAction.push_back(MouseStateAction(MouseState::AddSchemaPosConnectionNextPoint, std::bind(&EditSchemaWidget::mouseRightDown_AddSchemaPosConnectionNextPoint, this, std::placeholders::_1)));
	m_mouseRightDownStateAction.push_back(MouseStateAction(MouseState::MovingConnectionLinePoint, std::bind(&EditSchemaWidget::mouseRightDown_MovingEdgesOrVertex, this, std::placeholders::_1)));

	// Mouse Right Button Up
	//
	m_mouseRightUpStateAction.push_back(MouseStateAction(MouseState::None, std::bind(&EditSchemaWidget::mouseRightUp_None, this, std::placeholders::_1)));

	// Init Session Variables
	//
	schemaView()->session().setProject(dbController->currentProject().projectName());
	schemaView()->session().setUsername(dbController->currentUser().username());
	schemaView()->session().setHost(QHostInfo::localHostName());

	// --
	//
	connect(this, &QWidget::customContextMenuRequested, this, &EditSchemaWidget::contextMenu);
	setCorrespondingContextMenu();

	// Edit Engine
	//
	m_editEngine = new EditEngine::EditEngine(editSchemaView(), horizontalScrollBar(), verticalScrollBar(), this);

	connect(m_editEngine, &EditEngine::EditEngine::stateChanged, this, &EditSchemaWidget::editEngineStateChanged);
	connect(m_editEngine, &EditEngine::EditEngine::modifiedChanged, this, &EditSchemaWidget::modifiedChangedSlot);

	connect(editSchemaView(), &EditSchemaView::selectionChanged, this, &EditSchemaWidget::selectionChanged);

	// Clipboard
	//
	connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &EditSchemaWidget::clipboardDataChanged);

	clipboardDataChanged();		// Enable m_editPasteAction if somthing in clipborad

	return;
}

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
	m_f2Action->setShortcutVisibleInContextMenu(true);
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
	m_infoModeAction->setShortcutVisibleInContextMenu(true);
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

	m_detachWindow = new QAction(tr("Detach/Attach Window"), this);
	m_detachWindow->setStatusTip(tr("Detach/attach window..."));
	m_detachWindow->setEnabled(true);
	m_detachWindow->setShortcut(Qt::ALT + Qt::Key_D);
	m_detachWindow->setShortcutVisibleInContextMenu(true);
	connect(m_detachWindow, &QAction::triggered, this, &EditSchemaWidget::detachOrAttachWindow);

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
	m_fileSaveAction->setShortcutVisibleInContextMenu(true);
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
	m_fileCloseAction->setShortcutVisibleInContextMenu(true);
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

	m_addImageAction = new QAction(tr("Image"), this);
	m_addImageAction->setEnabled(true);
	m_addImageAction->setIcon(QIcon(":/Images/Images/SchemaItemImage.svg"));
	connect(m_addImageAction, &QAction::triggered,
			[this](bool)
			{
				auto image = std::make_shared<VFrame30::SchemaItemImage>(schema()->unit());
				addItem(image);
			});

	m_addFrameAction = new QAction(tr("Frame"), this);
	m_addFrameAction->setEnabled(true);
	m_addFrameAction->setIcon(QIcon(":/Images/Images/SchemaItemFrame.svg"));
	connect(m_addFrameAction, &QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::SchemaItemFrame>(schema()->unit()));
			});

	// ----------------------------------------
	m_addSeparatorAction0 = new QAction(this);
	m_addSeparatorAction0->setSeparator(true);

	m_addLinkAction = new QAction(tr("Link"), this);
	m_addLinkAction->setEnabled(true);
	m_addLinkAction->setIcon(QIcon(":/Images/Images/SchemaLink.svg"));
	connect(m_addLinkAction, &QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::SchemaItemLink>(schema()->unit()));
			});

	m_addInputSignalAction = new QAction(tr("Input"), this);
	m_addInputSignalAction->setEnabled(true);
	m_addInputSignalAction->setIcon(QIcon(":/Images/Images/SchemaInputSignal.svg"));
	connect(m_addInputSignalAction, &QAction::triggered,
			[this](bool)
			{
				auto item = std::make_shared<VFrame30::SchemaItemInput>(schema()->unit());
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

	m_addOutputSignalAction = new QAction(tr("Output"), this);
	m_addOutputSignalAction->setEnabled(true);
	m_addOutputSignalAction->setIcon(QIcon(":/Images/Images/SchemaOutputSignal.svg"));
	connect(m_addOutputSignalAction, &QAction::triggered,
			[this](bool)
			{
				auto item = std::make_shared<VFrame30::SchemaItemOutput>(schema()->unit());
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

	// ----------------------------------------
	m_addSeparatorAfb = new QAction(this);
	m_addSeparatorAfb->setSeparator(true);

	m_addAfbAction = new QAction(tr("App Functional Block"), this);
	m_addAfbAction->setEnabled(true);
	m_addAfbAction->setIcon(QIcon(":/Images/Images/SchemaFblElement.svg"));
	connect(m_addAfbAction, &QAction::triggered, this, &EditSchemaWidget::addAfbElement);

	m_addUfbAction = new QAction(tr("User Functional Block"), this);
	m_addUfbAction->setEnabled(true);
	m_addUfbAction->setIcon(QIcon(":/Images/Images/SchemaUfbElement.svg"));
	connect(m_addUfbAction, &QAction::triggered, this, &EditSchemaWidget::addUfbElement);

	// ----------------------------------------
	m_addSeparatorConn = new QAction(this);
	m_addSeparatorConn->setSeparator(true);

	m_addTransmitter = new QAction(tr("Transmitter"), this);
	m_addTransmitter->setEnabled(true);
	m_addTransmitter->setIcon(QIcon(":/Images/Images/SchemaTransmitter.svg"));
	connect(m_addTransmitter, &QAction::triggered, this, &EditSchemaWidget::addTransmitter);

	m_addReceiver = new QAction(tr("Receiver"), this);
	m_addReceiver->setEnabled(true);
	m_addReceiver->setIcon(QIcon(":/Images/Images/SchemaReceiver.svg"));
	connect(m_addReceiver, &QAction::triggered, this, &EditSchemaWidget::addReceiver);

	// ----------------------------------------
	m_addSeparatorLoop = new QAction(this);
	m_addSeparatorLoop->setSeparator(true);

	m_addLoopbackSource = new QAction(tr("Loopback Source"), this);
	m_addLoopbackSource->setEnabled(true);
	m_addLoopbackSource->setIcon(QIcon(":/Images/Images/SchemaLoopbackSource.svg"));
	connect(m_addLoopbackSource, &QAction::triggered, this, &EditSchemaWidget::addLoopbackSource);

	m_addLoopbackTarget = new QAction(tr("Loopback Target"), this);
	m_addLoopbackTarget->setEnabled(true);
	m_addLoopbackTarget->setIcon(QIcon(":/Images/Images/SchemaLoopbackTarget.svg"));
	connect(m_addLoopbackTarget, &QAction::triggered, this, &EditSchemaWidget::addLoopbackTarget);

	// ----------------------------------------
	m_addSeparatorBus = new QAction(this);
	m_addSeparatorBus->setSeparator(true);

	m_addBusComposer = new QAction(tr("Bus Composer"), this);
	m_addBusComposer->setEnabled(true);
	m_addBusComposer->setIcon(QIcon(":/Images/Images/SchemaBusComposer.svg"));
	connect(m_addBusComposer, &QAction::triggered, this, &EditSchemaWidget::addBusComposer);

	m_addBusExtractor = new QAction(tr("Bus Extractor"), this);
	m_addBusExtractor->setEnabled(true);
	m_addBusExtractor->setIcon(QIcon(":/Images/Images/SchemaBusExtractor.svg"));
	connect(m_addBusExtractor, &QAction::triggered, this, &EditSchemaWidget::addBusExtractor);


	m_addValueAction = new QAction(tr("Value"), this);
	m_addValueAction->setEnabled(true);
	m_addValueAction->setIcon(QIcon(":/Images/Images/SchemaItemValue.svg"));
	connect(m_addValueAction, &QAction::triggered,
			[this](bool)
			{
				auto item = std::make_shared<VFrame30::SchemaItemValue>(schema()->unit());
				addItem(item);
			});

	m_addImageValueAction = new QAction(tr("Image Value"), this);
	m_addImageValueAction->setEnabled(true);
	m_addImageValueAction->setIcon(QIcon(":/Images/Images/SchemaItemImageValue.svg"));
	connect(m_addImageValueAction, &QAction::triggered,
			[this](bool)
	{
		auto item = std::make_shared<VFrame30::SchemaItemImageValue>(schema()->unit());
		addItem(item);
	});

	m_addPushButtonAction = new QAction(tr("PushButton"), this);
	m_addPushButtonAction->setEnabled(true);
	m_addPushButtonAction->setIcon(QIcon(":/Images/Images/SchemaItemPushButton.svg"));
	connect(m_addPushButtonAction, &QAction::triggered,
			[this](bool)
			{
				auto item = std::make_shared<VFrame30::SchemaItemPushButton>(schema()->unit());
				addItem(item);
			});

	m_addLineEditAction = new QAction(tr("LineEdit"), this);
	m_addLineEditAction->setEnabled(true);
	m_addLineEditAction->setIcon(QIcon(":/Images/Images/SchemaItemLineEdit.svg"));
	connect(m_addLineEditAction, &QAction::triggered,
			[this](bool)
			{
				auto item = std::make_shared<VFrame30::SchemaItemLineEdit>(schema()->unit());
				addItem(item);
			});

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
	m_undoAction->setShortcutVisibleInContextMenu(true);
	connect(m_undoAction, &QAction::triggered, this, &EditSchemaWidget::undo);
	addAction(m_undoAction);

	// Edit->Redo
	//
	m_redoAction = new QAction(tr("Redo"), this);
	m_redoAction->setEnabled(false);
	m_redoAction->setShortcuts(QKeySequence::Redo);
	m_redoAction->setShortcutVisibleInContextMenu(true);
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
	m_selectAllAction->setShortcutVisibleInContextMenu(true);
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
	m_editCutAction->setShortcutVisibleInContextMenu(true);
	connect(m_editCutAction, &QAction::triggered, this, &EditSchemaWidget::editCut);
	addAction(m_editCutAction);

	// Edit->Copy
	//
	m_editCopyAction = new QAction(tr("Copy"), this);
	m_editCopyAction->setEnabled(false);
	m_editCopyAction->setShortcuts(QKeySequence::Copy);
	m_editCopyAction->setShortcutVisibleInContextMenu(true);
	connect(m_editCopyAction, &QAction::triggered, this, &EditSchemaWidget::editCopy);
	addAction(m_editCopyAction);

	// Edit->Paste
	//
	m_editPasteAction = new QAction(tr("Paste"), this);
	m_editPasteAction->setEnabled(false);
	m_editPasteAction->setShortcuts(QKeySequence::Paste);
	m_editPasteAction->setShortcutVisibleInContextMenu(true);
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
	m_deleteAction->setShortcutVisibleInContextMenu(true);
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
	m_propertiesAction->setShortcutVisibleInContextMenu(true);
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
	m_sameWidthAction->setShortcut(Qt::ALT + Qt::Key_W);
	m_sameWidthAction->setShortcutVisibleInContextMenu(true);
	connect(m_sameWidthAction, &QAction::triggered, this, &EditSchemaWidget::sameWidth);
	addAction(m_sameWidthAction);

	// Size/Pos->Same Height
	//
	m_sameHeightAction = new QAction(tr("Same Height"), this);
	m_sameHeightAction->setEnabled(false);
	m_sameHeightAction->setShortcut(Qt::ALT + Qt::Key_H);
	m_sameHeightAction->setShortcutVisibleInContextMenu(true);
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
	m_bringToFrontAction->setShortcutVisibleInContextMenu(true);
	connect(m_bringToFrontAction, &QAction::triggered, this, &EditSchemaWidget::bringToFront);
	addAction(m_bringToFrontAction);

	// Items Order->Bring Forward
	//
	m_bringForwardAction = new QAction(tr("Bring Forward"), this);
	m_bringForwardAction->setEnabled(false);
	m_bringForwardAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_PageUp));
	m_bringForwardAction->setShortcutVisibleInContextMenu(true);
	connect(m_bringForwardAction, &QAction::triggered, this, &EditSchemaWidget::bringForward);
	addAction(m_bringForwardAction);

	// Items Order->Send to Back
	//
	m_sendToBackAction = new QAction(tr("Send to Back"), this);
	m_sendToBackAction->setEnabled(false);
	m_sendToBackAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_End));
	m_sendToBackAction->setShortcutVisibleInContextMenu(true);
	connect(m_sendToBackAction, &QAction::triggered, this, &EditSchemaWidget::sendToBack);
	addAction(m_sendToBackAction);

	// Items Order->Send Backward
	//
	m_sendBackwardAction = new QAction(tr("Send Backward"), this);
	m_sendBackwardAction->setEnabled(false);
	m_sendBackwardAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_PageDown));
	m_sendBackwardAction->setShortcutVisibleInContextMenu(true);
	connect(m_sendBackwardAction, &QAction::triggered, this, &EditSchemaWidget::sendBackward);
	addAction(m_sendBackwardAction);

	//
	// Transform Into
	//
	m_transformAction = new QAction(tr("Transform Into"), this);
	m_transformAction->setEnabled(true);

	// Transform Into->Input
	//
	m_transformIntoInputAction = new QAction(tr("Input"), this);
	connect(m_transformIntoInputAction, &QAction::triggered, this, &EditSchemaWidget::transformIntoInput);

	// Transform Into->In/Out
	//
	m_transformIntoInOutAction = new QAction(tr("In/Out"), this);
	connect(m_transformIntoInOutAction, &QAction::triggered, this, &EditSchemaWidget::transformIntoInOut);

	// Transform Into->Output
	//
	m_transformIntoOutputAction = new QAction(tr("Output"), this);
	connect(m_transformIntoOutputAction, &QAction::triggered, this, &EditSchemaWidget::transformIntoOutput);

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
	m_zoomInAction->setShortcutVisibleInContextMenu(true);
	connect(m_zoomInAction, &QAction::triggered, this, &VFrame30::BaseSchemaWidget::zoomIn);
	addAction(m_zoomInAction);

	// View->ZoomOut
	//
	m_zoomOutAction = new QAction(tr("Zoom Out"), this);
	m_zoomOutAction->setEnabled(true);
	m_zoomOutAction->setShortcut(QKeySequence::ZoomOut);
	m_zoomOutAction->setShortcutVisibleInContextMenu(true);
	connect(m_zoomOutAction, &QAction::triggered, this, &VFrame30::BaseSchemaWidget::zoomOut);
	addAction(m_zoomOutAction);

	// View->Zoom100
	//
	m_zoom100Action = new QAction(tr("Zoom 100%"), this);
	m_zoom100Action->setEnabled(true);
	m_zoom100Action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Asterisk));
	m_zoom100Action->setShortcutVisibleInContextMenu(true);
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
	//m_layersAction->setShortcutVisibleInContextMenu(true);
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
	m_toggleCommentAction->setShortcutVisibleInContextMenu(true);
	connect(m_toggleCommentAction, &QAction::triggered, this, &EditSchemaWidget::toggleComment);
	addAction(m_toggleCommentAction);

	// Lock/Unlock
	//
	m_lockAction = new QAction(tr("Lock/Unlock"), this);
	m_lockAction->setEnabled(false);
	m_lockAction->setShortcut(Qt::CTRL + Qt::Key_L);
	m_lockAction->setShortcutVisibleInContextMenu(true);
	connect(m_lockAction, &QAction::triggered, this, &EditSchemaWidget::toggleLock);
	addAction(m_lockAction);

	// Find
	//
	m_findAction = new QAction(tr("Find..."), this);
	m_findAction->setEnabled(true);
	m_findAction->setShortcut(QKeySequence::Find);
	m_findAction->setShortcutVisibleInContextMenu(true);
	connect(m_findAction, &QAction::triggered, this, &EditSchemaWidget::find);
	addAction(m_findAction);

	m_findNextAction = new QAction(tr("Find Next"), this);
	m_findNextAction->setEnabled(true);
	m_findNextAction->setShortcut(QKeySequence::FindNext);
	m_findNextAction->setShortcutVisibleInContextMenu(true);
	connect(m_findNextAction, &QAction::triggered, this,
			[this]()
			{
				findNext(theSettings.m_findSchemaItemCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
			});
	addAction(m_findNextAction);

	m_findPrevAction = new QAction(tr("Find Previous"), this);
	m_findPrevAction->setEnabled(true);
	m_findPrevAction->setShortcut(QKeySequence::FindPrevious);
	m_findPrevAction->setShortcutVisibleInContextMenu(true);
	connect(m_findPrevAction, &QAction::triggered, this,
			[this]()
			{
				findPrev(theSettings.m_findSchemaItemCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
			});

	addAction(m_findPrevAction);

	// Other
	//
	m_addAppSignalAction = new QAction(tr("AddAppSignal"), this);
	m_addAppSignalAction->setShortcut(Qt::ALT + Qt::Key_N);
	m_addAppSignalAction->setShortcutVisibleInContextMenu(true);
	m_addAppSignalAction->setEnabled(false);
	connect(m_addAppSignalAction, &QAction::triggered, this, &EditSchemaWidget::addNewAppSignalSelected);
	addAction(m_addAppSignalAction);

	m_appSignalPropertiesAction = new QAction(tr("AppSignalProperties"), this);
	m_appSignalPropertiesAction->setShortcut(Qt::ALT + Qt::Key_S);
	m_appSignalPropertiesAction->setShortcutVisibleInContextMenu(true);
	m_appSignalPropertiesAction->setEnabled(false);
	connect(m_appSignalPropertiesAction, &QAction::triggered, this, &EditSchemaWidget::appSignalsSelectedProperties);
	addAction(m_appSignalPropertiesAction);

	//
	// Create Sub Menus
	//
	m_fileMenu = new QMenu(this);
	m_fileAction->setMenu(m_fileMenu);
		m_fileMenu->addAction(m_detachWindow);
		m_fileMenu->addSeparator();
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
		m_addMenu->addAction(m_addImageAction);
		//m_addMenu->addAction(m_addFrameAction);

		m_addMenu->addAction(m_addSeparatorAction0);

		if (isLogicSchema() == true)
		{
			m_addMenu->addAction(m_addLinkAction);
			m_addMenu->addAction(m_addInputSignalAction);
			m_addMenu->addAction(m_addInOutSignalAction);
			m_addMenu->addAction(m_addOutputSignalAction);
			m_addMenu->addAction(m_addConstantAction);
			m_addMenu->addAction(m_addTerminatorAction);

			m_addMenu->addAction(m_addSeparatorAfb);
			m_addMenu->addAction(m_addAfbAction);
			m_addMenu->addAction(m_addUfbAction);

			m_addMenu->addAction(m_addSeparatorConn);
			m_addMenu->addAction(m_addTransmitter);
			m_addMenu->addAction(m_addReceiver);

			m_addMenu->addAction(m_addSeparatorLoop);
			m_addMenu->addAction(m_addLoopbackSource);
			m_addMenu->addAction(m_addLoopbackTarget);

			m_addMenu->addAction(m_addSeparatorBus);
			m_addMenu->addAction(m_addBusComposer);
			m_addMenu->addAction(m_addBusExtractor);
		}

		if (isUfbSchema() == true)
		{
			m_addMenu->addAction(m_addLinkAction);
			m_addMenu->addAction(m_addInputSignalAction);
			m_addMenu->addAction(m_addOutputSignalAction);
			m_addMenu->addAction(m_addConstantAction);
			m_addMenu->addAction(m_addTerminatorAction);

			m_addMenu->addAction(m_addSeparatorAfb);
			m_addMenu->addAction(m_addAfbAction);

			m_addMenu->addAction(m_addSeparatorLoop);
			m_addMenu->addAction(m_addLoopbackSource);
			m_addMenu->addAction(m_addLoopbackTarget);

			m_addMenu->addAction(m_addSeparatorBus);
			m_addMenu->addAction(m_addBusComposer);
			m_addMenu->addAction(m_addBusExtractor);
		}

		if (isMonitorSchema() == true)
		{
			m_addMenu->addAction(m_addValueAction);
			m_addMenu->addAction(m_addImageValueAction);
			m_addMenu->addAction(m_addPushButtonAction);
			m_addMenu->addAction(m_addLineEditAction);
		}

		if (isTuningSchema() == true)
		{
			m_addMenu->addAction(m_addValueAction);
			m_addMenu->addAction(m_addImageValueAction);
			m_addMenu->addAction(m_addPushButtonAction);
			m_addMenu->addAction(m_addLineEditAction);
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

	m_transformMenu = new QMenu(this);
		m_transformAction->setMenu(m_transformMenu);
		m_transformMenu->addAction(m_transformIntoInputAction);
		m_transformMenu->addAction(m_transformIntoInOutAction);
		m_transformMenu->addAction(m_transformIntoOutputAction);

	m_viewMenu = new QMenu(this);
	m_viewAction->setMenu(m_viewMenu);
		m_viewMenu->addAction(m_zoomInAction);
		m_viewMenu->addAction(m_zoomOutAction);
		m_viewMenu->addAction(m_zoom100Action);
		m_viewMenu->addAction(m_viewSeparatorAction0);
		m_viewMenu->addAction(m_snapToGridAction);

	return;
}

void EditSchemaWidget::updateFileActions()
{
	// Version Control enable/disable items
	//
	m_fileSaveAction->setEnabled(readOnly() == false && modified() == true);
	m_fileCheckInAction->setEnabled(readOnly() == false && fileInfo().state() == VcsState::CheckedOut);
	m_fileCheckOutAction->setEnabled(readOnly() == true && fileInfo().state() == VcsState::CheckedIn);
	m_fileUndoChangesAction->setEnabled(readOnly() == false && fileInfo().state() == VcsState::CheckedOut);
	m_fileExportAction->setEnabled(true);
	m_fileImportAction->setEnabled(readOnly() == false && fileInfo().state() == VcsState::CheckedOut);

	return;
}

bool EditSchemaWidget::event(QEvent* event)
{
	// Show tool tip
	//
	if (event->type() == QEvent::ToolTip)
	{
		QHelpEvent* he = static_cast<QHelpEvent*>(event);

		// Get item under cursor
		//
		QPointF docPoint = widgetPointToDocument(he->pos(), false);
		std::shared_ptr<VFrame30::SchemaItem> itemUnderPoint = editSchemaView()->activeLayer()->getItemUnderPoint(docPoint);

		if (itemUnderPoint != nullptr)
		{
			QString toolTip = itemUnderPoint->toolTipText(this->physicalDpiX(), this->physicalDpiY());
			setToolTip(toolTip);
		}
		else
		{
			setToolTip("");
		}

		return VFrame30::BaseSchemaWidget::event(event);
	}

	return VFrame30::BaseSchemaWidget::event(event);
}

void EditSchemaWidget::keyPressEvent(QKeyEvent* e)
{
	switch (e->key())
	{
		case Qt::Key_Left:
			onLeftKey(e);
			return;
		case Qt::Key_Right:
			onRightKey(e);
			return;
		case Qt::Key_Up:
			onUpKey(e);
			return;
		case Qt::Key_Down:
			onDownKey(e);
			return;
	}

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

	// This will update if Moving item in progress and we try to move connection links
	//
	if (editSchemaView()->m_editConnectionLines.empty() == false)
	{
		if (e->key() == Qt::Key_Space)
		{
			editSchemaView()->m_doNotMoveConnectionLines = !editSchemaView()->m_doNotMoveConnectionLines;
		}

		bool ctrlIsPressed = e->modifiers() & Qt::ControlModifier;

		if (ctrlIsPressed != m_ctrlWasPressed ||
			e->key() == Qt::Key_Space)
		{
			m_ctrlWasPressed = ctrlIsPressed;

			editSchemaView()->update();
		}

		setFocus();	// As alt could be pressed and MainMenu activated
		e->ignore();
		return;
	}

	BaseSchemaWidget::keyPressEvent(e);

	return;
}

void EditSchemaWidget::keyReleaseEvent(QKeyEvent* event)
{
	// This will update if Moving item in progress and we try to move connection links
	//
	if (editSchemaView()->m_editConnectionLines.empty() == false)
	{
		bool ctrlIsPressed = event->modifiers() & Qt::ControlModifier;

		if (ctrlIsPressed != m_ctrlWasPressed)
		{
			m_ctrlWasPressed = ctrlIsPressed;
			editSchemaView()->update();
		}

		setFocus();	// As alt could be pressed and MainMenu activated
		event->accept();
		return;
	}

	BaseSchemaWidget::keyReleaseEvent(event);

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
	grabKeyboard();

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
	releaseKeyboard();

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
	if (mouseState() == MouseState::MovingConnectionLinePoint ||
		mouseState() == MouseState::AddSchemaPosConnectionNextPoint)
	{
		// It accidental double clicking, ignore it
		//
		event->ignore();
		return;
	}

	setMouseState(MouseState::None);

	if (event->button() == Qt::LeftButton &&
		selectedItems().empty() == false)
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

					EditConnectionLine ecl(std::dynamic_pointer_cast<VFrame30::PosConnectionImpl>(selectedItem),
										   EditConnectionLine::EditMode::EditEdge);

					ecl.setEditEdgeIndex(std::dynamic_pointer_cast<VFrame30::PosConnectionImpl>(selectedItem), movingEdgePointIndex);

					editSchemaView()->m_editConnectionLines.clear();
					editSchemaView()->m_editConnectionLines.push_back(ecl);

					setMouseState(MouseState::MovingHorizontalEdge);
					setMouseCursor(me->pos());

					editSchemaView()->update();

					return;
				}

				if (possibleAction == SchemaItemAction::MoveVerticalEdge)
				{
					assert(movingEdgePointIndex != -1);

					EditConnectionLine ecl(std::dynamic_pointer_cast<VFrame30::PosConnectionImpl>(selectedItem),
										   EditConnectionLine::EditMode::EditEdge);

					ecl.setEditEdgeIndex(std::dynamic_pointer_cast<VFrame30::PosConnectionImpl>(selectedItem), movingEdgePointIndex);

					editSchemaView()->m_editConnectionLines.clear();
					editSchemaView()->m_editConnectionLines.push_back(ecl);

					setMouseState(MouseState::MovingVerticalEdge);
					setMouseCursor(me->pos());

					editSchemaView()->update();

					return;
				}

				if (possibleAction == SchemaItemAction::MoveConnectionLinePoint)
				{
					assert(movingEdgePointIndex != -1);

					EditConnectionLine ecl(std::dynamic_pointer_cast<VFrame30::PosConnectionImpl>(selectedItem),
										   EditConnectionLine::EditMode::EditPoint);

					ecl.setEditPointIndex(std::dynamic_pointer_cast<VFrame30::PosConnectionImpl>(selectedItem), movingEdgePointIndex);

					editSchemaView()->m_editConnectionLines.clear();
					editSchemaView()->m_editConnectionLines.push_back(ecl);

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

	EditConnectionLine ecl(std::dynamic_pointer_cast<VFrame30::PosConnectionImpl>(editSchemaView()->m_newItem),
						   EditConnectionLine::AddToEnd);

	ecl.addBasePoint(docPoint);
	ecl.addExtensionPoint(docPoint);

	editSchemaView()->m_editConnectionLines.clear();
	editSchemaView()->m_editConnectionLines.push_back(ecl);

	setMouseState(MouseState::AddSchemaPosConnectionNextPoint);

	return;
}

void EditSchemaWidget::mouseLeftUp_Selection(QMouseEvent* me)
{
	m_nextSelectionFromLeft = {};		// Reset state for slection next items via Alt + Arrow Keys
	m_nextSelectionFromRight = {};

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

	// If Alt is pressed then moving in one dirrection horz or vert
	//
	if (bool altIsPressed = QApplication::queryKeyboardModifiers().testFlag(Qt::AltModifier);
		altIsPressed)
	{
		QPointF offset = mouseMovingEndPointIn - editSchemaView()->m_editStartDocPt;

		if (std::abs(offset.rx()) > std::abs(offset.ry()))
		{
			mouseMovingEndPointIn.setY(editSchemaView()->m_editStartDocPt.ry());
		}
		else
		{
			mouseMovingEndPointIn.setX(editSchemaView()->m_editStartDocPt.rx());
		}
	}

	// --
	//
	float xdif = mouseMovingEndPointIn.x() - mouseMovingStartPointIn.x();
	float ydif = mouseMovingEndPointIn.y() - mouseMovingStartPointIn.y();

	if (std::abs(xdif) < 0.0000001 && std::abs(ydif) < 0.0000001)
	{
		// SchemaItem's have not changed positions
		//
		resetAction();
		return;
	}

	if (bool ctrlIsPressed = event->modifiers().testFlag(Qt::ControlModifier);
		ctrlIsPressed == false)
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
			if (bool ok = m_editEngine->startBatch();
				ok == true)
			{
				m_editEngine->runMoveItem(xdif, ydif, itemsForMove, snapToGrid());
				finishMoveAfbsConnectionLinks();

				m_editEngine->endBatch();
			}
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
				bool result = si->saveToByteArray(&data);

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

	// Get new rect
	//
	QRectF newItemRect = editSchemaView()->sizingRectItem(xdif, ydif, itemPos);
	newItemRect = newItemRect.normalized();

	// --
	//
	std::vector<VFrame30::SchemaPoint> itemPoints;

	itemPoints.push_back(VFrame30::SchemaPoint(newItemRect.topLeft()));
	itemPoints.push_back(VFrame30::SchemaPoint(newItemRect.bottomRight()));

	if (bool ok = m_editEngine->startBatch();
		ok == true)
	{
		m_editEngine->runSetPoints(itemPoints, si, true);
		finishMoveAfbsConnectionLinks();

		m_editEngine->endBatch();
	}

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

	m_editEngine->runSetPoints(points, si, true);

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
	if (editSchemaView()->m_newItem == nullptr ||
		editSchemaView()->m_editConnectionLines.size() != 1)
	{
		assert(editSchemaView()->m_newItem != nullptr);
		assert(editSchemaView()->m_editConnectionLines.size() == 1);
		resetAction();
		return;
	}

	VFrame30::IPosConnection* itemPos = dynamic_cast<VFrame30::IPosConnection*>(editSchemaView()->m_newItem.get());
	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		resetAction();
		return;
	}

	// Add the last point, where cursor is now to ALL
	//
	mouseRightDown_AddSchemaPosConnectionNextPoint(e);

	const EditConnectionLine& ecl = editSchemaView()->m_editConnectionLines.front();
	ecl.setPointToItem(std::dynamic_pointer_cast<VFrame30::PosConnectionImpl>(editSchemaView()->m_newItem));

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


		std::list<std::shared_ptr<VFrame30::SchemaItem>> linksUnderStartPoint =
				activeLayer()->getItemListUnderPoint(QPointF(startPoint.X, startPoint.Y), editSchemaView()->m_newItem->metaObject()->className());

		std::list<std::shared_ptr<VFrame30::SchemaItem>> linksUnderEndPoint =
				activeLayer()->getItemListUnderPoint(QPointF(endPoint.X, endPoint.Y), editSchemaView()->m_newItem->metaObject()->className());

		std::shared_ptr<VFrame30::SchemaItem> linkUnderStartPoint = linksUnderStartPoint.size() == 1 ? linksUnderStartPoint.front() : std::shared_ptr<VFrame30::SchemaItem>();
		std::shared_ptr<VFrame30::SchemaItem> linkUnderEndPoint = linksUnderEndPoint.size() == 1 ? linksUnderEndPoint.front() : std::shared_ptr<VFrame30::SchemaItem>();

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
				newPoints = EditConnectionLine::removeUnwantedPoints(newPoints);

				m_editEngine->runSetPoints(newPoints, linkUnderStartPoint, true);
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
					newPoints = EditConnectionLine::removeUnwantedPoints(newPoints);

					m_editEngine->runSetPoints(newPoints, linkUnderStartPoint, true);
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
					newPoints = EditConnectionLine::removeUnwantedPoints(newPoints);

					m_editEngine->runSetPoints(newPoints, linkUnderEndPoint, true);
				}
				else
				{
					// � �������� schemaItemEndPoint �������� points
					//
					existingItemPoints.pop_front();

					points.insert(points.end(), existingItemPoints.begin(), existingItemPoints.end());

					std::vector<VFrame30::SchemaPoint> newPoints(points.begin(), points.end());
					newPoints = EditConnectionLine::removeUnwantedPoints(newPoints);

					m_editEngine->runSetPoints(newPoints, linkUnderEndPoint, true);
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
						newPoints = EditConnectionLine::removeUnwantedPoints(newPoints);

						m_editEngine->runSetPoints(newPoints, linkUnderEndPoint, true);
					}
					else
					{
						// � �������� schemaItemEndPoint �������� points
						//
						existingItemPoints.reverse();
						existingItemPoints.pop_front();
						points.insert(points.end(), existingItemPoints.begin(), existingItemPoints.end());

						std::vector<VFrame30::SchemaPoint> newPoints(points.begin(), points.end());
						newPoints = EditConnectionLine::removeUnwantedPoints(newPoints);

						m_editEngine->runSetPoints(newPoints, linkUnderEndPoint, true);
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

				std::list<VFrame30::SchemaPoint> newPoints = EditConnectionLine::removeUnwantedPoints(pointList);

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

	if (selectedItems().size() != 1 ||
		editSchemaView()->m_editConnectionLines.size() != 1)
	{
		assert(selectedItems().size() == 1);
		assert(editSchemaView()->m_editConnectionLines.size() == 1);

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

	// Check if the real change vertex or edge has been done
	//
	EditConnectionLine& ecl = editSchemaView()->m_editConnectionLines.front();

	if (ecl.mode() == EditConnectionLine::AddToBegin ||
		ecl.mode() == EditConnectionLine::AddToEnd)
	{
		QPointF lastExtPt = ecl.lastExtensionPoint();

		ecl.moveExtensionPointsToBasePoints();
		ecl.addExtensionPoint(lastExtPt);
	}

	auto basePoints = ecl.basePoints();

	std::list<VFrame30::SchemaPoint> newPoints = {basePoints.begin(), basePoints.end()};
	const std::list<VFrame30::SchemaPoint>& itemPoints = itemPos->GetPointList();

	if (newPoints == itemPoints)
	{
		// Nothing has changed, do not exec a command
		//
		resetAction();
		return;
	}

	if (ecl.mode() == EditConnectionLine::AddToBegin ||
		ecl.mode() == EditConnectionLine::AddToEnd)
	{
		newPoints = EditConnectionLine::removeUnwantedPoints(newPoints);
	}

	std::vector<VFrame30::SchemaPoint> setPoints(newPoints.begin(), newPoints.end());
	m_editEngine->runSetPoints(setPoints, si, true);

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

	// If Alt is pressed then moving in one dirrection horz or vert
	//
	if (bool altIsPressed = QApplication::queryKeyboardModifiers().testFlag(Qt::AltModifier);
		altIsPressed)
	{
		QPointF offset = editSchemaView()->m_editEndDocPt - editSchemaView()->m_editStartDocPt;

		if (std::abs(offset.rx()) > std::abs(offset.ry()))
		{
			editSchemaView()->m_editEndDocPt.setY(editSchemaView()->m_editStartDocPt.ry());
		}
		else
		{
			editSchemaView()->m_editEndDocPt.setX(editSchemaView()->m_editStartDocPt.rx());
		}
	}

	// Move links along item
	//
	QPointF offset = editSchemaView()->m_editEndDocPt - editSchemaView()->m_editStartDocPt;

	moveAfbsConnectionLinks(offset, mouseState());

	// --
	//
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

	VFrame30::IPosRect* itemPos = dynamic_cast<VFrame30::IPosRect*>(selectedItems().front().get());
	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		setMouseState(MouseState::None);
		return;
	}

	editSchemaView()->m_editEndDocPt = widgetPointToDocument(me->pos(), snapToGrid());

	// Get possible links offset
	//
	double xdif = editSchemaView()->m_editEndDocPt.x() - editSchemaView()->m_editStartDocPt.x();
	double ydif = editSchemaView()->m_editEndDocPt.y() - editSchemaView()->m_editStartDocPt.y();

	QRectF currentRect(itemPos->leftDocPt(),
						   itemPos->topDocPt(),
						   itemPos->widthDocPt(),
						   itemPos->heightDocPt());

	QRectF newRect = editSchemaView()->sizingRectItem(xdif, ydif, itemPos);

	switch (mouseState())
	{
	case MouseState::SizingTop:
		xdif = 0;
		ydif = newRect.top() - currentRect.top();
		break;
	case MouseState::SizingTopRight:
		xdif = newRect.right() - currentRect.right();
		ydif = newRect.top() - currentRect.top();
		break;
	case MouseState::SizingRight:
		xdif = newRect.right() - currentRect.right();
		ydif = 0;
		break;
	case MouseState::SizingBottomRight:
		xdif = newRect.right() - currentRect.right();
		ydif = newRect.bottom() - currentRect.bottom();
		break;
	case MouseState::SizingBottom:
		xdif = 0;
		ydif = newRect.bottom() - currentRect.bottom();
		break;
	case MouseState::SizingBottomLeft:
		xdif = newRect.left() - currentRect.left();
		ydif = newRect.bottom() - currentRect.bottom();
		break;
	case MouseState::SizingLeft:
		xdif = newRect.left() - currentRect.left();
		ydif = 0;
		break;
	case MouseState::SizingTopLeft:
		xdif = newRect.left() - currentRect.left();
		ydif = newRect.top() - currentRect.top();
		break;
	default:
		assert(false);
	}

	// Move links
	//
	moveAfbsConnectionLinks(QPointF(xdif, ydif), mouseState());

	// --
	//
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
	if (editSchemaView()->m_newItem == nullptr)
	{
		assert(editSchemaView()->m_newItem != nullptr);

		resetAction();
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	// magnet point to pin
	//
	docPoint = magnetPointToPin(docPoint);

	for (EditConnectionLine& ecl : editSchemaView()->m_editConnectionLines)
	{
		movePosConnectionEndPoint(editSchemaView()->m_newItem, &ecl, docPoint);
	}

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

	if (selectedItems().size() != 1 ||
		editSchemaView()->m_editConnectionLines.size() != 1)
	{
		assert(selectedItems().size() == 1);
		assert(editSchemaView()->m_editConnectionLines.size() != 1);
		resetAction();
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	EditConnectionLine& ecl = editSchemaView()->m_editConnectionLines.front();

	// --
	//
	switch (mouseState())
	{
	case MouseState::MovingHorizontalEdge:
		ecl.modifyEdge(docPoint.y());
		break;
	case MouseState::MovingVerticalEdge:
		ecl.modifyEdge(docPoint.x());
		break;
	case MouseState::MovingConnectionLinePoint:
		{
			switch (ecl.mode())
			{
			case EditConnectionLine::EditMode::EditPoint:
				docPoint = magnetPointToPin(docPoint);
				ecl.modifyPoint(docPoint);
				break;

			case EditConnectionLine::EditMode::AddToBegin:
			case EditConnectionLine::EditMode::AddToEnd:
				docPoint = magnetPointToPin(docPoint);
				movePosConnectionEndPoint(selectedItems().front(), &ecl, docPoint);
				break;

			default:
				assert(false);
			}
		}
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

void EditSchemaWidget::mouseRightDown_AddSchemaPosConnectionNextPoint(QMouseEvent* /*event*/)
{
	if (editSchemaView()->m_newItem == nullptr ||
		editSchemaView()->m_editConnectionLines.size() != 1)
	{
		assert(editSchemaView()->m_newItem != nullptr);
		assert(editSchemaView()->m_editConnectionLines.size() == 1);

		resetAction();
		return;
	}

	for (EditConnectionLine& ecl : editSchemaView()->m_editConnectionLines)
	{
		assert(ecl.extensionPoints().empty() == false);

		QPointF lastExtPt = ecl.lastExtensionPoint();

		ecl.moveExtensionPointsToBasePoints();
		ecl.addExtensionPoint(lastExtPt);
	}

	// --
	//
	editSchemaView()->update();

	return;
}

void EditSchemaWidget::mouseRightDown_MovingEdgesOrVertex(QMouseEvent* event)
{
	if (selectedItems().size() != 1 ||
		editSchemaView()->m_editConnectionLines.size() != 1)
	{
		assert(selectedItems().size() == 1 );
		assert(editSchemaView()->m_editConnectionLines.size() == 1);

		resetAction();
		return;
	}

	EditConnectionLine& ecl = editSchemaView()->m_editConnectionLines.front();

	if (ecl.mode() != EditConnectionLine::EditMode::EditPoint &&
		ecl.mode() != EditConnectionLine::EditMode::AddToBegin &&
		ecl.mode() != EditConnectionLine::EditMode::AddToEnd)
	{
		assert(false);

		resetAction();
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	switch (ecl.mode())
	{
	case EditConnectionLine::EditMode::EditPoint:
		ecl.addPointAndSwitchMode(docPoint);
		break;

	case EditConnectionLine::EditMode::AddToBegin:
	case EditConnectionLine::EditMode::AddToEnd:
		{
			assert(ecl.extensionPoints().empty() == false);
			QPointF lastExtPt = ecl.lastExtensionPoint();

			ecl.moveExtensionPointsToBasePoints();
			ecl.addExtensionPoint(lastExtPt);
		}
		break;
	default:
		assert(false);
		resetAction();
	}

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

bool EditSchemaWidget::isMonitorSchema() const
{
	return schema()->isMonitorSchema();
}

bool EditSchemaWidget::isTuningSchema() const
{
	return schema()->isTuningSchema();
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

std::vector<std::shared_ptr<VFrame30::SchemaItem>> EditSchemaWidget::selectedNonLockedItems() const
{
	return editSchemaView()->selectedNonLockedItems();
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
	QApplication::setOverrideCursor(Qt::WaitCursor);
	QApplication::processEvents();

	// Update Afb list
	//
	std::vector<std::shared_ptr<Afb::AfbElement>> afbs;

	bool ok = loadAfbsDescriptions(&afbs);
	if (ok == false)
	{
		QApplication::restoreOverrideCursor();
		return false;
	}

	QString errorMessage;
	int updatedItemCount = 0;

	ok = schema()->updateAllSchemaItemFbs(afbs, &updatedItemCount, &errorMessage);

	QApplication::restoreOverrideCursor();

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
		msgBox.setInformativeText("Please, check input/output pins and parameters.\nClose schema without saving to discard changes.");
		msgBox.exec();
	}

	return true;
}

bool EditSchemaWidget::updateUfbsForSchema()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	QApplication::processEvents();

	// Get Ufb list
	//
	std::vector<std::shared_ptr<VFrame30::UfbSchema>> ufbs;

	bool ok = loadUfbSchemas(&ufbs);

	if (ok == false)
	{
		QApplication::restoreOverrideCursor();
		return false;
	}

	// Update
	//
	QString errorMessage;
	int updatedItemCount = 0;

	ok = schema()->updateAllSchemaItemUfb(ufbs, &updatedItemCount, &errorMessage);

	QApplication::restoreOverrideCursor();

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
		msgBox.setInformativeText("Please, check input/output pins and parameters.\nClose schema without saving to discard changes.");
		msgBox.exec();
	}

	return true;
}

bool EditSchemaWidget::updateBussesForSchema()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	QApplication::processEvents();

	// Get Bus list
	//
	std::vector<VFrame30::Bus> busses;

	bool ok = loadBusses(db(), &busses, this);

	if (ok == false)
	{
		QApplication::restoreOverrideCursor();
		return false;
	}

	// Update
	//
	QString errorMessage;
	int updatedItemCount = 0;

	ok = schema()->updateAllSchemaItemBusses(busses, &updatedItemCount, &errorMessage);

	QApplication::restoreOverrideCursor();

	if (ok == false)
	{
		QMessageBox::critical(this, qApp->applicationName(), tr("Update Bus items error: ") + errorMessage);
		return false;
	}

	if (updatedItemCount != 0)
	{
		setModified();

		QMessageBox msgBox(this);
		msgBox.setWindowTitle(qApp->applicationName());
		msgBox.setText(tr("%1 Bus(s) are updated according to the latest Bus descriptions.").arg(updatedItemCount));
		msgBox.setInformativeText("Please, check input/output pins and parameters.\nClose schema without saving to discard changes.");
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

	// Setting cursor specific cases
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
			std::shared_ptr<VFrame30::SchemaItem> itemUnderPoint = editSchemaView()->activeLayer()->getItemUnderPoint(docPos);

			// ���� ������� �� �������, �� ��� ����� ������ ����������
			//
			if (itemUnderPoint != nullptr &&
				editSchemaView()->getPossibleAction(itemUnderPoint.get(), docPos, &movingEdgePointIndex) == SchemaItemAction::MoveItem)
			{
				setCursor(Qt::SizeAllCursor);
				return;
			}
		}

		for (std::shared_ptr<VFrame30::SchemaItem> si : editSchemaView()->selectedItems())
		{
			SchemaItemAction possibleAction = editSchemaView()->getPossibleAction(si.get(), docPos, &movingEdgePointIndex);

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

void EditSchemaWidget::movePosConnectionEndPoint(std::shared_ptr<VFrame30::SchemaItem> schemaItem,  EditConnectionLine* ecl, QPointF toPoint)
{
	assert(schemaItem);
	assert(ecl);

	ecl->moveEndPointPos(activeLayer(), toPoint, EditConnectionLine::Auto, schema()->gridSize());

	return;
}

void EditSchemaWidget::initMoveAfbsConnectionLinks(MouseState mouseState)
{
	editSchemaView()->m_editConnectionLines.clear();
	editSchemaView()->m_doNotMoveConnectionLines = false;

	// Go over all selected itmes pins, and add data to m_editConnectionLines
	//
	std::vector<std::shared_ptr<VFrame30::SchemaItem>> selected = selectedNonLockedItems();
	std::multiset<std::shared_ptr<VFrame30::SchemaItemLink>> commonLinks;

	for (std::shared_ptr<VFrame30::SchemaItem> item : selected)
	{
		if (item->isFblItemRect() == false)
		{
			continue;
		}

		VFrame30::FblItemRect* fblItemRect = item->toFblItemRect();
		assert(fblItemRect);

		// Get links with end on pins
		//
		double gridSize = schema()->gridSize();
		int pinGridStep = schema()->pinGridStep();

		fblItemRect->SetConnectionsPos(gridSize, pinGridStep);			// Calc pins' positions

		const std::vector<VFrame30::AfbPin>& inputs = fblItemRect->inputs();
		const std::vector<VFrame30::AfbPin>& outputs = fblItemRect->outputs();

		std::vector<VFrame30::AfbPin> inOuts;
		inOuts.reserve(inputs.size() + outputs.size());

		switch (mouseState)
		{
		case MouseState::Moving:
		case MouseState::SizingTop:
		case MouseState::SizingTopRight:
		case MouseState::SizingTopLeft:
			inOuts.insert(inOuts.end(), inputs.begin(), inputs.end());
			inOuts.insert(inOuts.end(), outputs.begin(), outputs.end());
			break;

		case MouseState::SizingRight:
		case MouseState::SizingBottomRight:
			inOuts.insert(inOuts.end(), outputs.begin(), outputs.end());
			break;

		case MouseState::SizingLeft:
		case MouseState::SizingBottomLeft:
			inOuts.insert(inOuts.end(), inputs.begin(), inputs.end());
			break;
		default:
			break;
		}

		for (const VFrame30::AfbPin& pin : inOuts)
		{
			VFrame30::SchemaPoint pinPos = pin.point();

			std::list<std::shared_ptr<VFrame30::SchemaItem>> linksUnderPin =
					activeLayer()->getItemListUnderPoint(pinPos, "VFrame30::SchemaItemLink");

			// Check if pin on the Start or End point
			//
			for (std::shared_ptr<VFrame30::SchemaItem> foundLinkItem : linksUnderPin)
			{
				std::shared_ptr<VFrame30::SchemaItemLink> link = std::dynamic_pointer_cast<VFrame30::SchemaItemLink>(foundLinkItem);
				assert(link);

				// If this link in selected items, slip it
				//
				auto foundInSelectedIt = std::find(selected.begin(), selected.end(), foundLinkItem);
				if (foundInSelectedIt != selected.end())
				{
					continue;
				}

				// Get end points of found link and check if they are on pin
				//
				VFrame30::SchemaPoint ptBegin = link->GetPointList().front();
				VFrame30::SchemaPoint ptEnd = link->GetPointList().back();

				if (pinPos == ptBegin)
				{
					EditConnectionLine ecl(link, EditConnectionLine::MoveToPin);
					ecl.moveToPin_init(link, pin.dirrection(), ptBegin);

					editSchemaView()->m_editConnectionLines.push_back(ecl);

					commonLinks.insert(link);
				}

				if (pinPos == ptEnd)
				{
					EditConnectionLine ecl(link, EditConnectionLine::MoveToPin);
					ecl.moveToPin_init(link, pin.dirrection(), ptEnd);

					editSchemaView()->m_editConnectionLines.push_back(ecl);

					commonLinks.insert(link);
				}
			}
		}
	}

	// Ckeck if there is EditConnectionLine which is going to be moved from both sides
	// If [SIGNAL1] and [SIGNAL2] are selected, the select their common links, and remove it from editSchemaView()->m_editConnectionLines
	//
	// [SIGNAL1]-+---------------+-[SIGNAL2]
	//
	for (std::shared_ptr<VFrame30::SchemaItemLink> cl: commonLinks)
	{
		size_t useCount = commonLinks.count(cl);

		if (useCount > 1)
		{
			auto it = std::find_if(editSchemaView()->m_editConnectionLines.begin(),
								   editSchemaView()->m_editConnectionLines.end(),
									[cl](const EditConnectionLine& ecl)
									{
										return ecl.moveToPin_schemaItem() == cl;
									});

			if (it != editSchemaView()->m_editConnectionLines.end())
			{
				it->moveToPin_setMoveWholeLink();

				// Remmove all other occurances of Link in m_editConnectionLines
				//
				auto removeIt = std::remove_if(++it, editSchemaView()->m_editConnectionLines.end(),
												[cl](const EditConnectionLine& ecl)
												{
													return ecl.moveToPin_schemaItem() == cl;
												});

				editSchemaView()->m_editConnectionLines.erase(removeIt, editSchemaView()->m_editConnectionLines.end());
			}
		}
	}

	return;
}

void EditSchemaWidget::moveAfbsConnectionLinks(QPointF offset, MouseState mouseState)
{
	for (EditConnectionLine& ecl : editSchemaView()->m_editConnectionLines)
	{
		QPointF eclOffset;

		switch (mouseState)
		{
		case MouseState::Moving:
			eclOffset = offset;
			break;

		case MouseState::SizingTop:
			eclOffset.ry() = offset.y();
			break;

		case MouseState::SizingBottom:
			assert(false);
			return;

		case MouseState::SizingTopLeft:
			if (ecl.moveToPin_isInput() == true)
			{
				eclOffset = offset;
			}
			else
			{
				eclOffset.ry() = offset.y();
			}
			break;

		case MouseState::SizingLeft:
			if (ecl.moveToPin_isInput() == true)
			{
				eclOffset.rx() = offset.x();
			}
			else
			{
				assert(false);	// Don't add outputs in this case
			}
			break;

		case MouseState::SizingBottomLeft:
			if (ecl.moveToPin_isInput() == true)
			{
				eclOffset.rx() = offset.x();
			}
			else
			{
				assert(false);	// Don't add outputs in this case
			}
			break;

		case MouseState::SizingTopRight:
			if (ecl.moveToPin_isInput() == true)
			{
				eclOffset.ry() = offset.y();
			}
			else
			{
				eclOffset = offset;
			}
			break;

		case MouseState::SizingRight:
			if (ecl.moveToPin_isInput() == true)
			{
				assert(false);	// Don't add outputs in this case
			}
			else
			{
				eclOffset.rx() = offset.x();
			}
			break;

		case MouseState::SizingBottomRight:
			if (ecl.moveToPin_isInput() == true)
			{
				assert(false);	// Don't add outputs in this case
			}
			else
			{
				eclOffset.rx() = offset.x();
			}
			break;

		default:
			assert(false);
		}

		ecl.moveToPin_offset(activeLayer(), eclOffset, schema()->gridSize());
	}

	return;
}

void EditSchemaWidget::finishMoveAfbsConnectionLinks()
{
	setFocus();	// As alt could be pressed and MainMeu activated

	bool ctrlIsPressed = QApplication::queryKeyboardModifiers().testFlag(Qt::ControlModifier);

	if (ctrlIsPressed == true ||
		editSchemaView()->m_doNotMoveConnectionLines == true)
	{
		editSchemaView()->m_editConnectionLines.clear();
		return;
	}

	std::vector<std::vector<VFrame30::SchemaPoint>> commandPoints;
	std::vector<std::shared_ptr<VFrame30::SchemaItem>> commandItems;

	for (EditConnectionLine& ecl : editSchemaView()->m_editConnectionLines)
	{
		ecl.moveExtensionPointsToBasePoints();
		std::vector<QPointF> points = ecl.points();

		std::list<VFrame30::SchemaPoint> uniquePoints(points.begin(), points.end());
		uniquePoints.unique();

		uniquePoints = EditConnectionLine::removeUnwantedPoints(uniquePoints);

		std::vector<VFrame30::SchemaPoint> resultPoinst(uniquePoints.begin(), uniquePoints.end());

		commandPoints.push_back(resultPoinst);
		commandItems.push_back(ecl.moveToPin_schemaItem());
	}

	if (commandPoints.empty() == false)
	{
		assert(commandPoints.size() == commandItems.size());

		m_editEngine->runSetPoints(commandPoints, commandItems, false);

		editSchemaView()->m_editConnectionLines.clear();
	}

	return;
}

bool EditSchemaWidget::loadAfbsDescriptions(std::vector<std::shared_ptr<Afb::AfbElement>>* out)
{
	if (out == nullptr)
	{
		assert(out);
		return false;
	}

	if (isLogicSchema() == false &&
		isUfbSchema() == false)
	{
		// this function is not applicable
		//
		return false;
	}

	QString LmDescriptionFile;

	if (isLogicSchema() == true)
	{
		LmDescriptionFile = schema()->toLogicSchema()->lmDescriptionFile();
	}

	if (isUfbSchema() == true)
	{
		LmDescriptionFile = schema()->toUfbSchema()->lmDescriptionFile();
	}

	if (LmDescriptionFile.isEmpty() == true)
	{
		QString errorMsg = tr("Scheme property LmDescription is empty. It must contain LogicModuleDescription filename.");
		QMessageBox::critical(this, qApp->applicationName(), errorMsg);
		return false;
	}

	std::vector<DbFileInfo> fileList;

	bool result = db()->getFileList(&fileList, db()->afblFileId(), LmDescriptionFile, true, this);
	if (result == false)
	{
		return false;
	}

	if (fileList.empty() == true)
	{
		QString errorMsg = tr("Cannot find file %1.").arg(LmDescriptionFile);
		QMessageBox::critical(this, qApp->applicationName(), errorMsg);
		return false;
	}

	// Get description file from the DB
	//
	std::shared_ptr<DbFile> file;
	result = db()->getLatestVersion(fileList[0], &file, this);
	if (result == false)
	{
		return false;
	}

	// Parse file
	//
	LmDescription lm;
	QString parseErrorMessage;

	result = lm.load(file->data(), &parseErrorMessage);

	if (result == false)
	{
		QString errorMsg = tr("Cannot parse file %1. Error message: %2").arg(LmDescriptionFile).arg(parseErrorMessage);
		QMessageBox::critical(this, qApp->applicationName(), errorMsg);
		return false;
	}

	// Get the AFBs and return them
	//
	std::vector<std::shared_ptr<Afb::AfbElement>> afbs = lm.afbs();

	std::swap(*out, afbs);

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
	DbFileTree filesTree;

	bool ok = db()->getFileListTree(&filesTree, db()->ufblFileId(), "%", true, this);
	if (ok == false)
	{
		return false;
	}

	std::vector<DbFileInfo> fileList = filesTree.toVectorIf(
		[](const DbFileInfo& file)
		{
			return file.fileName().endsWith(QString(".") + Db::File::UfbFileExtension, Qt::CaseInsensitive) == true &&
				file.isFolder() == false;
		});


	// Get UFBs where LmDescriptionFile same with this chema
	//
	std::vector<DbFileInfo> filteredFileList;
	filteredFileList.reserve(fileList.size());

	if (schema()->isLogicSchema() == true)
	{
		for (const auto& fi : fileList)
		{
			VFrame30::SchemaDetails details(fi.details());

			if (details.m_lmDescriptionFile == schema()->toLogicSchema()->lmDescriptionFile())
			{
				filteredFileList.push_back(fi);
			}
		}
	}
	else
	{
		filteredFileList = fileList;
	}

	// Get UFBs latest version from the DB
	//
	std::vector<std::shared_ptr<DbFile>> files;

	ok = db()->getLatestVersion(filteredFileList, &files, this);

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

bool EditSchemaWidget::loadBusses(DbController* db, std::vector<VFrame30::Bus>* out, QWidget* parentWidget)
{
	if (db == nullptr ||
		out == nullptr)
	{
		assert(db);
		assert(out);
		return false;
	}

	out->clear();

	// Get Busses
	//
	DbFileTree filesTree;

	bool ok = db->getFileListTree(&filesTree, db->busTypesFileId(), QString(".") + Db::File::BusFileExtension, true, parentWidget);
	if (ok == false)
	{
		return false;
	}

	const auto& fileMap = filesTree.files();

	std::vector<DbFileInfo> fileList;
	fileList.reserve(fileMap.size());

	for (auto&[fileId, fileInfo] : fileMap)
	{
		if (fileId != db->busTypesFileId())
		{
			fileList.push_back(*fileInfo);
		}
	}

	if (fileList.empty() == true)
	{
		return true;	// It is not error, just no any busses
	}

	// Get Busses latest version from the DB
	//
	std::vector<std::shared_ptr<DbFile>> files;

	ok = db->getLatestVersion(fileList, &files, parentWidget);
	if (ok == false)
	{
		return false;
	}

	// Parse files, create actual Busses
	//
	std::vector<VFrame30::Bus> busses;
	busses.reserve(files.size());

	for (const std::shared_ptr<DbFile>& f : files)
	{
		if (f->deleted() == true ||
			f->action() == VcsItemAction::Deleted)
		{
			continue;
		}

		VFrame30::Bus bus;
		ok = bus.Load(f->data());

		if (ok == false)
		{
			QMessageBox::critical(parentWidget, qAppName(), "Load file " + f->fileName() + " error.");
			return false;
		}

		busses.push_back(bus);
	}

	std::sort(busses.begin(), busses.end(),
			[](const VFrame30::Bus& b1, const VFrame30::Bus& b2) -> bool
			{
				return b1.busTypeId() < b2.busTypeId();
			});

	std::swap(busses, *out);
	return true;
}

void EditSchemaWidget::resetAction()
{
	setMouseState(MouseState::None);
	editSchemaView()->m_newItem.reset();

	editSchemaView()->m_editConnectionLines.clear();
	editSchemaView()->m_mouseSelectionStartPoint = QPoint();
	editSchemaView()->m_mouseSelectionEndPoint = QPoint();
	editSchemaView()->m_editStartDocPt = QPointF();
	editSchemaView()->m_editEndDocPt = QPointF();

	setMouseCursor(mapFromGlobal(QCursor::pos()));

	editSchemaView()->update();

	return;
}

void EditSchemaWidget::clearSelection()
{
	m_nextSelectionFromLeft = {};		// Reset state for slection next items via Alt + Arrow Keys
	m_nextSelectionFromRight = {};

	editSchemaView()->clearSelection();
}

void EditSchemaWidget::contextMenu(const QPoint& pos)
{
	if (mouseState() == MouseState::AddSchemaPosConnectionNextPoint ||
		mouseState() == MouseState::MovingConnectionLinePoint)
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

	// All selected are signals?
	//
	bool allSelectedAreSignals = selectedItems().empty() == true ? false : true;
	for (auto item : selectedItems())
	{
		if (item->isType<VFrame30::SchemaItemSignal>() == false)
		{
			allSelectedAreSignals = false;
			break;
		}
	}

	// Disable some actions in ReadOnly mode
	//
	m_addAction->setDisabled(readOnly());
	updateFileActions();

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

	if (allSelectedAreSignals == true)
	{
		actions << m_transformAction;
	}

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

					for (const QString& appSignal : itemReceiver->appSignalIdsAsList())
					{
						if (appSignal.isEmpty() == false)
						{
							signalStrIds << appSignal;
						}
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
					if (signalStrIds.size() == 1)	// If not 1, then this shorcut will be added to "All Signals %1 Properties..."
					{
						signalAction->setShortcut(Qt::ALT + Qt::Key_S);
						signalAction->setShortcutVisibleInContextMenu(true);
					}

					connect(signalAction, &QAction::triggered,
							[s, this](bool)
							{
								QStringList sl;
								sl << s;
								this->appSignalsProperties(sl);
							});

					actions << signalAction;
				}

				if (signalStrIds.size() > 1)
				{
					QAction* allSignals = new QAction(tr("All Signals %1 Properties...").arg(signalStrIds.size()), &menu);
					allSignals->setShortcut(Qt::ALT + Qt::Key_S);
					allSignals->setShortcutVisibleInContextMenu(true);
					connect(allSignals, &QAction::triggered, this, &EditSchemaWidget::appSignalsSelectedProperties);

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
				addSignal->setShortcut(Qt::ALT + Qt::Key_N);
				addSignal->setShortcutVisibleInContextMenu(true);

				// Highlight this menu item if it was selected last time
				//
				QFont f = addSignal->font();
				f.setBold(m_lastSelectedAddSignal);
				m_lastSelectedAddSignal = false;

				addSignal->setFont(f);

				// --
				//
				connect(addSignal, &QAction::triggered,
					[this, selected](bool)
					{
						this->addNewAppSignal(selected);
						m_lastSelectedAddSignal = true;
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

	editSchemaView()->exportToPdf(fileName, editSchemaView()->session(), theSettings.infoMode());

	return;
}


void EditSchemaWidget::appSignalsSelectedProperties()
{
	return appSignalsProperties(QStringList{});
}

void EditSchemaWidget::appSignalsProperties(QStringList strIds)
{
	if (isLogicSchema() == false)
	{
		assert(isLogicSchema() == false);
		return;
	}

	if (strIds.isEmpty() == true && selectedItems().empty() == false)
	{
		// Get AppSignals from SchemaItems
		//
		QSet<QString> appSignalSet;		// QSet for unique strIds

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
						appSignalSet << s;
					}
				}
			}

			if (dynamic_cast<VFrame30::SchemaItemReceiver*>(item.get()) != nullptr)
			{
				VFrame30::SchemaItemReceiver* itemReceiver = dynamic_cast<VFrame30::SchemaItemReceiver*>(item.get());
				assert(itemReceiver);

				for (const QString& appSignal : itemReceiver->appSignalIdsAsList())
				{
					if (appSignal.isEmpty() == false)
					{
						appSignalSet << appSignal;
					}
				}
			}
		}

		for (QString s : appSignalSet)
		{
			strIds << s;
		}
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

//			QString appSignalId = itemReceiver->appSignalId();

//			auto foundInChanged = newIdsMap.find(appSignalId);

//			if (foundInChanged != newIdsMap.end())
//			{
//				// AppSignalIdWasChanged
//				//
//				m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalId, QVariant(foundInChanged->second), item);
//			}
			QStringList signalStrIdList = itemReceiver->appSignalIdsAsList();
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

				m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalId, QVariant(oneStringIds), itemPtrCopy);
			}

			continue;
		}
	}

	return;
}

void EditSchemaWidget::addNewAppSignalSelected()
{
	if (isLogicSchema() == false ||
		selectedItems().size() != 1)
	{
		return;
	}

	std::shared_ptr<VFrame30::SchemaItem> selected = selectedItems().front();
	auto itemSignal = dynamic_cast<VFrame30::SchemaItemSignal*>(selected.get());

	if (itemSignal == nullptr)
	{
		// it is not VFrame30::SchemaItemSignal
		//
		return;
	}

	return addNewAppSignal(selected);
}

void EditSchemaWidget::addNewAppSignal(std::shared_ptr<VFrame30::SchemaItem> schemaItem)
{
	qDebug() << "RPCT-2286 log: " << "void EditSchemaWidget::addNewAppSignal(std::shared_ptr<VFrame30::SchemaItem> schemaItem)";

	if (isLogicSchema() == false ||
		schemaItem == nullptr ||
		dynamic_cast<VFrame30::SchemaItemSignal*>(schemaItem.get()) == nullptr)
	{
		assert(isLogicSchema() == false);
		assert(schemaItem);
		assert(dynamic_cast<VFrame30::SchemaItemSignal*>(schemaItem.get()) != nullptr);
		qDebug() << "RPCT-2286 log: return condition 1";
		return;
	}

	QStringList equipmentIdList = logicSchema()->equipmentIdList();
	if (equipmentIdList.isEmpty() == true)
	{
		QMessageBox::critical(this, qAppName(), tr("Cannot create Application Signal as schema property EquipmentIDs is empty."));
		qDebug() << "RPCT-2286 log: return condition 2";
		return;
	}

	const VFrame30::SchemaItemSignal* signalItem = dynamic_cast<VFrame30::SchemaItemSignal*>(schemaItem.get());

	QStringList itemsAppSignals = signalItem->appSignalIdList();

	if (itemsAppSignals.size() == 1 &&
		(itemsAppSignals[0] == QLatin1String("#OUT_STRID") ||			// Not good, subject to change. must get default signals value from somewhere
		itemsAppSignals[0] == QLatin1String("#IN_STRID") ||
		itemsAppSignals[0] == QLatin1String("#APPSIGNALID")))
	{
		// This is just created signal item
		//
		itemsAppSignals.clear();				// clear - means generate new AppSignalIds
		qDebug() << "RPCT-2286 log: Check point 1";
	}
	else
	{
		qDebug() << "RPCT-2286 log: Check point 2";
	}

	m_createSignalDialoOptions.init(schema()->schemaId(),
									schema()->caption(),
									equipmentIdList,
									itemsAppSignals);

	qDebug() << "RPCT-2286 log: Check point 3";

	QStringList signalsIds = CreateSignalDialog::showDialog(db(), &m_createSignalDialoOptions, this);

	qDebug() << "RPCT-2286 log: Check point 4";

	if (signalsIds.isEmpty() == false)
	{
		qDebug() << "RPCT-2286 log: Check point 5 signalsIds: " << signalsIds.join(", ");

		// Set value
		//
		QString oneStringIds;
		for (QString s : signalsIds)
		{
			oneStringIds += s + QChar::LineFeed;
		}

		qDebug() << "RPCT-2286 log: Check point 6";
		m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalIDs, QVariant(oneStringIds), schemaItem);
		qDebug() << "RPCT-2286 log: Check point 7";
	}

	qDebug() << "RPCT-2286 log: Check point 8";

	//--------------------------------------------------
//	QStringList equipmentIdList = logicSchema()->equipmentIdList();
//	if (equipmentIdList.isEmpty() == true)
//	{
//		QMessageBox::critical(this, qAppName(), tr("Cannot create Application Signal as schema property EquipmentIDs is empty."));
//		return;
//	}

//	const VFrame30::SchemaItemSignal* signalItem = dynamic_cast<VFrame30::SchemaItemSignal*>(schemaItem.get());
//	QStringList itemsAppSignals = signalItem->appSignalIdList();

//	if (itemsAppSignals.size() == 1 &&
//		(itemsAppSignals[0] == QLatin1String("#OUT_STRID")) ||
//		(itemsAppSignals[0] == QLatin1String("#IN_STRID")) ||
//		(itemsAppSignals[0] == QLatin1String("#APPSIGNALID")))
//	{
//	}
//	else
//	{
//		m_createSignalOptions.appSignalIdList = itemsAppSignals;
//	}

//	int counterValue = 0;
//	bool nextValRes = db()->nextCounterValue(&counterValue);
//	if (nextValRes == false)
//	{
//		return;
//	}

//	m_createSignalOptions.lmEquipmentIdList = equipmentIdList;

//	QStringList signalsIds = SignalsTabPage::createSignal(db(),
//														  counterValue,
//														  schema()->schemaId(),
//														  schema()->caption(),
//														  &m_createSignalOptions,
//														  this);

//	if (signalsIds.isEmpty() == false)
//	{
//		// Set value
//		//
//		QString oneStringIds;
//		for (QString s : signalsIds)
//		{
//			oneStringIds += s + QChar::LineFeed;
//		}

//		m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalIDs, QVariant(oneStringIds), schemaItem);
//	}

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
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	if (item->isType<VFrame30::SchemaItemRect>() == true)
	{
		f2KeyForRect(item);
		return;
	}

	if (item->isType<VFrame30::SchemaItemSignal>() == true)
	{
		f2KeyForSignal(item);
		return;
	}

	if (item->isType<VFrame30::SchemaItemConst>() == true)
	{
		f2KeyForConst(item);
		return;
	}

	if (item->isType<VFrame30::SchemaItemReceiver>() == true)
	{
		f2KeyForReceiver(item, true);
		return;
	}

	if (item->isType<VFrame30::SchemaItemTransmitter>() == true)
	{
		f2KeyForTransmitter(item, true);
		return;
	}

	if (item->isType<VFrame30::SchemaItemLoopback>() == true)
	{
		f2KeyForLoopback(item);
		return;
	}

	if (item->isType<VFrame30::SchemaItemValue>() == true)
	{
		f2KeyForValue(item);
		return;
	}

	if (item->isType<VFrame30::SchemaItemImageValue>() == true)
	{
		f2KeyForImageValue(item);
		return;
	}

	if (item->isType<VFrame30::SchemaItemBus>() == true)
	{
		f2KeyForBus(item);
		return;
	}

	return;
}

void EditSchemaWidget::f2KeyForRect(std::shared_ptr<VFrame30::SchemaItem> item)
{
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	VFrame30::SchemaItemRect* rectItem = dynamic_cast<VFrame30::SchemaItemRect*>(item.get());
	if (rectItem == nullptr)
	{
		assert(rectItem);
		return;
	}

	QString text = rectItem->text();

	// Show input dialog
	//
	bool ok;
	QString newValue = QInputDialog::getMultiLineText(this, tr("Set text"),
													  tr("Text:"), text, &ok);
	if (ok == true &&
		newValue.isEmpty() == false &&
		newValue != text)
	{
		m_editEngine->runSetProperty(VFrame30::PropertyNames::text, QVariant(newValue), item);
		editSchemaView()->update();
	}

	return;
}

bool EditSchemaWidget::f2KeyForReceiver(std::shared_ptr<VFrame30::SchemaItem> item, bool setViaEditEngine)
{
	if (item == nullptr)
	{
		assert(item);
		return false;
	}

	VFrame30::SchemaItemReceiver* receiver = dynamic_cast<VFrame30::SchemaItemReceiver*>(item.get());
	if (receiver == nullptr)
	{
		assert(receiver);
		return false;
	}

	QString recConnectionIds = receiver->connectionIds();
	QString appSignalId = receiver->appSignalIds();

	// Get all connections
	//
	Hardware::ConnectionStorage connections(db());

	QString errorMessage;

	bool ok = connections.load(&errorMessage);
	if (ok == false)
	{
		QMessageBox::critical(this, qAppName(), errorMessage);
		return false;
	}

	auto ls = logicSchema();
	if (ls == nullptr)
	{
		assert(ls);
		return false;
	}

	QStringList connectionIds = connections.filterByMoudules(ls->equipmentIdList());

	// Show input dialog
	//
	QDialog d(this);

	d.setWindowTitle(tr("Set Receiver Params"));
	d.setWindowFlags((d.windowFlags() &
					~Qt::WindowMinimizeButtonHint &
					~Qt::WindowMaximizeButtonHint &
					~Qt::WindowContextHelpButtonHint) | Qt::CustomizeWindowHint);

	QLabel* connectionIdLabel = new QLabel("ConnectionID:");

	QTextEditCompleter* connectionIdControl = new QTextEditCompleter(&d);
	connectionIdControl->setPlaceholderText("Enter ConnectionID(s). Press Ctrl+E to show completer.");
	connectionIdControl->setPlainText(recConnectionIds);

	QCompleter* connectionsCompleter = new QCompleter(connectionIds, &d);
	connectionsCompleter->setFilterMode(Qt::MatchContains);
	connectionsCompleter->setCaseSensitivity(Qt::CaseSensitive);
	connectionsCompleter->setMaxVisibleItems(20);
	connectionIdControl->setCompleter(connectionsCompleter);

	// QCompleter for signals
	//
	SignalsModel* signalModel = SignalsModel::instance();
	Q_ASSERT(signalModel);

	QStringList appSignalIdsCompleterList = signalModel->signalSet().appSignalIdsList(true, true);

	QCompleter* appSignalsCompleter = new QCompleter(appSignalIdsCompleterList, &d);
	appSignalsCompleter->setFilterMode(Qt::MatchContains);
	appSignalsCompleter->setCaseSensitivity(Qt::CaseSensitive);
	appSignalsCompleter->setMaxVisibleItems(20);

	// AppSignalIDs widgets
	//
	QLabel* appSignalIdLabel = new QLabel("AppSignalID:");
	QTextEditCompleter* appSignalIdEdit = new QTextEditCompleter(&d);
	appSignalIdEdit->setPlaceholderText("Enter AppSchemaIDs separated by lines. Press Ctrl+E to show completer.");
	appSignalIdEdit->setPlainText(appSignalId);
	appSignalIdEdit->setCompleter(appSignalsCompleter);

	QWidget* spacer = new QWidget;
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	// --
	//
	QVBoxLayout* layout = new QVBoxLayout;

	layout->addWidget(connectionIdLabel);
	layout->addWidget(connectionIdControl);

	layout->addWidget(appSignalIdLabel);
	layout->addWidget(appSignalIdEdit);

	layout->addWidget(spacer);

	layout->addWidget(buttonBox);

	d.setLayout(layout);

	connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	// --
	//
	int result = d.exec();

	if (result == QDialog::Accepted)
	{
		QString newConnectionId = connectionIdControl->toPlainText();
		QString newAppSignalId = appSignalIdEdit->toPlainText().trimmed();

		if (newConnectionId != recConnectionIds ||
			newAppSignalId != appSignalId)
		{
			if (setViaEditEngine == true)
			{
				if (bool ok = m_editEngine->startBatch();
					ok == true)
				{
					m_editEngine->runSetProperty(VFrame30::PropertyNames::connectionId, QVariant(newConnectionId), item);
					m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalId, QVariant(newAppSignalId), item);

					m_editEngine->endBatch();
				}
			}
			else
			{
				receiver->setConnectionIds(newConnectionId);
				receiver->setAppSignalIds(newAppSignalId);
			}
		}

		editSchemaView()->update();
		return true;
	}

	return false;
}

bool EditSchemaWidget::f2KeyForTransmitter(std::shared_ptr<VFrame30::SchemaItem> item, bool setViaEditEngine)
{
	if (item == nullptr)
	{
		assert(item);
		return false;
	}

	VFrame30::SchemaItemTransmitter* transmitter = dynamic_cast<VFrame30::SchemaItemTransmitter*>(item.get());
	if (transmitter == nullptr)
	{
		assert(transmitter);
		return false;
	}

	QString transmitterConnectionIds = transmitter->connectionIds();

	// Get all connections
	//
	Hardware::ConnectionStorage connections(db());

	QString errorMessage;

	bool ok = connections.load(&errorMessage);
	if (ok == false)
	{
		QMessageBox::critical(this, qAppName(), errorMessage);
		return false;
	}

	auto ls = logicSchema();
	if (ls == nullptr)
	{
		assert(ls);
		return false;
	}

	QStringList connectionIds = connections.filterByMoudules(ls->equipmentIdList());

	// Show input dialog
	//
	QDialog d(this);

	d.setWindowTitle(tr("Set Transmitter Params"));
	d.setWindowFlags((d.windowFlags() &
					~Qt::WindowMinimizeButtonHint &
					~Qt::WindowMaximizeButtonHint &
					~Qt::WindowContextHelpButtonHint) | Qt::CustomizeWindowHint);

	QLabel* connectionIdLabel = new QLabel("ConnectionID(s):");

	QTextEditCompleter* connectionIdControl = new QTextEditCompleter(&d);
	connectionIdControl->setPlaceholderText("Enter ConnectionID(s). Press Ctrl+E to show completer.");
	connectionIdControl->setPlainText(transmitterConnectionIds);

	QCompleter* completer = new QCompleter(connectionIds, &d);
	completer->setFilterMode(Qt::MatchContains);
	completer->setCaseSensitivity(Qt::CaseSensitive);
	completer->setMaxVisibleItems(20);
	connectionIdControl->setCompleter(completer);

	QWidget* spacer = new QWidget;
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	// --
	//
	QVBoxLayout* layout = new QVBoxLayout;

	layout->addWidget(connectionIdLabel);
	layout->addWidget(connectionIdControl);

	layout->addWidget(spacer);

	layout->addWidget(buttonBox);

	d.setLayout(layout);

	connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	// --
	//
	int result = d.exec();

	if (result == QDialog::Accepted)
	{
		QString newConnectionId = connectionIdControl->toPlainText();  //connectionIdControl->currentText().trimmed();

		if (newConnectionId != transmitterConnectionIds)
		{
			if (setViaEditEngine == true)
			{
				m_editEngine->runSetProperty(VFrame30::PropertyNames::connectionId, QVariant(newConnectionId), item);
			}
			else
			{
				transmitter->setConnectionIds(newConnectionId);
			}
		}

		editSchemaView()->update();
		return true;
	}

	return false;
}

void EditSchemaWidget::f2KeyForConst(std::shared_ptr<VFrame30::SchemaItem> item)
{
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	VFrame30::SchemaItemConst* constItem = dynamic_cast<VFrame30::SchemaItemConst*>(item.get());
	if (constItem == nullptr)
	{
		assert(constItem);
		return;
	}

	VFrame30::SchemaItemConst::ConstType type = constItem->type();
	int intValue = constItem->intValue();
	double floatValue = constItem->floatValue();
	int discreteValue = constItem->discreteValue();

	// Show input dialog
	//
	QDialog d(this);

	d.setWindowTitle(tr("Set Const Params"));
	d.setWindowFlags((d.windowFlags() &
					~Qt::WindowMinimizeButtonHint &
					~Qt::WindowMaximizeButtonHint &
					~Qt::WindowContextHelpButtonHint) | Qt::CustomizeWindowHint);

	// Type Items
	//
	QLabel* typeLabel = new QLabel("Const Type:");

	QComboBox* typeCombo = new QComboBox();
	typeCombo->addItem("IntegerType", QVariant::fromValue<VFrame30::SchemaItemConst::ConstType>(VFrame30::SchemaItemConst::ConstType::IntegerType));
	typeCombo->addItem("FloatType", QVariant::fromValue<VFrame30::SchemaItemConst::ConstType>(VFrame30::SchemaItemConst::ConstType::FloatType));
	typeCombo->addItem("Discrete", QVariant::fromValue<VFrame30::SchemaItemConst::ConstType>(VFrame30::SchemaItemConst::ConstType::Discrete));

	int dataIndex = typeCombo->findData(QVariant::fromValue<VFrame30::SchemaItemConst::ConstType>(type));
	assert(dataIndex != -1);
	if (dataIndex != -1)
	{
		typeCombo->setCurrentIndex(dataIndex);
	}

	// IntItems
	//
	QLabel* intValueLabel = new QLabel("IntegerValue:");
	QLineEdit* intValueEdit = new QLineEdit(QString::number(intValue));
	intValueEdit->setValidator(new QIntValidator(std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), intValueEdit));

	if (type != VFrame30::SchemaItemConst::ConstType::IntegerType)
	{
		intValueLabel->setEnabled(false);
		intValueEdit->setEnabled(false);
	}

	// FloatItems
	//
	QLocale locale;

	QLabel* floatValueLabel = new QLabel("FloatValue:");
	QLineEdit* floatValueEdit = new QLineEdit(locale.toString(floatValue));
	floatValueEdit->setValidator(new QDoubleValidator(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max(), 1000, floatValueEdit));

	if (type != VFrame30::SchemaItemConst::ConstType::FloatType)
	{
		floatValueLabel->setEnabled(false);
		floatValueEdit->setEnabled(false);
	}

	// DiscreteItems
	//
	QLabel* discreteValueLabel = new QLabel("DiscreteValue (0 or 1):");
	QLineEdit* discreteValueEdit = new QLineEdit(QString::number(discreteValue));
	discreteValueEdit->setValidator(new QIntValidator(0, 1, discreteValueEdit));

	if (type != VFrame30::SchemaItemConst::ConstType::Discrete)
	{
		discreteValueLabel->setEnabled(false);
		discreteValueEdit->setEnabled(false);
	}

	// Spacer
	//
	QWidget* spacer = new QWidget;
	spacer->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

	// --
	//
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	QGridLayout* layout = new QGridLayout;

	layout->addWidget(typeLabel, 0, 0);
	layout->addWidget(typeCombo, 0, 1);

	layout->addWidget(intValueLabel, 1, 0);
	layout->addWidget(intValueEdit, 1, 1);

	layout->addWidget(floatValueLabel, 2, 0);
	layout->addWidget(floatValueEdit, 2, 1);

	layout->addWidget(discreteValueLabel, 3, 0);
	layout->addWidget(discreteValueEdit, 3, 1);

	layout->addWidget(spacer, 4, 0, 1, 2);

	layout->addWidget(buttonBox, 5, 0, 1, 2);

	d.setLayout(layout);

	connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	connect(typeCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			[typeCombo, intValueLabel, intValueEdit, floatValueLabel, floatValueEdit, discreteValueLabel, discreteValueEdit](int)
			{
				VFrame30::SchemaItemConst::ConstType type = typeCombo->currentData().value<VFrame30::SchemaItemConst::ConstType>();

				if (type == VFrame30::SchemaItemConst::ConstType::IntegerType)
				{
					intValueLabel->setEnabled(true);
					intValueEdit->setEnabled(true);

					floatValueLabel->setEnabled(false);
					floatValueEdit->setEnabled(false);

					discreteValueLabel->setEnabled(false);
					discreteValueEdit->setEnabled(false);
				}

				if (type == VFrame30::SchemaItemConst::ConstType::FloatType)
				{
					intValueLabel->setEnabled(false);
					intValueEdit->setEnabled(false);

					floatValueLabel->setEnabled(true);
					floatValueEdit->setEnabled(true);

					discreteValueLabel->setEnabled(false);
					discreteValueEdit->setEnabled(false);
				}

				if (type == VFrame30::SchemaItemConst::ConstType::Discrete)
				{
					intValueLabel->setEnabled(false);
					intValueEdit->setEnabled(false);

					floatValueLabel->setEnabled(false);
					floatValueEdit->setEnabled(false);

					discreteValueLabel->setEnabled(true);
					discreteValueEdit->setEnabled(true);
				}
			});

	// --
	//
	int result = d.exec();

	if (result == QDialog::Accepted)
	{
		VFrame30::SchemaItemConst::ConstType newType = typeCombo->currentData().value<VFrame30::SchemaItemConst::ConstType>();

		int newIntValue = intValueEdit->text().toInt();

		QLocale locale;
		double newFloatValue = locale.toFloat(floatValueEdit->text());
		int newDiscreteValue = discreteValueEdit->text().toInt();

		if (newType != type)
		{
			m_editEngine->runSetProperty(VFrame30::PropertyNames::type, QVariant(newType), item);
		}

		if (newIntValue != intValue)
		{
			m_editEngine->runSetProperty(VFrame30::PropertyNames::valueInteger, QVariant(newIntValue), item);
		}

		if (newFloatValue != floatValue)
		{
			m_editEngine->runSetProperty(VFrame30::PropertyNames::valueFloat, QVariant(newFloatValue), item);
		}

		if (newDiscreteValue != discreteValue)
		{
			m_editEngine->runSetProperty(VFrame30::PropertyNames::valueDiscrete, QVariant(newDiscreteValue), item);
		}

		editSchemaView()->update();
	}

	return;
}

void EditSchemaWidget::f2KeyForSignal(std::shared_ptr<VFrame30::SchemaItem> item)
{
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	VFrame30::SchemaItemSignal* signalItem = dynamic_cast<VFrame30::SchemaItemSignal*>(item.get());
	if (signalItem == nullptr)
	{
		assert(signalItem);
		return;
	}

	QString appSignalIds = signalItem->appSignalIds();
	QString impactAppSignalIds = signalItem->impactAppSignalIds();

	// Show input dialog
	//
	QDialog d(this);

	d.setWindowTitle(tr("SchemaItemSignal"));
	d.setWindowFlags((d.windowFlags() & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowContextHelpButtonHint)
					 | Qt::CustomizeWindowHint | Qt::WindowMaximizeButtonHint);

	// QCompleter for signals
	//
	SignalsModel* signalModel = SignalsModel::instance();
	Q_ASSERT(signalModel);

	QStringList appSignalIdsCompleterList = signalModel->signalSet().appSignalIdsList(true, true);

	QCompleter* completer = new QCompleter(appSignalIdsCompleterList, &d);
	completer->setFilterMode(Qt::MatchContains);
	completer->setCaseSensitivity(Qt::CaseSensitive);
	completer->setMaxVisibleItems(20);


	// AppSchemaIDs
	//
	QLabel* appSignalIdsLabel = new QLabel("AppSchemaIDs:", &d);

	QTextEditCompleter* appSignalIdsEdit = new QTextEditCompleter(&d);
	appSignalIdsEdit->setPlaceholderText("Enter AppSchemaIDs separated by lines. Press Ctrl+E to show completer.");
	appSignalIdsEdit->setPlainText(appSignalIds);
	appSignalIdsEdit->setCompleter(completer);

	// Text
	//
	QLabel* impactAppSignalIdsLabel = new QLabel("ImpactAppSignalIDs:", &d);
	impactAppSignalIdsLabel->setEnabled(signalItem->hasImpactColumn());

	QTextEditCompleter* impactAppSignalIdsEdit = new QTextEditCompleter(&d);
	impactAppSignalIdsEdit->setEnabled(signalItem->hasImpactColumn());
	impactAppSignalIdsEdit->setPlaceholderText("Enter Impact AppSchemaIDs separated by lines. Press Ctrl+E to show completer.");
	impactAppSignalIdsEdit->setPlainText(impactAppSignalIds);
	impactAppSignalIdsEdit->setCompleter(completer);

	// --
	//
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &d);
	buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);

	QGridLayout* layout = new QGridLayout(&d);

	layout->addWidget(appSignalIdsLabel, 0, 0, 1, 3);
	layout->addWidget(appSignalIdsEdit, 1, 0, 1, 3);

	layout->addWidget(impactAppSignalIdsLabel, 2, 0, 1, 3);
	layout->addWidget(impactAppSignalIdsEdit, 3, 0, 1, 3);

	layout->addWidget(buttonBox, 4, 0, 1, 3);

	d.setLayout(layout);

	connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	// --
	//
	int width = QSettings().value("f2KeyForSignal\\width").toInt();
	int height = QSettings().value("f2KeyForSignal\\height").toInt();
	d.resize(width, height);

	int result = d.exec();

	if (result == QDialog::Accepted)
	{
		QString newAppSignaIds = appSignalIdsEdit->toPlainText();
		QString newImpactAppSignaIds = impactAppSignalIdsEdit->toPlainText();

		if (newAppSignaIds != appSignalIds ||
			newImpactAppSignaIds != impactAppSignalIds)
		{
			if (bool ok = m_editEngine->startBatch();
				ok == true)
			{
				m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalIDs, QVariant(newAppSignaIds), item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::impactAppSignalIDs, QVariant(newImpactAppSignaIds), item);

				m_editEngine->endBatch();
			}
		}
	}

	QSettings().setValue("f2KeyForSignal\\width", d.width());
	QSettings().setValue("f2KeyForSignal\\height", d.height());

	return;
}

void EditSchemaWidget::f2KeyForLoopback(std::shared_ptr<VFrame30::SchemaItem> item)
{
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	VFrame30::SchemaItemLoopback* loopbackItem = dynamic_cast<VFrame30::SchemaItemLoopback*>(item.get());

	if (loopbackItem == nullptr)
	{
		assert(loopbackItem);
		return;
	}

	QString loopbackId = loopbackItem->loopbackId();

	// Show input dialog
	//
	bool ok = false;
	QString newValue = QInputDialog::getText(this,
											 tr("Set LoopbackID"),
											 tr("LoopbackID:"),
											 QLineEdit::Normal,
											 loopbackId,
											 &ok).trimmed();

	if (ok == true &&
		newValue.isEmpty() == false &&
		newValue != loopbackId)
	{
		m_editEngine->runSetProperty(VFrame30::PropertyNames::loopbackId, QVariant(newValue), item);
		editSchemaView()->update();

		EditSchemaWidget::m_lastUsedLoopbackId = newValue;
	}

	return;
}

void EditSchemaWidget::f2KeyForValue(std::shared_ptr<VFrame30::SchemaItem> item)
{
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	VFrame30::SchemaItemValue* valueItem = dynamic_cast<VFrame30::SchemaItemValue*>(item.get());
	if (valueItem == nullptr)
	{
		assert(valueItem);
		return;
	}

	QString appSignalIds = valueItem->signalIdsString();
	QString text = valueItem->text();
	QString preDrawScript = valueItem->preDrawScript();

	// Show input dialog
	//
	QDialog d(this);

	d.setWindowTitle(tr("SchemaItemValue"));
	d.setWindowFlags((d.windowFlags() & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowContextHelpButtonHint)
					 | Qt::CustomizeWindowHint | Qt::WindowMaximizeButtonHint);

	// AppSchemaIDs
	//
	QLabel* appSignalIdsLabel = new QLabel("AppSchemaIDs:", &d);

	QTextEdit* appSignalIdsEdit = new QTextEdit(&d);
	appSignalIdsEdit->setPlaceholderText("Enter AppSchemaIDs separated by lines");
	appSignalIdsEdit->setPlainText(appSignalIds);

	// Text
	//
	QLabel* textLabel = new QLabel("Text:", &d);

	QLineEdit* textEdit = new QLineEdit(&d);
	textEdit->setPlaceholderText(VFrame30::PropertyNames::textValuePropDescription);
	textEdit->setToolTip(VFrame30::PropertyNames::textValuePropDescription);
	textEdit->setText(text);

	// PreDrawScript
	//
	QLabel* preDrawScriptLabel = new QLabel("PreDrawScript:", &d);

	QsciScintilla* preDrawScriptEdit = new QsciScintilla(&d);
	LexerJavaScript lexer;
	preDrawScriptEdit->setLexer(&lexer);
	preDrawScriptEdit->setText(preDrawScript);
	preDrawScriptEdit->setMarginType(0, QsciScintilla::NumberMargin);
	preDrawScriptEdit->setMarginWidth(0, 40);

#if defined(Q_OS_WIN)
	preDrawScriptEdit->setFont(QFont("Consolas"));
#else
	preDrawScriptEdit->setFont(QFont("Courier"));
#endif

	preDrawScriptEdit->setTabWidth(4);
	preDrawScriptEdit->setAutoIndent(true);

	QPushButton* preDrawScriptTemplate = new QPushButton(tr("Paste PreDrawScript Template"), &d);

	// --
	//
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &d);

	QGridLayout* layout = new QGridLayout(&d);

	layout->addWidget(appSignalIdsLabel, 0, 0, 1, 3);
	layout->addWidget(appSignalIdsEdit, 1, 0, 1, 3);

	layout->addWidget(textLabel, 2, 0, 1, 3);
	layout->addWidget(textEdit, 3, 0, 1, 3);

	layout->addWidget(preDrawScriptLabel, 4, 0, 1, 3);
	layout->addWidget(preDrawScriptEdit, 5, 0, 1, 3);
	layout->addWidget(preDrawScriptTemplate, 6, 0, 1, 1);

	layout->addWidget(buttonBox, 7, 0, 1, 3);

	layout->setRowStretch(5, 1);	// preDrawScriptEdit

	d.setLayout(layout);

	// RAW STRINg TEMPLATE FOR PreDrawScript
	//
	QString preDrawScriptTemplateString = R"((function(schemaItemValue)
{
	// var appSignalId = schemaItemValue.SignalIDs[0];

	// Get data from AppDataService
	// var signalParam = signals.signalParam(appSignalId);
	// var signalState = signals.signalState(appSignalId);

	// Get data from TuningService
	// var signalParam = tuning.signalParam(appSignalId);
	// var signalState = tuning.signalState(appSignalId);

	// Get signal state
	// if (signalState.Valid == true)
	// {
	//		schemaItemValue.Text = signalState.Value;
	//		schemaItemValue.TextColor = "white";
	//		schemaItemValue.FillColor = schemaItemValue.BlinkPhase ? "black" : "#A00000";
	//		schemaItemValue.LineColor = "#000000";
	// }
}))";

	connect(preDrawScriptTemplate, &QPushButton::clicked, &d, [preDrawScriptEdit, preDrawScriptTemplateString](){preDrawScriptEdit->setText(preDrawScriptTemplateString);});

	connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	// --
	//
	int width = QSettings().value("f2KeyForValue\\width").toInt();
	int height = QSettings().value("f2KeyForValue\\height").toInt();
	d.resize(width, height);

	int result = d.exec();

	if (result == QDialog::Accepted)
	{
		QString newAppSignaIds = appSignalIdsEdit->toPlainText();
		QString newText = textEdit->text();
		QString newPreDrawScript = preDrawScriptEdit->text();

		if (newAppSignaIds != appSignalIds ||
			newText != text ||
			newPreDrawScript != preDrawScript)
		{
			if (bool ok = m_editEngine->startBatch();
				ok == true)
			{
				m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalIDs, QVariant(newAppSignaIds), item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::text, QVariant(newText), item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::preDrawScript, QVariant(newPreDrawScript), item);

				m_editEngine->endBatch();
			}
		}
	}

	QSettings().setValue("f2KeyForValue\\width", d.width());
	QSettings().setValue("f2KeyForValue\\height", d.height());

	return;
}

void EditSchemaWidget::f2KeyForImageValue(std::shared_ptr<VFrame30::SchemaItem> item)
{
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	VFrame30::SchemaItemImageValue* valueItem = dynamic_cast<VFrame30::SchemaItemImageValue*>(item.get());
	if (valueItem == nullptr)
	{
		assert(valueItem);
		return;
	}

	QString appSignalIds = valueItem->signalIdsString();
	QString currentImageId = valueItem->currentImageId();
	QString preDrawScript = valueItem->preDrawScript();

	// Show input dialog
	//
	QDialog d(this);

	d.setWindowTitle(tr("SchemaItemImageValue"));
	d.setWindowFlags((d.windowFlags() & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowContextHelpButtonHint)
					 | Qt::CustomizeWindowHint | Qt::WindowMaximizeButtonHint);

	// AppSchemaIDs
	//
	QLabel* appSignalIdsLabel = new QLabel("AppSchemaIDs:", &d);

	QTextEdit* appSignalIdsEdit = new QTextEdit(&d);
	appSignalIdsEdit->setPlaceholderText("Enter AppSchemaIDs separated by lines");
	appSignalIdsEdit->setPlainText(appSignalIds);

	// CurrentImageID
	//
	QLabel* currentImageIdLabel = new QLabel("CurrentImageID:", &d);

	QLineEdit* currentImageIdEdit = new QLineEdit(&d);
	currentImageIdEdit->setText(currentImageId);

	// PreDrawScript
	//
	QLabel* preDrawScriptLabel = new QLabel("PreDrawScript:", &d);

	QsciScintilla* preDrawScriptEdit = new QsciScintilla(&d);
	LexerJavaScript lexer;
	preDrawScriptEdit->setLexer(&lexer);
	preDrawScriptEdit->setText(preDrawScript);
	preDrawScriptEdit->setMarginType(0, QsciScintilla::NumberMargin);
	preDrawScriptEdit->setMarginWidth(0, 40);

#if defined(Q_OS_WIN)
	preDrawScriptEdit->setFont(QFont("Consolas"));
#else
	preDrawScriptEdit->setFont(QFont("Courier"));
#endif

	preDrawScriptEdit->setTabWidth(4);
	preDrawScriptEdit->setAutoIndent(true);

	QPushButton* preDrawScriptTemplate = new QPushButton(tr("Paste PreDrawScript Template"), &d);

	// --
	//
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &d);

	QGridLayout* layout = new QGridLayout(&d);

	layout->addWidget(appSignalIdsLabel, 0, 0, 1, 3);
	layout->addWidget(appSignalIdsEdit, 1, 0, 1, 3);

	layout->addWidget(currentImageIdLabel, 2, 0, 1, 3);
	layout->addWidget(currentImageIdEdit, 3, 0, 1, 3);

	layout->addWidget(preDrawScriptLabel, 4, 0, 1, 3);
	layout->addWidget(preDrawScriptEdit, 5, 0, 1, 3);
	layout->addWidget(preDrawScriptTemplate, 6, 0, 1, 1);

	layout->addWidget(buttonBox, 7, 0, 1, 3);

	layout->setRowStretch(5, 1);	// preDrawScriptEdit

	d.setLayout(layout);

	// RAW STRINg TEMPLATE FOR PreDrawScript
	//
	QString preDrawScriptTemplateString = R"((function(schemaItemImageValue)
{
	// Get signal id by index from schema item
	var appSignalId = schemaItemImageValue.SignalIDs[0];

	// Get data from AppDataService or TuningService sources
	// var signalState = tuning.signalState(appSignalId);
	var signalState = signals.signalState(appSignalId);

	if (signalState.Valid == false)
	{
		schemaItemImageValue.CurrentImageID = "IMAGEID_NOT_VALID";
		return;
	}

	if (signalState.Value == 0)
		schemaItemImageValue.CurrentImageID = "IMAGEID_OFF";
	else
		schemaItemImageValue.CurrentImageID = "IMAGEID_ON";
}))";

	connect(preDrawScriptTemplate, &QPushButton::clicked, &d, [preDrawScriptEdit, preDrawScriptTemplateString](){preDrawScriptEdit->setText(preDrawScriptTemplateString);});

	connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	// --
	//
	int width = QSettings().value("f2KeyForImageValue\\width").toInt();
	int height = QSettings().value("f2KeyForImageValue\\height").toInt();
	d.resize(width, height);

	int result = d.exec();

	if (result == QDialog::Accepted)
	{
		QString newAppSignaIds = appSignalIdsEdit->toPlainText();
		QString newCurrentImageId = currentImageIdEdit->text();
		QString newPreDrawScript = preDrawScriptEdit->text();

		if (newAppSignaIds != appSignalIds ||
			newCurrentImageId != currentImageId ||
			newPreDrawScript != preDrawScript)
		{
			if (bool ok = m_editEngine->startBatch();
				ok == true)
			{
				m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalIDs, QVariant(newAppSignaIds), item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::currentImageId, QVariant(newCurrentImageId), item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::preDrawScript, QVariant(newPreDrawScript), item);

				m_editEngine->endBatch();
			}
		}
	}

	QSettings().setValue("f2KeyForImageValue\\width", d.width());
	QSettings().setValue("f2KeyForImageValue\\height", d.height());

	return;
}


void EditSchemaWidget::f2KeyForBus(std::shared_ptr<VFrame30::SchemaItem> item)
{
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	VFrame30::SchemaItemBus* busItem = dynamic_cast<VFrame30::SchemaItemBus*>(item.get());
	if (busItem == nullptr)
	{
		assert(busItem);
		return;
	}

	QString text = busItem->busTypeId();

	// Get Bus list
	//
	std::vector<VFrame30::Bus> busses;

	bool ok = loadBusses(db(), &busses, this);

	if (ok == false)
	{
		return;
	}

	// Show input dialog
	//
	QDialog d(this);

	d.setWindowTitle(tr("Set BusType"));
	d.setWindowFlags((d.windowFlags() &
					~Qt::WindowMinimizeButtonHint &
					~Qt::WindowMaximizeButtonHint &
					~Qt::WindowContextHelpButtonHint) | Qt::CustomizeWindowHint);

	// Type Items
	//
	QLabel* busTypeLabel = new QLabel("Select BusType:");

	QComboBox* busTypeCombo = new QComboBox();

	for (int i = 0; i < static_cast<int>(busses.size()); i++)
	{
		busTypeCombo->addItem(busses[i].busTypeId(), QVariant(i));
	}

	int dataIndex = busTypeCombo->findText(text);
	if (dataIndex != -1)
	{
		busTypeCombo->setCurrentIndex(dataIndex);
	}

	// --
	//
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	QVBoxLayout* layout = new QVBoxLayout;

	layout->addWidget(busTypeLabel);
	layout->addWidget(busTypeCombo);
	layout->addWidget(buttonBox);

	d.setLayout(layout);

	connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	// --
	//
	int result = d.exec();

	if (result == QDialog::Accepted && text != busTypeCombo->currentText())
	{
		int selectedBusIndex = busTypeCombo->currentData().toInt();
		const VFrame30::Bus& newBus = busses[selectedBusIndex];

		QByteArray oldState;
		busItem->saveToByteArray(&oldState);

		busItem->setBusType(newBus);

		QByteArray newState;
		busItem->saveToByteArray(&newState);

		// Return object to prev state, it is not neccessary indeed, as it will be loaded into the new state in edit engine
		//
		busItem->Load(oldState);

		// Run command
		//
		m_editEngine->runSetObject(oldState, newState, item);

		editSchemaView()->update();
	}


	return;
}

void EditSchemaWidget::deleteKey()
{
	auto items = editSchemaView()->selectedNonLockedItems();

	m_editEngine->runDeleteItem(items, activeLayer());

	return;
}

void EditSchemaWidget::undo()
{
	m_nextSelectionFromLeft = {};		// Reset state for slection next items via Alt + Arrow Keys
	m_nextSelectionFromRight = {};

	m_editEngine->undo();

	if (m_schemaPropertiesDialog != nullptr && m_schemaPropertiesDialog->isVisible())
	{
		m_schemaPropertiesDialog->setSchema(schema());
	}
}

void EditSchemaWidget::redo()
{
	m_nextSelectionFromLeft = {};		// Reset state for slection next items via Alt + Arrow Keys
	m_nextSelectionFromRight = {};

	m_editEngine->redo();

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

void EditSchemaWidget::selectItems(std::vector<std::shared_ptr<VFrame30::SchemaItem>> items)
{
	editSchemaView()->clearSelection();
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

	const std::vector<std::shared_ptr<VFrame30::SchemaItem>> selected = selectedNonLockedItems();

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

		// If selected one image, then copy to clipboard it also as image
		//
		if (selectedItems().size() == 1 && selectedItems().front()->isType<VFrame30::SchemaItemImage>() == true)
		{
			VFrame30::SchemaItemImage* imageItem = selectedItems().front()->toType<VFrame30::SchemaItemImage>();
			mime->setImageData(imageItem->image());
		}

		// --
		//
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
		bool schemaItemBusIsPresent = false;
		bool schemaItemConnectionIsPresent = false;
		bool schemaItemInOutIsPresent = false;

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

			if (schemaItem->isType<VFrame30::SchemaItemUfb>() == true)
			{
				schemaItemUfbIsPresent = true;
			}

			if (schemaItem->isType<VFrame30::SchemaItemBus>() == true)
			{
				schemaItemBusIsPresent = true;
			}

			if (schemaItem->isType<VFrame30::SchemaItemConnection>() == true)
			{
				schemaItemConnectionIsPresent = true;
			}

			if (schemaItem->isType<VFrame30::SchemaItemInOut>() == true)
			{
				schemaItemInOutIsPresent = true;
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
			if (schema()->isUfbSchema() == true &&
				(schemaItemUfbIsPresent == true ||
				 schemaItemConnectionIsPresent == true ||
				 schemaItemInOutIsPresent == true))
			{
				QMessageBox::critical(this, qAppName(), tr("Adding In/Outs, Transmiters/Receivers, User Functional Blocks to UFB Schema is impossible."));
				return;
			}

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

		if (schemaItemBusIsPresent == true)
		{
			updateBussesForSchema();
		}

		return;
	}

	// Specific pastes
	//
	const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& selected = editSchemaView()->selectedItems();

	// --
	//
	if (selected.empty() == true)
	{
		return;
	}

	// Paste image to selected SchemaItemImage
	//
	if (mimeData->hasImage() == true)
	{
		bool allItemsAreImages = true;

		for (std::shared_ptr<VFrame30::SchemaItem> item : selected)
		{
			VFrame30::SchemaItemImage* imageItem = dynamic_cast<VFrame30::SchemaItemImage*>(item.get());

			if (imageItem == nullptr)
			{
				allItemsAreImages = false;
				break;
			}
		}

		if (allItemsAreImages == true)
		{
			QImage image = qvariant_cast<QImage>(mimeData->imageData());

			for (std::shared_ptr<VFrame30::SchemaItem> item : selected)
			{
				VFrame30::SchemaItemImage* imageItem = dynamic_cast<VFrame30::SchemaItemImage*>(item.get());
				Q_ASSERT(imageItem);

				m_editEngine->runSetProperty(VFrame30::PropertyNames::image, QVariant(image), selected);
			}
		}

		return;
	}

	// All other itmes receives olny text
	//
	if (mimeData->hasText() == false)
	{
		return;
	}

	// Paste text to SchemaItemConst
	//
	{
		bool allItemsAreConsts = true;

		bool okInteger = false;
		bool okFloat = false;
		bool okDiscrete = false;

		int constInt = mimeData->text().toInt(&okInteger);
		double constFloat = mimeData->text().toDouble(&okFloat);
		int constDiscrete = mimeData->text().toInt(&okDiscrete);

		std::vector<std::shared_ptr<VFrame30::SchemaItem>> constIntItems;
		constIntItems.reserve(selected.size());

		std::vector<std::shared_ptr<VFrame30::SchemaItem>> constFloatItems;
		constFloatItems.reserve(selected.size());

		std::vector<std::shared_ptr<VFrame30::SchemaItem>> constDiscreteItems;
		constDiscreteItems.reserve(selected.size());

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
			case VFrame30::SchemaItemConst::ConstType::IntegerType:
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

			case VFrame30::SchemaItemConst::ConstType::Discrete:
				if (okDiscrete == true)
				{
					constDiscreteItems.push_back(item);
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

			if (okDiscrete == true && constDiscreteItems.empty() == false)
			{
				m_editEngine->runSetProperty(VFrame30::PropertyNames::valueDiscrete, QVariant(constDiscrete), constDiscreteItems);
			}
		}
	}

	// Paste text to SchemaItemRect
	//
	{
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
	}

	// Paste appSignalID to SchemaItemSignal
	//
	{
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
	}

	// Paste appSignalID to VFrame30::SchemaItemReceiver
	//
	{
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
	}

	// Paste appSignalID to VFrame30::SchemaItemLoopback
	//
	{
		bool allItemsAreLoopbacks = true;
		for (std::shared_ptr<VFrame30::SchemaItem> item : selected)
		{
			if (dynamic_cast<VFrame30::SchemaItemLoopback*>(item.get()) == nullptr)
			{
				allItemsAreLoopbacks = false;
				break;
			}
		}

		if (allItemsAreLoopbacks == true)
		{
			m_editEngine->runSetProperty(VFrame30::PropertyNames::loopbackId, QVariant(mimeData->text()), selected);
			EditSchemaWidget::m_lastUsedLoopbackId = mimeData->text();
		}
	}

	// Paste appSignalID to VFrame30::SchemaItemValue
	//
	{
		bool allItemsAreValues = true;
		for (std::shared_ptr<VFrame30::SchemaItem> item : selected)
		{
			if (dynamic_cast<VFrame30::SchemaItemValue*>(item.get()) == nullptr)
			{
				allItemsAreValues = false;
				break;
			}
		}

		if (allItemsAreValues == true &&
			mimeData->text().startsWith('#') == true)
		{
			m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalId, QVariant(mimeData->text()), selected);
		}
	}

	// Paste appSignalID to VFrame30::SchemaItemImageValue
	//
	{
		bool allItemsAreImageValues = true;
		for (std::shared_ptr<VFrame30::SchemaItem> item : selected)
		{
			if (dynamic_cast<VFrame30::SchemaItemImageValue*>(item.get()) == nullptr)
			{
				allItemsAreImageValues = false;
				break;
			}
		}

		if (allItemsAreImageValues == true &&
			mimeData->text().startsWith('#') == true)
		{
			m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalId, QVariant(mimeData->text()), selected);
		}
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
	m_itemsPropertiesDialog->setReadOnly(m_editEngine->readOnly());
	m_itemsPropertiesDialog->show();
	m_itemsPropertiesDialog->ensureVisible();
	m_itemsPropertiesDialog->setFocus();

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
	m_itemsPropertiesDialog->setReadOnly(m_editEngine->readOnly());
	m_itemsPropertiesDialog->ensureVisible();

	const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& selected = editSchemaView()->selectedItems();
	auto selectectNotLocked = selectedNonLockedItems();

	// Edit Menu
	//
	m_deleteAction->setEnabled(selectectNotLocked.empty() == false && readOnly() == false);
	m_editCutAction->setEnabled(selectectNotLocked.empty() == false && readOnly() == false);
	m_editCopyAction->setEnabled(selected.empty() == false);

	// Allign
	//
	bool allowAlign = selectectNotLocked.size() >= 2 && readOnly() == false;

	m_alignLeftAction->setEnabled(allowAlign);
	m_alignRightAction->setEnabled(allowAlign);
	m_alignTopAction->setEnabled(allowAlign);
	m_alignBottomAction->setEnabled(allowAlign);

	// Size
	//
	std::vector<std::shared_ptr<VFrame30::SchemaItem>> selectedFiltered;

	for (std::shared_ptr<VFrame30::SchemaItem> item : selectectNotLocked)
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
	bool allowSetOrder = selectectNotLocked.empty() == false  && readOnly() == false;

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

	// AppSignal properties
	//
	bool enableAppSignalProperies = false;

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
							enableAppSignalProperies = true;
							break;
						}
					}
				}

				if (dynamic_cast<VFrame30::SchemaItemReceiver*>(item.get()) != nullptr)
				{
					VFrame30::SchemaItemReceiver* itemReceiver = dynamic_cast<VFrame30::SchemaItemReceiver*>(item.get());
					assert(itemReceiver);

					for (const QString& appSignal : itemReceiver->appSignalIdsAsList())
					{
						if (appSignal.isEmpty() == false)
						{
							enableAppSignalProperies = true;
							break;
						}
					}
				}
			}

			if (signalStrIds.empty() == false)
			{
				enableAppSignalProperies = true;
			}
		}
	}

	m_appSignalPropertiesAction->setEnabled(enableAppSignalProperies);

	// Add new Application Logic signal
	//
	bool enableAddAppSignal = false;

	if (isLogicSchema() == true)
	{
		if (selectedItems().size() == 1)
		{
			std::shared_ptr<VFrame30::SchemaItem> selected = selectedItems().front();

			auto itemSignal = dynamic_cast<VFrame30::SchemaItemSignal*>(selected.get());

			if (itemSignal != nullptr)
			{
				enableAddAppSignal = true;
			}
			else
			{
				// it is not VFrame30::SchemaItemSignal
				//
			}
		}
	}

	m_addAppSignalAction->setEnabled(enableAddAppSignal);

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

	// if SchemaItemImage is(are) selected and Image is in the clipboard
	//
	bool allItemsAreImages = true;
	for (std::shared_ptr<VFrame30::SchemaItem> item : selected)
	{
		if (dynamic_cast<VFrame30::SchemaItemImage*>(item.get()) == nullptr)
		{
			allItemsAreImages = false;
			break;
		}
	}

	if (allItemsAreImages == true &&
		mimeData->hasImage() == true)
	{
		m_editPasteAction->setEnabled(true);
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

	std::vector<std::shared_ptr<VFrame30::SchemaItem>> constDiscreteItems;
	constDiscreteItems.reserve(selected.size());

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
		case VFrame30::SchemaItemConst::ConstType::IntegerType:
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
		case VFrame30::SchemaItemConst::ConstType::Discrete:
			if (okInteger == true)
			{
				constDiscreteItems.push_back(item);
			}
			break;

		default:
			assert(false);
			allItemsAreConsts = false;
		}
	}

	if (allItemsAreConsts == true)
	{
		if ((okInteger == true && constIntItems.empty() == false) ||
			(okFloat  == true && constFloatItems.empty() == false) ||
			(okInteger  == true && constDiscreteItems.empty() == false))
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

	// if Any SchemaItemLoopback is selected and Text is in the clipboard
	//
	bool allItemsAreLoopbacks = true;
	for (std::shared_ptr<VFrame30::SchemaItem> item : selected)
	{
		if (dynamic_cast<VFrame30::SchemaItemLoopback*>(item.get()) == nullptr)
		{
			allItemsAreLoopbacks = false;
			break;
		}
	}

	if (allItemsAreLoopbacks == true &&
		mimeData->hasText() == true)
	{
		m_editPasteAction->setEnabled(true);
		return;
	}

	// if Any SchemaItemValue is selected and AppSignalID is in the clipboard
	//
	bool allItemsAreValues = true;
	for (std::shared_ptr<VFrame30::SchemaItem> item : selected)
	{
		if (dynamic_cast<VFrame30::SchemaItemValue*>(item.get()) == nullptr)
		{
			allItemsAreValues = false;
			break;
		}
	}

	if (allItemsAreValues == true &&
		mimeData->hasText() == true &&
		mimeData->text().startsWith('#') == true)
	{
		m_editPasteAction->setEnabled(true);
		return;
	}

	// if Any SchemaItemImageValue is selected and AppSignalID is in the clipboard
	//
	{
		bool allItemsAreImageValues = true;
		for (std::shared_ptr<VFrame30::SchemaItem> item : selected)
		{
			if (dynamic_cast<VFrame30::SchemaItemImageValue*>(item.get()) == nullptr)
			{
				allItemsAreImageValues = false;
				break;
			}
		}

		if (allItemsAreImageValues == true &&
			mimeData->hasText() == true &&
			mimeData->text().startsWith('#') == true)
		{
			m_editPasteAction->setEnabled(true);
			return;
		}
	}

	// --
	//
	m_editPasteAction->setEnabled(false);

	return;
}

void EditSchemaWidget::addTransmitter()
{
	auto schemaItem = std::make_shared<VFrame30::SchemaItemTransmitter>(schema()->unit());

	bool ok = f2KeyForTransmitter(schemaItem, false);
	if (ok == false)
	{
		return;
	}

	addItem(schemaItem);
	return;
}

void EditSchemaWidget::addReceiver()
{
	auto schemaItem = std::make_shared<VFrame30::SchemaItemReceiver>(schema()->unit());

	bool ok = f2KeyForReceiver(schemaItem, false);

	if (ok == false)
	{
		return;
	}

	addItem(schemaItem);
	return;
}

void EditSchemaWidget::addLoopbackSource()
{
	auto schemaItem = std::make_shared<VFrame30::SchemaItemLoopbackSource>(schema()->unit());

	QString loopbackId = QString("LBID_%1").arg(db()->nextCounterValue());
	EditSchemaWidget::m_lastUsedLoopbackId = loopbackId;

	schemaItem->setLoopbackId(loopbackId);

	addItem(schemaItem);
	return;
}

void EditSchemaWidget::addLoopbackTarget()
{
	auto schemaItem = std::make_shared<VFrame30::SchemaItemLoopbackTarget>(schema()->unit());

	if (EditSchemaWidget::m_lastUsedLoopbackId.isEmpty() == false)
	{
		schemaItem->setLoopbackId(EditSchemaWidget::m_lastUsedLoopbackId);
	}

	addItem(schemaItem);
}

void EditSchemaWidget::addAfbElement()
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

void EditSchemaWidget::addBusComposer()
{
	auto schemaItem = std::make_shared<VFrame30::SchemaItemBusComposer>(schema()->unit());
	addBusItem(schemaItem);
}

void EditSchemaWidget::addBusExtractor()
{
	auto schemaItem = std::make_shared<VFrame30::SchemaItemBusExtractor>(schema()->unit());
	addBusItem(schemaItem);
}

void EditSchemaWidget::addBusItem(std::shared_ptr<VFrame30::SchemaItemBus> schemaItem)
{
	if (schema()->isLogicSchema() == false &&
		schema()->isUfbSchema() == false)
	{
		assert(false);		// No sense to add sconnection to non applogic schema
		return;
	}

	// Get Bus list
	//
	std::vector<VFrame30::Bus> busses;

	bool ok = loadBusses(db(), &busses, this);

	if (ok == false)
	{
		return;
	}

	// Select BustType from existing
	//

	// Show menu
	//
	QObject actionParent;
	QList<QAction*> menuActions;

	for (const VFrame30::Bus& bus : busses)
	{
		QString caption = QString("%1").arg(bus.busTypeId());
		QAction* a = new QAction(caption , &actionParent);
		a->setData(bus.busTypeId());

		menuActions << a;
	}

	QPoint menuPos = QCursor::pos();

	QAction* triggeredAction = QMenu::exec(menuActions, menuPos);
	if (triggeredAction == nullptr)
	{
		return;
	}

	// --
	//
	QString selectedBusId = triggeredAction->data().toString();

	for (const VFrame30::Bus& bus : busses)
	{
		if (bus.busTypeId() == selectedBusId)
		{
			schemaItem->setBusType(bus);

			// Add item
			//
			addItem(schemaItem);
			return;
		}
	}

	// Selected bus not found
	//
	assert(false);
	return;
}

void EditSchemaWidget::onLeftKey(QKeyEvent* e)
{
	if (selectedItems().empty() == false && e->modifiers().testFlag(Qt::AltModifier) == true)
	{
		selectNextLeftItem({});
		return;
	}

	// Move selected items to left
	//
	auto selected = selectedNonLockedItems();
	if (selected.empty() == true)
	{
		return;
	}

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

		initMoveAfbsConnectionLinks(MouseState::Moving);
		moveAfbsConnectionLinks(QPointF(dif, 0), MouseState::Moving);
		{
			if (bool ok = m_editEngine->startBatch();
				ok == true)
			{
				m_editEngine->runMoveItem(dif, 0, itemsForMove, snapToGrid());
				finishMoveAfbsConnectionLinks();

				m_editEngine->endBatch();
			}
		}
	}

	return;
}

void EditSchemaWidget::onRightKey(QKeyEvent* e)
{
	if (selectedItems().empty() == false && e->modifiers().testFlag(Qt::AltModifier) == true)
	{
		selectNextRightItem({});
		return;
	}

	// Move selected items to right
	//
	auto selected = selectedNonLockedItems();

	if (selected.empty() == true)
	{
		return;
	}

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

		initMoveAfbsConnectionLinks(MouseState::Moving);
		moveAfbsConnectionLinks(QPointF(dif, 0), MouseState::Moving);

		if (bool ok = m_editEngine->startBatch();
			ok == true)
		{
			m_editEngine->runMoveItem(dif, 0, itemsForMove, snapToGrid());
			finishMoveAfbsConnectionLinks();

			m_editEngine->endBatch();
		}
	}

	return;
}

void EditSchemaWidget::onUpKey(QKeyEvent* e)
{
	if (selectedItems().empty() == false && e->modifiers().testFlag(Qt::AltModifier) == true)
	{
		selectNextUpItem();
		return;
	}

	// --
	//
	auto selected = selectedNonLockedItems();

	if (selected.empty() == true)
	{
		return;
	}

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

		initMoveAfbsConnectionLinks(MouseState::Moving);
		moveAfbsConnectionLinks(QPointF(0, dif), MouseState::Moving);

		if (bool ok = m_editEngine->startBatch();
			ok == true)
		{
			m_editEngine->runMoveItem(0, dif, itemsForMove, snapToGrid());
			finishMoveAfbsConnectionLinks();

			m_editEngine->endBatch();
		}
	}

	return;
}

void EditSchemaWidget::onDownKey(QKeyEvent* e)
{
	if (selectedItems().empty() == false && e->modifiers().testFlag(Qt::AltModifier) == true)
	{
		selectNextDownItem();
		return;
	}

	// --
	//
	auto selected = selectedNonLockedItems();

	if (selected.empty() == true)
	{
		return;
	}

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

		initMoveAfbsConnectionLinks(MouseState::Moving);
		moveAfbsConnectionLinks(QPointF(0, dif), MouseState::Moving);

		if (bool ok = m_editEngine->startBatch();
			ok == true)
		{
			m_editEngine->runMoveItem(0, dif, itemsForMove, snapToGrid());
			finishMoveAfbsConnectionLinks();

			m_editEngine->endBatch();
		}
	}

	return;
}

bool EditSchemaWidget::selectNextLeftItem(NextSelectionItem switchToLeftItem)
{
	m_nextSelectionFromLeft = {};
	m_nextSelectionFromRight = {};

	if (selectedItems().empty() == true)
	{
		return false;
	}

	std::shared_ptr<VFrame30::SchemaItem> selectedItem;

	if (switchToLeftItem.isNull() == false)
	{
		selectedItem = switchToLeftItem.schemaItem;
	}
	else
	{
		selectedItem = selectedItems().front();
	}

	// Get selected item pos - left, top + midle heigh
	//
	QPointF point;

	if (selectedItem->isType<VFrame30::PosConnectionImpl>() == true ||
		selectedItem->isType<VFrame30::PosLineImpl>() == true)
	{
		std::vector<VFrame30::SchemaPoint> selectedConnectionPoints = selectedItem->getPointList();

		if (selectedConnectionPoints.size() >= 2)
		{
			VFrame30::SchemaPoint firstPoint = selectedConnectionPoints.front();
			VFrame30::SchemaPoint lastPoint = selectedConnectionPoints.back();

			if (firstPoint.X < lastPoint.X)
			{
				point = {firstPoint.X, firstPoint.Y};
			}
			else
			{
				point = {lastPoint.X, lastPoint.Y};
			}
		}
		else
		{
			Q_ASSERT(selectedConnectionPoints.size() >= 2);
		}
	}

	if (auto posRectImpl = selectedItem->toType<VFrame30::PosRectImpl>();
		posRectImpl != nullptr)
	{
		if (auto fblItem = posRectImpl->toFblItem();
			fblItem != nullptr && fblItem->hasInputs() == true)
		{
			VFrame30::AfbPin pin;

			if (switchToLeftItem.isNull() == false && switchToLeftItem.isFblItemRect() == true)
			{
				if (switchToLeftItem.pinIndex >= 0 && switchToLeftItem.pinIndex < fblItem->inputsCount())
				{
					pin = fblItem->inputs()[switchToLeftItem.pinIndex];
					m_nextSelectionFromLeft = {selectedItem, switchToLeftItem.pinIndex};
				}
				else
				{
					return false;
				}
			}
			else
			{
				pin = fblItem->inputs().front();
				m_nextSelectionFromLeft = {selectedItem, 0};
			}

			point = {pin.x(), pin.y()};
		}
		else
		{
			// If it does not have pins, then take left center point
			//
			point = {posRectImpl->leftDocPt(), posRectImpl->topDocPt() + posRectImpl->heightDocPt() / 2.0};
		}
	}

	//  --
	//
	const double VertFactor = 2.0;
	point.setY(point.y() * VertFactor);

	// Decay all other items to points
	//
	double minDistance = DBL_MAX;
	std::shared_ptr<VFrame30::SchemaItem> minDistanceItem;

	auto calcDistance = [&minDistance, &minDistanceItem, &point, VertFactor](double x, double y, auto schemaItem)
		{
			QPointF p = {x, y * VertFactor};

			if (point.x() - p.x() > -0.000001)
			{
				double d = QLineF(point, p).length();

				if (d < minDistance)
				{
					minDistance = d;
					minDistanceItem = schemaItem;
				}
			}

			return;
		};


	for (std::shared_ptr<VFrame30::SchemaItem> item : editSchemaView()->activeLayer()->Items)
	{
		if (item == selectedItem ||
			item == m_nextSelectionFromLeft.schemaItem ||
			item == m_nextSelectionFromRight.schemaItem)
		{
			continue;
		}

		if (auto posRectImpl = item->toType<VFrame30::PosRectImpl>();
			posRectImpl != nullptr)
		{
			if (item->isFblItem() == true)
			{
				const VFrame30::FblItem* fblItem = item->toFblItem();
				Q_ASSERT(fblItem);

//				for (const VFrame30::AfbPin& pin : fblItem->inputs())
//				{
//					calcDistance(pin.x(), pin.y(), item);
//				}

				for (const VFrame30::AfbPin& pin : fblItem->outputs())
				{
					calcDistance(pin.x(), pin.y(), item);
				}
			}

			QRectF rc = {posRectImpl->leftDocPt(), posRectImpl->topDocPt(),
						 posRectImpl->widthDocPt(), posRectImpl->heightDocPt()};

			QPointF center = rc.center();

			if (rc.left() < point.x() ||
				rc.right() < point.x())
			{
				calcDistance(center.x(), rc.top(), item);
				calcDistance(rc.right(), rc.top(), item);
				calcDistance(rc.right(), center.y(), item);
				calcDistance(rc.right(), rc.bottom(), item);
				calcDistance(center.x(), rc.bottom(), item);
				calcDistance(rc.left(), rc.top(), item);
				calcDistance(rc.left(), rc.bottom(), item);
				calcDistance(rc.left(), center.y(), item);
			}

			continue;
		}

		if (item->isType<VFrame30::PosConnectionImpl>() == true ||
			item->isType<VFrame30::PosLineImpl>() == true)
		{
			std::vector<VFrame30::SchemaPoint> points = item->getPointList();

			if (points.size() >= 2)
			{
				const VFrame30::SchemaPoint& firstPoint = points.front();
				const VFrame30::SchemaPoint& lastPoint = points.back();

				// At least one point must be at left OR
				// it is a vertical line on point.x
				//
				if ((firstPoint.X < point.x() || lastPoint.X < point.x()) ||
					(std::abs(firstPoint.X - lastPoint.X) <= 0.000001 && std::abs(firstPoint.X - point.x()) <= 0.000001) ||
					(std::abs(firstPoint.X - point.x()) <= 0.000001 && std::abs(firstPoint.Y * VertFactor - point.y()) <= 0.000001) ||
					(std::abs(lastPoint.X - point.x()) <= 0.000001 && std::abs(lastPoint.Y * VertFactor - point.y()) <= 0.000001))
				{
					calcDistance(firstPoint.X, firstPoint.Y, item);
					calcDistance(lastPoint.X, lastPoint.Y, item);
				}
			}
			else
			{
				Q_ASSERT(points.size() >= 2);
			}

			continue;
		}
	}

	// Get the closest item
	//
	if (minDistanceItem  != nullptr)
	{
		selectItem(minDistanceItem);
		return true;
	}

	return false;
}

bool EditSchemaWidget::selectNextRightItem(NextSelectionItem switchToRightItem)
{
	m_nextSelectionFromLeft = {};
	m_nextSelectionFromRight = {};

	if (selectedItems().empty() == true)
	{
		return false;
	}

	std::shared_ptr<VFrame30::SchemaItem> selectedItem;

	if (switchToRightItem.isNull() == false)
	{
		selectedItem = switchToRightItem.schemaItem;
	}
	else
	{
		selectedItem = selectedItems().front();
	}

	// Get selected item pos - right, top + midle heigh
	//
	QPointF point;

	if (selectedItem->isType<VFrame30::PosConnectionImpl>() == true ||
		selectedItem->isType<VFrame30::PosLineImpl>() == true)
	{
		std::vector<VFrame30::SchemaPoint> selectedConnectionPoints = selectedItem->getPointList();

		if (selectedConnectionPoints.size() >= 2)
		{
			VFrame30::SchemaPoint firstPoint = selectedConnectionPoints.front();
			VFrame30::SchemaPoint lastPoint = selectedConnectionPoints.back();

			if (firstPoint.X < lastPoint.X)
			{
				point = {lastPoint.X, lastPoint.Y};
			}
			else
			{
				point = {firstPoint.X, firstPoint.Y};
			}
		}
		else
		{
			Q_ASSERT(selectedConnectionPoints.size() >= 2);
		}
	}

	if (auto posRectImpl = selectedItem->toType<VFrame30::PosRectImpl>();
		posRectImpl != nullptr)
	{
		if (auto fblItem = posRectImpl->toFblItem();
			fblItem != nullptr && fblItem->hasOutputs() == true)
		{
			VFrame30::AfbPin pin;

			if (switchToRightItem.isNull() == false && switchToRightItem.isFblItemRect() == true)
			{
				if (switchToRightItem.pinIndex >= 0 && switchToRightItem.pinIndex < fblItem->outputsCount())
				{
					pin = fblItem->outputs()[switchToRightItem.pinIndex];
					m_nextSelectionFromRight = {selectedItem, switchToRightItem.pinIndex};
				}
				else
				{
					return false;
				}
			}
			else
			{
				pin = fblItem->outputs().front();
				m_nextSelectionFromRight = {selectedItem, 0};
			}

			point = {pin.x(), pin.y()};

			m_nextSelectionFromLeft = {};
		}
		else
		{
			// If it does not have pins, then take left center point
			//
			point = {posRectImpl->leftDocPt() + posRectImpl->widthDocPt(), posRectImpl->topDocPt() + posRectImpl->heightDocPt() / 2.0};
		}
	}

	//  --
	//
	const double VertFactor = 2.0;
	point.setY(point.y() * VertFactor);

	// Decay all other items to points
	//
	double minDistance = DBL_MAX;
	std::shared_ptr<VFrame30::SchemaItem> minDistanceItem;

	auto calcDistance = [&minDistance, &minDistanceItem, &point, VertFactor](double x, double y, auto schemaItem)
		{
			QPointF p = {x, y * VertFactor};

			if (point.x() - 0.000001 <= p.x())
			{
				double d = QLineF(point, p).length();

				if (d < minDistance)
				{
					minDistance = d;
					minDistanceItem = schemaItem;
				}
			}

			return;
		};


	for (std::shared_ptr<VFrame30::SchemaItem> item : editSchemaView()->activeLayer()->Items)
	{
		if (item == selectedItem ||
			item == m_nextSelectionFromLeft.schemaItem ||
			item == m_nextSelectionFromRight.schemaItem)
		{
			continue;
		}

		if (auto posRectImpl = item->toType<VFrame30::PosRectImpl>();
			posRectImpl != nullptr)
		{
			if (item->isFblItem() == true)
			{
				const VFrame30::FblItem* fblItem = item->toFblItem();
				Q_ASSERT(fblItem);

				for (const VFrame30::AfbPin& pin : fblItem->inputs())
				{
					calcDistance(pin.x(), pin.y(), item);
				}

//				for (const VFrame30::AfbPin& pin : fblItem->outputs())
//				{
//					calcDistance(pin.x(), pin.y(), item);
//				}
			}

			QRectF rc = {posRectImpl->leftDocPt(), posRectImpl->topDocPt(),
						 posRectImpl->widthDocPt(), posRectImpl->heightDocPt()};

			QPointF center = rc.center();

			if (point.x() < rc.left() ||
				point.x() < rc.right())
			{
				calcDistance(center.x(), rc.top(), item);
				calcDistance(rc.right(), rc.top(), item);
				calcDistance(rc.right(), center.y(), item);
				calcDistance(rc.right(), rc.bottom(), item);
				calcDistance(center.x(), rc.bottom(), item);
				calcDistance(rc.left(), rc.top(), item);
				calcDistance(rc.left(), rc.bottom(), item);
				calcDistance(rc.left(), center.y(), item);
			}

			continue;
		}

		if (item->isType<VFrame30::PosConnectionImpl>() == true ||
			item->isType<VFrame30::PosLineImpl>() == true)
		{
			std::vector<VFrame30::SchemaPoint> points = item->getPointList();

			if (points.size() >= 2)
			{
				const VFrame30::SchemaPoint& firstPoint = points.front();
				const VFrame30::SchemaPoint& lastPoint = points.back();

				// At least one point must be at right OR
				// it is a vertical line on point.x
				//
				if ((point.x() < firstPoint.X || point.x() < lastPoint.X) ||
					(std::abs(firstPoint.X - lastPoint.X) <= 0.000001 && std::abs(firstPoint.X - point.x()) <= 0.000001) ||
					(std::abs(firstPoint.X - point.x()) <= 0.000001 && std::abs(firstPoint.Y * VertFactor - point.y()) <= 0.000001) ||
					(std::abs(lastPoint.X - point.x()) <= 0.000001 && std::abs(lastPoint.Y * VertFactor - point.y()) <= 0.000001))
				{
					calcDistance(firstPoint.X, firstPoint.Y, item);
					calcDistance(lastPoint.X, lastPoint.Y, item);
				}
			}
			else
			{
				Q_ASSERT(points.size() >= 2);
			}

			continue;
		}
	}

	// Get the closest item
	//
	if (minDistanceItem  != nullptr)
	{
		selectItem(minDistanceItem);
		return true;
	}

	return false;
}

void EditSchemaWidget::selectNextUpItem()
{
	// If Alt is pressed then select the next left item
	//
	if (selectedItems().empty() == true)
	{
		return;
	}

	std::shared_ptr<VFrame30::SchemaItem> selectedItem = selectedItems().front();

	// Get selected item pos
	//
	QPointF point;
	bool pointInitialized = false;

	if (m_nextSelectionFromRight.isNull() == false &&
		m_nextSelectionFromRight.isFblItemRect() == true)
	{
		NextSelectionItem ni = m_nextSelectionFromRight;
		ni.pinIndex --;

		pointInitialized = selectNextRightItem(ni);

		if (pointInitialized == true)
		{
			// selectNextRightItem(ni) has made selection;
			//
			return;
		}
	}

	if (m_nextSelectionFromLeft.isNull() == false &&
		m_nextSelectionFromLeft.isFblItemRect() == true)
	{
		NextSelectionItem ni = m_nextSelectionFromLeft;
		ni.pinIndex --;

		pointInitialized = selectNextLeftItem(ni);

		if (pointInitialized == true)
		{
			// selectNextLeftItem(ni) has made selection;
			//
			return;
		}
	}


	if (auto posRectImpl = selectedItem->toType<VFrame30::PosRectImpl>();
		pointInitialized == false && posRectImpl != nullptr)
	{
		point = {posRectImpl->leftDocPt() + posRectImpl->widthDocPt() / 2.0, posRectImpl->topDocPt()};
		pointInitialized = true;
	}

	if (pointInitialized == false && selectedItem->isType<VFrame30::PosLineImpl>() == true)
	{
		std::vector<VFrame30::SchemaPoint> selectedConnectionPoints = selectedItem->getPointList();

		if (selectedConnectionPoints.size() >= 2)
		{
			// Get center of line
			//
			const VFrame30::SchemaPoint& firstPoint = selectedConnectionPoints.front();
			const VFrame30::SchemaPoint& lastPoint = selectedConnectionPoints.back();

			QLineF line = {firstPoint.X, firstPoint.Y, lastPoint.X, lastPoint.Y};

			point = {line.center().x(), std::max(firstPoint.Y, lastPoint.Y)};
			pointInitialized = true;
		}
		else
		{
			Q_ASSERT(selectedConnectionPoints.size() >= 2);
		}
	}

	if (pointInitialized == false && selectedItem->isType<VFrame30::PosConnectionImpl>() == true)
	{
		std::vector<VFrame30::SchemaPoint> selectedConnectionPoints = selectedItem->getPointList();

		if (selectedConnectionPoints.size() >= 2)
		{
			// FOR NOW JUST TAKE CENTER BY RECT END POINTS
			//
			const VFrame30::SchemaPoint& firstPoint = selectedConnectionPoints.front();
			const VFrame30::SchemaPoint& lastPoint = selectedConnectionPoints.back();

			QLineF line = {firstPoint.X, firstPoint.Y, lastPoint.X, lastPoint.Y};

			point = line.center();
			pointInitialized = true;
		}
		else
		{
			Q_ASSERT(selectedConnectionPoints.size() >= 2);
		}
	}

	if (pointInitialized = false)
	{
		Q_ASSERT(pointInitialized);
		return;
	}

	//  --
	const double HorzFactor = 1.5;
	point.setX(point.x() * HorzFactor);

	// Decay all other items to points
	//
	double minDistance = DBL_MAX;
	std::shared_ptr<VFrame30::SchemaItem> minDistanceItem;

	auto calcDistance = [&minDistance, &minDistanceItem, &point, HorzFactor](double x, double y, auto schemaItem)
		{
			QPointF p = {x * HorzFactor, y};

			if (point.y() - p.y() > -0.000001)
			{
				double d = QLineF(point, p).length();

				if (d < minDistance)
				{
					minDistance = d;
					minDistanceItem = schemaItem;
				}
			}

			return;
		};


	for (std::shared_ptr<VFrame30::SchemaItem> item : editSchemaView()->activeLayer()->Items)
	{
		if (item == selectedItem)
		{
			continue;
		}

		if (auto posRectImpl = item->toType<VFrame30::PosRectImpl>();
			posRectImpl != nullptr)
		{
			QRectF rc = {posRectImpl->leftDocPt(), posRectImpl->topDocPt(),
						 posRectImpl->widthDocPt(), posRectImpl->heightDocPt()};

			QPointF center = rc.center();

			if (rc.top() < point.y() ||
				rc.bottom() < point.y())
			{
				calcDistance(center.x(), rc.top(), item);
				calcDistance(rc.right(), rc.top(), item);
				calcDistance(rc.right(), center.y(), item);
				calcDistance(rc.right(), rc.bottom(), item);
				calcDistance(center.x(), rc.bottom(), item);
				calcDistance(rc.left(), rc.top(), item);
				calcDistance(rc.left(), rc.bottom(), item);
				calcDistance(rc.left(), center.y(), item);
			}

			continue;
		}

//		if (item->isType<VFrame30::PosLineImpl>() == true)
//		{
//			std::vector<VFrame30::SchemaPoint> points = item->getPointList();

//			if (points.size() >= 2)
//			{
//				const VFrame30::SchemaPoint& firstPoint = points.front();
//				const VFrame30::SchemaPoint& lastPoint = points.back();

//				QLineF line = {firstPoint.X, firstPoint.Y, lastPoint.X, lastPoint.Y};
//				calcDistance(line.center().x(), line.center().y(), item);
//			}
//			else
//			{
//				Q_ASSERT(points.size() >= 2);
//			}

//			continue;
//		}

//		if (item->isType<VFrame30::PosConnectionImpl>() == true)
//		{
//			std::vector<VFrame30::SchemaPoint> points = item->getPointList();

//			if (points.size() >= 2)
//			{
//				VFrame30::SchemaPoint prevPoint = points[0];

//				for (int i = 1; i < points.size(); i++)
//				{
//					const VFrame30::SchemaPoint& firstPoint = prevPoint;
//					const VFrame30::SchemaPoint& lastPoint = points[i];

//					QLineF line = {firstPoint.X, firstPoint.Y, lastPoint.X, lastPoint.Y};
//					calcDistance(line.center().x(), line.center().y(), item);

//					prevPoint = points[i];
//				}
//			}
//			else
//			{
//				Q_ASSERT(points.size() >= 2);
//			}

//			continue;
//		}
	}

	// Get the closest item
	//
	if (minDistanceItem  != nullptr)
	{
		selectItem(minDistanceItem);
	}

	return;
}

void EditSchemaWidget::selectNextDownItem()
{
	// If Alt is pressed then select the next left item
	//
	if (selectedItems().empty() == true)
	{
		return;
	}

	std::shared_ptr<VFrame30::SchemaItem> selectedItem = selectedItems().front();

	// Get selected item pos
	//
	QPointF point;
	bool pointInitialized = false;

	if (m_nextSelectionFromRight.isNull() == false &&
		m_nextSelectionFromRight.isFblItemRect() == true)
	{
		NextSelectionItem ni = m_nextSelectionFromRight;
		ni.pinIndex ++;

		pointInitialized = selectNextRightItem(ni);

		if (pointInitialized == true)
		{
			// selectNextRightItem(ni) has made selection;
			//
			return;
		}
	}

	if (m_nextSelectionFromLeft.isNull() == false &&
		m_nextSelectionFromLeft.isFblItemRect() == true)
	{
		NextSelectionItem ni = m_nextSelectionFromLeft;
		ni.pinIndex ++;

		pointInitialized = selectNextLeftItem(ni);

		if (pointInitialized == true)
		{
			// selectNextLeftItem(ni) has made selection;
			//
			return;
		}
	}

	if (auto posRectImpl = selectedItem->toType<VFrame30::PosRectImpl>();
		pointInitialized == false && posRectImpl != nullptr)
	{
		point = {posRectImpl->leftDocPt() + posRectImpl->widthDocPt() / 2.0,
				 posRectImpl->topDocPt() + posRectImpl->heightDocPt()};
		pointInitialized = true;
	}

	if (pointInitialized == false && selectedItem->isType<VFrame30::PosLineImpl>() == true)
	{
		std::vector<VFrame30::SchemaPoint> selectedConnectionPoints = selectedItem->getPointList();

		if (selectedConnectionPoints.size() >= 2)
		{
			// Get center of line
			//
			const VFrame30::SchemaPoint& firstPoint = selectedConnectionPoints.front();
			const VFrame30::SchemaPoint& lastPoint = selectedConnectionPoints.back();

			QLineF line = {firstPoint.X, firstPoint.Y, lastPoint.X, lastPoint.Y};

			point = {line.center().x(), std::min(firstPoint.Y, lastPoint.Y)};
			pointInitialized = true;
		}
		else
		{
			Q_ASSERT(selectedConnectionPoints.size() >= 2);
		}
	}

	if (pointInitialized == false && selectedItem->isType<VFrame30::PosConnectionImpl>() == true)
	{
		std::vector<VFrame30::SchemaPoint> selectedConnectionPoints = selectedItem->getPointList();

		if (selectedConnectionPoints.size() >= 2)
		{
			// FOR NOW JUST TAKE CENTER BY RECT END POINTS
			//
			const VFrame30::SchemaPoint& firstPoint = selectedConnectionPoints.front();
			const VFrame30::SchemaPoint& lastPoint = selectedConnectionPoints.back();

			QLineF line = {firstPoint.X, firstPoint.Y, lastPoint.X, lastPoint.Y};

			point = line.center();
			pointInitialized = true;
		}
		else
		{
			Q_ASSERT(selectedConnectionPoints.size() >= 2);
		}
	}

	if (pointInitialized = false)
	{
		Q_ASSERT(pointInitialized);
		return;
	}

	//  --
	const double HorzFactor = 1.5;
	point.setX(point.x() * HorzFactor);

	// Decay all other items to points
	//
	double minDistance = DBL_MAX;
	std::shared_ptr<VFrame30::SchemaItem> minDistanceItem;

	auto calcDistance = [&minDistance, &minDistanceItem, &point, HorzFactor](double x, double y, auto schemaItem)
		{
			QPointF p = {x * HorzFactor, y};

			if (point.y() - 0.000001 <= p.y())
			{
				double d = QLineF(point, p).length();

				if (d < minDistance)
				{
					minDistance = d;
					minDistanceItem = schemaItem;
				}
			}

			return;
		};


	for (std::shared_ptr<VFrame30::SchemaItem> item : editSchemaView()->activeLayer()->Items)
	{
		if (item == selectedItem)
		{
			continue;
		}

		if (auto posRectImpl = item->toType<VFrame30::PosRectImpl>();
			posRectImpl != nullptr)
		{
			QRectF rc = {posRectImpl->leftDocPt(), posRectImpl->topDocPt(),
						 posRectImpl->widthDocPt(), posRectImpl->heightDocPt()};

			QPointF center = rc.center();

			if (point.y() < rc.top()  ||
				point.y() < rc.bottom())
			{
				calcDistance(center.x(), rc.top(), item);
				calcDistance(rc.right(), rc.top(), item);
				calcDistance(rc.right(), center.y(), item);
				calcDistance(rc.right(), rc.bottom(), item);
				calcDistance(center.x(), rc.bottom(), item);
				calcDistance(rc.left(), rc.top(), item);
				calcDistance(rc.left(), rc.bottom(), item);
				calcDistance(rc.left(), center.y(), item);
			}

			continue;
		}
	}

	// Get the closest item
	//
	if (minDistanceItem  != nullptr)
	{
		selectItem(minDistanceItem);
	}

	return;
}

void EditSchemaWidget::sameWidth()
{
	auto selected = selectedNonLockedItems();

	if (selected.empty() == true)
	{
		return;
	}

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

	m_editEngine->runSetPoints(newPoints, selectedFiltered, true);

	return;
}

void EditSchemaWidget::sameHeight()
{
	auto selected = selectedNonLockedItems();

	if (selected.empty() == true)
	{
		return;
	}

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

	m_editEngine->runSetPoints(newPoints, selectedFiltered, true);

	return;
}

void EditSchemaWidget::sameSize()
{
	auto selected = selectedNonLockedItems();

	if (selected.empty() == true)
	{
		return;
	}

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

	m_editEngine->runSetPoints(newPoints, selectedFiltered, true);

	return;
}

void EditSchemaWidget::alignLeft()
{
	auto selected = selectedNonLockedItems();

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

	m_editEngine->runSetPoints(newPoints, selected, true);

	return;
}

void EditSchemaWidget::alignRight()
{
	auto selected = selectedNonLockedItems();

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

	m_editEngine->runSetPoints(newPoints, selected, true);

	return;
}

void EditSchemaWidget::alignTop()
{
	auto selected = selectedNonLockedItems();

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

	m_editEngine->runSetPoints(newPoints, selected, true);

	return;
}

void EditSchemaWidget::alignBottom()
{
	auto selected = selectedNonLockedItems();

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

	m_editEngine->runSetPoints(newPoints, selected, true);

	return;
}

void EditSchemaWidget::bringToFront()
{
	auto selected = selectedNonLockedItems();

	if (selected.empty() == true)
	{
		return;
	}

	m_editEngine->runSetOrder(EditEngine::SetOrder::BringToFront, selected, activeLayer());
}

void EditSchemaWidget::bringForward()
{
	auto selected = selectedNonLockedItems();

	if (selected.empty() == true)
	{
		return;
	}

	m_editEngine->runSetOrder(EditEngine::SetOrder::BringForward, selected, activeLayer());
}

void EditSchemaWidget::sendToBack()
{
	auto selected = selectedNonLockedItems();

	if (selected.empty() == true)
	{
		return;
	}

	m_editEngine->runSetOrder(EditEngine::SetOrder::SendToBack, selected, activeLayer());
}

void EditSchemaWidget::sendBackward()
{
	auto selected = selectedNonLockedItems();

	if (selected.empty() == true)
	{
		return;
	}

	m_editEngine->runSetOrder(EditEngine::SetOrder::SendBackward, selected, activeLayer());
}

void EditSchemaWidget::transformIntoInput()
{
	if (selectedItems().empty() == true)
	{
		return;
	}

	for (auto item : selectedItems())
	{
		if (item->isType<VFrame30::SchemaItemSignal>() == false)
		{
			assert(item->isType<VFrame30::SchemaItemSignal>() == true);
			return;
		}
	}

	const std::vector<std::shared_ptr<VFrame30::SchemaItem>> selected = selectedItems();
	std::list<std::shared_ptr<VFrame30::SchemaItem>> newItems;

	for (auto item : selected)
	{
		auto signalItem = item->toType<VFrame30::SchemaItemSignal>();
		assert(signalItem);

		auto transformedItem = signalItem->transformIntoInput();
		assert(transformedItem);

		newItems.push_back(transformedItem);
	}

	if (bool ok = m_editEngine->startBatch();
		ok == true)
	{
		m_editEngine->runDeleteItem(selected, activeLayer());
		m_editEngine->runAddItem(newItems, activeLayer());

		m_editEngine->endBatch();
	}

	return;
}

void EditSchemaWidget::transformIntoInOut()
{
	if (selectedItems().empty() == true)
	{
		return;
	}

	for (auto item : selectedItems())
	{
		if (item->isType<VFrame30::SchemaItemSignal>() == false)
		{
			assert(item->isType<VFrame30::SchemaItemSignal>() == true);
			return;
		}
	}

	const std::vector<std::shared_ptr<VFrame30::SchemaItem>> selected = selectedItems();
	std::list<std::shared_ptr<VFrame30::SchemaItem>> newItems;

	for (auto item : selected)
	{
		auto signalItem = item->toType<VFrame30::SchemaItemSignal>();
		assert(signalItem);

		auto transformedItem = signalItem->transformIntoInOut();
		assert(transformedItem);

		newItems.push_back(transformedItem);
	}

	if (bool ok = m_editEngine->startBatch();
		ok == true)
	{
		m_editEngine->runDeleteItem(selected, activeLayer());
		m_editEngine->runAddItem(newItems, activeLayer());

		m_editEngine->endBatch();
	}

	return;
}

void EditSchemaWidget::transformIntoOutput()
{
	if (selectedItems().empty() == true)
	{
		return;
	}

	for (auto item : selectedItems())
	{
		if (item->isType<VFrame30::SchemaItemSignal>() == false)
		{
			assert(item->isType<VFrame30::SchemaItemSignal>() == true);
			return;
		}
	}

	const std::vector<std::shared_ptr<VFrame30::SchemaItem>> selected = selectedItems();
	std::list<std::shared_ptr<VFrame30::SchemaItem>> newItems;

	for (auto item : selected)
	{
		auto signalItem = item->toType<VFrame30::SchemaItemSignal>();
		assert(signalItem);

		auto transformedItem = signalItem->transformIntoOutput();
		assert(transformedItem);

		newItems.push_back(transformedItem);
	}

	if (bool ok = m_editEngine->startBatch();
		ok == true)
	{
		m_editEngine->runDeleteItem(selected, activeLayer());
		m_editEngine->runAddItem(newItems, activeLayer());

		m_editEngine->endBatch();
	}

	return;
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

		connect(m_findDialog, &SchemaFindDialog::replaceAndFind, this, &EditSchemaWidget::replaceAndFind);
		connect(m_findDialog, &SchemaFindDialog::replaceAll, this, &EditSchemaWidget::replaceAll);
	}

	m_findDialog->show();
	m_findDialog->raise();
	m_findDialog->activateWindow();

	return;
}

void EditSchemaWidget::findNext(Qt::CaseSensitivity cs)
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

		auto result = item->searchTextByProps(searchText, cs);

		if (result.empty() == false)
		{
			selectItem(item);
			m_findDialog->updateFoundInformation(item, result, searchText, cs);
			return;
		}
	}

	// Serach text from the beginning to selected
	//
	for (auto it = layer->Items.begin(); it != searchStartIterator; ++it)
	{
		std::shared_ptr<VFrame30::SchemaItem> item = *it;

		auto result = item->searchTextByProps(searchText, cs);

		if (result.empty() == false)
		{
			selectItem(item);
			m_findDialog->updateFoundInformation(item, result, searchText, cs);
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

void EditSchemaWidget::findPrev(Qt::CaseSensitivity cs)
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

		auto result = item->searchTextByProps(searchText, cs);

		if (result.empty() == false)
		{
			selectItem(item);
			m_findDialog->updateFoundInformation(item, result, searchText, cs);
			return;
		}
	}

	// Serach text from the beginning to selected
	//
	for (auto it = layer->Items.rbegin(); it != searchStartIterator; ++it)
	{
		std::shared_ptr<VFrame30::SchemaItem> item = *it;

		auto result = item->searchTextByProps(searchText, cs);

		if (result.empty() == false)
		{
			selectItem(item);
			m_findDialog->updateFoundInformation(item, result, searchText, cs);
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

int EditSchemaWidget::replace(std::shared_ptr<VFrame30::SchemaItem> item, QString findText, QString replaceWith, Qt::CaseSensitivity cs)
{
	if (item == nullptr)
	{
		assert(item);
		return 0;
	}

	QByteArray oldState;
	item->saveToByteArray(&oldState);

	int replaceCount = item->replace(findText, replaceWith, cs);

	QByteArray newState;
	item->saveToByteArray(&newState);

	item->Load(oldState);

	if (replaceCount != 0)
	{
		m_editEngine->runSetObject(oldState, newState, item);
	}

	return replaceCount;
}

void EditSchemaWidget::replaceAndFind(QString findText, QString replaceWith, Qt::CaseSensitivity cs)
{
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

	if (selected.empty() == true)
	{
		searchStartIterator = layer->Items.begin();
	}
	else
	{
		searchStartIterator = std::find(layer->Items.begin(), layer->Items.end(), selected.front());

		if (searchStartIterator == layer->Items.end())
		{
			searchStartIterator = layer->Items.begin();
		}
		else
		{
			// Replace text in selected item
			//
			replace(*searchStartIterator, findText, replaceWith, cs);
			searchStartIterator ++;
		}
	}

	// Text in current selected item replaced, find and select next item
	//
	if (searchStartIterator == layer->Items.end())
	{
		searchStartIterator = layer->Items.begin();
	}

	for (auto it = searchStartIterator; it != layer->Items.end(); ++it)
	{
		std::shared_ptr<VFrame30::SchemaItem> item = *it;

		auto result = item->searchTextByProps(findText, cs);

		if (result.empty() == false)
		{
			selectItem(item);

			if (m_findDialog != nullptr)
			{
				m_findDialog->updateFoundInformation(item, result, findText, cs);
			}
			return;
		}
	}

	// Serach text from the beginning
	//
	for (auto it = layer->Items.begin(); it != searchStartIterator; ++it)
	{
		std::shared_ptr<VFrame30::SchemaItem> item = *it;

		auto result = item->searchTextByProps(findText, cs);

		if (result.empty() == false)
		{
			selectItem(item);

			if (m_findDialog != nullptr)
			{
				m_findDialog->updateFoundInformation(item, result, findText, cs);
			}
			return;
		}
	}

	// Text not found
	//
	clearSelection();

	QMessageBox::information(this, qApp->applicationName(), tr("Text <b>%1</b> not found.").arg(findText));

	if (m_findDialog != nullptr)
	{
		m_findDialog->show();
		m_findDialog->raise();
		m_findDialog->activateWindow();
		m_findDialog->setFocusToEditLine();
	}

	return;
}

void EditSchemaWidget::replaceAll(QString findText, QString replaceWith, Qt::CaseSensitivity cs)
{
	// Look for text
	//
	std::shared_ptr<VFrame30::SchemaLayer> layer = activeLayer();
	assert(layer);

	if (layer->Items.empty() == true)
	{
		clearSelection();
		return;
	}

	// If there are selected items, then replace only in selected
	// else relace in all layer's items
	//
	std::vector<std::shared_ptr<VFrame30::SchemaItem>> items = selectedItems();
	if (items.empty() == true)
	{
		items.assign(layer->Items.begin(), layer->Items.end());
	}

	// Replace
	//
	int count = 0;

	std::vector<std::shared_ptr<VFrame30::SchemaItem>> replacedInItems;
	replacedInItems.reserve(items.size());

	for(std::shared_ptr<VFrame30::SchemaItem> item : items)
	{
		int itemReplaces = replace(item, findText, replaceWith, cs);

		if (itemReplaces != 0)
		{
			count += itemReplaces;
			replacedInItems.push_back(item);
		}
	}

	if (count == 0)
	{
		QMessageBox::information(this, tr("Replace All Result"), tr("Text <b>%1</b> not found.").arg(findText));
	}
	else
	{
		selectItems(replacedInItems);
		QMessageBox::information(this, tr("Replace All Result"), tr("%1 replaced to %2 in %3 item(s).").arg(findText).arg(replaceWith).arg(replacedInItems.size()));
	}

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

	if (state == MouseState::Moving ||
		state == MouseState::SizingTopLeft ||
		state == MouseState::SizingTopLeft ||
		state == MouseState::SizingTop ||
		state == MouseState::SizingTopRight ||
		state == MouseState::SizingRight ||
		state == MouseState::SizingBottomRight ||
		state == MouseState::SizingBottom ||
		state == MouseState::SizingBottomLeft ||
		state == MouseState::SizingLeft)
	{
		initMoveAfbsConnectionLinks(state);
	}

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
	m_initialSchemaId = schema()->schemaId();

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
	setWindowTitle(tr("Find and Replace"));

	setWindowFlags((windowFlags() &
					~Qt::WindowMinimizeButtonHint &
					~Qt::WindowMaximizeButtonHint &
					~Qt::WindowContextHelpButtonHint) | Qt::CustomizeWindowHint);

	// FindText/Replace - text for search
	//
	m_findTextEdit = new QLineEdit();
	m_replaceTextEdit = new QLineEdit();

	QCompleter* searchCompleter = new QCompleter(theSettings.buildSearchCompleter(), this);
	searchCompleter->setCaseSensitivity(Qt::CaseInsensitive);

	m_findTextEdit->setCompleter(searchCompleter);
	m_replaceTextEdit->setCompleter(searchCompleter);

	// CaseSensivity check box
	//
	m_caseSensitiveCheckBox = new QCheckBox(tr("Case Sensitive"));
	m_caseSensitiveCheckBox->setChecked(theSettings.m_findSchemaItemCaseSensitive);

	// Find Result
	//
	m_findResult = new QTextEdit;
	m_findResult->setReadOnly(true);

	auto p = qApp->palette("QListView");

	QColor highlight = p.highlight().color();
	QColor highlightText = p.highlightedText().color();

	QString selectionColor = QString("QTextEdit { selection-background-color: %1; selection-color: %2; }")
							 .arg(highlight.name())
							 .arg(highlightText.name());

	m_findResult->setStyleSheet(selectionColor);

	// Find buttons
	//
	m_prevButton = new QPushButton(tr("Find Previous"));
	m_nextButton = new QPushButton(tr("Find Next"));

	//m_nextButton->setShortcut(QKeySequence::FindNext);		// Done via Actions, works much faster
	//m_nextButton->setShortcutVisibleInContextMenu(true);
	//m_prevButton->setShortcut(QKeySequence::FindPrevious);	// Done via Actions, works much faster
	//m_prevButton->setShortcutVisibleInContextMenu(true);

	// Replace buttons
	//
	m_replaceAllButton = new QPushButton(tr("Replace All"));
	m_replaceButton = new QPushButton(tr("Replace && Find"));

	connect(m_replaceButton, &QPushButton::clicked, this, &SchemaFindDialog::replaceAndFindPressed);
	connect(m_replaceAllButton, &QPushButton::clicked, this, &SchemaFindDialog::replaceAllPressed);

	// --
	//
	QGridLayout* layout = new QGridLayout();

	layout->addWidget(new QLabel("Find:"), 0, 0, 1, 1);
	layout->addWidget(m_findTextEdit, 0, 1, 1, 3);

	layout->addWidget(new QLabel("Replace with:"), 1, 0, 1, 1);
	layout->addWidget(m_replaceTextEdit, 1, 1, 1, 3);

	layout->addWidget(m_caseSensitiveCheckBox, 2, 0, 1, 4);

	layout->addWidget(m_findResult, 3, 0, 1, 4);

	layout->addWidget(m_replaceAllButton, 4, 0);
	layout->addWidget(m_replaceButton, 4, 1);

	layout->addWidget(m_prevButton, 4, 2);
	layout->addWidget(m_nextButton, 4, 3);

	setLayout(layout);

	setMinimumWidth(300);

	QAction* nextAction = new QAction(tr("Find Next"), this);
	nextAction->setShortcut(QKeySequence::FindNext);
	nextAction->setShortcutVisibleInContextMenu(true);
	addAction(nextAction);

	QAction* prevAction = new QAction(tr("Find Prev"), this);
	prevAction->setShortcut(QKeySequence::FindPrevious);
	prevAction->setShortcutVisibleInContextMenu(true);
	addAction(prevAction);

	// Find buttons
	//
	connect(m_caseSensitiveCheckBox, &QCheckBox::toggled, this,
			[](bool checked)
			{
				theSettings.m_findSchemaItemCaseSensitive = checked;
			});

	auto findNextFunc = [this]()
		{
			emit findNext(m_caseSensitiveCheckBox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);
		};

	auto findPrevFunc = [this]()
		{
			emit findPrev(m_caseSensitiveCheckBox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);
		};

	connect(nextAction, &QAction::triggered, this, findNextFunc);
	connect(prevAction, &QAction::triggered, this, findPrevFunc);

	// --
	//
	connect(m_nextButton, &QPushButton::clicked, this, findNextFunc);
	connect(m_prevButton, &QPushButton::clicked, this, findPrevFunc);

	m_nextButton->setDefault(true);

	return;
}

QString SchemaFindDialog::findText() const
{
	assert(m_findTextEdit);

	QString text = m_findTextEdit->text().trimmed();

	return text;
}

void SchemaFindDialog::setFocusToEditLine()
{
	assert(m_findTextEdit);

	m_findTextEdit->setFocus();
	m_findTextEdit->selectAll();

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

		QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_findTextEdit->completer()->model());
		assert(completerModel);

		if (completerModel != nullptr)
		{
			completerModel->setStringList(theSettings.buildSearchCompleter());
		}
	}
}

void SchemaFindDialog::updateFoundInformation(std::shared_ptr<VFrame30::SchemaItem> item,
											  const std::list<std::pair<QString, QString>>& foundProps,
											  QString /*searchText*/,
											  Qt::CaseSensitivity /*cs*/)
{

	QString itemCaption = QString(item->metaObject()->className());

	if (itemCaption.startsWith("VFrame30::SchemaItem") == true)
	{
		itemCaption.remove(0, 20);		// 20 is length of VFrame30::SchemaItem
	}

	QString infoText = itemCaption + "\n";

	for (auto p : foundProps)
	{
		infoText.append(QString("%1 : %2\n").arg(p.first).arg(p.second));
	}

	m_findResult->setText(infoText);

	// To do: highlight all searchText with CaseSensitivity
	//

	return;
}

void SchemaFindDialog::replaceAndFindPressed()
{
	QString findText = m_findTextEdit->text();
	QString replaceWith = m_replaceTextEdit->text();
	Qt::CaseSensitivity cs = m_caseSensitiveCheckBox->isChecked() == true ? Qt::CaseSensitive : Qt::CaseInsensitive;

	if (findText.trimmed().isEmpty() == true)
	{
		return;
	}

	emit replaceAndFind(findText, replaceWith, cs);

	return;
}

void SchemaFindDialog::replaceAllPressed()
{
	QString findText = m_findTextEdit->text();
	QString replaceWith = m_replaceTextEdit->text();
	Qt::CaseSensitivity cs = m_caseSensitiveCheckBox->isChecked() == true ? Qt::CaseSensitive : Qt::CaseInsensitive;

	if (findText.trimmed().isEmpty() == true)
	{
		return;
	}

	emit replaceAll(findText, replaceWith, cs);

	return;
}
