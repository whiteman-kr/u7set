#pragma once

#include "Types.h"

struct LanControllerInfo
{
	QString equipmentID;
	int controllerNo = -1;				// == place
	E::LanControllerType lanControllerType = E::LanControllerType::Unknown;


	// only for adapterType == E::LanControllerType::Tuning
	//
	bool tuningProvided = false;
	bool tuningEnable = true;
	QString tuningIP;
	int tuningPort = 0;
	QString tuningServiceID;

	// only for adapterType == E::LanControllerType::AppData or E::LanControllerType::AppAndDiagData
	//
	bool appDataProvided = false;
	bool appDataEnable = true;
	QString appDataIP;
	int appDataPort = 0;
	QString appDataServiceID;
	quint32 appDataUID = 0;
	int appDataSize = 0;
	int appDataFramesQuantity = 0;

	// only for adapterType == E::LanControllerType::DiagData or E::LanControllerType::AppAndDiagData
	//
	bool diagDataProvided = false;
	bool diagDataEnable = true;
	QString diagDataIP;
	int diagDataPort = 0;
	QString diagDataServiceID;
	quint32 diagDataUID = 0;
	int diagDataSize = 0;
	int diagDataFramesQuantity = 0;
};
