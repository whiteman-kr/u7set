#pragma once

#include "../AppSignalSetProvider.h"
#include "EditSchemaTypes.h"
#include <VFrame30/Schema.h>


class EditSchemaWidget;

//
//
// EditSchemaTabPage
//
//
class EditSchemaTabPage : public QMainWindow,
						  public HasDbController
{
	Q_OBJECT

public:
	EditSchemaTabPage() = delete;
	EditSchemaTabPage(QTabWidget* tabWidget,
					  std::shared_ptr<VFrame30::Schema> schema,
					  const DbFileInfo& fileInfo,
					  DbController* db,
					  AppSignalSetProvider* signalSetProvider);
	virtual ~EditSchemaTabPage();

protected:
	virtual void closeEvent(QCloseEvent* event) override;

	// Public methods
	//
public:
	void ensureVisible();
	void setPageTitle();
	void updateZoomAndScrolls(bool fitToScreen, bool repaint);

	double zoom() const;
	void setZoom(double zoom, bool repaint);

	QString activeLayer() const;
	void setActiveLayer(QString layer);
	void layersDialog();

	void updateAfbSchemaItems();
	void updateUfbSchemaItems();
	void updateBussesSchemaItems();
	void updateActuatorSchemaItems();

signals:
	void vcsFileStateChanged();
	void aboutToClose(EditSchemaTabPage*);
	void pleaseDetachOrAttachWindow(EditSchemaTabPage*);
	void fileWasSaved(QString schemaDetails);

public slots:
	void detachOrAttachWindow();
	void closeTab();

protected slots:
	void projectClosed();

	void modifiedChanged(bool modified);

	void checkInFile();
	void checkOutFile();
	void undoChangesFile();

	void fileMenuTriggered();
	void sizeAndPosMenuTriggered();
	void itemsOrderTriggered();

public:
	bool saveWorkcopy();

protected:
	void getCurrentWorkcopy(); // Save current schema to a file
	void setCurrentWorkcopy(); // Load a schema from a file

							   // Properties
	//
public:
	VFrame30::Schema* schema();
	const VFrame30::Schema* schema() const;

	[[nodiscard]] const DbFileInfo& fileInfo() const;
	void setFileInfo(const DbFileInfo& fi);

	[[nodiscard]] bool readOnly() const;
	void setReadOnly(bool value);

	[[nodiscard]] bool modified() const;
	void resetModified();

	[[nodiscard]] bool compareWidget() const;
	[[nodiscard]] bool isCompareWidget() const;
	void setCompareWidget(bool value, std::shared_ptr<VFrame30::Schema> source, std::shared_ptr<VFrame30::Schema> target);

	void setCompareItemActions(const std::map<QUuid, CompareAction>& itemsActions);

	// Data
	//
private:
	EditSchemaWidget* m_schemaWidget = nullptr;
	QToolBar* m_toolBar = nullptr;
	QTabWidget* m_tabWidget = nullptr;

	QAction* m_fileAction = nullptr;
	QAction* m_alignAction = nullptr;
	QAction* m_orderAction = nullptr;
};
