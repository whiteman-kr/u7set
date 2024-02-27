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
	struct Property
	{
		QString name;
		QVariant value;
	};

	bool update(DbController& db, QWidget* parentWidget);

	bool parse(const QString& value);

	const std::vector<ProjectDefaults::Property>& values(const QString& section) const;

private:
	std::map<QString, std::vector<Property>>
		m_defaults; // Key is a section name, like SchemaItemRect, value is a list of properties

	static const std::vector<ProjectDefaults::Property> s_empty;
};