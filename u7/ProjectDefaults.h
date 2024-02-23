#pragma once

class DbController;

class ProjectDefaults
{
private:
	ProjectDefaults() = default;

public:
	ProjectDefaults(const ProjectDefaults&) = delete;
	ProjectDefaults& operator=(const ProjectDefaults&) = delete;
	ProjectDefaults(ProjectDefaults&&) = delete;
	ProjectDefaults& operator=(ProjectDefaults&&) = delete;

	~ProjectDefaults() = default;

	static ProjectDefaults& instance();

public:
	bool update(DbController& db, QWidget* parentWidget);

	/// @brief Load the project defaults from the ini file, value is a ini file content
	bool parse(const QString& value);

	/// @brief Get the default value for a property.
	/// @param section The section name.
	/// @param key The property name.
	/// @return The default value for the property.
	QVariant value(const QString& section, const QString& key) const;

private:
	using Key = std::pair<QString, QString>; // Key is a pair of section and key (property name).

	std::map<Key, QVariant> m_defaults;      // The default values for the properties.
};