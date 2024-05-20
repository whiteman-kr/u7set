#pragma once

class SimSchemaView;

/*! \class ScriptSimApplication
	\ingroup controllers
	\brief Represents a class for access to application wide properties and functions (this is a stub, it does not provide all the functionality).

	In scripts this object can be accessed by global <b>app</b> object.
*/
class ScriptSimApplication : public QObject
{
	Q_OBJECT

	/// \brief Application unique identifier (read only property).
	Q_PROPERTY(QString equipmentID READ equipmentId)
	Q_PROPERTY(QString EquipmentID READ equipmentId)

public:
	ScriptSimApplication(const SimSchemaView* simSchemaView, QObject* parent = nullptr);

public slots:
	/// \brief Stub, does not provide functionality.
	void showArchive(QStringList signalsList, QDateTime startTime, QDateTime endTime, int timeType);

	/// \brief Stub, does not provide functionality.
	void showSnapshot(QStringList signalsList);

	/// \brief Stub, does not provide functionality.
	void showSnapshotByMask(QStringList masks);

	/// \brief Stub, does not provide functionality.
	void showSnapshotByTag(QStringList tags);

	/// \brief Stub, does not provide functionality.
	void setVisibleSchemaTree(bool visible);

	/// \brief Stub, does not provide functionality.
	void toggleSchemaTree();

	/// \brief Stub, does not provide functionality.
	void setVisibleTabBar(bool visible);

	/// \brief Stub, does not provide functionality.
	void setVisibleToolBar(bool visible);

	/// \brief Stub, does not provide functionality.
	void setVisibleStatusBar(bool visible);

	/// \brief Stub, does not provide functionality.
	void setVisibleMenu(bool visible);

	/// \brief Stub, does not provide functionality.
	void setFullScreen(bool fullScreen);

public:
	QString equipmentId() const;

private:
	const SimSchemaView* m_simSchemaView = nullptr;
};

