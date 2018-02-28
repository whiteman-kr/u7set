#ifndef SCHEMASWORKSPACE_H
#define SCHEMASWORKSPACE_H

#include "TuningSchemaManager.h"
#include "../lib/Tuning/TuningModel.h"
#include "../lib/Tuning/TuningSignalState.h"
#include "../lib/Tuning/TuningSignalManager.h"
#include "TuningClientTcpClient.h"
#include "TuningSchemaWidget.h"

class SchemasWorkspace : public QWidget
{
	Q_OBJECT

public:
	SchemasWorkspace(ConfigController* configController,
					 TuningSignalManager* tuningSignalManager,
					 TuningClientTcpClient* tuningTcpClient,
					 QWidget* parent);
	virtual ~SchemasWorkspace();

private slots:
	void slot_schemaListSelectionChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

private:
	VFrame30::TuningController m_tuningController;
	TuningSignalManager* m_tuningSignalManager = nullptr;
	TuningClientTcpClient* m_tuninTcpClient = nullptr;
	TuningSchemaManager m_schemaManager;

	QSplitter* m_hSplitter = nullptr;				// This is used only with LIST mode!

	QTreeWidget* m_schemasList = nullptr;			// This is used only with LIST mode!

	TuningSchemaWidget* m_schemaWidget = nullptr;	// This is used only with LIST mode!
};

#endif // SCHEMASWORKSPACE_H
