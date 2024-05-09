#pragma once
#include <DbLib/DbObjectStorage.h>
#include <HardwareLib/DiagSignalType.h>

class DiagSignalTypesStorage : public DbObjectStorage<std::shared_ptr<Hardware::DiagSignalTypeObject>>
{
public:
	explicit DiagSignalTypesStorage(DbController* db);
	virtual ~DiagSignalTypesStorage();

public:
	using DbObjectStorage::get;

	void get(std::vector<Hardware::DiagSignalType>* types) const;
	std::shared_ptr<Hardware::DiagSignalTypeObject> get(const QString& diagSignalTypeId) const;
	bool hasSignalTypeId(const QString& diagSignalTypeId) const;

	int count() const;

	// --
	//
	bool load(QString* errorMessage) override;
	bool save(const QUuid& uuid, QString* errorMessage) override;

	// use in build time only!
	//
	void buildDiagSignalTypesMap();
	bool isKnownDiagSignalTypeId(const QString& diagSignalTypeId);

private:
	std::map<QString, int> m_typesMap;
};

