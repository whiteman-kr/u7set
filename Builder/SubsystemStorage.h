#pragma once
#include "../HardwareLib/Subsystem.h"

class DbController;

namespace Builder
{
	//
	// SubsystemStorage
	//
	class SubsystemStorage : public QObject
	{
		Q_OBJECT

	public:
		SubsystemStorage();

		void add(std::shared_ptr<Hardware::Subsystem> subsystem);
		int count() const;
		std::shared_ptr<Hardware::Subsystem> get(int index) const;
		void clear();

		const std::vector<std::shared_ptr<Hardware::Subsystem>>& subsystems();

		bool load(DbController* db, QString &errorCode);
		bool save(DbController* db, const QString &comment);

		Q_INVOKABLE int ssKey(QString subsysId);

		int subsystemKey(const QString& subsystemID);

	private:
		std::vector<std::shared_ptr<Hardware::Subsystem>> m_subsystems;
		const QString fileName = "SubsystemsList.xml";
	};

}
