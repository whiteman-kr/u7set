#include "SchemaItemImageValue.h"
#include "SchemaView.h"
#include "MacrosExpander.h"
#include "PropertyNames.h"
#include "DrawParam.h"
#include "TuningController.h"
#include "AppSignalController.h"
#include "../lib/AppSignal.h"
#include "../lib/Tuning/TuningSignalState.h"


namespace VFrame30
{
	SchemaItemImageValue::SchemaItemImageValue(void) :
		SchemaItemImageValue(SchemaUnit::Inch)
	{
		// This constructor can called while serialization
		//
	}

	SchemaItemImageValue::SchemaItemImageValue(SchemaUnit unit)
	{
		Property* p = nullptr;

		// Functional
		//
		auto strIdProperty = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::appSignalIDs, PropertyNames::functionalCategory, true, SchemaItemImageValue::signalIdsString, SchemaItemImageValue::setSignalIdsString);
		strIdProperty->setValidator(PropertyNames::appSignalIDsValidator);

		ADD_PROPERTY_GET_SET_CAT(E::SignalSource, PropertyNames::signalSource, PropertyNames::functionalCategory, true, SchemaItemImageValue::signalSource, SchemaItemImageValue::setSignalSource);

		// Appearance
		//
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::keepAspectRatio, PropertyNames::appearanceCategory, true, SchemaItemImageValue::keepAspectRatio, SchemaItemImageValue::setKeepAspectRatio);

		// --
		//
		m_static = false;
		setItemUnit(unit);
	}

	SchemaItemImageValue::~SchemaItemImageValue(void)
	{
	}

	// Serialization
	//
	bool SchemaItemImageValue::SaveData(Proto::Envelope* message) const
	{
		bool result = PosRectImpl::SaveData(message);
		if (result == false ||
			message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}
		
		// --
		//
		Proto::SchemaItemImageValue* valueMessage = message->mutable_schemaitem()->mutable_imagevalue();

		valueMessage->set_signalids(signalIdsString().toStdString());
		valueMessage->set_signalsource(static_cast<int32_t>(m_signalSource));
		valueMessage->set_keepaspectratio(m_keepAspectRatio);

		return true;
	}

	bool SchemaItemImageValue::LoadData(const Proto::Envelope& message)
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
		if (message.schemaitem().has_imagevalue() == false)
		{
			assert(message.schemaitem().has_imagevalue());
			return false;
		}

		const Proto::SchemaItemImageValue& valueMessage = message.schemaitem().imagevalue();

		setSignalIdsString(valueMessage.signalids().data());
		m_signalSource = static_cast<E::SignalSource>(valueMessage.signalsource());
		m_keepAspectRatio = valueMessage.keepaspectratio();

		return true;
	}

	// Drawing Functions
	//
	void SchemaItemImageValue::Draw(CDrawParam* drawParam, const Schema* /*schema*/, const SchemaLayer* /*layer*/) const
	{
		QPainter* p = drawParam->painter();

		// Initialization drawing resources
		//
		initDrawingResources();
						
		// Calculate rectangle
		//
		QRectF r = boundingRectInDocPt();

		// Drawing frame rect
		//

		return;
	}

	void SchemaItemImageValue::initDrawingResources() const
	{
		return;
	}

	bool SchemaItemImageValue::getSignalState(CDrawParam* drawParam, AppSignalParam* signalParam, AppSignalState* appSignalState, TuningSignalState* tuningSignalState) const
	{
		if (drawParam == nullptr ||
			signalParam == nullptr ||
			appSignalState == nullptr ||
			tuningSignalState == nullptr)
		{
			assert(drawParam);
			assert(signalParam);
			assert(appSignalState);
			assert(tuningSignalState);
			return false;
		}

		if (drawParam->isMonitorMode() == false)
		{
			assert(drawParam->isMonitorMode());
			return false;
		}

		bool ok = false;

		switch (signalSource())
		{
		case E::SignalSource::AppDataService:
			if (drawParam->appSignalController() == nullptr)
			{
			}
			else
			{
				*signalParam = drawParam->appSignalController()->signalParam(signalParam->appSignalId(), &ok);
				*appSignalState = drawParam->appSignalController()->signalState(signalParam->appSignalId(), nullptr);
			}
			break;

		case E::SignalSource::TuningService:
			if (drawParam->tuningController() == nullptr)
			{
			}
			else
			{
				*signalParam = drawParam->tuningController()->signalParam(signalParam->appSignalId(), &ok);
				*tuningSignalState = drawParam->tuningController()->signalState(signalParam->appSignalId(), nullptr);

				appSignalState->m_hash = signalParam->hash();
				appSignalState->m_flags.valid = tuningSignalState->valid();
				appSignalState->m_value = tuningSignalState->value().toDouble();
			}
			break;

		default:
			assert(false);
			ok = false;
		}

		return ok;
	}

	double SchemaItemImageValue::minimumPossibleHeightDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
	}

	double SchemaItemImageValue::minimumPossibleWidthDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
	}

	QString SchemaItemImageValue::signalIdsString() const
	{
		return m_signalIds.join(QChar::LineFeed);
	}

	void SchemaItemImageValue::setSignalIdsString(const QString& value)
	{
		m_signalIds = value.split(QRegExp(PropertyNames::appSignalId), QString::SkipEmptyParts);
	}

	const QStringList& SchemaItemImageValue::signalIds() const
	{
		return m_signalIds;
	}

	void SchemaItemImageValue::setSignalIds(const QStringList& value)
	{
		m_signalIds = value;
	}

	E::SignalSource SchemaItemImageValue::signalSource() const
	{
		return m_signalSource;
	}

	void SchemaItemImageValue::setSignalSource(E::SignalSource value)
	{
		m_signalSource = value;
	}

	// KeepAspectRatio
	//
	bool SchemaItemImageValue::keepAspectRatio() const
	{
		return m_keepAspectRatio;
	}

	void SchemaItemImageValue::setKeepAspectRatio(bool value)
	{
		m_keepAspectRatio = value;
	}

}

