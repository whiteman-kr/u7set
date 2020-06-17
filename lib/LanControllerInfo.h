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
	QString tuningServiceIP;
	int tuningServicePort = 0;
	QString tuningServiceNetmask;


	// only for adapterType == E::LanControllerType::AppData or E::LanControllerType::AppAndDiagData
	//
	bool appDataProvided = false;
	bool appDataEnable = true;
	QString appDataIP;
	int appDataPort = 0;
	QString appDataServiceID;
	QString appDataServiceIP;
	int appDataServicePort = 0;
	QString appDataServiceNetmask;
	quint32 appDataUID = 0;
	int appDataSize = 0;
	int appDataFramesQuantity = 0;
	int overrideAppDataWordCount = -1;


	// only for adapterType == E::LanControllerType::DiagData or E::LanControllerType::AppAndDiagData
	//
	bool diagDataProvided = false;
	bool diagDataEnable = true;
	QString diagDataIP;
	int diagDataPort = 0;
	QString diagDataServiceID;
	QString diagDataServiceIP;
	int diagDataServicePort = 0;
	QString diagDataServiceNetmask;
	quint32 diagDataUID = 0;
	int diagDataSize = 0;
	int diagDataFramesQuantity = 0;
	int overrideDiagDataWordCount = -1;
};
