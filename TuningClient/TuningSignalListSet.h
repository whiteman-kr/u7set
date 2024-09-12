#pragma once

#include <AppSignalLists/SignalList.h>

class TuningSignalListSet : public AppSignalLists::AppSignalListSet
{
public:
	std::set<Hash> filtersSetHashes(const std::set<QString>& filtersSet) const;

Q_OBJECT 
public:
	virtual bool load(QString* errorMessage) override;
	virtual bool save(QString* errorMessage) const override;
};