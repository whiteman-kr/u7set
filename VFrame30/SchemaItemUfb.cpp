#include "Stable.h"
#include "SchemaItemUfb.h"
#include "Schema.h"

namespace VFrame30
{
	SchemaItemUfb::SchemaItemUfb(void) :
		SchemaItemUfb(SchemaUnit::Inch)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	SchemaItemUfb::SchemaItemUfb(SchemaUnit unit) :
		FblItemRect(unit)
	{
		auto schemaIdProp = ADD_PROPERTY_GETTER(QString, PropertyNames::ufbSchemaId, true, SchemaItemUfb::ufbSchemaId());
		auto versionProp = ADD_PROPERTY_GETTER(int, PropertyNames::ufbSchemaVersion, true, SchemaItemUfb::ufbSchemaVersion);

		schemaIdProp->setCategory(PropertyNames::functionalCategory);
		versionProp->setCategory(PropertyNames::functionalCategory);
	}

	SchemaItemUfb::SchemaItemUfb(SchemaUnit unit, const UfbSchema* ufbSchema, QString* errorMsg) :
		FblItemRect(unit)
	{
		assert(errorMsg);

		assert(ufbSchema);
		if (ufbSchema == nullptr)
		{
			errorMsg = tr("Pointer to UfbSchema is nullptr.");
			return;
		}

		auto schemaIdProp = ADD_PROPERTY_GETTER(QString, PropertyNames::ufbSchemaId, true, SchemaItemUfb::ufbSchemaId());
		auto versionProp = ADD_PROPERTY_GETTER(int, PropertyNames::ufbSchemaVersion, true, SchemaItemUfb::ufbSchemaVersion);

		schemaIdProp->setCategory(PropertyNames::functionalCategory);
		versionProp->setCategory(PropertyNames::functionalCategory);

		// Create input output signals in VFrame30::FblEtem
		//
		updateElement(ufbSchema, errorMsg);

		return;
	}

	SchemaItemUfb::~SchemaItemUfb(void)
	{
	}

