#include "DialogShortcuts.h"

DialogShortcuts::DialogShortcuts(QWidget* parent)
	:QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
	setWindowTitle(tr("Shortcuts"));

	setAttribute(Qt::WA_DeleteOnClose);

	QVBoxLayout* mainLayout = new QVBoxLayout();

	m_treeWidget = new QTreeWidget();
	mainLayout->addWidget(m_treeWidget);

	QStringList headerLabels;
	headerLabels << tr("Section");
	headerLabels << tr("Shortcut");
	headerLabels << tr("Description");

	m_treeWidget->setColumnCount(headerLabels.size());
	m_treeWidget->setHeaderLabels(headerLabels);

	//
	// Fill Shortcuts
	//

	QTreeWidgetItem* section = addSection("Main Window");

	addShortcut("Ctrl + B, F7", "Build project", section);
	addShortcut("Alt + F4", "Exit the application", section);

	section->setExpanded(true);

	//

	section = addSection("Project Tab Page");

	addShortcut("F5", "Refresh projects list", section);

	section->setExpanded(true);

	//

	section = addSection("Equipment Tab Page");

	addShortcut("Ctrl + C", "Copy equipment ebject", section);
	addShortcut("Ctrl + V", "Paste equipment object", section);
	addShortcut("Delete", "Delete equipment object", section);
	addShortcut("F5", "Refresh equipment tree", section);

	section->setExpanded(true);

	//

	section = addSection("Files Tab Page");

	addShortcut("F5", "Refresh files tree", section);

	section->setExpanded(true);

	//

	section = addSection("Schemas Tab Page");

	addShortcut("Ctrl + N", "Create a new schema", section);
	addShortcut("Delete", "Delete schema", section);
	addShortcut("Ctrl + F", "Find text in schemas ID and description", section);
	addShortcut("F5", "Refresh schemas list", section);

	section->setExpanded(true);

	//

	section = addSection("Schema Editor");

	addShortcut("Escape", "Remove selection", section);
	addShortcut("F2", "Edit signals list for selected item", section);
	addShortcut("Ctrl + I", "Toggle Info mode", section);
	addShortcut("Alt + D", "Attach/Detach window", section);
	addShortcut("Ctrl + S", "Save schema", section);
	addShortcut("Ctrl + W, Ctrl + F4", "Close schema", section);
	addShortcut("Ctrl + Z", "Undo", section);
	addShortcut("Ctrl + Y", "Redo", section);
	addShortcut("Ctrl + A", "Select All", section);
	addShortcut("Ctrl + X", "Cut", section);
	addShortcut("Ctrl + C", "Copy", section);
	addShortcut("Ctrl + V", "Paste", section);
	addShortcut("Delete", "Delete", section);
	addShortcut("Alt + Enter", "Properties", section);
	addShortcut("Alt + Arrow Keys", "Schema Items Navigation", section);
	addShortcut("Alt + W", "Same Width", section);
	addShortcut("Alt + H", "Same Height", section);
	addShortcut("Ctrl + Home", "Move to Front", section);
	addShortcut("Ctrl + End", "Send to Back", section);
	addShortcut("Ctrl + PgUp", "Move forward", section);
	addShortcut("Ctrl + PgDown", "Move backward", section);
	addShortcut("Ctrl + /", "Comment/Uncomment", section);
	addShortcut("Ctrl + L", "Lock/Unlock", section);
	addShortcut("Ctrl + F", "Find Dialog", section);
	addShortcut("F3", "Find Next (on Schema and in Find Dialog)", section);
	addShortcut("Shift + F3", "Find Previous (on Schema and in Find Dialog)", section);
	addShortcut("Alt + N", "Add Application Signal (on selected input/output/internal)", section);
	addShortcut("Alt + S", "Application Signal Properties (on selected input/output/internal)", section);
	addShortcut("Ctrl + \"+\"", "Zoom In", section);
	addShortcut("Ctrl + \"-\"", "Zoom Out", section);
	addShortcut("Ctrl + \"*\"", "Zoom 100%", section);
	addShortcut("Ctrl + Drag Items", "Create copies of dragged items", section);
	addShortcut("Alt + Drag Items", "Items are dragged only horizontally or vertically", section);
	addShortcut("Space + Drag AFB Items", "Remove binding to links while dragging AFB elements", section);

	section->setExpanded(true);

	//

	section = addSection("Build Tab Page");

	addShortcut("F3", "Find", section);
	addShortcut("F6", "Find next issue", section);
	addShortcut("Ctrl + F6", "Find previous issue", section);

	section->setExpanded(true);

	//

	section = addSection("Pending Changes Dialog");

	addShortcut("F5", "Refresh pending changes list", section);

	section->setExpanded(true);

	//

	for (int i = 0; i < m_treeWidget->columnCount(); i++)
	{
		m_treeWidget->resizeColumnToContents(i);
	}

	m_treeWidget->setSortingEnabled(true);
	m_treeWidget->sortByColumn(1, Qt::AscendingOrder);
	m_treeWidget->sortByColumn(0, Qt::AscendingOrder);

	QHBoxLayout* bottomLayout = new QHBoxLayout();
	mainLayout->addLayout(bottomLayout);

	bottomLayout->addStretch();

	QPushButton* closeButton = new QPushButton(tr("Close"));
	connect(closeButton, &QPushButton::clicked, this, &DialogShortcuts::reject);
	bottomLayout->addWidget(closeButton);

	setLayout(mainLayout);

	setMinimumSize(700, 700);
}


void DialogShortcuts::reject()
{
	emit dialogClosed();
	QDialog::reject();
}

QTreeWidgetItem* DialogShortcuts::addSection(const QString& name)
{
	QTreeWidgetItem* section = new QTreeWidgetItem();

	section->setText(0, name);

	m_treeWidget->addTopLevelItem(section);

	return section;
}

void DialogShortcuts::addShortcut(const QString& name, const QString& description, QTreeWidgetItem* sectionItem)
{
	QStringList l;

	l << QString();
	l << name;
	l << description;

	QTreeWidgetItem* item = new QTreeWidgetItem(l);
	sectionItem->addChild(item);
}
