#ifndef SUBSYSTEM_H
#define SUBSYSTEM_H

#include "../lib/DbController.h"

namespace Hardware
{
	//
	//
	// Subsystem
	//
	//
	class Subsystem : public QObject
	{
		Q_OBJECT
		Q_PROPERTY(int Index READ index WRITE setIndex)
		Q_PROPERTY(int Key READ key WRITE setKey)
		Q_PROPERTY(QString SubsystemID READ subsystemId WRITE setSubsystemId)
		Q_PROPERTY(QString Caption READ caption WRITE setCaption)

	public:
		Subsystem();
		Subsystem(int index, int key, const QString& subsystemId, const QString& caption);

		bool save(QXmlStreamWriter& writer);
		bool load(QXmlStreamReader& reader);

		// Properties
		//
	public:
		const QString& subsystemId() const;
		void setSubsystemId(const QString& value);

		const QString& caption() const;
		void setCaption(const QString& value);

		int index() const;
		void setIndex(int value);

		int key() const;
		void setKey(int value);

	public:
		static const int MaxKeyValue = 63;

	private:
		int m_index;
		int m_key;
		QString m_subsystemId;
		QString m_caption;
	};

	//
	//
	// SubsystemStorage
	//
	//
	class SubsystemStorage : public QObject
	{
		Q_OBJECT

	public:
		SubsystemStorage();

		void add(std::shared_ptr<Subsystem> subsystem);
		int count() const;
		std::shared_ptr<Subsystem> get(int index) const;
		void clear();

		const std::vector<std::shared_ptr<Subsystem>>& subsystems();

		bool load(DbController* db, QString &errorCode);
		bool save(DbController* db, const QString &comment);

		Q_INVOKABLE int ssKey(QString subsysId);

	private:
		std::vector<std::shared_ptr<Subsystem>> m_subsystems;
		const QString fileName = "SubsystemsList.xml";
	};
}

#endif // SUBSYSTEM_H
