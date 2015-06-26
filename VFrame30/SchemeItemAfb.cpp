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

	SchemeItemAfb::SchemeItemAfb(SchemeUnit unit, const Afbl::AfbElement& fblElement) :
		FblItemRect(unit),
		m_afbStrID(fblElement.strID()),
		m_params(fblElement.params())
	{
		// Создать входные и выходные сигналы в VFrame30::FblEtem
		//
		const std::vector<Afbl::AfbSignal>& inputSignals = fblElement.inputSignals();
		for (const Afbl::AfbSignal& s : inputSignals)
		{
			addInput(s);
		}

		const std::vector<Afbl::AfbSignal>& outputSignals = fblElement.outputSignals();
		for (const Afbl::AfbSignal& s : outputSignals)
		{
			addOutput(s);
		}

		// Проинициализировать паремтры значением по умолчанию and add Afb properties to class meta object
		//
		for (Afbl::AfbParam& p : m_params)
		{
			p.setValue(p.defaultValue());
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
		std::shared_ptr<Afbl::AfbElement> afb = scheme->afbCollection().get(afbStrID());
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

		const std::vector<Afbl::AfbParam>& params = afb->params();

		for (size_t i = 0; i < params.size(); i++)
		{
			const Afbl::AfbParam& param = params[i];

			if (param.visible() == false)
			{
				continue;
			}

			QString paramStr = QString("%1 = %2")
				.arg(param.caption())
				.arg(property(param.caption().toStdString().c_str()).toString());

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

		for (const Afbl::AfbParam& p : m_params)
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
			Afbl::AfbParam p;
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

		auto found = std::find_if(m_params.begin(), m_params.end(), [&name](const Afbl::AfbParam& p)
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
			std::shared_ptr<Afbl::AfbElement> afb = scheme->afbCollection().get(afbStrID());
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

	bool SchemeItemAfb::setAfbElementParams(Afbl::AfbElement* afbElement) const
	{
		if (afbElement == nullptr)
		{
			assert(afbElement);
			return false;
		}

		for (Afbl::AfbParam& param : afbElement->params())
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
		for (Afbl::AfbParam& p : m_params)
		{
			if (p.user() == false)
			{
				continue;
			}

			QVariant value = p.value();
			setProperty(p.caption().toStdString().c_str(), value);
		}
	}

	bool SchemeItemAfb::executeScript(const QString& script, const Afbl::AfbElement& afb)
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

		Afbl::AfbElement jsAfb = afb;

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
		for (Afbl::AfbParam& p : m_params)
		{
			if (p.caption() == name)
			{
				if (p.type() == Afbl::AnalogIntegral && p.value().isValid() == true)
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

	const std::vector<Afbl::AfbParam>& SchemeItemAfb::params() const
	{
		return m_params;
	}
}
