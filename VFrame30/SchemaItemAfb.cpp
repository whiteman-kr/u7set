#include "SchemaItemAfb.h"
#include "Schema.h"
#include "PropertyNames.h"
#include "DrawParam.h"

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
		auto precisionProp = ADD_PROPERTY_GETTER_SETTER(int, PropertyNames::precision, true, SchemaItemAfb::precision, SchemaItemAfb::setPrecision);
		precisionProp->setCategory(PropertyNames::functionalCategory);
	}

	SchemaItemAfb::SchemaItemAfb(SchemaUnit unit, const Afb::AfbElement& fblElement, QString* errorMsg) :
		FblItemRect(unit),
		m_afbElement(fblElement)
	{
		assert(errorMsg);

		auto precisionProp = ADD_PROPERTY_GETTER_SETTER(int, PropertyNames::precision, true, SchemaItemAfb::precision, SchemaItemAfb::setPrecision);
		precisionProp->setCategory(PropertyNames::functionalCategory);

		// Create input output signals in VFrame30::FblEtem
		//
		updateAfbElement(fblElement, errorMsg);

		addSpecificParamProperties();

		return;
	}

	SchemaItemAfb::~SchemaItemAfb(void)
	{
	}

	void SchemaItemAfb::Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* pLayer) const
	{
		QPainter* p = drawParam->painter();

		int dpiX = drawParam->dpiX();
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

		r.setLeft(r.left() + m_font.drawSize() / 4.0);
		r.setRight(r.right() - m_font.drawSize() / 4.0);

		// Draw caption
		//
		QString text = m_afbElement.caption();

		p->setPen(textColor());
		DrawHelper::drawText(p, m_font, itemUnit(), text, r, Qt::AlignHCenter | Qt::AlignTop);

		// Draw params
		//
		text.clear();
		r.setTop(topDocPt() + m_font.drawSize() * 1.4);
		r.setHeight(heightDocPt() - m_font.drawSize() * 1.4);

		const std::vector<Afb::AfbParam>& params = m_afbElement.params();

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
				case E::SignalType::Analog:
					{
						QVariant a = propertyValue(param.caption());

						switch (param.dataFormat())
						{
							case E::DataFormat::UnsignedInt:
								paramValue = a.toString();
								break;

							case E::DataFormat::SignedInt:
								paramValue = a.toString();
								break;

							case E::DataFormat::Float:

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

				case E::SignalType::Discrete:
					{
						QVariant d = property(param.caption().toStdString().c_str());
						paramValue = d.toString();
					}
					break;
				default:
					assert(false);
			}

			// Param string LOWERCASED
			//
			QString paramStr = QString("%1: %2 %3")
							   .arg(param.caption())
							   .arg(paramValue)
							   .arg(param.units())
									.toLower();

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
		DrawHelper::drawText(p, smallFont, itemUnit(), text, r, Qt::AlignLeft | Qt::AlignBottom);

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

		vifble->set_precision(m_precision);
		m_afbElement.saveToXml(vifble->mutable_afbelement());

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
		
		m_precision = vifble.precision();

		if (vifble.has_deprecated_afbelement() == true)
		{
			QString errorMsg;
			bool ok = m_afbElement.deprecatedFormatLoad(vifble.deprecated_afbelement(), errorMsg);

			if (ok == false)
			{
				qDebug() << "SchemaItemAfb::LoadData: Parsing AFB element error: " << errorMsg;
				return false;
			}
		}
		else
		{
			assert(vifble.has_afbelement());

			QString errorMsg;
			bool ok = m_afbElement.loadFromXml(vifble.afbelement(), &errorMsg);

			if (ok == false)
			{
				qDebug() << "SchemaItemAfb::LoadData: Parsing AFB element error: " << errorMsg;
				return false;
			}
		}

		// Add afb properties to class meta object
		//
		addSpecificParamProperties();

		return true;
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

	Afb::AfbParam SchemaItemAfb::afbParam(const QString& name)
	{
		for (Afb::AfbParam& p : m_afbElement.params())
		{
			if (p.caption() == name)
			{
				return p;
			}
		}

		return Afb::AfbParam();
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

	bool SchemaItemAfb::updateAfbElement(const Afb::AfbElement& sourceAfb, QString* errorMessage)
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

	bool SchemaItemAfb::setParamVisible(const QString& name, bool visible)
	{
		for (Afb::AfbParam& p : m_afbElement.params())
		{
			if (p.caption() == name)
			{
				p.setVisible(visible);
				return true;
			}
		}
		return false;
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

	Afb::AfbElement& SchemaItemAfb::afbElement()
	{
		return m_afbElement;
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
		for (Afb::AfbParam& p : m_afbElement.params())
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