	void SchemaItemUfb::Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* pLayer) const
	{
		QPainter* p = drawParam->painter();

		int dpiX = 96;
		QPaintDevice* pPaintDevice = p->device();
		if (pPaintDevice == nullptr)
		{
			assert(pPaintDevice);
			dpiX = 96;
		}
		else
		{
			dpiX = pPaintDevice->logicalDpiX();
		}

		double pinWidth = GetPinWidth(itemUnit(), dpiX);

		FontParam smallFont = m_font;
		smallFont.setDrawSize(m_font.drawSize() * 0.75);

		// Draw rect and pins
		//
		FblItemRect::Draw(drawParam, schema, pLayer);

		// Draw other
		//
		QRectF r(leftDocPt(), topDocPt(), widthDocPt(), heightDocPt());

		if (std::abs(r.left() - r.right()) < 0.000001)
		{
			r.setRight(r.left() + 0.000001);
		}

		if (std::abs(r.bottom() - r.top()) < 0.000001)
		{
			r.setBottom(r.top() + 0.000001);
		}

		if (inputsCount() > 0)
		{
			r.setLeft(r.left() + pinWidth);
		}

		if (outputsCount() > 0)
		{
			r.setRight(r.right() - pinWidth);
		}

		QRectF labelRect(r);	// save rect for future use

		r.setLeft(r.left() + m_font.drawSize() / 4.0);
		r.setRight(r.right() - m_font.drawSize() / 4.0);

		// Draw caption
		//
		QString text = m_afbElement.caption();

		p->setPen(textColor());
		DrawHelper::DrawText(p, m_font, itemUnit(), text, r, Qt::AlignHCenter | Qt::AlignTop);

		// Draw Label
		//
		if (drawParam->infoMode() == true)
		{
			QString labelText = label();

			labelRect.moveBottomLeft(labelRect.topRight());

			p->setPen(Qt::darkGray);
			DrawHelper::DrawText(p, smallFont, itemUnit(), labelText, labelRect, Qt::TextDontClip | Qt::AlignLeft | Qt::AlignBottom);
		}

		// Draw line under caption
		//

//		QPen captionLinePen(lineColor());
//		captionLinePen.setWidthF(0.0);		// Don't use getter!

//		p->setPen(captionLinePen);

//		p->drawLine(QPointF(r.left(), topDocPt() + m_font.drawSize() * 1.5),
//					QPointF(r.left() + r.width(), topDocPt() + m_font.drawSize() * 1.5));

		return;
	}

	// Serialization
	//
	bool SchemaItemUfb::SaveData(Proto::Envelope* message) const
	{
		bool result = FblItemRect::SaveData(message);
		if (result == false || message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}
	
		// --
		//
		Proto::SchemaItemUfb* ufbpb = message->mutable_schemaitem()->mutable_ufb();

		ufbpb->set_precision(m_precision);
		ufbpb->set_label(m_label.toStdString());

		return true;
	}

	bool SchemaItemUfb::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemaitem() == false)
		{
			assert(message.has_schemaitem());
			return false;
		}

		// --
		//
		bool result = FblItemRect::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemaitem().has_ufb() == false)
		{
			assert(message.schemaitem().has_ufb());
			return false;
		}
		
		const Proto::SchemaItemUfb& ufbpb = message.schemaitem().ufb();
		
		m_precision = ufbpb.precision();

		if (ufbpb.has_label() == true)
		{
			m_label = QString::fromStdString(ufbpb.label());
		}

		// Add afb properties to class meta object
		//
		addSpecificParamProperties();

		return ok;
	}

	QString SchemaItemAfb::buildName() const
	{
		return QString("%1 %2")
				.arg(afbStrID())
				.arg(label());
	}

	bool SchemaItemAfb::setAfbParam(const QString& name, QVariant value, std::shared_ptr<Schema> schema, QString* errorMsg)
	{
		if (name.isEmpty() == true ||
			schema == nullptr ||
			errorMsg == nullptr)
		{
			assert(name.isEmpty() != true);
			assert(schema != nullptr);
			assert(errorMsg);
			return false;
		}

		auto found = std::find_if(m_afbElement.params().begin(), m_afbElement.params().end(), [&name](const Afb::AfbParam& p)
			{
				return p.caption() == name;
			});

		if (found == m_afbElement.params().end())
		{
			assert(found != m_afbElement.params().end());
			return false;
		}



		if (found->value() != value)
		{
			qDebug() << tr("Param %1 was changed from %2 to %3").
						arg(name).
						arg(found->value().toString()).
						arg(value.toString());

			found->setValue(value);

			// Call script here
			//
			QString changedScript = found->changedScript();
			if (changedScript.isEmpty() == false)
			{
				bool result = executeScript(changedScript, m_afbElement, errorMsg);
				if (result == false)
				{
					return false;
				}
			}

		}

		return true;
	}

	bool SchemaItemAfb::setAfbParamByOpName(const QString& opName, QVariant value)
	{
		if (opName.isEmpty() == true)
		{
			assert(opName.isEmpty() != true);
			return false;
		}

		auto found = std::find_if(m_afbElement.params().begin(), m_afbElement.params().end(), [&opName](const Afb::AfbParam& p)
			{
				return p.opName() == opName;
			});

		if (found == m_afbElement.params().end())
		{
			assert(found != m_afbElement.params().end());
			return false;
		}

//		qDebug() << tr("Param %1 was changed from %2 to %3").
//					arg(opName).
//					arg(found->value().toString()).
//					arg(value.toString());

		found->setValue(value);

		return true;
	}

	QVariant SchemaItemAfb::getAfbParam(const QString& name)
	{
		for (Afb::AfbParam& p : m_afbElement.params())
		{
			if (p.caption() == name)
			{
				return p.value();
			}
		}
		return QVariant();
	}


	bool SchemaItemAfb::setAfbElementParams(Afb::AfbElement* afbElement) const
	{
		if (afbElement == nullptr)
		{
			assert(afbElement);
			return false;
		}

		for (Afb::AfbParam& param : afbElement->params())
		{
			if (param.user() == false)
			{
				continue;
			}

			QVariant propValue = propertyValue(param.caption());

			if (propValue.isValid() == false)
			{
				// Was not found
				//
				assert(propValue.isValid() == true);
				return false;
			}

			param.setValue(propValue);
		}

		return true;
	}

	bool SchemaItemUfb::updateElement(const UfbSchema* ufbSchema, QString* errorMessage)
	{
		if (errorMessage == nullptr)
		{
			assert(errorMessage);
			return false;
		}

		if (m_afbElement.strID() != sourceAfb.strID())
		{
			assert(m_afbElement.strID() == sourceAfb.strID());
			return false;
		}

		// Update params, m_afbElement contains old parameters
		//
		std::vector<Afb::AfbParam> newParams = sourceAfb.params();
		const std::vector<Afb::AfbParam>& currentParams = m_afbElement.params();

		for (Afb::AfbParam& p : newParams)
		{
			if (p.user() == false)
			{
				continue;
			}

			auto foundExistingParam = std::find_if(currentParams.begin(), currentParams.end(),
					[&p](const Afb::AfbParam& mp)
					{
						return p.caption() == mp.caption();		// Don't use opIndex, it can be same (-1)
					});

			if (foundExistingParam != currentParams.end())
			{
				// Try to set old value to the param
				//
				const Afb::AfbParam& currentParam = *foundExistingParam;

				if (p.value().type() == currentParam.value().type())
				{
					p.setValue(currentParam.value());

					//qDebug() << "Param: " << currentParam.caption() << ", value: " << p.value();
				}
			}
		}

		// Update description
		//
		m_afbElement = sourceAfb;

		std::swap(params(), newParams);		// The prev assignemnt (m_afbElement = sourceAfb) just reseted all paramas
											// Set them to the actual values

		// Update in/out pins
		//
		removeAllInputs();
		removeAllOutputs();

		const std::vector<Afb::AfbSignal>& inputSignals = m_afbElement.inputSignals();
		for (const Afb::AfbSignal& s : inputSignals)
		{
			addInput(s);
		}

		const std::vector<Afb::AfbSignal>& outputSignals = m_afbElement.outputSignals();
		for (const Afb::AfbSignal& s : outputSignals)
		{
			addOutput(s);
		}

		// Run afterCreationScript, lets assume we create allmost new itesm, as we deleted all inputs/outs and updated params
		//
		QString afterCreationScript = m_afbElement.afterCreationScript();

		if (afterCreationScript.isEmpty() == false)
		{
			executeScript(afterCreationScript, m_afbElement, errorMessage);
		}

		// Here is remove all props and add new from m_params
		//
		addSpecificParamProperties();

		return true;
	}

	double SchemaItemAfb::minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const
	{
		// Cache values
		//
		m_cachedGridSize = gridSize;
		m_cachedPinGridStep = pinGridStep;

		// --
		//
		int pinCount = std::max(inputsCount(), outputsCount());
		if (pinCount == 0)
		{
			pinCount = 1;
		}

		// 1 string for caption
		// N param count
		//
		int textLineCount = 1;
		for (const Afb::AfbParam& p : m_afbElement.params())
		{
			if (p.visible() == true)
			{
				textLineCount ++;
			}
		}

		double pinVertGap =	CUtils::snapToGrid(gridSize * static_cast<double>(pinGridStep), gridSize);

		double minPinHeight = CUtils::snapToGrid(pinVertGap * static_cast<double>(pinCount), gridSize);
		double minTextHeight = CUtils::snapToGrid(m_font.drawSize() * static_cast<double>(textLineCount), gridSize);

		if (minTextHeight < minPinHeight)
		{
			return minPinHeight;
		}

		return minTextHeight;
	}

	double SchemaItemAfb::minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const
	{
		// Cache values
		//
		m_cachedGridSize = gridSize;
		m_cachedPinGridStep = pinGridStep;

		return m_cachedGridSize * 16;

//		std::shared_ptr<Afb::AfbElement> afb = schema->afbCollection().get(afbStrID());
//		if (afb.get() == nullptr)
//		{
//			// Such AfbItem was not found
//			//
//			assert(afb.get() != nullptr);
//			return m_cachedGridSize * 16;
//			return;
//		}

//		QFont font(m_font.name(), m_font.drawSize());
//		QFontMetricsF fm(font);

//		double minTextWide = fm.width(afb->caption());

//		double minWidth = CUtils::snapToGrid(minTextWide, gridSize);

//		minWidth = std::max(minWidth, m_cachedGridSize * 16);

//		// --
//		//
//		return m_cachedGridSize * 16;
	}

	void SchemaItemAfb::addSpecificParamProperties()
	{
		// Clear all dynamic properties
		//
		removeSpecificProperties();

		// Set new Param Propereties
		//
		for (Afb::AfbParam& p : m_afbElement.params())
		{
			if (p.user() == false)
			{
				continue;
			}

			QVariant value = p.value();

			auto prop = this->addProperty(p.caption(), PropertyNames::parametersCategory, true);

			prop->setValue(value);
			prop->setReadOnly(false);
			prop->setSpecific(true);
			if (p.isAnalog())
			{
				prop->setLimits(p.lowLimit(), p.highLimit());
			}
			prop->setPrecision(precision());
		}
	}

	bool SchemaItemAfb::executeScript(const QString& script, const Afb::AfbElement& afb, QString* errorMessage)
	{
		if (errorMessage == nullptr)
		{
			assert(errorMessage);
			return false;
		}

		if (script.isEmpty() == true)
		{
			return true;
		}

		QString exeScript = afb.libraryScript() + script;

		exeScript.replace(QString("&lt;"), QString("<"));
		exeScript.replace(QString("&gt;"), QString(">"));
		exeScript.replace(QString("&amp;"), QString("&"));

		QJSEngine jsEngine;

		Afb::AfbElement jsAfb = afb;

		QJSValue jsElement = jsEngine.newQObject(this);
		QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

		QJSValue jsAfbElement = jsEngine.newQObject(&jsAfb);
		QQmlEngine::setObjectOwnership(&jsAfb, QQmlEngine::CppOwnership);

		// Run script
		//
		QJSValue jsEval = jsEngine.evaluate(exeScript);
		if (jsEval.isError() == true)
		{
			*errorMessage += tr("Script evaluation failed in AFB element '%1': %2\n").arg(afb.caption()).arg(jsEval.toString());
			return false;
		}

		QJSValueList args;

		args << jsElement;
		args << jsAfbElement;

		QJSValue jsResult = jsEval.call(args);

		if (jsResult.isError() == true)
		{
			*errorMessage += tr("Script evaluation failed in AFB element '%1': %2\n").arg(afb.caption()).arg(jsResult.toString());
			return false;
		}

		return true;

	}

	int SchemaItemAfb::getParamIntValue(const QString& name)
	{
		QVariant result = getAfbParam(name);
		if (result.isNull() || result.isValid() == false)
		{
			return -1;
		}

		return result.toInt();
	}

	bool SchemaItemAfb::getParamBoolValue(const QString& name)
	{
		QVariant result = getAfbParam(name);
		if (result.isNull() || result.isValid() == false)
		{
			return false;
		}

		return result.toBool();
	}

	void SchemaItemAfb::addInputSignal(QString caption, int /*type*/, int opIndex, int /*size*/)
	{
		addInput(opIndex, caption);
	}

	void SchemaItemAfb::addOutputSignal(QString caption, int /*type*/, int opIndex, int /*size*/)
	{
		addOutput(opIndex, caption);
	}

	void SchemaItemAfb::removeInputSignals()
	{
		removeAllInputs();
	}

	void SchemaItemAfb::removeOutputSignals()
	{
		removeAllOutputs();
	}

	const QString& SchemaItemAfb::afbStrID() const
	{
		return m_afbElement.strID();
	}

	const Afb::AfbElement& SchemaItemAfb::afbElement() const
	{
		return m_afbElement;
	}

	std::vector<Afb::AfbParam>& SchemaItemAfb::params()
	{
		return m_afbElement.params();
	}

	const std::vector<Afb::AfbParam>& SchemaItemAfb::params() const
	{
		return m_afbElement.params();
	}

	QString SchemaItemUfb::ufbSchemaId() const
	{
		return m_ufbSchemaId;
	}

	void SchemaItemUfb::setUfbSchemaId(const QString& value)
	{
		m_ufbSchemaId = value;
		return;
	}

	int SchemaItemUfb::ufbSchemaVersion() const
	{
		return m_version;
	}

	void SchemaItemUfb::setUfbSchemaVersion(int value)
	{
		m_version = value;
		return;
	}

	QString SchemaItemAfb::label() const
	{
		return m_label;
	}

	void SchemaItemAfb::setLabel(const QString& value)
	{
		m_label = value;
	}

}
