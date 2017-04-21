#ifndef SCHEMASWORKSPACE_H
#define SCHEMASWORKSPACE_H

#include "SchemaStorage.h"
#include "../lib/Tuning/TuningModel.h"
#include "../lib/Tuning/TuningSignal.h"
#include "../lib/Tuning/TuningSignalManager.h"
#include "../lib/Tuning/TuningFilter.h"
#include "TuningSchemaWidget.h"

class SchemasWorkspace : public QWidget
{
	Q_OBJECT

public:
	explicit SchemasWorkspace(ConfigController* configController, TuningSignalManager *tuningSignalManager, const TuningSignalStorage* objects, const QString& globalScript, QWidget* parent);
	virtual ~SchemasWorkspace();

private slots:
	void slot_schemaListSelectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

private:

	TuningSignalManager *m_tuningSignalManager = nullptr;

	TuningSignalStorage m_objects;

	SchemaStorage *m_schemaStorage = nullptr;

	QSplitter* m_hSplitter = nullptr;				// This is used only with LIST mode!

	QTreeWidget* m_schemasList = nullptr;			// This is used only with LIST mode!

	TuningSchemaWidget* m_schemaWidget = nullptr;	// This is used only with LIST mode!
};

#endif // SCHEMASWORKSPACE_H
