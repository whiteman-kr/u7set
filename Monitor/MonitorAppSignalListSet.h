#pragma once

#include <AppSignalLists/SignalList.h>

class MonitorAppSignalListSet : public AppSignalLists::AppSignalListSet
{
public:
	virtual bool load(QString* errorMessage) override;
	virtual bool save(QString* errorMessage) const override;
};