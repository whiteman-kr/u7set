#pragma once

namespace AppSignalLists
{
	// Column names
	//
	static const QString col_CustomAppSignalId = QObject::tr("CustomAppSignalID");
	static const QString col_AppSignalId = QObject::tr("AppSignalID");
	static const QString col_EquipmentID = QObject::tr("EquipmentID");
	static const QString col_Caption = QObject::tr("Caption");
	static const QString col_Units = QObject::tr("Units");
	static const QString col_Type = QObject::tr("Type");
	static const QString col_LowLimit = QObject::tr("LowLimit");
	static const QString col_HighLimit = QObject::tr("HighLimit");
	static const QString col_Default = QObject::tr("Default");
	static const QString col_Value = QObject::tr("Value");

	// Property names
	//
	static const QLatin1String prop_Caption = QLatin1String("Caption");
	static const QLatin1String prop_SignalType = QLatin1String("SignalType");
	static const QLatin1String prop_ID = QLatin1String("ID");
	static const QLatin1String prop_Uuid = QLatin1String("Uuid");
	static const QLatin1String prop_Tags = QLatin1String("Tags");
	static const QLatin1String prop_CustomAppSignalMasks = QLatin1String("CustomAppSignalMasks");
	static const QLatin1String prop_AppSignalMasks = QLatin1String("AppSignalMasks");
	static const QLatin1String prop_EquipmentIDMasks = QLatin1String("EquipmentIDMasks");
	static const QLatin1String prop_AppSignalTags = QLatin1String("AppSignalTags");
} // namespace AppSignalLists