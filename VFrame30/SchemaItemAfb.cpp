#include "Stable.h"
#include "SchemaItemAfb.h"
#include "Schema.h"

namespace VFrame30
{
	SchemaItemAfb::SchemaItemAfb(void) :
		SchemaItemAfb(SchemaUnit::Inch)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	SchemaItemAfb::SchemaItemAfb(SchemaUnit unit) :
		FblItemRect(unit)
	{
		ADD_PROPERTY_GETTER_SETTER(int, Precision, true, SchemaItemAfb::precision, SchemaItemAfb::setPrecision);
	}

	SchemaItemAfb::SchemaItemAfb(SchemaUnit unit, const Afb::AfbElement& fblElement) :
		FblItemRect(unit),
		m_afbStrID(fblElement.strID()),
        m_afbImplementationVersion(fblElement.implementationVersion()),
        m_afbImplementationOpIndex(fblElement.implementationOpIndex()),
		m_params(fblElement.params())
	{
		ADD_PROPERTY_GETTER_SETTER(int, Precision, true, SchemaItemAfb::precision, SchemaItemAfb::setPrecision);

		// Create input output signals in VFrame30::FblEtem
		//
		const std::vector<Afb::AfbSignal>& inputSignals = fblElement.inputSignals();
		for (const Afb::AfbSignal& s : inputSignals)
		{
			addInput(s);
		}

		const std::vector<Afb::AfbSignal>& outputSignals = fblElement.outputSignals();
		for (const Afb::AfbSignal& s : outputSignals)
		{
			addOutput(s);
		}

		addSpecificParamProperties();

		QString afterCreationScript = fblElement.afterCreationScript();
		if (afterCreationScript.isEmpty() == false)
		{
			executeScript(afterCreationScript, fblElement);
		}

	}

	SchemaItemAfb::~SchemaItemAfb(void)
	{
	}

