#ifndef LOGICMODULESET_H
#define LOGICMODULESET_H
#include "../lib/LmDescription.h"
#include "../lib/DbController.h"

class LogicModuleSet : public QObject
{
	Q_OBJECT
public:
	LogicModuleSet();

public:
	bool loadFile(DbController* db, QString fileName, QString* errorString);

	void add(QString fileName, std::shared_ptr<LmDescription> lm);
	bool has(QString fileName) const;
	QStringList fileList() const;

	std::shared_ptr<LmDescription> get(QString fileName) const;
	std::shared_ptr<LmDescription> get(QString fileName);

	std::shared_ptr<LmDescription> get(const Hardware::DeviceModule* logicModule) const;
	std::shared_ptr<LmDescription> get(Hardware::DeviceModule* logicModule);

	static QString lmDescriptionFile(const Hardware::DeviceModule* logicModule);

	// Data
	//
private:
	std::map<QString, std::shared_ptr<LmDescription>> m_lmDescriptions;		// Key is LogicModule description file name
};


#endif // LOGICMODULESET_H
