#pragma once

/*! \class ScriptTestSuiteApplication
	\ingroup controllers
	\brief Represents a class for access to TestSuite application wide properties and functions.

	In scripts this object can be accessed by global <b>app</b> object.
*/
class ScriptTestSuiteApplication : public QObject
{
	Q_OBJECT

	/// \brief Application unique identifier (read only property).
	Q_PROPERTY(QString equipmentID READ equipmentId)
	Q_PROPERTY(QString EquipmentID READ equipmentId)

public:
	explicit ScriptTestSuiteApplication(const QString& equipmentId);

public:
	QString equipmentId() const;

private:
	QString m_equipmentId;
};

