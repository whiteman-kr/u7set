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
	void slot_itemSelectionChanged();
	void slot_schemaChanged(VFrame30::ClientSchemaWidget* widget, VFrame30::Schema* schema);

public:
	void zoomIn();
	void zoomOut();
	void zoom100();
	void zoomToFit();

private:
	TuningSchemaWidget* activeSchemaWidget();

private:
	TuningClientTuningController m_tuningController;
	TuningSignalManager* m_tuningSignalManager = nullptr;
	TuningClientTcpClient* m_tuninTcpClient = nullptr;
	TuningSchemaManager m_schemaManager;

	QTabWidget* m_tabWidget = nullptr;				// This is used only with TAB mode!

	QSplitter* m_hSplitter = nullptr;				// This is used only with LIST mode!
	QTreeWidget* m_schemasList = nullptr;			// This is used only with LIST mode!

	TuningSchemaWidget* m_schemaWidget = nullptr;	// This is used with LIST and NO_LIST_AND_TAB mode!
};

#endif // SCHEMASWORKSPACE_H
