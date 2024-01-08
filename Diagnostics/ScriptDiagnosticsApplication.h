#pragma once

#include <QObject>

class DiagnosticsMainWindow;

/*! \class ScriptDiagnosticsApplication
	\ingroup controllers
	\brief Represents a class for access to application wide properties and functions.

	In scripts this object can be accessed by global <b>app</b> object.
*/
class ScriptDiagnosticsApplication : public QObject
{
	Q_OBJECT

	/// \brief Application unique identifier (read only property).
	Q_PROPERTY(QString equipmentID READ equipmentId)
	Q_PROPERTY(QString EquipmentID READ equipmentId)

public:
	ScriptDiagnosticsApplication();

public slots:
	/// \brief Shows signals archive for specified signals list, time period and time type
	/*!
	Shows signals archive for specified signals list, time period and time type
	\warning Month number is 0-based in JavaScript!
	<br>

	\code
	//Show archive from specified time to current time
	//
	const TIME_PLANT =  0;
	const TIME_SYSTEM = 1;
	const TIME_LOCAL =  2;

	let signalsList = ["#APPSIGNALID01", "#APPSIGNALID02", "#APPSIGNALID03"];

	// Check if signals database is loaded
	//
	if (signals.signalsCount() == 0)
	{
		view.errorMessageBox("Signal database is not loaded!");
		return;
	}

	let startTime = new Date(2021, 11, 25, 1, 23, 40);	// 25 of December '2021, 01:23:40; month number is 0-based
	let endTime = new Date();							// Current date and time

	app.showArchive(signalsList, startTime, endTime, TIME_PLANT);
	\endcode
	<br>

	\code
	// Show archive for last day
	//
	const TIME_PLANT =  0;
	const TIME_SYSTEM = 1;
	const TIME_LOCAL =  2;

	let signalsList = ["#ANALOG_004", "#ANALOG_040"];

	// Check if signals database is loaded
	//
	if (signals.signalsCount() == 0)
	{
		view.errorMessageBox("Signal database is not loaded!");
		return;
	}

	let startTime = new Date(new Date() - (1000 * 60 * 60 * 24));// Subtract one day from current time
	let endTime = new Date();

	// Alternative way to subtract one day from current time
	// let startTime = new Date();
	// startTime.setDate(endTime.getDate() - 1);

	// One more alternative way to subtract one day from current time
	// let startTime = new Date(endTime - (1000 * 60 * 60 * 24));

	app.showArchive(signalsList, startTime, endTime, TIME_LOCAL);

	\endcode

	*/
	//void showArchive(QStringList signalsList, QDateTime startTime, QDateTime endTime, int timeType);
	/// \brief Show snapshot for signals specified by list of Application Signal IDs
	/*!
	Show snapshot for signals specified by list of Application Signal IDs

	\code
	let signalsList = ["#APPSIGNALID01", "#APPSIGNALID02", "#APPSIGNALID03"];
	app.showSnapshot(signalsList);
	\endcode

	*/
	//void showSnapshot(QStringList signalsList);

	/// \brief Show snapshot for signals specified by mask or by text fragment.
	/*!
	Show snapshot for signals specified by mask. Several masks can be specified by an array.
	If mask contains '*' or '?' symbols it is processed as a wildcard, otherwise specified test is searched
	in signal identifiers.

	Detailed information about masks can be found in \ref maskDescription "Masks Description".

	\code
	// Search by mask
	//
	app.showSnapshotByMask("REG*");

	// Search by several masks
	//
	app.showSnapshotByMask(["#APPSIGNAL_IN_BL*", "#APPSIGNAL_IN_SIM??"]);
	\endcode
	*/
	//void showSnapshotByMask(QStringList masks);

	/// \brief Show snapshot for signals specified by tags
	/*!
	Show snapshot for signals specified by tags. Several tags can be specified by an array.

	\code
	// Search by tag
	//
	app.showSnapshotByTag("sim");

	// Search by several tags
	//
	app.showSnapshotByTag(["sim", "lock"]);
	\endcode
	*/
	//void showSnapshotByTag(QStringList tags);

	///// \brief Show or hide schemas tree.
	//void setVisibleSchemaTree(bool visible);

	///// \brief Show or hide schemas tree oppositely to current visible state.
	//void toggleSchemaTree();

	///// \brief Show or hide schemas tab bar.
	//void setVisibleTabBar(bool visible);

	///// \brief Show or hide tool bar.
	//void setVisibleToolBar(bool visible);

	///// \brief Show or hide status bar.
	//void setVisibleStatusBar(bool visible);

	///// \brief Show or hide main menu bar.
	//void setVisibleMenu(bool visible);

	///// \brief Set or rest full screen mode.
	//void setFullScreen(bool fullScreen);

	///// \brief Starts the program 'program' with the arguments 'arguments' in 'workingDir' directory in a new process.
	///// Multiple arguments are divided by a semicolon.
	//bool start(QString program, QString arguments = QString(), QString workDir = QString());

signals:
	//void signal_showArchive(QStringList signalsList, QDateTime startTime, QDateTime endTime, int timeType);

	//void signal_showSnapshot(QStringList signalsList);
	//void signal_showSnapshotByMask(QStringList masks);
	//void signal_showSnapshotByTag(QStringList tags);

	//void signal_toggleSchemaTree();
	//void signal_setVisibleSchemaTree(bool visible);
	//void signal_setVisibleTabBar(bool visible);
	//void signal_setVisibleToolBar(bool visible);
	//void signal_setVisibleStatusBar(bool visible);
	//void signal_setVisibleMenu(bool visible);
	//void signal_setFullScreen(bool value);

public:
	QString equipmentId() const;

	void setMainWindow(DiagnosticsMainWindow* mainWindow);
	DiagnosticsMainWindow* mainWindow();
	const DiagnosticsMainWindow* mainWindow() const;

private:
	DiagnosticsMainWindow* m_mainWindow = nullptr;
};


