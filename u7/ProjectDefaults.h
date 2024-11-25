#pragma once

class ProjectDefaults
{
private:
	ProjectDefaults() = default;

public:
	ProjectDefaults(const ProjectDefaults&) = default;
	ProjectDefaults(ProjectDefaults&&) = default;
	~ProjectDefaults() = default;

	ProjectDefaults& operator=(const ProjectDefaults&) = default;
	ProjectDefaults& operator=(ProjectDefaults&&) = default;

	// Singleton - for the common usage in the editor.
	// Ge the copy for using in the edit engine (schema editor).
	//
	[[nodiscard]] static ProjectDefaults& instance();

public:
	struct Property
	{
		QString name;
		QVariant value;
	};

public:
	bool update(DbController& db, QWidget* parentWidget);
	bool parse(const QString& value);

	const std::vector<ProjectDefaults::Property>& values(const QString& section) const;

	[[nodiscard]] bool hasSection(const QString& section) const;

private:
	std::map<QString, std::vector<Property>>
		m_defaults; // Key - section name (like SchemaItemRect), value - list of properties

	static const std::vector<ProjectDefaults::Property> s_empty;
};