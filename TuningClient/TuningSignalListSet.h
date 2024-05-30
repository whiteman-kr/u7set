#pragma once

#include "../libs/AppSignalLists/include/AppSignalLists/SignalList.h"

class TuningSignalListSet : public AppSignalLists::AppSignalListSet
{
public:
	virtual bool load(QString* errorMessage) override;
	virtual bool save(QString* errorMessage) const override;
};