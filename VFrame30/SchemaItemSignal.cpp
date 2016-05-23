#include "Stable.h"
#include "SchemaItemSignal.h"
#include "LogicSchema.h"

#include "../include/AppSignalManager.h"

namespace VFrame30
{
	//
	// SchemaItemSignal
	//
	SchemaItemSignal::SchemaItemSignal(void) :
		SchemaItemSignal(SchemaUnit::Inch)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	SchemaItemSignal::SchemaItemSignal(SchemaUnit unit) :
		FblItemRect(unit)
	{
		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::appSignalIDs, PropertyNames::functionalCategory, true, SchemaItemSignal::appSignalIds, SchemaItemSignal::setAppSignalIds);
	}

	SchemaItemSignal::~SchemaItemSignal(void)
	{
	}

	void SchemaItemSignal::Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const
	{
		FblItemRect::Draw(drawParam, schema, layer);

		if (drawParam->isMonitorMode() == true)
		{
			if (drawParam->appSignalManager() ==  nullptr)
			{
				assert(drawParam->appSignalManager() != nullptr);
				return;
			}
		}

		//--
		//
		QPainter* p = drawParam->painter();

		QRectF r(leftDocPt(), topDocPt(), widthDocPt(), heightDocPt());

		if (std::abs(r.left() - r.right()) < 0.000001)
		{
			r.setRight(r.left() + 0.000001);
		}

		if (std::abs(r.bottom() - r.top()) < 0.000001)
		{
			r.setBottom(r.top() + 0.000001);
		}
		int dpiX = 96;
		QPaintDevice* paintDevice = p->device();
		if (paintDevice == nullptr)
		{
			assert(paintDevice);
			dpiX = 96;
		}
		else
		{
			dpiX = paintDevice->logicalDpiX();
		}

		double pinWidth = GetPinWidth(itemUnit(), dpiX);


		if (multiChannel() == true)
		{
			QPen pen(lineColor());
			pen.setWidthF(m_weight);		// Don't use getter!
			p->setPen(pen);

			if (inputsCount() > 0)
			{
				const std::list<AfbPin>& inputPins = inputs();
				assert(inputPins.empty() == false);

				p->drawLine(QPointF(r.left() + (pinWidth / 3.0) * 2.0, inputPins.front().y() - pinWidth / 4.0),
							QPointF(r.left() + (pinWidth / 3.0) * 1.0, inputPins.front().y() + pinWidth / 4.0));
			}

			if (outputsCount() > 0)
			{
				const std::list<AfbPin>& pins = outputs();
				assert(pins.empty() == false);

				p->drawLine(QPointF(r.right() - (pinWidth / 3.0) * 2.0, pins.front().y() + pinWidth / 4.0),
							QPointF(r.right() - (pinWidth / 3.0) * 1.0, pins.front().y() - pinWidth / 4.0));
			}
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

		// Draw Signals StrIDs
		//
		p->setPen(textColor());

		QString text;

		const VFrame30::LogicSchema* logicSchema = dynamic_cast<const VFrame30::LogicSchema*>(drawParam->schema());

		if (multiChannel() == true && logicSchema != nullptr && appSignalIds().size() >= 1)
		{
			text = appSignalIds();
		}
		else
		{
			text = appSignalIds();

			if (drawParam->isMonitorMode() == true)
			{
				AppSignalState signalState = drawParam->appSignalManager()->signalState(text);
				text += QString(" %1").arg(QString::number(signalState.value));
			}
		}

		DrawHelper::DrawText(p, m_font, itemUnit(), text, r, Qt::AlignLeft | Qt::AlignTop);

		return;
	}

	QString SchemaItemSignal::appSignalIds() const
	{
		QString result;

		for (QString s : m_appSignalIds)
		{
			s = s.trimmed();

			if (result.isEmpty() == false)
			{
				result.append(QChar::LineFeed);
			}

			result.append(s);
		}

		return result;
	}

	const QStringList& SchemaItemSignal::appSignalIdList() const
	{
		return m_appSignalIds;
	}

	void SchemaItemSignal::setAppSignalIds(const QString& s)
	{
		m_appSignalIds = s.split(QChar::LineFeed, QString::SkipEmptyParts);
	}

	QStringList* SchemaItemSignal::mutable_appSignalIds()
	{
		return &m_appSignalIds;
	}

	bool SchemaItemSignal::multiChannel() const
	{
		return m_appSignalIds.size() > 1;
	}

	bool SchemaItemSignal::SaveData(Proto::Envelope* message) const
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
		Proto::SchemaItemSignal* signal = message->mutable_schemaitem()->mutable_signal();

		for (const QString& strId : m_appSignalIds)
		{
			::Proto::wstring* ps = signal->add_appsignalids();
			Proto::Write(ps, strId);
		}

		return true;
	}

	bool SchemaItemSignal::LoadData(const Proto::Envelope& message)
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
		if (message.schemaitem().has_signal() == false)
		{
			assert(message.schemaitem().has_signal());
			return false;
		}

		const Proto::SchemaItemSignal& signal = message.schemaitem().signal();

		m_appSignalIds.clear();
		m_appSignalIds.reserve(signal.appsignalids_size());

		for (int i = 0; i < signal.appsignalids_size(); i++)
		{
			QString s;
			Proto::Read(signal.appsignalids().Get(i), &s);
			m_appSignalIds.push_back(s);
		}

		return true;
	}


	//
	// CSchemaItemInputSignal
	//
	SchemaItemInput::SchemaItemInput(void)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	SchemaItemInput::SchemaItemInput(SchemaUnit unit) :
		SchemaItemSignal(unit)
	{
		addOutput();
		setAppSignalIds("#IN_STRID");
	}

	SchemaItemInput::~SchemaItemInput(void)
	{
		assert(outputsCount() == 1);
	}

	QString SchemaItemInput::buildName() const
	{
		return QString("Input (%1)").arg(appSignalIds());
	}

	// Serialization
	//
	bool SchemaItemInput::SaveData(Proto::Envelope* message) const
	{
		bool result = SchemaItemSignal::SaveData(message);
		
		if (result == false || message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}

		// --
		//
		/*Proto::VideoItemInputSignal* inputSignal = */message->mutable_schemaitem()->mutable_inputsignal();

		//inputSignal->set_weight(weight);

		return true;
	}

	bool SchemaItemInput::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemaitem() == false)
		{
			assert(message.has_schemaitem());
			return false;
		}

		// --
		//
		bool result = SchemaItemSignal::LoadData(message);
		if (result == false)
		{
			return false;
		}
		
		// --
		//
		if (message.schemaitem().has_inputsignal() == false)
		{
			assert(message.schemaitem().has_inputsignal());
			return false;
		}

		/*const Proto::VideoItemInputSignal& inputSignal = */message.schemaitem().inputsignal();
		//fill = inputSignal.fill();

		return true;
	}
	
	//
	// CSchemaItemOutputSignal
	//
	SchemaItemOutput::SchemaItemOutput(void)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	SchemaItemOutput::SchemaItemOutput(SchemaUnit unit) :
		SchemaItemSignal(unit)
	{
		addInput();
		setAppSignalIds("#OUT_STRID");
	}

	SchemaItemOutput::~SchemaItemOutput(void)
	{
		assert(inputsCount() == 1);
	}

	QString SchemaItemOutput::buildName() const
	{
		return QString("Output (%1)").arg(appSignalIds());
	}

	// Serialization
	//
	bool SchemaItemOutput::SaveData(Proto::Envelope* message) const
	{
		bool result = SchemaItemSignal::SaveData(message);
		
		if (result == false || message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}

		// --
		//
		/*Proto::VideoItemOutputSignal* outputSignal = */message->mutable_schemaitem()->mutable_outputsignal();

		//inputSignal->set_weight(weight);

		return true;
	}

	bool SchemaItemOutput::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemaitem() == false)
		{
			assert(message.has_schemaitem());
			return false;
		}

		// --
		//
		bool result = SchemaItemSignal::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemaitem().has_outputsignal() == false)
		{
			assert(message.schemaitem().has_outputsignal());
			return false;
		}

		/*const Proto::VideoItemOutputSignal& outputSignal = */message.schemaitem().outputsignal();
		//fill = inputSignal.fill();

		return true;
	}

}

