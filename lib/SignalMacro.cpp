#include "Signal.h"
#include "SignalMacro.h"


namespace Builder
{
	const QString SignalMacro::START_TOKEN("$(");
	const QString SignalMacro::END_TOKEN(")");

	const QString SignalMacro::BUS_SIGNAL_ID_SEPARATOR(".");

	const QString SignalMacro::BUS_TYPE("$(BusType)");
	const QString SignalMacro::BUS_APP_SIGNAL_ID("$(BusAppSignalID)");
	const QString SignalMacro::BUS_CUSTOM_APP_SIGNAL_ID("$(BusCustomAppSignalID)");
	const QString SignalMacro::BUS_CAPTION("$(BusCaption)");

	QString SignalMacro::expandDeviceSignalTemplate(const Hardware::DeviceObject& startDeviceObject,
								   const QString& templateStr,
								   QString* errMsg)
	{
		QString resultStr;

		int searchStartPos = 0;

		do
		{
			int macroStartPos = templateStr.indexOf(START_TOKEN, searchStartPos);

			if (macroStartPos == -1)
			{
				// no more macroses
				//
				resultStr += templateStr.mid(searchStartPos);
				break;
			}

			resultStr += templateStr.mid(searchStartPos, macroStartPos - searchStartPos);

			int macroEndPos = templateStr.indexOf(END_TOKEN, macroStartPos + 2);

			if (macroEndPos == -1)
			{
				*errMsg = QString("End of macro is not found in template %1 of device object %2. ").
							arg(templateStr).arg(startDeviceObject.equipmentIdTemplate());
				return QString();
			}

			QString macroStr = templateStr.mid(macroStartPos + 2, macroEndPos - (macroStartPos + 2));

			QString expandedMacroStr = expandDeviceObjectMacro(startDeviceObject, macroStr, errMsg);

			if (errMsg->isEmpty() == false)
			{
				return QString();
			}

			resultStr += expandedMacroStr;

			searchStartPos = macroEndPos + 1;
		}
		while(true);

		return resultStr;
	}

	QString SignalMacro::expandBusSignalCaptionTemplate(const Signal& busParentSignal, BusShared bus, const BusSignal& busSignal)
	{
		QString caption = busSignal.caption;

		caption.replace(SignalMacro::BUS_TYPE, bus->busTypeID());
		caption.replace(SignalMacro::BUS_APP_SIGNAL_ID, busParentSignal.appSignalID());
		caption.replace(SignalMacro::BUS_CUSTOM_APP_SIGNAL_ID, busParentSignal.customAppSignalID());
		caption.replace(SignalMacro::BUS_CAPTION, busParentSignal.caption());

		return caption;
	}

	QString SignalMacro::expandDeviceObjectMacro(const Hardware::DeviceObject& startDeviceObject,
								const QString& macroStr,
								QString* errMsg)
	{
		QStringList macroFields = macroStr.split(".");

		const Hardware::DeviceObject* deviceObject = nullptr;
		QString propertyCaption;

		switch(macroFields.count())
		{
		case 1:
			{
				// property only
				//
				deviceObject = &startDeviceObject;
				propertyCaption = macroFields.at(0);
			}
			break;

		case 2:
			{
				// parentObject.property
				//
				QString parentObjectType = macroFields.at(0);
				propertyCaption = macroFields.at(1);

				deviceObject = getParentDeviceObjectOfType(startDeviceObject, parentObjectType, errMsg);

				if (errMsg->isEmpty() == false)
				{
					return QString();
				}

				if (deviceObject == nullptr)
				{
					*errMsg = QString("Macro expand error! Parent device object of type '%1' is not found for device object %2").
									arg(parentObjectType).arg(startDeviceObject.equipmentIdTemplate());
					return QString();
				}

			}
			break;

		default:
			*errMsg = QString("Unknown format of macro %1 in template of device signal %2").
					arg(macroStr).arg(startDeviceObject.equipmentIdTemplate());
			return QString();
		}

		if (deviceObject->propertyExists(propertyCaption) == false)
		{
			*errMsg = QString("Device signal %1 macro expand error! Property '%2' is not found in device object %3.").
								arg(startDeviceObject.equipmentIdTemplate()).
								arg(propertyCaption).
								arg(deviceObject->equipmentIdTemplate());
			return QString();
		}

		QString propertyValue = deviceObject->propertyValue(propertyCaption).toString();

		return propertyValue;
	}

	const Hardware::DeviceObject* SignalMacro::getParentDeviceObjectOfType(const Hardware::DeviceObject& startObject,
																	  const QString& parentObjectType,
																	  QString* errMsg)
	{
		static const std::map<QString, Hardware::DeviceType> objectTypes {
				std::make_pair(QString("root"), Hardware::DeviceType::Root),
				std::make_pair(QString("system"), Hardware::DeviceType::System),
				std::make_pair(QString("rack"), Hardware::DeviceType::Rack),
				std::make_pair(QString("chassis"), Hardware::DeviceType::Chassis),
				std::make_pair(QString("module"), Hardware::DeviceType::Module),
				std::make_pair(QString("workstation"), Hardware::DeviceType::Workstation),
				std::make_pair(QString("software"), Hardware::DeviceType::Software),
				std::make_pair(QString("controller"), Hardware::DeviceType::Controller),
				std::make_pair(QString("signal"), Hardware::DeviceType::Signal),
		};

		std::map<QString, Hardware::DeviceType>::const_iterator it = objectTypes.find(parentObjectType.toLower());

		if (it == objectTypes.end())
		{
			*errMsg = QString("Unknown object type '%1' in call of getParentObjectOfType(...) for device object %2").
							arg(parentObjectType).arg(startObject.equipmentIdTemplate());
			return nullptr;
		}

		Hardware::DeviceType requestedDeviceType = it->second;

		const Hardware::DeviceObject* parent = &startObject;

		do
		{
			if (parent == nullptr)
			{
				break;
			}

			if (parent->deviceType() == requestedDeviceType)
			{
				return parent;
			}

			parent = parent->parent();
		}
		while(true);

		return nullptr;
	}
}
