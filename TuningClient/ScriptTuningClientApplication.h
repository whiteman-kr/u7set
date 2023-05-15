#pragma once

#include <QObject>

/*! \class ScriptTuningClientApplication
	\ingroup controllers
	\brief Represents a class for access to TuningClient application wide properties and functions.

	In scripts this object can be accessed by global <b>app</b> object.
*/
class ScriptTuningClientApplication : public QObject
{
	Q_OBJECT

	/// \brief Application unique identifier (read only property).
	Q_PROPERTY(QString equipmentID READ equipmentId)
	Q_PROPERTY(QString EquipmentID READ equipmentId)

public:
	explicit ScriptTuningClientApplication();

public:
	QString equipmentId() const;

public slots:
	/// \brief Starts the program 'program' with the arguments 'arguments' in 'workingDir' directory in a new process.
	/// Multiple arguments are divided by a semicolon.
	bool start(QString program, QString arguments = QString(), QString workDir = QString());


private:
};

