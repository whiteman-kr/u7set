#include "ReportSchemaView.h"
#include "../VFrame30/Schema.h"

namespace ReportLib
{
	//
	// ReportSchemaView
	//

	ReportSchemaView::ReportSchemaView(bool infoMode):
		VFrame30::SchemaView(),
		m_infoMode(infoMode)
	{
	}

	ReportSchemaView::~ReportSchemaView()
	{
		qDebug() << "ReportSchemaView deleted";
	}

	void ReportSchemaView::adjust(QPainter* painter, double startX, double startY, double zoom) const
	{
		Ajust(painter, schema()->unit(), startX, startY, zoom);
	}

	void ReportSchemaView::drawCompareOutlines(VFrame30::CDrawParam* drawParam, const QRectF& clipRect, const std::map<QUuid, ReportSchemaCompareAction>& compareActions)
	{
		if (drawParam == nullptr)
		{
			assert(drawParam != nullptr);
			return;
		}

		if (schema() == nullptr)
		{
			assert(schema() != nullptr);
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
		for (const auto& layer : schema()->layers())
		{
			if (layer->show() == false)
			{
				continue;
			}

			for (const auto& item : layer->items())
			{
				auto actionIt = compareActions.find(item->guid());
				if (actionIt == compareActions.end())
				{
					assert(actionIt != compareActions.end());
					continue;
				}

				ReportSchemaCompareAction compareAction = actionIt->second;

				QColor color;

				switch (compareAction)
				{
				case ReportSchemaCompareAction::Unmodified:
					color = QColor(0, 0, 0, 0);			// Full transparent, as is
					break;
				case ReportSchemaCompareAction::Modified:
					color = QColor(0, 0, 0xC0, 128);
					break;
				case ReportSchemaCompareAction::Added:
					color = QColor(0, 0xC0, 0, 128);
					break;
				case ReportSchemaCompareAction::Deleted:
					color = QColor(0xC0, 0, 0, 128);
					break;
				default:
					assert(false);
				}

				if (compareAction != ReportSchemaCompareAction::Unmodified &&
						item->isIntersectRect(clipX, clipY, clipWidth, clipHeight) == true)
				{
					// Draw item issue
					//
					item->drawCompareAction(drawParam, color);
				}
			}
		}
	}

	VFrame30::DrawMode ReportSchemaView::drawMode() const
	{
		return VFrame30::DrawMode::Editor;
	}

	bool ReportSchemaView::infoMode() const
	{
		return m_infoMode;
	}

	void ReportSchemaView::setInfoMode(bool value)
	{
		m_infoMode = value;
	}
}
