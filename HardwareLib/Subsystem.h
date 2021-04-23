#pragma once

#include <vector>
#include <memory>
#include <QObject>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

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
		Subsystem(QObject* parent = nullptr);
		Subsystem(int index, int key, const QString& subsystemId, const QString& caption, QObject* parent = nullptr);

		bool save(QXmlStreamWriter& writer);
		bool load(QXmlStreamReader& reader);

		// Properties
		//
	public:
		[[nodiscard]] const QString& subsystemId() const;
		void setSubsystemId(const QString& value);

		[[nodiscard]] const QString& caption() const;
		void setCaption(const QString& value);

		[[nodiscard]] int index() const;
		void setIndex(int value);

		[[nodiscard]] int key() const;
		void setKey(int value);

	public:
		static const int MaxKeyValue = 63;

	private:
		int m_index = 0;
		int m_key = 5;
		QString m_subsystemId;
		QString m_caption;
	};
}

