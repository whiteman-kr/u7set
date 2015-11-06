#include "Stable.h"
#include "SchemeItemAfb.h"
#include "Scheme.h"

namespace VFrame30
{
	SchemeItemAfb::SchemeItemAfb(void)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	SchemeItemAfb::SchemeItemAfb(SchemeUnit unit) :
		FblItemRect(unit)
	{
	}

	SchemeItemAfb::SchemeItemAfb(SchemeUnit unit, const Afb::AfbElement& fblElement) :
		FblItemRect(unit),
		m_afbStrID(fblElement.strID()),
		m_params(fblElement.params())
	{
		// Создать входные и выходные сигналы в VFrame30::FblEtem
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

		addQtDynamicParamProperties();

		QString afterCreationScript = fblElement.afterCreationScript();
		if (afterCreationScript.isEmpty() == false)
		{
			executeScript(afterCreationScript, fblElement);
		}

	}

	SchemeItemAfb::~SchemeItemAfb(void)
	{
	}

	void SchemeItemAfb::Draw(CDrawParam* drawParam, const Scheme* scheme, const SchemeLayer* pLayer) const
	{
		std::shared_ptr<Afb::AfbElement> afb = scheme->afbCollection().get(afbStrID());
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
		FblItemRect::Draw(drawParam, scheme, pLayer);

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
						QVariant a = property(param.caption().toStdString().c_str());

						switch (param.dataFormat())
						{
							case Afb::AfbDataFormat::UnsignedInt:
								paramValue = a.toString();
								break;
							case Afb::AfbDataFormat::SignedInt:
								paramValue = a.toString();
								break;
							case Afb::AfbDataFormat::Float:
								paramValue.setNum(a.toDouble(), 'f', 3);	// There is no Precision field in Afb::AfbParam
								while(paramValue.endsWith('0'))
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
	bool SchemeItemAfb::SaveData(Proto::Envelope* message) const
	{
		bool result = FblItemRect::SaveData(message);
		if (result == false || message->has_schemeitem() == false)
		{
			assert(result);
			assert(message->has_schemeitem());
			return false;
		}
	
		// --
		//
		Proto::SchemeItemAfb* vifble = message->mutable_schemeitem()->mutable_afb();

		Proto::Write(vifble->mutable_afbstrid(), m_afbStrID);

		for (const Afb::AfbParam& p : m_params)
		{
			::Proto::AfbParam* protoParam = vifble->mutable_params()->Add();
			p.SaveData(protoParam);
		}

		return true;
	}

	bool SchemeItemAfb::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemeitem() == false)
		{
			assert(message.has_schemeitem());
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
		if (message.schemeitem().has_afb() == false)
		{
			assert(message.schemeitem().has_afb());
			return false;
		}
		
		const Proto::SchemeItemAfb& vifble = message.schemeitem().afb();
		
		Proto::Read(vifble.afbstrid(), &m_afbStrID);

		m_params.clear();
		m_params.reserve(vifble.params_size());

		for (int i = 0; i < vifble.params_size(); i++)
		{
			Afb::AfbParam p;
			p.LoadData(vifble.params(i));

			m_params.push_back(p);
		}

		// Add afb properties to class meta object
		//
		addQtDynamicParamProperties();

		return true;
	}

	QString SchemeItemAfb::buildName() const
	{
		return QString("AFB (%1)").arg(afbStrID());
	}

	bool SchemeItemAfb::setAfbParam(const QString& name, QVariant value, std::shared_ptr<Scheme> scheme)
	{
		if (name.isEmpty() == true || scheme == nullptr)
		{
			assert(name.isEmpty() != true);
			assert(scheme != nullptr);
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
			std::shared_ptr<Afb::AfbElement> afb = scheme->afbCollection().get(afbStrID());
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

	bool SchemeItemAfb::setAfbParamByOpName(const QString& opName, QVariant value)
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


	bool SchemeItemAfb::setAfbElementParams(Afb::AfbElement* afbElement) const
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

			QVariant propValue = property(param.caption().toStdString().c_str());

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

	double SchemeItemAfb::minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const
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

	double SchemeItemAfb::minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const
	{
		// Cache values
		//
		m_cachedGridSize = gridSize;
		m_cachedPinGridStep = pinGridStep;

		return m_cachedGridSize * 16;

//		std::shared_ptr<Afb::AfbElement> afb = scheme->afbCollection().get(afbStrID());
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

	void SchemeItemAfb::addQtDynamicParamProperties()
	{
		// Clear all dynamic properties
		//
		QList<QByteArray> dynamicProperties = dynamicPropertyNames();
		for (QByteArray& p : dynamicProperties)
		{
			QString name(p);
			setProperty(name.toStdString().c_str(), QVariant());		// Delete property by setting invalid QVariant()
		}

		// Set new Param Propereties
		//
		for (Afb::AfbParam& p : m_params)
		{
			if (p.user() == false)
			{
				continue;
			}

			QVariant value = p.value();
			setProperty(p.caption().toStdString().c_str(), value);
		}
	}

	bool SchemeItemAfb::executeScript(const QString& script, const Afb::AfbElement& afb)
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
			qDebug()<<tr("Script evaluation failed: %1").arg(jsEval.toString());
			assert(false);
			return false;
		}

		QJSValueList args;

		args << jsElement;
		args << jsAfbElement;

		QJSValue jsResult = jsEval.call(args);

		if (jsResult.isError() == true)
		{
			qDebug()<<tr("Script execution failed: %1").arg(jsResult.toString());
			assert(false);
			return false;
		}

		return true;

	}

	int SchemeItemAfb::getParamIntValue(const QString& name)
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
					qDebug()<<"ERROR: SchemeItemAfb::getParamIntValue, parameter "<<name<<" is not integer or is not valid!";
					assert(false);
					return -1;
				}
			}
		}
		return -1;
	}

	void SchemeItemAfb::addInputSignal(QString caption, int /*type*/, int opIndex, int /*size*/)
	{
		addInput(opIndex, caption);
	}

	void SchemeItemAfb::addOutputSignal(QString caption, int /*type*/, int opIndex, int /*size*/)
	{
		addOutput(opIndex, caption);
	}

	void SchemeItemAfb::removeInputSignals()
	{
		removeAllInputs();
	}

	void SchemeItemAfb::removeOutputSignals()
	{
		removeAllOutputs();
	}

	const QString& SchemeItemAfb::afbStrID() const
	{
		return m_afbStrID;
	}

	const std::vector<Afb::AfbParam>& SchemeItemAfb::params() const
	{
		return m_params;
	}
}
