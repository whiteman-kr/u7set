#include "SchemaItemFrame.h"
#include "PropertyNames.h"
#include "DrawParam.h"
#include "Schema.h"
#include "FblItemRect.h"

namespace VFrame30
{
	SchemaItemFrame::SchemaItemFrame(void) :
		SchemaItemFrame(SchemaUnit::Inch)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	SchemaItemFrame::SchemaItemFrame(SchemaUnit unit)
	{
		ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::schemaId, true, SchemaItemFrame::schemaId, SchemaItemFrame::setSchemaId);

		ADD_PROPERTY_GETTER_SETTER(bool, PropertyNames::allowScale, true, SchemaItemFrame::allowScale, SchemaItemFrame::setAllowScale);
		ADD_PROPERTY_GETTER_SETTER(bool, PropertyNames::keepAspectRatio, true, SchemaItemFrame::keepAspectRatio, SchemaItemFrame::setKeepAspectRatio);

		m_static = true;
		setItemUnit(unit);

		return;
	}

	SchemaItemFrame::~SchemaItemFrame(void)
	{
	}

	// Serialization
	//
	bool SchemaItemFrame::SaveData(Proto::Envelope* message) const
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
		Proto::SchemaItemFrame* frameMessage = message->mutable_schemaitem()->mutable_frame();

		frameMessage->set_schemaid(m_schemaId.toStdString());
		frameMessage->set_allowscale(m_allowScale);
		frameMessage->set_keepaspectratio(m_keepAspectRatio);

		return true;
	}

	bool SchemaItemFrame::LoadData(const Proto::Envelope& message)
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
		if (message.schemaitem().has_frame() == false)
		{
			assert(message.schemaitem().has_frame());
			return false;
		}

		const Proto::SchemaItemFrame& frameMessage = message.schemaitem().frame();

		m_schemaId = QString::fromStdString(frameMessage.schemaid());
		m_allowScale = frameMessage.allowscale();
		m_keepAspectRatio = frameMessage.keepaspectratio();

		return true;
	}

	// Drawing Functions
	//
	void SchemaItemFrame::draw(CDrawParam* drawParam, const Schema* /*schema*/, const SchemaLayer* /*layer*/) const
	{
		QPainter* p = drawParam->painter();

		// Initialization drawing resources
		//

		// Calculate rectangle
		//
		QRectF r(leftDocPt(), topDocPt(), widthDocPt(), heightDocPt());

		r.setTopRight(drawParam->gridToDpi(r.topRight()));
		r.setBottomLeft(drawParam->gridToDpi(r.bottomLeft()));

		if (std::abs(r.left() - r.right()) < 0.000001)
		{
			r.setRight(r.left() + 0.000001f);
		}

		if (std::abs(r.bottom() - r.top()) < 0.000001)
		{
			r.setBottom(r.top() + 0.000001f);
		}

		// Filling rect 
		//
		QPainter::RenderHints oldrenderhints = p->renderHints();
		p->setRenderHint(QPainter::Antialiasing, false);

		QBrush fillBrush{QColor{0xFF, 0xFF, 0xFF, 0xC0}};
		p->fillRect(r, fillBrush);

		p->setRenderHints(oldrenderhints);

		// Drawing rect 
		//
		QPen pen(Qt::black);
		pen.setWidthF(drawParam->cosmeticPenWidth());

		p->setPen(pen);
		p->drawRect(r);

		// Drawing Text
		//
		QString text = schemaId();

		p->setPen(Qt::black);

		QFont f;		// Default application font
		p->setFont(f);

		DrawHelper::drawText(p, drawParam->schema()->unit(), text, r, Qt::AlignCenter | Qt::AlignVCenter);

		return;
	}

	SchemaItemFrame::ErrorCode SchemaItemFrame::setSchemaToFrame(VFrame30::Schema* destSchema, const VFrame30::Schema* sourceSchema)
	{
		if (destSchema == nullptr || sourceSchema == nullptr)
		{
			Q_ASSERT(destSchema);
			Q_ASSERT(sourceSchema);

			return ErrorCode::ParamError;
		}

		if (destSchema->Layers.empty() == true)
		{
			Q_ASSERT(destSchema->Layers.empty() == false);

			return ErrorCode::ParamError;
		}

		if (schemaId() == destSchema->schemaId())
		{
			return ErrorCode::SourceSchemaHasSameId;
		}

		if (destSchema->unit() != sourceSchema->unit())
		{
			return ErrorCode::SchemasHasDiffrenetUnits;
		}

		// Copy layer by layer, if layer does not exist on destSchema, then copy to compile layer,
		// if compile layer does not exists either, then copy to the first layer
		//
		QRectF destRect{leftDocPt(), topDocPt(), widthDocPt(), heightDocPt()};
		QRectF sourceRect{0, 0, sourceSchema->docWidth(), sourceSchema->docHeight()};

		for (std::shared_ptr<SchemaLayer> sourceLayer : sourceSchema->Layers)
		{
			Q_ASSERT(sourceLayer);

			auto foundDestLayerIt = std::find_if(destSchema->Layers.begin(), destSchema->Layers.end(),
												 [sourceLayer](auto l) { return l->name() == sourceLayer->name(); } );

			if (foundDestLayerIt == destSchema->Layers.end())
			{
				// Source layer is not found in destination, copy to compile layer,
				// if compile layer does not exists either, then copy to the first layer
				//
				foundDestLayerIt = std::find_if(destSchema->Layers.begin(), destSchema->Layers.end(),
												[](auto l) { return l->compile(); } );

				if (foundDestLayerIt == destSchema->Layers.end())
				{
					foundDestLayerIt = destSchema->Layers.begin();
				}
			}

			Q_ASSERT(foundDestLayerIt != destSchema->Layers.end());

			// Copy all form sourceLayer to destLayer, keep the order of items and insert all them right at the end of items
			//
			std::shared_ptr<SchemaLayer> destLayer = *foundDestLayerIt;

			if (destLayer == nullptr)
			{
				Q_ASSERT(destLayer);
				return ErrorCode::InternalError;
			}

			for (std::shared_ptr<SchemaItem> sourceItem : sourceLayer->Items)
			{
				// Make a deep copy of source item, set new guid and label to it
				//
				Proto::Envelope savedItem;

				if (bool saveOk = sourceItem->Save(&savedItem);
					saveOk == false)
				{
					return ErrorCode::InternalError;
				}

				std::shared_ptr<SchemaItem> newItem = SchemaItem::Create(savedItem);
				if (newItem == nullptr)
				{
					return ErrorCode::InternalError;
				}

				newItem->setNewGuid();		// generate new guids for item and its pins

				// Form new label for SchemaItem: SchemaID_FblItemRectLabel
				//
				newItem->setLabel(destSchema->schemaId() + "_" + newItem->label());

				// Insert newItem to destionation schema layer
				//
				if (allowScale() == false)
				{
					// Suppose that units on both schermas are equal. there is a check for it at the function beginning
					//

					// Direct copy, with shifting itmes to destRect
					//
					newItem->moveItem(destRect.left(), destRect.top());
				}
				else
				{
					// moving and scaling to dest rect
					//
					double scaleFactorHorz = destRect.width() / sourceRect.width();
					double scaleFactorVert = destRect.height() / sourceRect.height();

					// Set new Pos
					//
					newItem->setLeft(newItem->left() * scaleFactorHorz);
					newItem->setTop(newItem->top() * scaleFactorVert);

					newItem->moveItem(destRect.left(), destRect.top());

					// Set new width and height
					//
					newItem->SetWidthInDocPt(newItem->GetWidthInDocPt() * scaleFactorHorz);
					newItem->SetHeightInDocPt(newItem->GetHeightInDocPt() * scaleFactorVert);

					// set font size, leave it for the same units (example Pixel vs Pixel, most likely the font must be similar)
					//
					if (auto fontSizeProp = newItem->propertyByCaption(PropertyNames::fontSize);
						fontSizeProp != nullptr)
					{
						bool convOk = false;
						double v = fontSizeProp->value().toDouble(&convOk);

						if (convOk == true)
						{
							fontSizeProp->setValue(v * scaleFactorVert);
						}
					}

					if (VFrame30::FblItemRect* fblItemRect = newItem->toType<VFrame30::FblItemRect>();
						fblItemRect != nullptr)
					{
						//fblItemRect->adjustHeight(destSchema->gridSize() * scaleFactorVert);
					}
				}

				// --
				//
				destLayer->Items.push_back(newItem);
			}
		}

		return ErrorCode::Ok;
	}


	// Properties and Data
	//
	QString SchemaItemFrame::schemaId() const
	{
		return m_schemaId;
	}

	void SchemaItemFrame::setSchemaId(QString value)
	{
		m_schemaId = value;
	}

	bool SchemaItemFrame::allowScale() const
	{
		return m_allowScale;
	}

	void SchemaItemFrame::setAllowScale(bool value)
	{
		m_allowScale = value;
	}

	bool SchemaItemFrame::keepAspectRatio() const
	{
		return m_keepAspectRatio;
	}

	void SchemaItemFrame::setKeepAspectRatio(bool value)
	{
		m_keepAspectRatio = value;
	}

}