	void SchemaItemAfb::Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* pLayer) const
	{
		std::shared_ptr<Afb::AfbElement> afb = schema->afbCollection().get(afbStrID());
		if (afb.get() == nullptr)
		{
			// Such AfbItem was not found
			//
			assert(afb.get() != nullptr);
			return;
		}

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

		r.setLeft(r.left() + m_font.drawSize() / 4.0);
		r.setRight(r.right() - m_font.drawSize() / 4.0);

		// Draw caption
		//
		QString text = afb->caption();

		p->setPen(textColor());
		DrawHelper::DrawText(p, m_font, itemUnit(), text, r, Qt::AlignHCenter | Qt::AlignTop);

		// Draw params
		//
		text.clear();
		r.setTop(topDocPt() + m_font.drawSize() * 1.4);
		r.setHeight(heightDocPt() - m_font.drawSize() * 1.4);

		const std::vector<Afb::AfbParam>& params = afb->params();

		for (size_t i = 0; i < params.size(); i++)
		{
			const Afb::AfbParam& param = params[i];

			if (param.visible() == false)
			{
				continue;
			}

			QString paramValue;
			switch (param.type())
			{
				case Afb::AfbSignalType::Analog:
					{
						QVariant a = propertyValue(param.caption());

						switch (param.dataFormat())
						{
							case Afb::AfbDataFormat::UnsignedInt:
								paramValue = a.toString();
								break;

							case Afb::AfbDataFormat::SignedInt:
								paramValue = a.toString();
								break;

							case Afb::AfbDataFormat::Float:

								paramValue.setNum(a.toDouble(), 'f', precision());

								while(paramValue.endsWith('0'))
								{
									paramValue.chop(1);
								}

								if (paramValue.endsWith('.'))
								{
									paramValue.chop(1);
								}
								break;

							default:
								assert(false);
						}
					}
					break;

				case Afb::AfbSignalType::Discrete:
					{
						QVariant d = property(param.caption().toStdString().c_str());
						paramValue = d.toString();
					}
					break;
				default:
					assert(false);
			}

			QString paramStr = QString("%1: %2")
				.arg(param.caption())
				.arg(paramValue);

			if (text.isEmpty() == true)
			{
				text = paramStr;
			}
			else
			{
				text.append(QString("\n%1").arg(paramStr));
			}
		}

		p->setPen(textColor());
		DrawHelper::DrawText(p, m_font, itemUnit(), text, r, Qt::AlignLeft | Qt::AlignBottom);

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
	bool SchemaItemAfb::SaveData(Proto::Envelope* message) const
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
		Proto::SchemaItemAfb* vifble = message->mutable_schemaitem()->mutable_afb();

		Proto::Write(vifble->mutable_afbstrid(), m_afbStrID);

		for (const Afb::AfbParam& p : m_params)
		{
			::Proto::AfbParam* protoParam = vifble->mutable_params()->Add();
			p.SaveData(protoParam);
		}

		vifble->set_precision(m_precision);

		return true;
	}

	bool SchemaItemAfb::LoadData(const Proto::Envelope& message)
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
		if (message.schemaitem().has_afb() == false)
		{
			assert(message.schemaitem().has_afb());
			return false;
		}
		
		const Proto::SchemaItemAfb& vifble = message.schemaitem().afb();
		
		Proto::Read(vifble.afbstrid(), &m_afbStrID);

		m_params.clear();
		m_params.reserve(vifble.params_size());

		for (int i = 0; i < vifble.params_size(); i++)
		{
			Afb::AfbParam p;
			p.LoadData(vifble.params(i));

			m_params.push_back(p);
		}

		m_precision = vifble.precision();

		// Add afb properties to class meta object
		//
		addSpecificParamProperties();

		return true;
	}

	QString SchemaItemAfb::buildName() const
	{
		return QString("AFB (%1)").arg(afbStrID());
	}

	bool SchemaItemAfb::setAfbParam(const QString& name, QVariant value, std::shared_ptr<Schema> schema)
	{
		if (name.isEmpty() == true || schema == nullptr)
		{
			assert(name.isEmpty() != true);
			assert(schema != nullptr);
			return false;
		}

		auto found = std::find_if(m_params.begin(), m_params.end(), [&name](const Afb::AfbParam& p)
			{
				return p.caption() == name;
			});

		if (found == m_params.end())
		{
			assert(found != m_params.end());
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
			std::shared_ptr<Afb::AfbElement> afb = schema->afbCollection().get(afbStrID());
			if (afb == nullptr)
			{
				assert(afb != nullptr);
				return false;
			}

			QString changedScript = found->changedScript();
			if (changedScript.isEmpty() == false)
			{
				executeScript(changedScript, *afb);
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

		auto found = std::find_if(m_params.begin(), m_params.end(), [&opName](const Afb::AfbParam& p)
			{
				return p.opName() == opName;
			});

		if (found == m_params.end())
		{
			assert(found != m_params.end());
			return false;
		}

		found->setValue(value);

		return true;
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
		for (const Afb::AfbParam& p : m_params)
		{
			if (p.visible() == true)
			{
				textLineCount ++;
			}
		}

		double pinVertGap =	CUtils::snapToGrid(gridSize * static_cast<double>(pinGridStep), gridSize);

		textLineCount = std::max(pinCount, textLineCount);
		double minHeight = CUtils::snapToGrid(pinVertGap * static_cast<double>(textLineCount), gridSize);

		return minHeight;
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
		for (Afb::AfbParam& p : m_params)
		{
			if (p.user() == false)
			{
				continue;
			}

			QVariant value = p.value();

			auto prop = this->addProperty<QVariant>(p.caption(),true);

			prop->setValue(value);
			prop->setReadOnly(false);
			prop->setSpecific(true);
			prop->setCategory(tr("Parameters"));
			prop->setLimits(p.lowLimit(), p.highLimit());
			prop->setPrecision(precision());
		}
	}

	bool SchemaItemAfb::executeScript(const QString& script, const Afb::AfbElement& afb)
	{
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
			qDebug() << tr("Script evaluation failed: %1").arg(jsEval.toString());
			assert(false);
			return false;
		}

		QJSValueList args;

		args << jsElement;
		args << jsAfbElement;

		QJSValue jsResult = jsEval.call(args);

		if (jsResult.isError() == true)
		{
			qDebug() << tr("Script execution failed: %1").arg(jsResult.toString());
			assert(false);
			return false;
		}

		return true;

	}

	int SchemaItemAfb::getParamIntValue(const QString& name)
	{
		for (Afb::AfbParam& p : m_params)
		{
			if (p.caption() == name)
			{
                if (p.isAnalog() && (p.dataFormat() ==Afb::AfbDataFormat::SignedInt || p.dataFormat() == Afb::AfbDataFormat::UnsignedInt) && p.value().isValid() == true)
				{
					return p.value().toInt();
				}
				else
				{
					qDebug()<<"ERROR: SchemaItemAfb::getParamIntValue, parameter "<<name<<" is not integer or is not valid!";
					assert(false);
					return -1;
				}
			}
		}
		return -1;
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
		return m_afbStrID;
	}

    int SchemaItemAfb::afbImplementationVersion() const
    {
        return m_afbImplementationVersion;
    }

    int SchemaItemAfb::afbImplementationOpIndex() const
    {
        return m_afbImplementationOpIndex;
    }


	const std::vector<Afb::AfbParam>& SchemaItemAfb::params() const
	{
		return m_params;
	}

	int SchemaItemAfb::precision() const
	{
		return m_precision;
	}

	void SchemaItemAfb::setPrecision(int value)
	{
		if (value < 0)
		{
			value = 0;
		}

		if (value > 24)
		{
			value = 24;
		}

		m_precision = value;

		// Set new precsion to all dynamic properties
		//
		for (Afb::AfbParam& p : m_params)
		{
			if (p.user() == false)
			{
				continue;
			}

			auto prop = this->propertyByCaption(p.caption());

			if (prop.get() != nullptr && prop->specific() == true)
			{
				prop->setPrecision(m_precision);
			}
		}

		return;
	}
}
