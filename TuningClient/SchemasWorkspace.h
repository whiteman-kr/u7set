#ifndef SCHEMASWORKSPACE_H
#define SCHEMASWORKSPACE_H

#include "TuningSchemaManager.h"
#include "../lib/Tuning/TuningModel.h"
#include "../AppSignalLib/TuningSignalState.h"
#include "../AppSignalLib/TuningSignalManager.h"
#include "../ClientLib/TuningConnection.h"
#include "../VFrame30/LogController.h"
#include "TuningSchemaWidget.h"

class SchemasWorkspace : public QWidget
{
	Q_OBJECT

public:
	SchemasWorkspace(TuningConfigController& configController,
					 const QString& caption,
					 const QStringList& schemasTags,
					 QString startSchemaId,
					 ILogFile* logFile,
					 QWidget* parent);
	virtual ~SchemasWorkspace();

	const QString& caption() const;

private slots:
	void slot_itemSelectionChanged();
	void slot_schemaChanged(VFrame30::ClientSchemaWidget* widget, VFrame30::Schema* schema);

public:
	void zoomIn();
	void zoomOut();
	void zoom100();
	void zoomToFit();

private:

	void createSchemasList();
	void createSchemasTabs();
	void createSchemasView();

	TuningSchemaWidget* activeSchemaWidget();

private:
	VFrame30::LogController m_logController;
	TuningSchemaManager m_schemaManager;
	TuningConfigController& m_configController;

	// Interface members
	//
	QString m_caption;
	QString m_startSchemaId;
	QStringList m_schemasTags;

	QTabWidget* m_tabWidget = nullptr;				// This is used only with TAB mode!

	QSplitter* m_hSplitter = nullptr;				// This is used only with LIST mode!
	QTreeWidget* m_schemasList = nullptr;			// This is used only with LIST mode!

	TuningSchemaWidget* m_schemaWidget = nullptr;	// This is used with LIST and NO_LIST_AND_TAB mode!
};

#endif // SCHEMASWORKSPACE_H
