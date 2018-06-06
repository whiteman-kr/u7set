#ifndef SCHEMASWORKSPACE_H
#define SCHEMASWORKSPACE_H

#include "SchemaStorage.h"
#include "../lib/Tuning/TuningModel.h"
#include "../lib/Tuning/TuningSignalState.h"
#include "../lib/Tuning/TuningSignalManager.h"
#include "TuningClientTcpClient.h"
#include "TuningSchemaWidget.h"

class SchemasWorkspace : public QWidget
{
	Q_OBJECT

public:
	explicit SchemasWorkspace(ConfigController* configController, TuningSignalManager* tuningSignalManager, TuningClientTcpClient* tuningTcpClient, const QString& globalScript, QWidget* parent);
	virtual ~SchemasWorkspace();

private slots:
	void slot_schemaListSelectionChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

private:

	TuningController m_tuningController;

	TuningSignalManager* m_tuningSignalManager = nullptr;

	TuningClientTcpClient* m_tuninTcpClient = nullptr;

	SchemaStorage* m_schemaStorage = nullptr;

	QSplitter* m_hSplitter = nullptr;				// This is used only with LIST mode!

	QTreeWidget* m_schemasList = nullptr;			// This is used only with LIST mode!

	TuningSchemaWidget* m_schemaWidget = nullptr;	// This is used only with LIST mode!
};

#endif // SCHEMASWORKSPACE_H
