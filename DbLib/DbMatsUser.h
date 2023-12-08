#pragma once

#include <QObject>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <memory>
#include <vector>

class DbMatsUser
{
public:
	DbMatsUser() = default;
	DbMatsUser(const QString& login, const QString& description);

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

	[[nodiscard]] const QString& tuningTags() const;
	void setTuningTags(const QString& value);

private:
	QString m_login;
	QString m_description;
	bool m_enabled = true;
	QString m_tuningTags;
};
