#ifndef SUBSYSTEM_H
#define SUBSYSTEM_H

#include "../include/DbController.h"

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
		Q_PROPERTY(QString StrID READ strId WRITE setStrId)
		Q_PROPERTY(QString Caption READ caption WRITE setCaption)

	public:
		Subsystem();
		Subsystem(int index, int key, const QString& strId, const QString& caption);

		bool save(QXmlStreamWriter& writer);
		bool load(QXmlStreamReader& reader);

		// Properties
		//
	public:
		const QString& strId() const;
		void setStrId(const QString& value);

		const QString& caption() const;
		void setCaption(const QString& value);

		int index() const;
		void setIndex(int value);

		int key() const;
		void setKey(int value);

	private:
		int m_index;
		int m_key;
		QString m_strId;
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

		bool load(DbController* db, QString &errorCode);
		bool save(DbController* db);

		Q_INVOKABLE int jsGetSsKey(QString subsysId);


	private:
		std::vector<std::shared_ptr<Subsystem>> m_subsystems;
		const QString fileName = "SubsystemsList.xml";

	};
}

#endif // SUBSYSTEM_H
