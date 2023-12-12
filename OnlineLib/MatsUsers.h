#pragma once

#include <QObject>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <memory>
#include <vector>

namespace OnlineLib
{
	class MatsUser
	{
	public:
		MatsUser() = default;
		MatsUser(const QString& login, const QString& description);

		bool save(QXmlStreamWriter& writer) const;
		bool load(QXmlStreamReader& reader);

		// Properties
		//
	public:
		[[nodiscard]] const QString& login() const;
		void setLogin(const QString& value);

		[[nodiscard]] const QString& description() const;
		void setDescription(const QString& value);

		[[nodiscard]] bool enabled() const;
		void setEnabled(bool value);

		[[nodiscard]] const std::set<QString>& tuningTags() const;
		void setTuningTags(const std::set<QString>& value);

		QString tuningTagsToString() const;
		void setTuningTagsFromString(QString value);

	private:
		QString m_login;
		QString m_description;
		bool m_enabled = true;
		std::set<QString> m_tuningTags;
	};

	//
	// MatsUserStorage
	//
	class MatsUserStorage
	{
	public:
		MatsUserStorage();

		void add(const MatsUser& user);
		int count() const;
		const MatsUser& get(int index) const;
		void clear();

		const std::vector<MatsUser>& users() const;
		const std::set<QString>& tuningTags(const QString& login, bool* found = nullptr) const;

		bool load(const QByteArray& data, QString& errorCode);
		bool save(QByteArray& data, const QString& comment) const;

	private:
		std::vector<MatsUser> m_users;
	};
} // namespace OnlineLib