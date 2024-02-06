#include "SchemaItemVduValue.h"
#include "DrawParam.h"
#include "MacrosExpander.h"
#include "PropertyNames.h"
#include "SchemaView.h"


namespace VFrame30
{
	SchemaItemVduValue::SchemaItemVduValue(void) :
		SchemaItemVduValue(SchemaUnit::Display)
	{
	}

	SchemaItemVduValue::SchemaItemVduValue(SchemaUnit units) :
		PosRectImpl{}
	{
		assert(units == SchemaUnit::Display);

		ADD_PROPERTY_GET_SET_CAT(int,
								 PropertyNames::lineWeight,
								 PropertyNames::appearanceCategory,
								 true,
								 SchemaItemVduValue::weight,
								 SchemaItemVduValue::setWeight);

		ADD_PROPERTY_GET_SET_CAT(bool,
								 PropertyNames::drawRect,
								 PropertyNames::appearanceCategory,
								 true,
								 SchemaItemVduValue::drawRect,
								 SchemaItemVduValue::setDrawRect);

		ADD_PROPERTY_GET_SET_CAT(QColor,
								 PropertyNames::lineColor,
								 PropertyNames::appearanceCategory,
								 true,
								 SchemaItemVduValue::lineColor,
								 SchemaItemVduValue::setLineColor);

		ADD_PROPERTY_GET_SET_CAT(QColor,
								 PropertyNames::fillColor,
								 PropertyNames::appearanceCategory,
								 true,
								 SchemaItemVduValue::fillColor,
								 SchemaItemVduValue::setFillColor);

		// Text Category Properties
		//
		ADD_PROPERTY_GET_SET_CAT(QColor,
								 PropertyNames::textColor,
								 PropertyNames::textCategory,
								 true,
								 SchemaItemVduValue::textColor,
								 SchemaItemVduValue::setTextColor);

		ADD_PROPERTY_GET_SET_CAT(QString,
								 PropertyNames::fontName,
								 PropertyNames::textCategory,
								 true,
								 SchemaItemVduValue::fontName,
								 SchemaItemVduValue::setFontName);

		ADD_PROPERTY_GET_SET_CAT(QString,
								 PropertyNames::appSignalId,
								 PropertyNames::textCategory,
								 true,
								 SchemaItemVduValue::appSignalId,
								 SchemaItemVduValue::setAppSignalId);

		// --
		//
		m_static = false;
		setItemUnit(units);

		return;
	}

	// Serialization
	//
	bool SchemaItemVduValue::SaveData(Proto::Envelope* message) const
	{
		bool result = PosRectImpl::SaveData(message);
		if (result == false || message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}

		// --
		//
		auto* valueMessage = message->mutable_schemaitem()->mutable_vduvalue();

		valueMessage->set_weight(m_weight);
		valueMessage->set_drawrect(m_drawRect);

		valueMessage->set_linecolor(m_lineColor.rgba());
		valueMessage->set_fillcolor(m_fillColor.rgba());
		valueMessage->set_textcolor(m_textColor.rgba());

		valueMessage->set_fontname(m_fontName.toUtf8());
		valueMessage->set_appsignalid(m_appSignalId.toUtf8());

		return true;
	}

	bool SchemaItemVduValue::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemaitem() == false)
		{
			assert(message.has_schemaitem());
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
		if (message.schemaitem().has_vduvalue() == false)
		{
			assert(message.schemaitem().has_vduvalue());
			return false;
		}

		const auto& valueMessage = message.schemaitem().vduvalue();

		m_weight = valueMessage.weight();
		m_drawRect = valueMessage.drawrect();

		m_lineColor = QColor::fromRgba(valueMessage.linecolor());
		m_fillColor = QColor::fromRgba(valueMessage.fillcolor());
		m_textColor = QColor::fromRgba(valueMessage.textcolor());

		m_fontName = QString::fromUtf8(valueMessage.fontname().c_str());
		m_appSignalId = QString::fromUtf8(valueMessage.appsignalid().c_str());

		return true;
	}

	// Drawing Functions
	//
	void SchemaItemVduValue::draw(CDrawParam* drawParam) const
	{
		QPainter* painter = drawParam->painter();
		QRectF boundingRect = boundingRectInDocPt(drawParam);

		// Draw rect
		//
		QBrush fillBrush{m_fillColor, Qt::SolidPattern};
		painter->setBrush(fillBrush);

		QPen rectPen{Qt::NoPen};

		if (drawRect() == true)
		{
			rectPen = QPen{m_lineColor};
			rectPen.setWidth(m_weight);
		}

		painter->setPen(rectPen);

		painter->drawRect(boundingRect);

		// Drawing Text
		//
		QFont font{m_fontName};

		// Assume that font is "Arial_12", then we get 12 from it and use it as a font size.
		//
		{
			int fontSize = 0;

			auto splittedFontName = m_fontName.split(QChar('_'), Qt::SkipEmptyParts);

			if (splittedFontName.isEmpty() == false)
			{
				bool convertOk = false;
				fontSize = splittedFontName.last().toInt(&convertOk);
				if (convertOk == false)
				{
					fontSize = 12;
				}
			}

			font.setPixelSize(fontSize);
		}

		painter->setFont(font);
		painter->setPen(m_textColor);

		DrawHelper::drawText(painter,
							 itemUnit(),
							 "?",
							 boundingRect,
							 static_cast<int>(Qt::AlignHCenter) | static_cast<int>(Qt::AlignVCenter),
							 nullptr);

		return;
	}

	double SchemaItemVduValue::minimumPossibleHeightDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
	}

	double SchemaItemVduValue::minimumPossibleWidthDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
	}

	int SchemaItemVduValue::weight() const
	{
		return m_weight;
	}

	void SchemaItemVduValue::setWeight(int weight)
	{
		m_weight = std::clamp(weight, 1, 16);
	}

	bool SchemaItemVduValue::drawRect() const
	{
		return m_drawRect;
	}

	void SchemaItemVduValue::setDrawRect(bool value)
	{
		m_drawRect = value;
	}

	QColor SchemaItemVduValue::lineColor() const
	{
		return m_lineColor;
	}

	void SchemaItemVduValue::setLineColor(QColor color)
	{
		m_lineColor = color;
	}

	QColor SchemaItemVduValue::fillColor() const
	{
		return m_fillColor;
	}

	void SchemaItemVduValue::setFillColor(QColor color)
	{
		m_fillColor = color;
	}

	QColor SchemaItemVduValue::textColor() const
	{
		return m_textColor;
	}

	void SchemaItemVduValue::setTextColor(QColor color)
	{
		m_textColor = color;
	}

	const QString& SchemaItemVduValue::fontName() const
	{
		return m_fontName;
	}

	void SchemaItemVduValue::setFontName(const QString& value)
	{
		m_fontName = value;
	}

	const QString& SchemaItemVduValue::appSignalId() const
	{
		return m_appSignalId;
	}

	void SchemaItemVduValue::setAppSignalId(const QString& value)
	{
		m_appSignalId = value;
	}

} // namespace VFrame30
