#pragma once
#include "../DbLib/DbObjectStorage.h"
#include "../HardwareLib/DiagSignalType.h"

class DiagSignalTypesStorage : public DbObjectStorage<std::shared_ptr<Hardware::DiagSignalType>>
{
public:
	explicit DiagSignalTypesStorage(DbController* db);
	virtual ~DiagSignalTypesStorage();

public:
	using DbObjectStorage::get;

	std::shared_ptr<Hardware::DiagSignalType> get(const QString& diagSignalTypeId) const;
	bool hasSignalTypeId(const QString& diagSignalTypeId) const;

	// --
	//
	bool load(QString* errorMessage) override;
	bool save(const QUuid& uuid, QString* errorMessage) override;

	void writeToXml(XmlWriteHelper& xml) const;
	bool readFromXml(XmlReadHelper& xml);
};

