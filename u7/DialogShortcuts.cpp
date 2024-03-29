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

	m_treeWidget->setColumnCount(static_cast<int>(headerLabels.size()));
	m_treeWidget->setHeaderLabels(headerLabels);

	//
	// Fill Shortcuts
	//

	QTreeWidgetItem* section = addSection("Main Window");

	addShortcut(QKeySequence(Qt::CTRL | Qt::Key_B).toString() + ", " + QKeySequence(Qt::Key_F7).toString(), "Build project", section);
	addShortcut(QKeySequence(Qt::CTRL | Qt::Key_K).toString(), "Locator", section);

	QString exitShortcut = QKeySequence(QKeySequence::Quit).toString();
	if (exitShortcut.isEmpty() == true) 
	{
		exitShortcut = "Alt+F4";
	}
	addShortcut(exitShortcut, "Exit the application", section);

	section->setExpanded(true);

	//

	section = addSection("Project Tab Page");

	addShortcut(QKeySequence(QKeySequence::Refresh).toString(), "Refresh projects list", section);

	section->setExpanded(true);

	//

	section = addSection("Equipment Tab Page");

	addShortcut(QKeySequence(QKeySequence::Copy).toString(), "Copy equipment object", section);
	addShortcut(QKeySequence(QKeySequence::Paste).toString(), "Paste equipment object", section);
	addShortcut(QKeySequence(QKeySequence::Find).toString(), "Find equipment object", section);
	addShortcut(QKeySequence(QKeySequence::Delete).toString(), "Delete equipment object", section);
	addShortcut(QKeySequence(QKeySequence::Refresh).toString(), "Refresh equipment tree", section);

	section->setExpanded(true);

	//

	section = addSection("AppSignals Tab Page");

	addShortcut(QKeySequence(QKeySequence::New).toString(), "Create new application signal", section);
	addShortcut(QKeySequence(QKeySequence::Copy).toString(), "Copy AppSignalID in clipboard", section);
	addShortcut(QKeySequence(QKeySequence::Find).toString() + ", " + QKeySequence(QKeySequence::Replace).toString(), "Find and replace application signals", section);
	addShortcut(QKeySequence(QKeySequence::Delete).toString(), "Delete application signal", section);
	addShortcut(QKeySequence(QKeySequence::Refresh).toString(), "Refresh application signals list", section);

	section->setExpanded(true);

	//

	section = addSection("Files Tab Page");

	addShortcut(QKeySequence(QKeySequence::Refresh).toString(), "Refresh files tree", section);
	addShortcut(QKeySequence(QKeySequence::Delete).toString(), "Delete file", section);

	section->setExpanded(true);

	//

	section = addSection("Schemas Tab Page");

	addShortcut(QKeySequence(QKeySequence::New).toString(), "Create a new schema", section);
	addShortcut(QKeySequence(QKeySequence::Delete).toString(), "Delete schema", section);
	addShortcut(QKeySequence(QKeySequence::Find).toString(), "Find text in schemas ID and description", section);
	addShortcut(QKeySequence(QKeySequence::Refresh).toString(), "Refresh schemas list", section);

	addShortcut(QKeySequence(Qt::CTRL | Qt::Key_QuoteLeft).toString() + ", " + QKeySequence(Qt::CTRL | Qt::Key_AsciiTilde).toString(), "Switch to control tab page", section);

	section->setExpanded(true);

	//

	section = addSection("Schema Editor");

	addShortcut(QKeySequence(Qt::Key_Escape).toString(), "Remove selection", section);
	addShortcut(QKeySequence(Qt::Key_F2).toString(), "Edit signals list for selected item", section);
	addShortcut(QKeySequence(Qt::CTRL | Qt::Key_I).toString(), "Toggle Info mode", section);
	addShortcut(QKeySequence(Qt::ALT | Qt::Key_D).toString(), "Attach/Detach window", section);
	addShortcut(QKeySequence(QKeySequence::Save).toString(), "Save schema", section);
	addShortcut(QKeySequence(QKeySequence::Close).toString(), "Close schema", section);
	addShortcut(QKeySequence(QKeySequence::Undo).toString(), "Undo", section);
	addShortcut(QKeySequence(QKeySequence::Redo).toString(), "Redo", section);
	addShortcut(QKeySequence(QKeySequence::SelectAll).toString(), "Select All", section);
	addShortcut(QKeySequence(QKeySequence::Cut).toString(), "Cut", section);
	addShortcut(QKeySequence(QKeySequence::Copy).toString(), "Copy", section);
	addShortcut(QKeySequence(QKeySequence::Paste).toString(), "Paste", section);
	addShortcut(QKeySequence(QKeySequence::Delete).toString(), "Delete", section);
	addShortcut(QKeySequence(Qt::ALT | Qt::Key_Return).toString(), "Properties", section);
	addShortcut(QKeySequence(Qt::ALT | Qt::Key_W).toString(), "Same Width", section);
	addShortcut(QKeySequence(Qt::ALT | Qt::Key_H).toString(), "Same Height", section);
	addShortcut(QKeySequence(Qt::CTRL | Qt::Key_Home).toString(), "Move to Front", section);
	addShortcut(QKeySequence(Qt::CTRL | Qt::Key_End).toString(), "Send to Back", section);
	addShortcut(QKeySequence(Qt::CTRL | Qt::Key_PageUp).toString(), "Move forward", section);
	addShortcut(QKeySequence(Qt::CTRL | Qt::Key_PageDown).toString(), "Move backward", section);
	addShortcut(QKeySequence(QKeySequence::ZoomIn).toString(), "Zoom In", section);
	addShortcut(QKeySequence(QKeySequence::ZoomOut).toString(), "Zoom Out", section);
	addShortcut(QKeySequence(Qt::CTRL | Qt::Key_Asterisk).toString(), "Zoom 100%", section);
	addShortcut(QKeySequence(Qt::CTRL | Qt::Key_Slash).toString(), "Comment/Uncomment", section);
	addShortcut(QKeySequence(Qt::CTRL | Qt::Key_L).toString(), "Lock/Unlock", section);
	addShortcut(QKeySequence(QKeySequence::Find).toString(), "Find Dialog", section);
	addShortcut(QKeySequence(QKeySequence::FindNext).toString(), "Find Next (on Schema and in Find Dialog)", section);
	addShortcut(QKeySequence(QKeySequence::FindPrevious).toString(), "Find Previous (on Schema and in Find Dialog)", section);
	addShortcut(QKeySequence(Qt::ALT | Qt::Key_N).toString(), "Add Application Signal (on selected input/output/internal)", section);
	addShortcut(QKeySequence(Qt::ALT | Qt::Key_S).toString(), "Application Signal Properties (on selected input/output/internal)", section);
	addShortcut("Alt + Arrow Keys", "Schema Items Navigation", section);
	addShortcut("Ctrl + Drag Items", "Create copies of dragged items", section);
	addShortcut("Alt + Drag Items", "Items are dragged only horizontally or vertically", section);
	addShortcut("Space + Drag AFB Items", "Remove binding to links while dragging AFB elements", section);

	section->setExpanded(true);

	//

	section = addSection("Build Tab Page");

	addShortcut(QKeySequence(Qt::Key_F3).toString(), "Find", section);
	addShortcut(QKeySequence(Qt::Key_F6).toString(), "Find next issue", section);
	addShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F6).toString(), "Find previous issue", section);

	section->setExpanded(true);

	//

	section = addSection("Pending Changes Dialog");

	addShortcut(QKeySequence(QKeySequence::Refresh).toString(), "Refresh pending changes list", section);

	section->setExpanded(true);

	//

	section = addSection("Bus Editor Dialog");

	addShortcut(QKeySequence("Insert").toString(), "Create new bus", section);
	addShortcut(QKeySequence(QKeySequence::Copy).toString(), "Copy bus to clipboard", section);
	addShortcut(QKeySequence(QKeySequence::Paste).toString(), "Paste bus from clipboard", section);
	addShortcut(QKeySequence(QKeySequence::Delete).toString(), "Delete bus", section);
	addShortcut(QKeySequence(QKeySequence::Refresh).toString(), "Refresh busses list", section);

	section->setExpanded(true);

	//
	
	section = addSection("Connections Editor Dialog");

	addShortcut(QKeySequence(QKeySequence::Copy).toString(), "Copy connection in clipboard", section);
	addShortcut(QKeySequence(QKeySequence::Paste).toString(), "Paste connection from clipboard", section);
	addShortcut(QKeySequence(QKeySequence::Delete).toString(), "Delete connection", section);
	addShortcut(QKeySequence(QKeySequence::Refresh).toString(), "Refresh connections list", section);

	section->setExpanded(true);

	//
	
	section = addSection("Tags Editor Dialog");

	addShortcut(QKeySequence(QKeySequence::Delete).toString(), "Delete tag", section);

	section->setExpanded(true);

	//

	section = addSection("Tests Tab Page");

	addShortcut(QKeySequence(QKeySequence::New).toString(), "Create new test file", section);
	addShortcut(QKeySequence(QKeySequence::Refresh).toString(), "Refresh test files", section);
	addShortcut(QKeySequence(QKeySequence::Save).toString(), "Save test file", section);
	addShortcut(QKeySequence(QKeySequence::Close).toString() + ", " + QKeySequence(Qt::CTRL | Qt::Key_W).toString(), "Close test file", section);
	addShortcut(QKeySequence(Qt::CTRL | Qt::Key_G).toString(), "Go to line", section);
	
	addShortcut(QKeySequence(QKeySequence::Delete).toString(), "Delete selected output", section);
	addShortcut(QKeySequence(QKeySequence::SelectAll).toString(), "Select all output", section);
	addShortcut(QKeySequence(QKeySequence::Cut).toString(), "Cut text from output", section);
	addShortcut(QKeySequence(QKeySequence::Find).toString(), "Find text in output", section);

	section->setExpanded(true);

	//

	section = addSection("Simulator Main Page");

	addShortcut(QKeySequence(Qt::CTRL | Qt::Key_W).toString(), "Close Simulator Page", section);

	addShortcut(QKeySequence(Qt::CTRL | Qt::Key_QuoteLeft).toString() + ", " + QKeySequence(Qt::CTRL | Qt::Key_AsciiTilde).toString(), "Switch to control tab page", section);

	addShortcut(QKeySequence(QKeySequence::Open).toString(), "Open simulation project", section);
	addShortcut(QKeySequence(QKeySequence::Close).toString(), "Close simulation project", section);
	addShortcut(QKeySequence(QKeySequence::Refresh).toString(), "Refresh simulation project", section);
	addShortcut(QKeySequence(QKeySequence::New).toString(), "Add simulation window", section);
	addShortcut(QKeySequence(Qt::CTRL | Qt::Key_R).toString() + ", " + QKeySequence(Qt::CTRL | Qt::Key_F5).toString(), "Run simulation", section);
	addShortcut(QKeySequence(QKeySequence::Find).toString(), "Find signal", section);

	section->setExpanded(true);

	//

	section = addSection("Simulator Output Window");

	addShortcut(QKeySequence(QKeySequence::Delete).toString(), "Delete selected output", section);
	addShortcut(QKeySequence(QKeySequence::SelectAll).toString(), "Select all output", section);
	addShortcut(QKeySequence(QKeySequence::Cut).toString(), "Cut text from output", section);

	section->setExpanded(true);

	//

	section = addSection("Simulator Overrides Window");

	addShortcut(QKeySequence(QKeySequence::Delete).toString(), "Delete overriden signal", section);
	
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
