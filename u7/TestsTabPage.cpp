#include "TestsTabPage.h"
#include "GlobalMessanger.h"
#include "Forms/ComparePropertyObjectDialog.h"
#include "Simulator/SimSelectBuildDialog.h"


#ifdef _DEBUG
	#include <QAbstractItemModelTester>
#endif
//
// TestsFileTreeModel
//

TestsFileTreeModel::TestsFileTreeModel(DbController* dbcontroller, QString rootFilePath, QWidget* parentWidget, QObject* parent):
	FileTreeModel(dbcontroller, rootFilePath, parentWidget, parent)
{
}

TestsFileTreeModel::~TestsFileTreeModel()
{
}

QString TestsFileTreeModel::customColumnText(Columns column, const FileTreeModelItem* item) const
{
	// Demo function
	Q_UNUSED(column);
	Q_UNUSED(item);
	return QObject::tr("Custom %1").arg(item->fileName());
}

QString TestsFileTreeModel::customColumnName(Columns column) const
{
	// Demo function
	Q_UNUSED(column);
	return QObject::tr("Custom %1").arg(static_cast<int>(column));
}

//
// TestTabPageDocument
//


TestTabPageDocument::TestTabPageDocument(const QString& fileName, IdeCodeEditor* codeEditor, QTreeWidgetItem* openFilesTreeWidgetItem):
	m_fileName(fileName),
	m_codeEditor(codeEditor),
	m_openFilesTreeWidgetItem(openFilesTreeWidgetItem)
{

}

QString TestTabPageDocument::fileName() const
{
	return m_fileName;
}

void TestTabPageDocument::setFileName(const QString& fileName)
{
	m_fileName = fileName;
}

bool TestTabPageDocument::modified() const
{
	return m_modified;
}

void TestTabPageDocument::setModified(bool value)
{
	m_modified = value;
}

IdeCodeEditor* TestTabPageDocument::codeEditor() const
{
	return m_codeEditor;
}

QTreeWidgetItem* TestTabPageDocument::openFilesTreeWidgetItem() const
{
	return m_openFilesTreeWidgetItem;
}

//
// TestsLogWidget
//

OutputDockLog::OutputDockLog()
{
}

void OutputDockLog::swapData(QStringList* data, int* errorCount, int* warningCount)
{
	if (data == nullptr || errorCount == nullptr || warningCount == nullptr)
	{
		Q_ASSERT(data);
		Q_ASSERT(errorCount);
		Q_ASSERT(warningCount);
		return;
	}

	{
		QMutexLocker l(&m_mutex);
		m_data.swap(*data);

		*errorCount = m_errorCount;
		*warningCount = m_warningCount;

		m_errorCount = 0;
		m_warningCount = 0;
	}

	return;
}

bool OutputDockLog::writeAlert(const QString& text)
{
	write(QtMsgType::QtCriticalMsg, text);
	return true;
}

bool OutputDockLog::writeError(const QString& text)
{
	write(QtMsgType::QtCriticalMsg, text);
	return true;
}

bool OutputDockLog::writeWarning(const QString& text)
{
	write(QtMsgType::QtWarningMsg, text);
	return true;
}

bool OutputDockLog::writeMessage(const QString& text)
{
	write(QtMsgType::QtInfoMsg, text);
	return true;
}

bool OutputDockLog::writeText(const QString& text)
{
	write(QtMsgType::QtInfoMsg, text);
	return true;
}

void OutputDockLog::write(QtMsgType type, const QString& msg)
{
	QString color;
	QString prefix;
	switch (type)
	{
	case QtMsgType::QtWarningMsg:
		color = "#F87217";
		prefix = "WRN ";
		break;
	case QtMsgType::QtCriticalMsg:
		color = "#D00000";
		prefix = "ERR ";
		break;
	case QtMsgType::QtInfoMsg:
		color = "black";
		break;
	}

	QString time = QTime::currentTime().toString(QLatin1String("hh:mm:ss.zzz"));

	QString html = QString("<font face=\"Courier\" size=\"4\" color=#808080>%1 </font>"
						   "<font face=\"Courier\" size=\"4\" color=%2>%3%4</font>")
				   .arg(time)
				   .arg(color)
				   .arg(prefix)
				   .arg(msg);

	QMutexLocker l(&m_mutex);
	m_data.push_back(html);

	if (type == QtMsgType::QtWarningMsg)
	{
		m_warningCount++;
	}
	else
	{
		if (type == QtMsgType::QtCriticalMsg)
		{
			m_errorCount++;
		}
	}

	if (m_data.size() > 1000)
	{
		m_data.pop_front();
	}

	return;
}

//
// OutputLogWidget
//

OutputLogTextEdit::OutputLogTextEdit(QWidget* parent):
	QTextEdit(parent)
{
	setReadOnly(true);
	setLineWrapMode(QTextEdit::NoWrap);
	setAutoFormatting(QTextEdit::AutoNone);
	document()->setUndoRedoEnabled(false);
}

void OutputLogTextEdit::contextMenuEvent(QContextMenuEvent* event)
{
	QMenu* menu = this->createStandardContextMenu();
	menu->addSeparator();

	QAction* clearAction = menu->addAction(tr("Clear"));

	QAction* selectedAction = menu->exec(event->globalPos());

	if (selectedAction == clearAction)
	{
		clear();
	}

	delete menu;
	return;
}

void OutputLogTextEdit::keyPressEvent(QKeyEvent* e)
{
	if (e->matches(QKeySequence::Delete) == true)
	{
		QTextCursor cursor = this->textCursor();
		cursor.removeSelectedText();
		this->setTextCursor(cursor);

		if (this->toPlainText().isEmpty() == true)
		{
			emit textIsEmpty();
		}

		e->accept();
		return;
	}

	if (e->matches(QKeySequence::SelectAll) == true)
	{
		this->selectAll();

		e->accept();
		return;
	}

	if (e->matches(QKeySequence::Cut) == true)
	{
		this->copy();		// this->cut() does not work

		QTextCursor cursor = this->textCursor();
		cursor.removeSelectedText();
		this->setTextCursor(cursor);

		e->accept();
		return;
	}

	if (e->matches(QKeySequence::Find) == true)
	{

		QTextCursor cursor = this->textCursor();

		emit findKeyEvent(cursor.selectedText());

		e->accept();
		return;
	}

	QTextEdit::keyPressEvent(e);
	return;
}

//
// OutputDockWidget
//



OutputDockWidgetTitleButton::OutputDockWidgetTitleButton(QDockWidget *dockWidget, bool drawActualIconSizeOnWindows)
	: QAbstractButton(dockWidget),
	  m_drawActualIconSizeOnWindows(drawActualIconSizeOnWindows)
{
	setFocusPolicy(Qt::NoFocus);
}

bool OutputDockWidgetTitleButton::event(QEvent *event)
{
	switch (event->type()) {
	case QEvent::StyleChange:
	case QEvent::ScreenChangeInternal:
		m_iconSize = -1;
		break;
	default:
		break;
	}
	return QAbstractButton::event(event);
}

static inline bool isWindowsStyle(const QStyle *style)
{
	// Note: QStyleSheetStyle inherits QWindowsStyle
	const QStyle *effectiveStyle = style;

	if (style->inherits("QProxyStyle"))
	  effectiveStyle = static_cast<const QProxyStyle *>(style)->baseStyle();

	return effectiveStyle->inherits("QWindowsStyle");
}

QSize OutputDockWidgetTitleButton::dockButtonIconSize() const
{
	if (m_iconSize < 0) {
		m_iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, this);
		// Dock Widget title buttons on Windows where historically limited to size 10
		// (from small icon size 16) since only a 10x10 XPM was provided.
		// Adding larger pixmaps to the icons thus caused the icons to grow; limit
		// this to qpiScaled(10) here.
		if (m_drawActualIconSizeOnWindows == false && isWindowsStyle(style()))
			m_iconSize = qMin((10 * logicalDpiX()) / 96, m_iconSize);
	}
	return QSize(m_iconSize, m_iconSize);
}

QSize OutputDockWidgetTitleButton::sizeHint() const
{
	ensurePolished();

	int size = 2*style()->pixelMetric(QStyle::PM_DockWidgetTitleBarButtonMargin, nullptr, this);
	if (!icon().isNull()) {
		const QSize sz = icon().actualSize(dockButtonIconSize());
		size += qMax(sz.width(), sz.height());
	}

	return QSize(size, size);
}

void OutputDockWidgetTitleButton::enterEvent(QEvent* event)
{
	if (isEnabled()) update();
	QAbstractButton::enterEvent(event);
}

void OutputDockWidgetTitleButton::leaveEvent(QEvent *event)
{
	if (isEnabled()) update();
	QAbstractButton::leaveEvent(event);
}

void OutputDockWidgetTitleButton::paintEvent(QPaintEvent *)
{
	QPainter p(this);

	QStyleOptionToolButton opt;
	opt.initFrom(this);
	opt.state |= QStyle::State_AutoRaise;

	if (style()->styleHint(QStyle::SH_DockWidget_ButtonsHaveFrame, nullptr, this))
	{
		if (isEnabled() && underMouse() && !isChecked() && !isDown())
			opt.state |= QStyle::State_Raised;
		if (isChecked())
			opt.state |= QStyle::State_On;
		if (isDown())
			opt.state |= QStyle::State_Sunken;
		style()->drawPrimitive(QStyle::PE_PanelButtonTool, &opt, &p, this);
	}

	opt.icon = icon();
	opt.subControls = { };
	opt.activeSubControls = { };
	opt.features = QStyleOptionToolButton::None;
	opt.arrowType = Qt::NoArrow;
	opt.iconSize = dockButtonIconSize();
	style()->drawComplexControl(QStyle::CC_ToolButton, &opt, &p, this);
}

OutputDockWidget::OutputDockWidget(const QString& title, OutputDockLog* log, QWidget* parent):
	QDockWidget(title, parent),
	m_log(log)
{
	QSettings settings;
	m_findCompleterStrings = settings.value("TestsTabPage/OutputFindCompleter").toStringList();

	// Output window
	//
	m_logTextEdit = new OutputLogTextEdit(this);
	QDockWidget::setWidget(m_logTextEdit);

	connect(m_logTextEdit, &OutputLogTextEdit::findKeyEvent, this, &OutputDockWidget::findKeyEvent);
	connect(m_logTextEdit, &OutputLogTextEdit::textIsEmpty, [this]()
	{
		clear();
	});

	createToolbar();

	connect(this, &QDockWidget::topLevelChanged, this, &OutputDockWidget::floatingChanged);

	startTimer(25, Qt::CoarseTimer);

	setBackgroundRole(QPalette::Light);
	setAutoFillBackground(true);

	return;
}

OutputDockWidget::~OutputDockWidget()
{
	QSettings settings;
	settings.setValue("TestsTabPage/OutputFindCompleter", m_findCompleterStrings);
}

void OutputDockWidget::clear()
{
	m_logTextEdit->clear();

	m_errorLabel->setText("E: 0000");
	m_errorLabel->setStyleSheet(QString());
	m_warningLabel->setText("W: 0000");
	m_warningLabel->setStyleSheet(QString());

	m_prevErrorButton->setEnabled(false);
	m_nextErrorButton->setEnabled(false);
	m_prevWarningButton->setEnabled(false);
	m_nextWarningButton->setEnabled(false);

	m_errorCount = 0;
	m_warningCount = 0;
}

void OutputDockWidget::setWidget(QWidget *widget)
{
	Q_UNUSED(widget);
	Q_ASSERT(false);	// Widget is set in constructor
	return;
}

void OutputDockWidget::floatingChanged(bool floating)
{
	m_floatButton->setVisible(floating == false);
}

void OutputDockWidget::prevIssue(const QLatin1String& prefix)
{
	assert(m_logTextEdit);

	//15:46:41.438 ERR

	QString regExpVal(tr("(\\d){2}:(\\d){2}:(\\d){2}.(\\d){3} %1 ").arg(prefix));

	//  --
	//
	if ((m_lastNavIsNextIssue == true || m_lastNavIsPrevIssue == true) &&
		m_logTextEdit->textCursor() == m_lastNavCursor)
	{
		m_lastNavCursor.movePosition(QTextCursor::StartOfLine);
		m_logTextEdit->setTextCursor(m_lastNavCursor);
	}

	// Find issue
	//
	QRegExp rx(regExpVal);
	bool found = m_logTextEdit->find(rx, QTextDocument::FindBackward);

	if (found == false)
	{
		// Try to find one more time from the end
		//
		QTextCursor textCursor = m_logTextEdit->textCursor();
		textCursor.movePosition(QTextCursor::End);
		m_logTextEdit->setTextCursor(textCursor);

		found = m_logTextEdit->find(rx, QTextDocument::FindBackward);
	}

	if (found == true)
	{
		// Set cursor int middle of the word, as now it is after selected word and backward find will give the same result
		//
		QTextCursor textCursor = m_logTextEdit->textCursor();
		textCursor.movePosition(QTextCursor::PreviousCharacter);
		m_logTextEdit->setTextCursor(textCursor);

		// Hightlight the line
		//
		QTextEdit::ExtraSelection highlight;
		highlight.cursor = m_logTextEdit->textCursor();
		highlight.format.setProperty(QTextFormat::FullWidthSelection, true);
		highlight.format.setBackground(Qt::yellow);

		QList<QTextEdit::ExtraSelection> extras;
		extras << highlight;

		m_logTextEdit->setExtraSelections(extras);

		// Save this search data
		//
		m_lastNavIsPrevIssue = true;
		m_lastNavIsNextIssue = false;
		m_lastNavCursor = m_logTextEdit->textCursor();
	}
	else
	{
		QTextCursor textCursor = m_logTextEdit->textCursor();
		textCursor.clearSelection();
		m_logTextEdit->setTextCursor(textCursor);

		m_logTextEdit->setExtraSelections({});
	}

	return;
}

void OutputDockWidget::nextIssue(const QLatin1String& prefix)
{
	assert(m_logTextEdit);

	QString regExpVal(tr("(\\d){2}:(\\d){2}:(\\d){2}.(\\d){3} %1 ").arg(prefix));

	//  --
	//
	if (m_lastNavIsPrevIssue == true &&
		m_logTextEdit->textCursor() == m_lastNavCursor)
	{
		m_lastNavCursor.movePosition(QTextCursor::EndOfLine);
		m_logTextEdit->setTextCursor(m_lastNavCursor);
	}

	// Find Issue
	//
	QRegExp rx(regExpVal);
	bool found = m_logTextEdit->find(rx);

	if (found == false)
	{
		// Try to find one more time from the beginning
		//
		QTextCursor textCursor = m_logTextEdit->textCursor();
		textCursor.movePosition(QTextCursor::Start);
		m_logTextEdit->setTextCursor(textCursor);

		found = m_logTextEdit->find(rx);
	}

	if (found == true)
	{
		// Set cursor int middle of the word, as now it is after selected word and backward find will give the same result
		//
		QTextCursor textCursor = m_logTextEdit->textCursor();
		textCursor.clearSelection();
		m_logTextEdit->setTextCursor(textCursor);

		// Hightlight the line
		//
		QTextEdit::ExtraSelection highlight;
		highlight.cursor = m_logTextEdit->textCursor();
		highlight.format.setProperty(QTextFormat::FullWidthSelection, true);
		highlight.format.setBackground(Qt::yellow);

		QList<QTextEdit::ExtraSelection> extras;
		extras << highlight;

		m_logTextEdit->setExtraSelections(extras);

		// Save this search data
		//
		m_lastNavIsPrevIssue = false;
		m_lastNavIsNextIssue = true;
		m_lastNavCursor = m_logTextEdit->textCursor();
	}
	else
	{
		QTextCursor textCursor = m_logTextEdit->textCursor();
		textCursor.clearSelection();
		m_logTextEdit->setTextCursor(textCursor);

		m_logTextEdit->setExtraSelections({});
	}

	return;
}

void OutputDockWidget::findEvent()
{
	QString text = m_findEdit->text();
	if (text.isEmpty() == true)
	{
		return;
	}

	// Save completer

	QString findText = m_findEdit->text();

	if (findText.isEmpty() == false && m_findCompleterStrings.contains(findText) == false)
	{
		m_findCompleterStrings.append(findText);

		QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_findCompleter->model());
		if (completerModel != nullptr)
		{
			completerModel->setStringList(m_findCompleterStrings);
		}
		while (m_findCompleterStrings.size() > 100)
		{
			m_findCompleterStrings.pop_front();
		}
	}

	// Find

	bool result = m_logTextEdit->find(text);
	if (result == true)
	{
		return;
	}

	// Try to move cursor to the beginning and repeat

	QTextCursor textCursor = m_logTextEdit->textCursor();
	textCursor.movePosition(QTextCursor::Start);
	m_logTextEdit->setTextCursor(textCursor);

	result = m_logTextEdit->find(text);
	if (result == false)
	{
		QMessageBox::warning(this, qAppName(), tr("Text was not found."));
	}

	return;
}

void OutputDockWidget::findKeyEvent(const QString& selectedText)
{
	if (selectedText.isEmpty() == false)
	{
		m_findEdit->setText(selectedText);
	}

	m_findEdit->selectAll();
	m_findEdit->setFocus();

	return;
}

void OutputDockWidget::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);

	if (isFloating() == false)
	{
		QStylePainter p(this);

		QStyleOptionDockWidget titleOpt;
		initStyleOption(&titleOpt);

		p.drawControl(QStyle::CE_DockWidgetTitle, titleOpt);
	}
	else
	{
		QPainter p(this);

		QStyleOptionDockWidget titleOpt;
		initStyleOption(&titleOpt);

		bool normalColorGroup = isActiveWindow() == true && isEnabled() == true;

		QColor color = palette().color(normalColorGroup ? QPalette::Normal : QPalette::Disabled, QPalette::Text);

		p.setPen(color);

		int margin = style()->pixelMetric(QStyle::PM_DockWidgetTitleBarButtonMargin, nullptr, this);

		QRect textRect = titleOpt.rect;
		textRect.moveLeft(margin * 2);

		p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, windowTitle());

	}

	return;
}

void OutputDockWidget::timerEvent(QTimerEvent* /*event*/)
{
	QStringList data;

	int errorCount = 0;
	int warningCount = 0;

	m_log->swapData(&data, &errorCount, &warningCount);

	if (data.isEmpty() == true)
	{
		return;
	}

	QString outputMessagesBuffer;
	outputMessagesBuffer.reserve(128000);

	for (int i = 0; i < data.size(); i++)
	{
		const QString& str = data[i];
		outputMessagesBuffer.append(str);

		if (i != data.size() - 1)
		{
			outputMessagesBuffer += QLatin1String("<br>");
		}
	}

	if (outputMessagesBuffer.isEmpty() == false)
	{
		m_logTextEdit->append(outputMessagesBuffer);
	}

	errorCount += m_errorCount;
	warningCount += m_warningCount;

	if (m_errorCount > 9999)
	{
		m_errorCount = 9999;
	}
	if (m_warningCount > 9999)
	{
		m_warningCount = 9999;
	}

	if (m_errorCount != errorCount)
	{
		if (m_errorCount == 0)
		{
			m_prevErrorButton->setEnabled(true);
			m_nextErrorButton->setEnabled(true);

			m_errorLabel->setStyleSheet("QLabel { color : #D00000; }");
		}
		m_errorLabel->setText(tr("E: %1").arg(QString::number(errorCount).rightJustified(4, '0')));
	}

	if (m_warningCount == 0 && warningCount > 0)
	{
		if (m_warningCount == 0)
		{
			m_prevWarningButton->setEnabled(true);
			m_nextWarningButton->setEnabled(true);

			m_warningLabel->setStyleSheet("QLabel { color : #F87217; }");
		}
		m_warningLabel->setText(tr("W: %1").arg(QString::number(warningCount).rightJustified(4, '0')));
	}

	m_errorCount = errorCount;
	m_warningCount = warningCount;

	return;
}

void OutputDockWidget::createToolbar()
{
	// Create Dock Panel Widget

	QWidget* outputDockPanelWidget = new QWidget(this);

	// Calculate left layout offset

	int margin = style()->pixelMetric(QStyle::PM_DockWidgetTitleBarButtonMargin, nullptr, this);

	// Create layouts and controls

	QFontMetrics fm(font());
	int pixelsWide = fm.horizontalAdvance(windowTitle());

	QHBoxLayout* l = new QHBoxLayout(outputDockPanelWidget);
	l->setContentsMargins(pixelsWide + margin * 4, 0, margin, 0);

	m_errorLabel = new QLabel(tr("E: 0000"));
	l->addWidget(m_errorLabel);

	m_prevErrorButton = new OutputDockWidgetTitleButton(this, true);
	m_prevErrorButton->setEnabled(false);
	QIcon icon = QIcon(":/Images/Images/PreviousIssue.svg");
	m_prevErrorButton->setIcon( icon );
	l->addWidget(m_prevErrorButton);
	connect(m_prevErrorButton, &QPushButton::clicked, [this](){
		prevIssue(QLatin1String("ERR"));
	});

	m_nextErrorButton = new OutputDockWidgetTitleButton(this, true);
	m_nextErrorButton->setEnabled(false);
	icon = QIcon(":/Images/Images/NextIssue.svg");
	m_nextErrorButton->setIcon( icon );
	l->addWidget(m_nextErrorButton);
	connect(m_nextErrorButton, &QPushButton::clicked, [this](){
		nextIssue(QLatin1String("ERR"));
	});

	l->addSpacerItem(new QSpacerItem(margin * 2, margin * 2));

	m_warningLabel = new QLabel(tr("W: 0000"));
	l->addWidget(m_warningLabel);

	m_prevWarningButton = new OutputDockWidgetTitleButton(this, true);
	m_prevWarningButton->setEnabled(false);
	icon = QIcon(":/Images/Images/PreviousIssue.svg");
	m_prevWarningButton->setIcon( icon );
	l->addWidget(m_prevWarningButton);
	connect(m_prevWarningButton, &QPushButton::clicked, [this](){
		prevIssue(QLatin1String("WRN"));

	});

	m_nextWarningButton = new OutputDockWidgetTitleButton(this, true);
	m_nextWarningButton->setEnabled(false);
	icon = QIcon(":/Images/Images/NextIssue.svg");
	m_nextWarningButton->setIcon( icon );
	l->addWidget(m_nextWarningButton);
	connect(m_nextWarningButton, &QPushButton::clicked, [this](){
		nextIssue(QLatin1String("WRN"));
	});

	l->addSpacerItem(new QSpacerItem(margin * 2, margin * 2));

	m_findEdit = new QLineEdit();
	m_findEdit->setClearButtonEnabled(true);
	m_findEdit->setPlaceholderText(tr("Find Text"));
	connect(m_findEdit, &QLineEdit::returnPressed, this, &OutputDockWidget::findEvent);
	l->addWidget(m_findEdit);

	m_findCompleter = new QCompleter(m_findCompleterStrings, this);
	m_findCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	m_findEdit->setCompleter(m_findCompleter);
	connect(m_findEdit, &QLineEdit::textEdited, [this](){m_findCompleter->complete();});
	connect(m_findCompleter, static_cast<void(QCompleter::*)(const QString&)>(&QCompleter::highlighted), m_findEdit, &QLineEdit::setText);

	m_findButton = new QPushButton("Find");
	l->addWidget(m_findButton);
	connect(m_findButton, &QPushButton::clicked, this, &OutputDockWidget::findEvent);

	l->addStretch();

	m_floatButton = new OutputDockWidgetTitleButton(this, false);
	icon = outputDockPanelWidget->style()->standardIcon(QStyle::SP_TitleBarNormalButton, 0, outputDockPanelWidget);
	m_floatButton->setIcon( icon );
	l->addWidget(m_floatButton);
	connect(m_floatButton, &QPushButton::clicked, [this](){
		setFloating(true);
	});

	m_closeButton = new OutputDockWidgetTitleButton(this, false);
	icon = outputDockPanelWidget->style()->standardIcon(QStyle::SP_TitleBarCloseButton, 0, outputDockPanelWidget);
	m_closeButton->setIcon( icon );
	l->addWidget(m_closeButton);
	connect(m_closeButton, &QPushButton::clicked, [this](){
		setVisible(false);
	});

	setTitleBarWidget(outputDockPanelWidget);
}

//
// TestsTabPage
//

TestsTabPage::TestsTabPage(DbController* dbc, QWidget* parent) :
	MainTabPage(dbc, parent)//,
{
	assert(dbc != nullptr);

	// Controls
	//
	m_testsWidget = new TestsWidget(dbc, this);

	QVBoxLayout* layout = new QVBoxLayout;
	setLayout(layout);

	layout->setContentsMargins(0, 6, 0, 0);
	layout->addWidget(m_testsWidget);

	// --
	//
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &TestsTabPage::projectOpened);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &TestsTabPage::projectClosed);

	// Evidently, project is not opened yet
	//

	//int do_not_forget_to_uncommnet_the_next_line;
	this->setEnabled(false);

	return;
}

TestsTabPage::~TestsTabPage()
{
}

bool TestsTabPage::hasUnsavedTests() const
{
	if (m_testsWidget == nullptr)
	{
		Q_ASSERT(m_testsWidget);
		return false;
	}

	return m_testsWidget->hasUnsavedTests();
}

void TestsTabPage::saveUnsavedTests()
{
	if (m_testsWidget == nullptr)
	{
		Q_ASSERT(m_testsWidget);
		return;
	}

	m_testsWidget->saveUnsavedTests();
}

void TestsTabPage::resetModified()
{
	if (m_testsWidget == nullptr)
	{
		Q_ASSERT(m_testsWidget);
		return;
	}

	m_testsWidget->resetModified();
}

void TestsTabPage::projectOpened()
{
	this->setEnabled(true);
	return;
}

void TestsTabPage::projectClosed()
{
	this->setEnabled(false);
	return;
}

//
// TestsWidget
//

TestsWidget::TestsWidget(DbController* dbc, QWidget* parent) :
	QMainWindow(parent),
	HasDbController(dbc),
	m_outputDockWidget(tr("Output"), &m_log, this)
{
	setWindowFlags(Qt::Widget);
	setDockOptions(AnimatedDocks | AllowTabbedDocks | GroupedDragging);

	setCorner(Qt::Corner::BottomLeftCorner, Qt::DockWidgetArea::LeftDockWidgetArea);
	setCorner(Qt::Corner::BottomRightCorner, Qt::DockWidgetArea::BottomDockWidgetArea);
	//setCorner(Qt::Corner::TopRightCorner, Qt::DockWidgetArea::RightDockWidgetArea);


	m_editableExtensions << tr("js");

	// Set up default font
	//
#if defined(Q_OS_WIN)
		m_editorFont = QFont("Consolas");
#elif defined(Q_OS_MAC)
		m_editorFont = QFont("Courier");
#else
		m_editorFont = QFont("Courier");
#endif

	createToolbar();

	createTestsDock();

	createLogDock();

	createEditorWidget();

	createActions();

	restoreSettings();

	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &TestsWidget::projectOpened);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &TestsWidget::projectClosed);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::buildStarted, this, &TestsWidget::saveAllDocuments);

	connect(&GlobalMessanger::instance(), &GlobalMessanger::compareObject, this, &TestsWidget::compareObject);

	connect(m_testsTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &TestsWidget::testsTreeSelectionChanged);
	connect(m_testsTreeModel, &FileTreeModel::dataChanged, this, &TestsWidget::testsTreeModelDataChanged);
	connect(m_testsTreeModel, &FileTreeModel::modelReset, this, &TestsWidget::testsTreeModelReset);

	connect(m_testsTreeView, &QTreeWidget::doubleClicked, this, &TestsWidget::testsTreeDoubleClicked);
	connect(m_openFilesTreeWidget, &QTreeWidget::clicked, this, &TestsWidget::openFilesClicked);

	connect(&m_simulator, &Sim::Simulator::scriptStarted, this, &TestsWidget::scriptStarted);
	connect(&m_simulator, &Sim::Simulator::scriptFinished, this, &TestsWidget::scriptFinished);

	// Evidently, project is not opened yet
	//
	this->setEnabled(false);

	return;
}

TestsWidget::~TestsWidget()
{
	saveSettings();

	m_simulator.stopScript();

	return;
}

bool TestsWidget::hasUnsavedTests() const
{
	for (auto& it : m_openDocuments)
	{
		const TestTabPageDocument& doc = it.second;

		if (doc.modified() == true)
		{
			return true;
		}
	}

	return false;
}

void TestsWidget::saveUnsavedTests()
{
	saveAllDocuments();

	return;
}

void TestsWidget::resetModified()
{
	for (auto& it : m_openDocuments)
	{
		TestTabPageDocument& doc = it.second;
		doc.setModified(false);
	}

	return;
}

void TestsWidget::projectOpened()
{
	this->setEnabled(true);

	m_testsTreeModel->fetch(QModelIndex());

	m_testsTreeView->expandRecursively(QModelIndex(), 1);

	// Restore build path
	//
	QSettings settings;

	QString project = db()->currentProject().projectName().toLower();
	m_buildPath = settings.value("TestsTabPage/ProjectLastPath/" + project).toString();

	if (m_buildPath.isEmpty() == true)
	{
		m_buildLabel->setText(tr("Build: Not loaded"));
	}
	else
	{
		m_buildLabel->setText(tr("Build: <a href=\"%1\">%1</a>").arg(m_buildPath));
	}

	return;
}

void TestsWidget::projectClosed()
{
	m_simulator.clear();

	m_buildPath.clear();
	m_buildLabel->setText(tr("Build: Not loaded"));

	m_outputDockWidget.clear();

	m_scriptIsRunning = false;

	closeAllDocuments();
	hideEditor();

	m_openFilesTreeWidget->clear();

	this->setEnabled(false);
	return;
}

void TestsWidget::buildStarted()
{
	saveAllDocuments();

	return;
}

void TestsWidget::testsTreeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	Q_UNUSED(selected);
	Q_UNUSED(deselected);

	setTestsTreeActionsState();

	return;
}

void TestsWidget::testsTreeModelDataChanged(const QModelIndex& topLeft,
									const QModelIndex& bottomRight, const QVector<int>& roles /*= QVector<int>()*/)
{
	Q_UNUSED(topLeft);
	Q_UNUSED(bottomRight);
	Q_UNUSED(roles);

	setTestsTreeActionsState();

	return;
}

void TestsWidget::testsTreeModelReset()
{
	setTestsTreeActionsState();

	return;
}

void TestsWidget::testsTreeDoubleClicked(const QModelIndex &index)
{
	Q_UNUSED(index);

	if (m_openFileAction->isEnabled() == true)
	{
		openFile();
	}

	return;
}

void TestsWidget::openFilesClicked(const QModelIndex &index)
{
	Q_UNUSED(index);
	QTreeWidgetItem* item = m_openFilesTreeWidget->currentItem();
	if (item == nullptr)
	{
		return;
	}

	bool ok = false;
	int fileId = item->data(0, Qt::UserRole).toInt(&ok);

	if (ok == true)
	{
		setCurrentDocument(fileId);
	}
	else
	{
		Q_ASSERT(ok);
	}

	return;
}

void TestsWidget::newFile()
{
	// Get new file name
	//
	QString fileName = QInputDialog::getText(this, "New File", tr("Enter the file name:"), QLineEdit::Normal, tr("NewFile_%1.js").arg(db()->nextCounterValue()));
	if (fileName.isEmpty() == true)
	{
		return;
	}

	if (fileName.contains('.') == false)
	{
		fileName += ".js";
	}

	// Read script template
	//
	QByteArray templateScript;
	QFile rcFile{":/Simulator/ScriptSample.js"};

	if (rcFile.open(QIODevice::ReadOnly) == false)
	{
		qDebug() << "TestsTabPage::newFile: " << rcFile.errorString();
	}
	else
	{
		templateScript = rcFile.readAll();
	}

	// Create file and open it
	//
	bool result = m_testsTreeView->newFile(fileName, templateScript);
	if (result == true)
	{
		// addNewFile will select new document
		//
		openFile();
	}

	return;
}

void TestsWidget::openFile()
{
	QModelIndexList selectedIndexList = m_testsTreeView->selectedSourceRows();
	if (selectedIndexList.size() != 1)
	{
		return;
	}

	QModelIndex& mi = selectedIndexList[0];

	FileTreeModelItem* f = m_testsTreeModel->fileItem(mi);
	if (f == nullptr)
	{
		Q_ASSERT(f);
		return;
	}

	int fileId = f->fileId();

	// Check if file is already open

	if (documentIsOpen(fileId) == true)
	{
		setCurrentDocument(fileId);
		return;
	}

	// Load file

	bool readOnly = f->state() != VcsState::CheckedOut ||
					  (db()->currentUser().isAdminstrator() == false
					   && db()->currentUser().userId() != f->userId());

	std::shared_ptr<DbFile> dbFile;
	if (db()->getLatestVersion(*f, &dbFile, this) == false)
	{
		return;
	}

	// Add and select tree item

	QString itemName = f->fileName();
	if (readOnly == true)
	{
		itemName += QObject::tr(" [Read-only]");
	}

	QTreeWidgetItem* openFilesTreeWidgetItem = new QTreeWidgetItem(QStringList() << itemName);
	openFilesTreeWidgetItem->setToolTip(0, f->fileName());
	openFilesTreeWidgetItem->setData(0, Qt::UserRole, fileId);
	openFilesTreeWidgetItem->setSelected(true);

	m_openFilesTreeWidget->addTopLevelItem(openFilesTreeWidgetItem);
	m_openFilesTreeWidget->setCurrentItem(openFilesTreeWidgetItem);

	// Create document

	IdeCodeEditor* codeEditor = new IdeCodeEditor(CodeType::JavaScript, this);
	codeEditor->setText(dbFile->data());
	codeEditor->setReadOnly(readOnly);
	connect(codeEditor, &IdeCodeEditor::customContextMenuAboutToBeShown, this, &TestsWidget::setCodeEditorActionsState, Qt::DirectConnection);

	if (m_documentSeparatorAction1 == nullptr)
	{
		m_documentSeparatorAction1 = new QAction(this);
		m_documentSeparatorAction1->setSeparator(true);
	}

	if (m_documentSeparatorAction2 == nullptr)
	{
		m_documentSeparatorAction2 = new QAction(this);
		m_documentSeparatorAction2->setSeparator(true);
	}

	QList<QAction*> customMenuActions;
	customMenuActions.push_back(m_checkOutCurrentDocumentAction);
	customMenuActions.push_back(m_checkInCurrentDocumentAction);
	customMenuActions.push_back(m_undoChangesCurrentDocumentAction);
	customMenuActions.push_back(m_documentSeparatorAction1);
	customMenuActions.push_back(m_runTestCurrentDocumentAction);
	customMenuActions.push_back(m_documentSeparatorAction2);
	customMenuActions.push_back(m_saveCurrentDocumentAction);
	customMenuActions.push_back(m_closeCurrentDocumentAction);

	codeEditor->setCustomMenuActions(customMenuActions);

	connect(codeEditor, &IdeCodeEditor::textChanged, this, &TestsWidget::textChanged);
	connect(codeEditor, &IdeCodeEditor::cursorPositionChanged, this, &TestsWidget::cursorPositionChanged);
	connect(codeEditor, &IdeCodeEditor::closeKeyPressed, this, &TestsWidget::onCloseKeyPressed);
	connect(codeEditor, &IdeCodeEditor::saveKeyPressed, this, &TestsWidget::onSaveKeyPressed);
	connect(codeEditor, &IdeCodeEditor::ctrlTabKeyPressed, this, &TestsWidget::onCtrlTabKeyPressed);
	m_editorLayout->addWidget(codeEditor);

	m_openDocumentsCombo->blockSignals(true);
	m_openDocumentsCombo->addItem(itemName, fileId);
	m_openDocumentsCombo->model()->sort(0, Qt::AscendingOrder);
	m_openDocumentsCombo->blockSignals(false);

	// Add a document

	TestTabPageDocument document(f->fileName(), codeEditor, openFilesTreeWidgetItem);
	m_openDocuments.insert( std::map< int, TestTabPageDocument >::value_type ( fileId, document ) );

	// Open file in editor

	setCurrentDocument(fileId);

	return;
}


void TestsWidget::newFolder()
{
	QString folderName = QInputDialog::getText(this, "New Folder", tr("Enter the folder name:"), QLineEdit::Normal, tr("FOLDER_%1").arg(db()->nextCounterValue()));
	if (folderName.isEmpty() == true)
	{
		return;
	}

	m_testsTreeView->addFolder(folderName);

	return;
}

void TestsWidget::renameFile()
{
	// Save modified files
	//
	QModelIndexList selectedIndexList = m_testsTreeView->selectedSourceRows();
	if (selectedIndexList.size() != 1)
	{
		return;
	}

	FileTreeModelItem* f = m_testsTreeModel->fileItem(selectedIndexList[0]);
	if (f == nullptr)
	{
		Q_ASSERT(f);
		return;
	}

	int fileId = -1;

	if (documentIsOpen(f->fileId()) == true)
	{
		fileId = f->fileId();

		if (documentIsModified(f->fileId()) == true)
		{
			saveDocument(f->fileId());
		}
	}
	// rename


	m_testsTreeView->renameFile();

	// Set new file name to document

	if (documentIsOpen(fileId) == true)
	{
		TestTabPageDocument& document = m_openDocuments.at(f->fileId());

		DbFileInfo fi;
		if (db()->getFileInfo(fileId, &fi, this) == false)
		{
			QMessageBox::critical(this, "Error", "Get file information error!");
			return;
		}

		document.setFileName(fi.fileName());

		updateOpenDocumentInfo(fileId);
	}

	return;
}

void TestsWidget::checkInSelectedFiles()
{
	// Save modified files
	//
	QModelIndexList selectedIndexList = m_testsTreeView->selectedSourceRows();
	for (QModelIndex& mi : selectedIndexList)
	{
		FileTreeModelItem* f = m_testsTreeModel->fileItem(mi);
		if (f == nullptr)
		{
			Q_ASSERT(f);
			return;
		}

		if (documentIsOpen(f->fileId()) == true && documentIsModified(f->fileId()) == true)
		{
			saveDocument(f->fileId());
		}
	}

	m_testsTreeView->checkInSelectedFiles();

	// Some folders could be deleted, so close their documents

	closeDocumentsForDeletedFiles();

	// Set editors to read-only
	//
	selectedIndexList = m_testsTreeView->selectedSourceRows();
	for (QModelIndex& mi : selectedIndexList)
	{
		FileTreeModelItem* f = m_testsTreeModel->fileItem(mi);
		if (f == nullptr)
		{
			Q_ASSERT(f);
			return;
		}

		if (documentIsOpen(f->fileId()) == true)
		{
			setDocumentReadOnly(f->fileId(), f->state() == VcsState::CheckedIn);
		}
	}

	return;
}

void TestsWidget::checkOutSelectedFiles()
{
	m_testsTreeView->checkOutSelectedFiles();

	// Set editors to editable
	//
	QModelIndexList selectedIndexList = m_testsTreeView->selectedSourceRows();
	for (QModelIndex& mi : selectedIndexList)
	{
		FileTreeModelItem* f = m_testsTreeModel->fileItem(mi);
		if (f == nullptr)
		{
			Q_ASSERT(f);
			return;
		}

		int fileId = f->fileId();

		if (documentIsOpen(fileId) == true)
		{
			// Re-read file info and contents

			DbFileInfo fi;
			if (db()->getFileInfo(fileId, &fi, this) == false)
			{
				QMessageBox::critical(this, "Error", "Get file information error!");
				return;
			}

			std::shared_ptr<DbFile> dbFile;
			if (db()->getLatestVersion(fi, &dbFile, this) == false)
			{
				return;
			}

			if (documentIsOpen(fileId) == false)
			{
				Q_ASSERT(false);
				return;
			}

			TestTabPageDocument& document = m_openDocuments.at(fileId);

			document.setFileName(fi.fileName());
			document.setModified(false);

			IdeCodeEditor* codeEditor = document.codeEditor();
			if (codeEditor == nullptr)
			{
				Q_ASSERT(codeEditor);
				return;
			}
			codeEditor->setText(dbFile->data());

			setDocumentReadOnly(fileId, fi.state() == VcsState::CheckedIn);
		}
	}
}

void TestsWidget::undoChangesSelectedFiles()
{
	// Remember the list of open documents to undo

	QList<int> openUndoDocuments;

	QModelIndexList selectedIndexList = m_testsTreeView->selectedSourceRows();
	for (QModelIndex& mi : selectedIndexList)
	{
		FileTreeModelItem* f = m_testsTreeModel->fileItem(mi);
		assert(f);

		if (f->state() == VcsState::CheckedOut)
		{
			if (documentIsOpen(f->fileId()) == true)
			{
				openUndoDocuments.push_back(f->fileId());
			}
		}
	}

	if (m_testsTreeView->undoChangesSelectedFiles() == false)
	{
		return;
	}

	// Close documents that are deleted and re-read other

	selectedIndexList = m_testsTreeView->selectedSourceRows();

	for (QModelIndex& mi : selectedIndexList)
	{
		FileTreeModelItem* f = m_testsTreeModel->fileItem(mi);
		assert(f);

		if (openUndoDocuments.contains(f->fileId()) == true)
		{
			// Undo operation was performed, re-read information

			std::shared_ptr<DbFile> dbFile;

			if (db()->getLatestVersion(*f, &dbFile, this) == false)
			{
				return;
			}

			if (documentIsOpen(f->fileId()) == false)
			{
				Q_ASSERT(false);
				return;
			}

			TestTabPageDocument& document = m_openDocuments.at(f->fileId());

			IdeCodeEditor* codeEditor = document.codeEditor();
			if (codeEditor == nullptr)
			{
				Q_ASSERT(codeEditor);
				return;
			}

			codeEditor->setText(dbFile->data());
			document.setModified(false);

			setDocumentReadOnly(f->fileId(), f->state() == VcsState::CheckedIn);

			openUndoDocuments.removeOne(f->fileId());
		}
	}

	// And now close documents that were removed

	for (int undoOpenDocument : openUndoDocuments)
	{
		closeDocument(undoOpenDocument, true/*force*/);
	}

	return;
}

void TestsWidget::deleteSelectedFiles()
{
	int selectedFilesCount = 0;
	int selectedFoldersCount = 0;

	QStringList deleteFiles;

	QModelIndexList selectedIndexList = m_testsTreeView->selectedSourceRows();
	for (QModelIndex& mi : selectedIndexList)
	{
		if (mi.parent().isValid() == false)
		{
			// Forbid root items deleting
			//
			continue;
		}

		FileTreeModelItem* f = m_testsTreeModel->fileItem(mi);
		assert(f);

		if (f->isFolder() == true)
		{
			selectedFoldersCount++;
		}
		else
		{
			selectedFilesCount++;
		}

		deleteFiles.push_back(f->fileName());
	}

	QString filesText;

	if (selectedFilesCount > 0)
	{
		filesText = tr("file(s)");
	}

	if (selectedFoldersCount > 0)
	{
		if (selectedFilesCount > 0)
		{
			filesText += tr("/");
		}

		filesText += tr("folder(s)");
	}

	// Ask user to confirm operation
	//
	QMessageBox mb(this);

	mb.setWindowTitle(qApp->applicationName());
	mb.setText(tr("Are you sure you want to delete selected %1 %2?").arg(selectedFilesCount + selectedFoldersCount).arg(filesText));
	mb.setInformativeText(tr("If files have not been checked in before they will be deleted permanently.\nIf files were checked in at least one time they will be marked as deleted, to confirm operation perform Check In."));

	QString detailedText = deleteFiles.join('\n');
	mb.setDetailedText(detailedText.trimmed());

	mb.setIcon(QMessageBox::Question);
	mb.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

	if (int mbResult = mb.exec();
		mbResult == QMessageBox::Cancel)
	{
		return;
	}

	// Close documents before deleting

	for (QModelIndex& mi : selectedIndexList)
	{
		if (mi.parent().isValid() == false)
		{
			// Forbid root items deleting
			//
			continue;
		}

		FileTreeModelItem* f = m_testsTreeModel->fileItem(mi);
		assert(f);

		if (documentIsOpen(f->fileId()) == true)
		{
			closeDocument(f->fileId(), true/*force*/);
		}
	}

	m_testsTreeView->deleteFile();
}

void TestsWidget::moveSelectedFiles()
{
	// Close documents before deleting

	QModelIndexList selectedIndexList = m_testsTreeView->selectedSourceRows();
	for (QModelIndex& mi : selectedIndexList)
	{
		if (mi.parent().isValid() == false)
		{
			// Forbid root items deleting
			//
			continue;
		}

		FileTreeModelItem* f = m_testsTreeModel->fileItem(mi);
		assert(f);

		if (documentIsOpen(f->fileId()) == true)
		{
			QMessageBox::critical(this, qAppName(), tr("Can't move file %1, as it is opened for edit. Close schema and repeat operation.").arg(f->fileName()));
			return;

		}
	}

	m_testsTreeView->moveFile(db()->testsFileId());
}

void TestsWidget::refreshFileTree()
{
	m_testsTreeView->refreshFileTree();

	setTestsTreeActionsState();

	return;
}

void TestsWidget::runAllTestFiles()
{
	saveUnsavedTests();

	// Find all files with scripts extention
	DbFileTree testsTree;

	bool ok = db()->getFileListTree(&testsTree, db()->testsFileId(), true, parentWidget());
	if (ok == false)
	{
		return;
	}

	std::vector<DbFileInfo> testsFiles = testsTree.toVectorIf([this](const DbFileInfo& f)
	{
		return isEditableExtension(f.fileName());
	});

	if (testsFiles.empty() == true)
	{
		QMessageBox::critical(parentWidget(), qAppName(), tr("No tests files found!"));
		return;
	}

	if (m_buildPath.isEmpty() == true)
	{
		selectBuild();

		if (m_buildPath.isEmpty() == true)
		{
			return;
		}
	}

	runSimTests(m_buildPath, testsFiles);
}

void TestsWidget::runSelectedTestFiles()
{
	std::vector<int> fileIds;

	QModelIndexList selectedIndexList = m_testsTreeView->selectedSourceRows();
	for (QModelIndex& mi : selectedIndexList)
	{
		if (mi.parent().isValid() == false)
		{
			continue;
		}

		FileTreeModelItem* f = m_testsTreeModel->fileItem(mi);
		assert(f);

		// Check file extension
		//
		if (isEditableExtension(f->fileName()) == true)
		{
			fileIds.push_back(f->fileId());
		}
	}

	if (fileIds.empty() == true)
	{
		return;
	}

	runTests(fileIds);
	return;
}

void TestsWidget::filterChanged()
{
	QString filterText = m_filterLineEdit->text().trimmed();

	m_filterResetButton->setEnabled(filterText.isEmpty() == false);

	if (filterText.isEmpty() == true)
	{
		m_filterSetButton->setStyleSheet("");
	}
	else
	{
		m_filterSetButton->setStyleSheet("font: bold;");
	}

	m_testsTreeView->setFileNameFilter(filterText);

	// Save completer
	//
	QStringList completerStringList = QSettings{}.value("TestsTabPage/FilterCompleter").toStringList();

	if (filterText.isEmpty() == false &&
		completerStringList.contains(filterText, Qt::CaseInsensitive) == false)
	{
		completerStringList.push_back(filterText);
		QSettings{}.setValue("TestsTabPage/FilterCompleter", completerStringList);

		QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_filterCompleter->model());
		Q_ASSERT(completerModel);

		if (completerModel != nullptr)
		{
			completerModel->setStringList(completerStringList);
		}
	}

	return;
}

void TestsWidget::textChanged()
{
	if (documentIsOpen(m_currentFileId) == false)
	{
		Q_ASSERT(false);
		return;
	}

	TestTabPageDocument& document = m_openDocuments.at(m_currentFileId);

	if (document.modified() == false)
	{
		// Mark document as modified
		//
		document.setModified(true);
		updateOpenDocumentInfo(m_currentFileId);
	}

	return;

}

void TestsWidget::cursorPositionChanged(int line, int index)
{
	if (m_cursorPosButton == nullptr)
	{
		Q_ASSERT(m_cursorPosButton);
		return;
	}

	m_cursorPosButton->setText(tr(" Line: %1  Col: %2 ").arg(line + 1).arg(index + 1));

	return;
}

void TestsWidget::checkInCurrentFile()
{
	std::vector<int> fileIds;
	fileIds.push_back(m_currentFileId);
	checkInDocument(fileIds);
	return;
}

void TestsWidget::checkOutCurrentFile()
{
	std::vector<int> fileIds;
	fileIds.push_back(m_currentFileId);
	checkOutDocument(fileIds);
	return;
}

void TestsWidget::undoChangesCurrentFile()
{
	std::vector<int> fileIds;
	fileIds.push_back(m_currentFileId);
	undoChangesDocument(fileIds);
	return;
}

void TestsWidget::saveCurrentFile()
{
	saveDocument(m_currentFileId);
	return;
}

void TestsWidget::closeCurrentFile()
{
	closeDocument(m_currentFileId, false/*force*/);
	return;
}

void TestsWidget::runTestCurrentFile()
{
	if (m_openDocuments.empty() == true || m_currentFileId == -1)
	{
		Q_ASSERT(false);
		return;
	}

	std::vector<int> fileIds;
	fileIds.push_back(m_currentFileId);
	runTests(fileIds);
	return;
}

void TestsWidget::stopTests()
{
	stopSimTests();
}

void TestsWidget::onGoToLine()
{
	if (documentIsOpen(m_currentFileId) == false)
	{
		Q_ASSERT(false);
		return;
	}

	TestTabPageDocument& document = m_openDocuments.at(m_currentFileId);

	if (document.codeEditor() == nullptr)
	{
		Q_ASSERT(false);
		return;
	}

	int line = 0;
	int index = 0;
	document.codeEditor()->getCursorPosition(&line, &index);

	bool ok = false;

	int newLine = QInputDialog::getInt(this, "Go to Line", tr("Enter line number:"), line + 1, 1, 2147483647, 1, &ok);
	if (ok == false)
	{
		return;
	}

	IdeCodeEditor* codeEditor = document.codeEditor();
	if (codeEditor == nullptr)
	{
		Q_ASSERT(codeEditor);
		return;
	}

	int maxLine = codeEditor->lines();
	if (newLine > maxLine)
	{
		newLine  = maxLine;
	}

	codeEditor->setCursorPosition(newLine - 1, 0);
	codeEditor->activateEditor();

	return;
}

void TestsWidget::openFilesMenuRequested(const QPoint& pos)
{
	Q_UNUSED(pos);

	QList<QTreeWidgetItem*> selectedItems = m_openFilesTreeWidget->selectedItems();
	if (selectedItems.empty() == true)
	{
		return;
	}

	m_checkInOpenDocumentAction->setEnabled(false);
	m_checkOutOpenDocumentAction->setEnabled(false);
	m_undoChangesOpenDocumentAction->setEnabled(false);
	m_saveOpenDocumentAction->setEnabled(false);

	for (QTreeWidgetItem* item : selectedItems)
	{
		bool ok = false;
		int fileId = item->data(0, Qt::UserRole).toInt(&ok);
		if (ok == false)
		{
			Q_ASSERT(false);
			continue;
		}

		if (documentIsOpen(fileId) == false)
		{
			Q_ASSERT(false);
			return;
		}

		const TestTabPageDocument& document = m_openDocuments.at(m_currentFileId);

		if (document.modified() == true)
		{
			m_saveOpenDocumentAction->setEnabled(true);
		}

		DbFileInfo fi;
		if (db()->getFileInfo(fileId, &fi, this) == false)
		{
			QMessageBox::critical(this, "Error", "Get file information error!");
			return;
		}

		if (fi.state() == VcsState::CheckedOut &&
			(fi.userId() == db()->currentUser().userId() || db()->currentUser().isAdminstrator()))
		{
			m_checkInOpenDocumentAction->setEnabled(true);
			m_undoChangesOpenDocumentAction->setEnabled(true);
		}

		if (fi.state() == VcsState::CheckedIn)
		{
			m_checkOutOpenDocumentAction->setEnabled(true);
		}
	}

	QMenu menu;
	menu.addAction(m_checkInOpenDocumentAction);
	menu.addAction(m_checkOutOpenDocumentAction);
	menu.addAction(m_undoChangesOpenDocumentAction);
	menu.addSeparator();
	menu.addAction(m_runTestOpenDocumentAction);
	menu.addSeparator();
	menu.addAction(m_saveOpenDocumentAction);
	menu.addAction(m_closeOpenDocumentAction);

	menu.exec(QCursor::pos());
}


void TestsWidget::checkInOpenFile()
{
	std::vector<int> fileIds;

	QList<QTreeWidgetItem*> selectedItems = m_openFilesTreeWidget->selectedItems();

	for (QTreeWidgetItem* item : selectedItems)
	{
		bool ok = false;
		int fileId = item->data(0, Qt::UserRole).toInt(&ok);
		if (ok == false)
		{
			Q_ASSERT(false);
			continue;
		}

		DbFileInfo fi;
		if (db()->getFileInfo(fileId, &fi, this) == false)
		{
			QMessageBox::critical(this, "Error", "Get file information error!");
			return;
		}

		if (fi.state() == VcsState::CheckedOut &&
			(fi.userId() == db()->currentUser().userId() || db()->currentUser().isAdminstrator()))
		{
			fileIds.push_back(fileId);
		}
	}

	checkInDocument(fileIds);
}

void TestsWidget::checkOutOpenFile()
{
	std::vector<int> fileIds;

	QList<QTreeWidgetItem*> selectedItems = m_openFilesTreeWidget->selectedItems();

	for (QTreeWidgetItem* item : selectedItems)
	{
		bool ok = false;
		int fileId = item->data(0, Qt::UserRole).toInt(&ok);
		if (ok == false)
		{
			Q_ASSERT(false);
			continue;
		}

		DbFileInfo fi;
		if (db()->getFileInfo(fileId, &fi, this) == false)
		{
			QMessageBox::critical(this, "Error", "Get file information error!");
			return;
		}

		if (fi.state() == VcsState::CheckedIn)
		{
			fileIds.push_back(fileId);
		}
	}

	checkOutDocument(fileIds);
}

void TestsWidget::undoChangesOpenFile()
{
	std::vector<int> fileIds;

	QList<QTreeWidgetItem*> selectedItems = m_openFilesTreeWidget->selectedItems();

	for (QTreeWidgetItem* item : selectedItems)
	{
		bool ok = false;
		int fileId = item->data(0, Qt::UserRole).toInt(&ok);
		if (ok == false)
		{
			Q_ASSERT(false);
			continue;
		}

		DbFileInfo fi;
		if (db()->getFileInfo(fileId, &fi, this) == false)
		{
			QMessageBox::critical(this, "Error", "Get file information error!");
			return;
		}

		if (fi.state() == VcsState::CheckedOut &&
			(fi.userId() == db()->currentUser().userId() || db()->currentUser().isAdminstrator()))
		{
			fileIds.push_back(fileId);
		}
	}

	undoChangesDocument(fileIds);
}

void TestsWidget::saveOpenFile()
{
	std::vector<int> fileIds;

	QList<QTreeWidgetItem*> selectedItems = m_openFilesTreeWidget->selectedItems();

	for (QTreeWidgetItem* item : selectedItems)
	{
		bool ok = false;
		int fileId = item->data(0, Qt::UserRole).toInt(&ok);
		if (ok == false)
		{
			Q_ASSERT(false);
			continue;
		}

		if (documentIsOpen(fileId) == false)
		{
			Q_ASSERT(false);
			return;
		}

		const TestTabPageDocument& document = m_openDocuments.at(fileId);

		if (document.modified() == true)
		{
			fileIds.push_back(fileId);
		}
	}

	for (int fileId : fileIds)
	{
		saveDocument(fileId);
	}
}

void TestsWidget::closeOpenFile()
{
	std::vector<int> fileIds;

	QList<QTreeWidgetItem*> selectedItems = m_openFilesTreeWidget->selectedItems();

	for (QTreeWidgetItem* item : selectedItems)
	{
		bool ok = false;
		int fileId = item->data(0, Qt::UserRole).toInt(&ok);
		if (ok == false)
		{
			Q_ASSERT(false);
			continue;
		}

		fileIds.push_back(fileId);
	}

	for (int fileId : fileIds)
	{
		closeDocument(fileId, false/*force*/);
	}

	return;
}

void TestsWidget::runTestOpenFile()
{
	std::vector<int> fileIds;

	QList<QTreeWidgetItem*> selectedItems = m_openFilesTreeWidget->selectedItems();

	for (QTreeWidgetItem* item : selectedItems)
	{
		bool ok = false;
		int fileId = item->data(0, Qt::UserRole).toInt(&ok);
		if (ok == false)
		{
			Q_ASSERT(false);
			continue;
		}

		fileIds.push_back(fileId);
	}

	if (fileIds.empty() == true)
	{
		return;
	}

	runTests(fileIds);
	return;
}


void TestsWidget::onSaveKeyPressed()
{
	if (documentIsOpen(m_currentFileId) == true)
	{
		const TestTabPageDocument& document = m_openDocuments.at(m_currentFileId);

		if (document.modified() == true)
		{
			saveDocument(m_currentFileId);
		}
	}
}

void TestsWidget::onCloseKeyPressed()
{
	if (documentIsOpen(m_currentFileId) == true)
	{
		closeDocument(m_currentFileId, false/*force*/);
	}
}

void TestsWidget::onCtrlTabKeyPressed()
{
	if (documentIsOpen(m_currentFileId) == true)
	{
		const TestTabPageDocument& document = m_openDocuments.at(m_currentFileId);

		QTreeWidgetItem* openFilesTreeWidgetItem = document.openFilesTreeWidgetItem();
		if (openFilesTreeWidgetItem == nullptr)
		{
			Q_ASSERT(openFilesTreeWidgetItem);
			return;
		}

		int currentIndex = m_openFilesTreeWidget->indexOfTopLevelItem(openFilesTreeWidgetItem);

		int openIndex = 0;

		if (currentIndex < m_openFilesTreeWidget->topLevelItemCount() - 1)
		{
			openIndex = currentIndex + 1;
		}

		QTreeWidgetItem* openItem = m_openFilesTreeWidget->topLevelItem(openIndex);
		if (openItem == nullptr)
		{
			Q_ASSERT(openItem);
			return;
		}

		bool ok = false;
		int fileId = openItem->data(0, Qt::UserRole).toInt(&ok);

		if (ok == true)
		{
			setCurrentDocument(fileId);
		}
		else
		{
			Q_ASSERT(ok);
		}
	}
}

bool TestsWidget::documentIsModified(int fileId) const
{
	if (documentIsOpen(fileId) == false)
	{
		return false;
	}

	const TestTabPageDocument& document = m_openDocuments.at(fileId);
	return document.modified();
}

void TestsWidget::checkInDocument(std::vector<int> fileIds)
{
	if (fileIds.empty() == true)
	{
		return;
	}

	for (int fileId : fileIds)
	{
		if (documentIsOpen(fileId) == false)
		{
			Q_ASSERT(false);
			return;
		}

		// Save modified files
		//
		if (documentIsOpen(fileId) == true && documentIsModified(fileId) == true)
		{
			saveDocument(fileId);
		}

	}

	std::vector<int> deletedFileIds;

	m_testsTreeView->checkInFilesById(fileIds, &deletedFileIds);

	for (int fileId : fileIds)
	{
		if (std::find(deletedFileIds.begin(), deletedFileIds.end(), fileId) != deletedFileIds.end())
		{
			// File was deleted
			closeDocument(fileId, true/*force*/);
		}
		else
		{
			DbFileInfo fi;
			if (db()->getFileInfo(fileId, &fi, this) == false)
			{
				QMessageBox::critical(this, "Error", "Get file information error!");
				return;
			}

			setDocumentReadOnly(fileId, fi.state() == VcsState::CheckedIn);
		}
	}

	return;
}

void TestsWidget::checkOutDocument(std::vector<int> fileIds)
{
	if (fileIds.empty() == true)
	{
		return;
	}

	m_testsTreeView->checkOutFilesById(fileIds);

	for (int fileId : fileIds)
	{
		// Re-read file info and contents

		DbFileInfo fi;
		if (db()->getFileInfo(fileId, &fi, this) == false)
		{
			QMessageBox::critical(this, "Error", "Get file information error!");
			return;
		}

		std::shared_ptr<DbFile> dbFile;
		if (db()->getLatestVersion(fi, &dbFile, this) == false)
		{
			return;
		}

		if (documentIsOpen(fileId) == false)
		{
			Q_ASSERT(false);
			return;
		}

		TestTabPageDocument& document = m_openDocuments.at(fileId);

		document.setFileName(fi.fileName());
		document.setModified(false);

		IdeCodeEditor* codeEditor = document.codeEditor();
		if (codeEditor == nullptr)
		{
			Q_ASSERT(codeEditor);
			return;
		}
		codeEditor->setText(dbFile->data());

		setDocumentReadOnly(fileId, fi.state() == VcsState::CheckedIn);

	}

	return;
}

void TestsWidget::undoChangesDocument(std::vector<int> fileIds)
{
	if (fileIds.empty() == true)
	{
		return;
	}

	std::vector<int> deletedFileIds;

	if (m_testsTreeView->undoChangesFilesById(fileIds, &deletedFileIds) == false)
	{
		return;
	}

	for (int fileId : fileIds)
	{
		if (std::find(deletedFileIds.begin(), deletedFileIds.end(), fileId) != deletedFileIds.end())
		{
			// File was deleted
			closeDocument(fileId, true/*force*/);
		}
		else
		{
			// Re-read file info and contents

			DbFileInfo fi;
			if (db()->getFileInfo(fileId, &fi, this) == false)
			{
				QMessageBox::critical(this, "Error", "Get file information error!");
				return;
			}

			std::shared_ptr<DbFile> dbFile;
			if (db()->getLatestVersion(fi, &dbFile, this) == false)
			{
				return;
			}

			if (documentIsOpen(fileId) == false)
			{
				Q_ASSERT(false);
				return;
			}

			TestTabPageDocument& document = m_openDocuments.at(fileId);

			document.setFileName(fi.fileName());
			document.setModified(false);

			IdeCodeEditor* codeEditor = document.codeEditor();
			if (codeEditor == nullptr)
			{
				Q_ASSERT(codeEditor);
				return;
			}
			codeEditor->setText(dbFile->data());

			setDocumentReadOnly(fileId, fi.state() == VcsState::CheckedIn);
		}
	}
}

void TestsWidget::setCurrentDocument(int fileId)
{
	if (m_currentFileId == fileId)
	{
		return;
	}

	if (documentIsOpen(fileId) == false)
	{
		Q_ASSERT(false);
		return;
	}

	// Show Editor
	m_editorToolBar->setVisible(true);

	// Hide current editor

	for (auto& it : m_openDocuments)
	{
		int docFileId = it.first;
		TestTabPageDocument& document = it.second;

		IdeCodeEditor* codeEditor = document.codeEditor();
		if (codeEditor == nullptr)
		{
			Q_ASSERT(codeEditor);
			return;
		}

		if (docFileId != fileId && codeEditor->isVisible())
		{
			codeEditor->setVisible(false);
		}
	}

	// Show new editor

	for (auto& it : m_openDocuments)
	{
		int docFileId = it.first;
		TestTabPageDocument& document = it.second;

		IdeCodeEditor* codeEditor = document.codeEditor();
		if (codeEditor == nullptr)
		{
			Q_ASSERT(codeEditor);
			return;
		}

		if (docFileId == fileId)
		{
			codeEditor->setVisible(true);
			codeEditor->activateEditor();

			if (m_cursorPosButton == nullptr)
			{
				Q_ASSERT(m_cursorPosButton);
				return;
			}

			int line = 0;
			int index = 0;
			codeEditor->getCursorPosition(&line, &index);

			m_cursorPosButton->setText(tr(" Line: %1  Col: %2").arg(line + 1).arg(index + 1));
		}
	}

	// Set current document to new

	m_currentFileId = fileId;

	// Select Open files tree widget item

	m_openFilesTreeWidget->clearSelection();

	const TestTabPageDocument& openFile = m_openDocuments.at(fileId);

	QTreeWidgetItem* openFilesTreeWidgetItem = openFile.openFilesTreeWidgetItem();
	if (openFilesTreeWidgetItem == nullptr)
	{
		Q_ASSERT(openFilesTreeWidgetItem);
		return;
	}

	openFilesTreeWidgetItem->setSelected(true);

	// Select combo box item

	int comboIndex = m_openDocumentsCombo->findData(fileId, Qt::UserRole);
	if (comboIndex == -1)
	{
		Q_ASSERT(false);
	}
	else
	{
		m_openDocumentsCombo->blockSignals(true);
		m_openDocumentsCombo->setCurrentIndex(comboIndex);
		m_openDocumentsCombo->blockSignals(false);
	}

	//

	setRunTestsActionsState();

	return;
}

void TestsWidget::setDocumentReadOnly(int fileId, bool readOnly)
{
	if (documentIsOpen(fileId) == false)
	{
		Q_ASSERT(false);
		return;
	}

	TestTabPageDocument& document = m_openDocuments.at(fileId);

	IdeCodeEditor* codeEditor = document.codeEditor();
	if (codeEditor == nullptr)
	{
		Q_ASSERT(codeEditor);
		return;
	}

	codeEditor->setReadOnly(readOnly);

	updateOpenDocumentInfo(fileId);

	return;
}

void TestsWidget::openDocumentsComboTextChanged(int index)
{
	if (index < 0 || index >= m_openDocumentsCombo->count())
	{
		return;
	}

	bool ok = false;
	int fileId = m_openDocumentsCombo->itemData(index).toInt(&ok);

	if (ok == true)
	{
		setCurrentDocument(fileId);
	}
	else
	{
		Q_ASSERT(ok);
	}

	return;
}

void TestsWidget::closeDocumentsForDeletedFiles()
{
	std::vector<int> documentsToClose;

	for (auto& it : m_openDocuments)
	{
		int fileId = it.first;

		QModelIndexList matched = m_testsTreeModel->match(m_testsTreeModel->childIndex(0, 0, QModelIndex()),
														  Qt::UserRole,
														  QVariant::fromValue(fileId),
														  1,
														  Qt::MatchExactly | Qt::MatchRecursive);

		if (matched.size() == 0)
		{
			documentsToClose.push_back(fileId);
		}
	}

	for (int fileId : documentsToClose)
	{
		closeDocument(fileId, true/*force*/);
	}
}

void TestsWidget::compareObject(DbChangesetObject object, CompareData compareData)
{
	// Can compare only files which are EquipmentObjects
	//
	if (object.isFile() == false)
	{
		return;
	}

	// Check file extension
	//
	if (isEditableExtension(object.name()) == false)
	{
		return;
	}

	// Get vesrions from the project database
	//
	QString source;

	switch (compareData.sourceVersionType)
	{
	case CompareVersionType::Changeset:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getSpecificCopy(file, compareData.sourceChangeset, &outFile, this);
			if (ok == true)
			{
				source = QString::fromUtf8(outFile->data());
			}
		}
		break;
	case CompareVersionType::Date:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getSpecificCopy(file, compareData.sourceDate, &outFile, this);
			if (ok == true)
			{
				source = QString::fromUtf8(outFile->data());
			}
		}
		break;
	case CompareVersionType::LatestVersion:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getLatestVersion(file, &outFile, this);
			if (ok == true)
			{
				source = QString::fromUtf8(outFile->data());
			}
		}
		break;
		break;
	default:
		assert(false);
	}

	if (source == nullptr)
	{
		return;
	}

	// Get target file version
	//
	QString target = nullptr;

	switch (compareData.targetVersionType)
	{
	case CompareVersionType::Changeset:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getSpecificCopy(file, compareData.targetChangeset, &outFile, this);
			if (ok == true)
			{
				target = QString::fromUtf8(outFile->data());
			}
		}
		break;
	case CompareVersionType::Date:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getSpecificCopy(file, compareData.targetDate, &outFile, this);
			if (ok == true)
			{
				target = QString::fromUtf8(outFile->data());
			}
		}
		break;
	case CompareVersionType::LatestVersion:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getLatestVersion(file, &outFile, this);
			if (ok == true)
			{
				target = QString::fromUtf8(outFile->data());
			}
		}
		break;
	default:
		assert(false);
	}

	if (target == nullptr)
	{
		return;
	}

	// Compare
	//
	ComparePropertyObjectDialog::showDialog(object, compareData, source, target, this);

	return;
}

void TestsWidget::selectBuild()
{
	QString project = db()->currentProject().projectName().toLower();

	SimSelectBuildDialog d(project, m_buildPath, this);
	int result = d.exec();

	if (result == QDialog::Accepted)
	{
		m_buildPath = d.resultBuildPath();
		m_buildLabel->setText(tr("Build: <a href=\"%1\">%1</a>").arg(m_buildPath));

		// Save build path
		//
		QSettings settings;
		settings.setValue("TestsTabPage/ProjectLastPath/" + project, m_buildPath);
	}
}

void TestsWidget::runSimTests(const QString& buildPath, const std::vector<DbFileInfo>& files)
{
	if (files.empty() == true)
	{
		return;
	}

	if (m_simulator.control().isRunning() == true)
	{
		return;
	}

	m_outputDockWidget.setVisible(true);

	m_outputDockWidget.clear();

	std::vector<std::shared_ptr<DbFile>> latestFiles;
	bool ok = db()->getLatestVersion(files, &latestFiles, this);

	DbProjectProperties projectProperties;
	ok &= db()->getProjectProperties(&projectProperties, this);

	if (ok == false)
	{
		return;
	}

	// Load project in the main thread
	//
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	ok = m_simulator.load(buildPath);
	QApplication::restoreOverrideCursor();

	if (ok == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Cannot open project for simultaion. For details see Output window."));
		return;
	}

	// Run simualtion for all LogicModule(s)
	//
	m_simulator.control().setRunList({});

	// Start tests
	//
	std::vector<Sim::SimScriptItem> scripts;
	scripts.reserve(latestFiles.size());

	for (const std::shared_ptr<DbFile>& file : latestFiles)
	{
		scripts.emplace_back(Sim::SimScriptItem{file->data(), file->fileName()});
	}

	m_simulator.runScripts(scripts, projectProperties.simTestsTimeout());

	return;
}

void TestsWidget::stopSimTests()
{
	m_simulator.stopScript();
}

void TestsWidget::scriptStarted()
{
	m_scriptIsRunning = true;

	setRunTestsActionsState();
}

void TestsWidget::scriptFinished()
{
	m_scriptIsRunning = false;

	setRunTestsActionsState();
}

void TestsWidget::createToolbar()
{
	m_testsToolbar = addToolBar("Toolbar");
	m_testsToolbar->setObjectName("TestsToolBar");

	m_testsToolbar->setMovable(false);
	m_testsToolbar->toggleViewAction()->setDisabled(true);

	return;
}

void TestsWidget::createTestsDock()
{
	// Tests tree and model
	//

	QWidget* testsWidget = new QWidget();
	QVBoxLayout* testsLayout = new QVBoxLayout(testsWidget);
	testsLayout->setContentsMargins(6, 0, 6, 6);

	m_testsTreeModel = new TestsFileTreeModel(db(), DbFileInfo::fullPathToFileName(Db::File::TestsFileName), this, this);

#ifdef _DEBUG
	[[maybe_unused]]QAbstractItemModelTester* modelTester = new QAbstractItemModelTester(m_testsTreeModel,
																	 QAbstractItemModelTester::FailureReportingMode::Fatal,
																		 this);
#endif

	std::vector<FileTreeModel::Columns> columns;
	columns.push_back(FileTreeModel::Columns::FileNameColumn);
	columns.push_back(FileTreeModel::Columns::FileStateColumn);
	columns.push_back(FileTreeModel::Columns::FileUserColumn);
	//columns.push_back(FileTreeModel::Columns::CustomColumnIndex);
	m_testsTreeModel->setColumns(columns);

	m_testsTreeView = new FileTreeView(db(), m_testsTreeModel);
	m_testsTreeView->setSortingEnabled(true);
	m_testsTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
	connect(m_testsTreeView->header(), &QHeaderView::sortIndicatorChanged, [this](int index, Qt::SortOrder order)
	{
		m_testsTreeView->sortByColumn(index, order);
	});
	m_testsTreeView->sortByColumn(0, Qt::AscendingOrder);

	testsLayout->addWidget(m_testsTreeView);

	// Filter widgets
	//
	QHBoxLayout* filterLayout = new QHBoxLayout();

	m_filterLineEdit = new QLineEdit();
	m_filterLineEdit->setPlaceholderText(tr("Filter File Name"));
	m_filterLineEdit->setClearButtonEnabled(true);

	connect(m_filterLineEdit, &QLineEdit::returnPressed, this, &TestsWidget::filterChanged);
	filterLayout->addWidget(m_filterLineEdit);

	m_filterSetButton = new QPushButton(tr("Filter"));
	connect(m_filterSetButton, &QPushButton::clicked, this, &TestsWidget::filterChanged);
	filterLayout->addWidget(m_filterSetButton);

	m_filterResetButton = new QPushButton(tr("Reset Filter"));
	connect(m_filterResetButton, &QPushButton::clicked, [this](){
		m_filterLineEdit->clear();
		filterChanged();
	});
	m_filterResetButton->setEnabled(false);
	filterLayout->addWidget(m_filterResetButton);

	testsLayout->addLayout(filterLayout);

	// Filter completer
	//
	QStringList completerStringList = QSettings{}.value("TestsTabPage/FilterCompleter").toStringList();
	m_filterCompleter = new QCompleter(completerStringList, this);
	m_filterCompleter->setCaseSensitivity(Qt::CaseInsensitive);

	m_filterLineEdit->setCompleter(m_filterCompleter);

	// Opened files
	//
	QWidget* filesWidget = new QWidget();

	QVBoxLayout* filesLayout = new QVBoxLayout(filesWidget);

	filesLayout->addWidget(new QLabel(tr("Open Files")));

	m_openFilesTreeWidget = new QTreeWidget();
	QStringList header;
	header << tr("File Name");
	m_openFilesTreeWidget->setHeaderLabels(header);
	m_openFilesTreeWidget->setColumnCount(header.size());
	m_openFilesTreeWidget->setHeaderHidden(true);
	m_openFilesTreeWidget->setSortingEnabled(true);
	m_openFilesTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	m_openFilesTreeWidget->sortByColumn(0, Qt::AscendingOrder);
	m_openFilesTreeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	connect(m_openFilesTreeWidget, &QTreeWidget::customContextMenuRequested, this, &TestsWidget::openFilesMenuRequested);
	filesLayout->addWidget(m_openFilesTreeWidget);

	// Left splitter
	//
	m_leftSplitter = new QSplitter(Qt::Vertical);
	m_leftSplitter->addWidget(testsWidget);
	m_leftSplitter->addWidget(filesWidget);
	m_leftSplitter->setStretchFactor(0, 2);
	m_leftSplitter->setStretchFactor(1, 1);
	m_leftSplitter->setCollapsible(0, false);
	m_leftSplitter->setCollapsible(1, false);

	// Left Dock Widget

	m_testsDockWidget = new QDockWidget(tr("Project Tests"), this);
	m_testsDockWidget->setObjectName("TestsDockWidget");

	m_testsDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea |
								Qt::RightDockWidgetArea);
	m_testsDockWidget->setWidget(m_leftSplitter);
	m_testsDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);

	addDockWidget(Qt::LeftDockWidgetArea, m_testsDockWidget);

	return;
}

void TestsWidget::createLogDock()
{
	// Log Dock Widget
	m_outputDockWidget.setObjectName("OutputDockWidget");
	m_outputDockWidget.setAllowedAreas(Qt::BottomDockWidgetArea);
	addDockWidget(Qt::BottomDockWidgetArea, &m_outputDockWidget);

	return;
}

void TestsWidget::createEditorWidget()
{
	// Editor layout
	//
	QWidget* editorWidget = new QWidget();

	m_editorLayout = new QVBoxLayout(editorWidget);
	m_editorLayout->setContentsMargins(0, 0, 0, 0);
	m_editorLayout->setSpacing(0);

	// Editor toolbar
	//
	const int editorToolbarButtonSize = static_cast<int>(0.25 * logicalDpiY());

	m_editorToolBar = new QToolBar();
	m_editorToolBar->setVisible(false);
	m_editorToolBar->setAutoFillBackground(true);
	m_editorToolBar->setStyleSheet("QToolBar{spacing:3px; \
								   background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #6c6c6c, stop: 1 #4b4b4b); }");

	m_openDocumentsCombo = new QComboBox();

	m_openDocumentsCombo->setStyleSheet(tr("QComboBox{\
												border: 0px;\
												color: white;\
												background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #6c6c6c, stop: 1 #4b4b4b);\
											}\
											QComboBox:hover {\
												background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #808080, stop: 1 #6c6c6c);\
											}\
											QComboBox:!editable:on, QComboBox::drop-down:editable:on {\
												background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #6c6c6c, stop: 1 #4b4b4b);\
											}\
											QComboBox:on { \
												padding-top: 3px;\
												padding-left: 4px;\
											}\
											QComboBox::drop-down{\
												border: 1px;\
											}\
											QComboBox::down-arrow {\
													image: url(:/Images/Images/ComboDownArrow.svg);\
													width: %1px;\
													height: %1px;\
											}").arg(editorToolbarButtonSize/3));

	m_openDocumentsCombo->setFixedHeight(editorToolbarButtonSize);
	QString longFileName = QString().fill('0', 32);
	m_openDocumentsCombo->setMinimumWidth(QFontMetrics(m_openDocumentsCombo->font()).horizontalAdvance(longFileName));
	connect(m_openDocumentsCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &TestsWidget::openDocumentsComboTextChanged);

	m_editorToolBar->addWidget(m_openDocumentsCombo);

	QString toolbarButtonStyle = "QPushButton {\
												background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #6c6c6c, stop: 1 #4b4b4b);\
												color: white;\
											}\
											QPushButton:hover {\
												border-style: none;\
												background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #808080, stop: 1 #6c6c6c);\
											}\
											QPushButton:pressed {\
												background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #6c6c6c, stop: 1 #4b4b4b);\
											}";

	// Close Button
	//
	QPushButton* closeButton = new QPushButton(QIcon(":/Images/Images/CloseButtonWhite.svg"), QString());
	closeButton->setFlat(true);
	closeButton->setStyleSheet(toolbarButtonStyle);
	closeButton->setFixedSize(editorToolbarButtonSize, editorToolbarButtonSize);
	connect(closeButton, &QPushButton::clicked, this, &TestsWidget::closeCurrentFile);

	m_editorToolBar->addWidget(closeButton);

	// Spacer
	//
	QWidget* empty = new QWidget();
	empty->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	m_editorToolBar->addWidget(empty);

	// Cursor Pos Button
	//
	m_cursorPosButton = new QPushButton("Line: 1  Col: 1");
	m_cursorPosButton->setFlat(true);
	m_cursorPosButton->setStyleSheet(toolbarButtonStyle);

	QString longCursorPos = "Line: 9999 Col: 9999";
	m_cursorPosButton->setMinimumWidth(QFontMetrics(m_cursorPosButton->font()).horizontalAdvance(longCursorPos));
	m_cursorPosButton->setFixedHeight(editorToolbarButtonSize);
	connect(m_cursorPosButton, &QPushButton::clicked, this, &TestsWidget::onGoToLine);
	m_editorToolBar->addWidget(m_cursorPosButton);

	m_editorLayout->addWidget(m_editorToolBar);

	// CentralWidget

	setCentralWidget(editorWidget);

	centralWidget()->setBackgroundRole(QPalette::Dark);
	centralWidget()->setAutoFillBackground(true);

	return;
}

void TestsWidget::createActions()
{
	if (m_testsTreeView == nullptr)
	{
		Q_ASSERT(m_testsTreeView);
		return;
	}

	QList<QAction*> testsToolbarActions;

	// Tests file tree actions

	m_openFileAction = new QAction(QIcon(":/Images/Images/SchemaOpen.svg"), tr("Open File"), this);
	m_openFileAction->setStatusTip(tr("Open File..."));
	m_openFileAction->setEnabled(false);
	connect(m_openFileAction, &QAction::triggered, this, &TestsWidget::openFile);
	testsToolbarActions.push_back(m_openFileAction);

	QAction* separatorAction1 = new QAction(this);
	separatorAction1->setSeparator(true);
	testsToolbarActions.push_back(separatorAction1);

	m_newFileAction = new QAction(QIcon(":/Images/Images/SchemaAddFile.svg"), tr("New File..."), this);
	m_newFileAction->setStatusTip(tr("New File..."));
	m_newFileAction->setEnabled(false);
	m_newFileAction->setShortcut(QKeySequence::StandardKey::New);
	connect(m_newFileAction, &QAction::triggered, this, &TestsWidget::newFile);
	testsToolbarActions.push_back(m_newFileAction);

	m_newFolderAction = new QAction(QIcon(":/Images/Images/SchemaAddFolder2.svg"), tr("New Folder..."), this);
	m_newFolderAction->setStatusTip(tr("New Folder..."));
	m_newFolderAction->setEnabled(false);
	connect(m_newFolderAction, &QAction::triggered, this, &TestsWidget::newFolder);
	testsToolbarActions.push_back(m_newFolderAction);

	m_addFileAction = new QAction(tr("Add File..."), this);
	m_addFileAction->setStatusTip(tr("Add File..."));
	m_addFileAction->setEnabled(false);
	connect(m_addFileAction, &QAction::triggered, m_testsTreeView, &FileTreeView::addFileToFolder);

	m_renameFileAction = new QAction(tr("Rename..."), this);
	m_renameFileAction->setStatusTip(tr("Rename..."));
	m_renameFileAction->setEnabled(false);
	connect(m_renameFileAction, &QAction::triggered, this, &TestsWidget::renameFile);

	m_deleteFileAction = new QAction(QIcon(":/Images/Images/SchemaDelete.svg"), tr("Delete"), this);
	m_deleteFileAction->setStatusTip(tr("Delete"));
	m_deleteFileAction->setEnabled(false);
	connect(m_deleteFileAction, &QAction::triggered, this, &TestsWidget::deleteSelectedFiles);
	testsToolbarActions.push_back(m_deleteFileAction);

	m_moveFileAction = new QAction(tr("Move File..."), this);
	m_moveFileAction->setStatusTip(tr("Move"));
	m_moveFileAction->setEnabled(false);
	connect(m_moveFileAction, &QAction::triggered, this, &TestsWidget::moveSelectedFiles);

	//----------------------------------
	QAction* separatorAction2 = new QAction(this);
	separatorAction2->setSeparator(true);
	testsToolbarActions.push_back(separatorAction2);

	m_checkOutAction = new QAction(QIcon(":/Images/Images/SchemaCheckOut.svg"), tr("CheckOut"), this);
	m_checkOutAction->setStatusTip(tr("Check out file for edit"));
	m_checkOutAction->setEnabled(false);
	connect(m_checkOutAction, &QAction::triggered, this, &TestsWidget::checkOutSelectedFiles);
	testsToolbarActions.push_back(m_checkOutAction);

	m_checkInAction = new QAction(QIcon(":/Images/Images/SchemaCheckIn.svg"), tr("CheckIn"), this);
	m_checkInAction->setStatusTip(tr("Check in changes"));
	m_checkInAction->setEnabled(false);
	connect(m_checkInAction, &QAction::triggered, this, &TestsWidget::checkInSelectedFiles);
	testsToolbarActions.push_back(m_checkInAction);

	m_undoChangesAction = new QAction(QIcon(":/Images/Images/SchemaUndo.svg"), tr("Undo Changes"), this);
	m_undoChangesAction->setStatusTip(tr("Undo all pending changes for the object"));
	m_undoChangesAction->setEnabled(false);
	connect(m_undoChangesAction, &QAction::triggered, this, &TestsWidget::undoChangesSelectedFiles);
	testsToolbarActions.push_back(m_undoChangesAction);

	m_historyAction = new QAction(QIcon(":/Images/Images/SchemaHistory.svg"), tr("History"), this);
	m_historyAction->setStatusTip(tr("View History"));
	m_historyAction->setEnabled(false);
	connect(m_historyAction, &QAction::triggered, m_testsTreeView, &FileTreeView::showHistory);

	m_compareAction = new QAction(tr("Compare..."), this);
	m_compareAction->setStatusTip(tr("Compare"));
	m_compareAction->setEnabled(false);
	connect(m_compareAction, &QAction::triggered, m_testsTreeView, &FileTreeView::showCompare);

	//----------------------------------
	QAction* separatorAction3 = new QAction(this);
	separatorAction3->setSeparator(true);
	testsToolbarActions.push_back(separatorAction3);

	m_refreshAction = new QAction(QIcon(":/Images/Images/SchemaRefresh.svg"), tr("Refresh"), this);
	m_refreshAction->setStatusTip(tr("Refresh Objects List"));
	m_refreshAction->setEnabled(false);
	m_refreshAction->setShortcut(QKeySequence::StandardKey::Refresh);
	connect(m_refreshAction, &QAction::triggered, this, &TestsWidget::refreshFileTree);
	testsToolbarActions.push_back(m_refreshAction);
	addAction(m_refreshAction);

	QAction* separatorAction4 = new QAction(this);
	separatorAction4->setSeparator(true);
	testsToolbarActions.push_back(separatorAction4);

	m_runAllTestsAction = new QAction(QIcon(":/Images/Images/TestsRunAll.svg"), tr("Run All Tests..."), this);
	m_runAllTestsAction->setStatusTip(tr("Run All Tests"));
	connect(m_runAllTestsAction, &QAction::triggered, this, &TestsWidget::runAllTestFiles);
	testsToolbarActions.push_back(m_runAllTestsAction);

	m_runSelectedTestsAction = new QAction(tr("Run Selected Test(s)..."), this);
	m_runSelectedTestsAction->setStatusTip(tr("Run Selected Test(s)"));
	m_runSelectedTestsAction->setEnabled(false);
	connect(m_runSelectedTestsAction, &QAction::triggered, this, &TestsWidget::runSelectedTestFiles);

	m_runCurrentTestsAction = new QAction(QIcon(":/Images/Images/TestsRunSelected.svg"), tr("Run Current Test..."), this);
	m_runCurrentTestsAction->setStatusTip(tr("Run Current Test"));
	m_runCurrentTestsAction->setEnabled(false);
	connect(m_runCurrentTestsAction, &QAction::triggered, this, &TestsWidget::runTestCurrentFile);
	testsToolbarActions.push_back(m_runCurrentTestsAction);

	m_stopTestsAction = new QAction(QIcon(":/Images/Images/SimStop.svg"), tr("Stop Testing"), this);
	m_stopTestsAction->setStatusTip(tr("Stop testing"));
	m_stopTestsAction->setEnabled(false);
	connect(m_stopTestsAction, &QAction::triggered, this, &TestsWidget::stopTests);
	testsToolbarActions.push_back(m_stopTestsAction);

	QAction* separatorAction5 = new QAction(this);
	separatorAction5->setSeparator(true);
	testsToolbarActions.push_back(separatorAction5);

	m_selectBuildAction = new QAction(QIcon(":/Images/Images/SimOpen.svg"), tr("Select Build..."), this);
	m_selectBuildAction->setStatusTip(tr("Select Build..."));
	connect(m_selectBuildAction, &QAction::triggered, this, &TestsWidget::selectBuild);
	testsToolbarActions.push_back(m_selectBuildAction);

	m_testsTreeView->setContextMenuPolicy(Qt::ActionsContextMenu);

	m_testsTreeView->addAction(m_openFileAction);
	m_testsTreeView->addAction(separatorAction1);

	m_testsTreeView->addAction(m_newFileAction);
	m_testsTreeView->addAction(m_newFolderAction);
	m_testsTreeView->addAction(m_addFileAction);
	m_testsTreeView->addAction(m_renameFileAction);
	m_testsTreeView->addAction(m_deleteFileAction);
	m_testsTreeView->addAction(m_moveFileAction);
	m_testsTreeView->addAction(separatorAction2);

	m_testsTreeView->addAction(m_checkOutAction);
	m_testsTreeView->addAction(m_checkInAction);
	m_testsTreeView->addAction(m_undoChangesAction);
	m_testsTreeView->addAction(m_historyAction);
	m_testsTreeView->addAction(m_compareAction);
	m_testsTreeView->addAction(separatorAction3);
	m_testsTreeView->addAction(m_runSelectedTestsAction);
	m_testsTreeView->addAction(separatorAction4);
	m_testsTreeView->addAction(m_refreshAction);

	m_testsToolbar->addActions(testsToolbarActions);

	m_buildLabel = new QLabel("Build: Not loaded");
	m_buildLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
	m_buildLabel->setTextFormat(Qt::RichText);
	m_buildLabel->setAlignment(Qt::AlignRight |Qt::AlignVCenter);
	m_buildLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	m_buildLabel->setOpenExternalLinks(true);

	m_testsToolbar->addWidget(m_buildLabel);

	// Editor context menu actions

	m_checkInCurrentDocumentAction = new QAction(QIcon(":/Images/Images/SchemaCheckIn.svg"), tr("CheckIn"), this);
	m_checkInCurrentDocumentAction->setStatusTip(tr("Check in changes"));
	m_checkInCurrentDocumentAction->setEnabled(false);
	connect(m_checkInCurrentDocumentAction, &QAction::triggered, this, &TestsWidget::checkInCurrentFile);

	m_checkOutCurrentDocumentAction = new QAction(QIcon(":/Images/Images/SchemaCheckOut.svg"), tr("CheckOut"), this);
	m_checkOutCurrentDocumentAction->setStatusTip(tr("Check out file for edit"));
	m_checkOutCurrentDocumentAction->setEnabled(false);
	connect(m_checkOutCurrentDocumentAction, &QAction::triggered, this, &TestsWidget::checkOutCurrentFile);

	m_undoChangesCurrentDocumentAction = new QAction(QIcon(":/Images/Images/SchemaUndo.svg"), tr("Undo Changes"), this);
	m_undoChangesCurrentDocumentAction->setStatusTip(tr("Undo all pending changes for the object"));
	m_undoChangesCurrentDocumentAction->setEnabled(false);
	connect(m_undoChangesCurrentDocumentAction, &QAction::triggered, this, &TestsWidget::undoChangesCurrentFile);

	m_runTestCurrentDocumentAction = new QAction(tr("Run Test..."), this);
	m_runTestCurrentDocumentAction->setStatusTip(tr("Run Test"));
	m_runTestCurrentDocumentAction->setEnabled(false);
	connect(m_runTestCurrentDocumentAction, &QAction::triggered, this, &TestsWidget::runTestCurrentFile);

	m_saveCurrentDocumentAction = new QAction(tr("Save"), this);
	m_saveCurrentDocumentAction->setShortcut(QKeySequence::StandardKey::Save);
	connect(m_saveCurrentDocumentAction, &QAction::triggered, this, &TestsWidget::saveCurrentFile);

	m_closeCurrentDocumentAction = new QAction(tr("Close"), this);
	m_closeCurrentDocumentAction->setShortcut(QKeySequence::StandardKey::Close);
	connect(m_closeCurrentDocumentAction, &QAction::triggered, this, &TestsWidget::closeCurrentFile);

	// Open documents list actions

	m_checkInOpenDocumentAction = new QAction(QIcon(":/Images/Images/SchemaCheckIn.svg"), tr("CheckIn"), this);
	m_checkInOpenDocumentAction->setStatusTip(tr("Check in changes"));
	m_checkInOpenDocumentAction->setEnabled(false);
	connect(m_checkInOpenDocumentAction, &QAction::triggered, this, &TestsWidget::checkInOpenFile);

	m_checkOutOpenDocumentAction = new QAction(QIcon(":/Images/Images/SchemaCheckOut.svg"), tr("CheckOut"), this);
	m_checkOutOpenDocumentAction->setStatusTip(tr("Check out file for edit"));
	m_checkOutOpenDocumentAction->setEnabled(false);
	connect(m_checkOutOpenDocumentAction, &QAction::triggered, this, &TestsWidget::checkOutOpenFile);

	m_undoChangesOpenDocumentAction = new QAction(QIcon(":/Images/Images/SchemaUndo.svg"), tr("Undo Changes"), this);
	m_undoChangesOpenDocumentAction->setStatusTip(tr("Undo all pending changes for the object"));
	m_undoChangesOpenDocumentAction->setEnabled(false);
	connect(m_undoChangesOpenDocumentAction, &QAction::triggered, this, &TestsWidget::undoChangesOpenFile);

	m_runTestOpenDocumentAction = new QAction(tr("Run Test..."), this);
	m_runTestOpenDocumentAction->setStatusTip(tr("Run Test"));
	m_runTestOpenDocumentAction->setEnabled(false);
	connect(m_runTestOpenDocumentAction, &QAction::triggered, this, &TestsWidget::runTestOpenFile);

	m_saveOpenDocumentAction = new QAction(tr("Save"), this);
	m_saveOpenDocumentAction->setShortcut(QKeySequence::StandardKey::Save);
	connect(m_saveOpenDocumentAction, &QAction::triggered, this, &TestsWidget::saveOpenFile);

	m_closeOpenDocumentAction = new QAction(tr("Close"), this);
	m_closeOpenDocumentAction->setShortcut(QKeySequence("Ctrl+W"));
	connect(m_closeOpenDocumentAction, &QAction::triggered, this, &TestsWidget::closeOpenFile);

	return;
}

void TestsWidget::setTestsTreeActionsState()
{
	// Disable all
	//
	m_openFileAction->setEnabled(false);
	m_newFileAction->setEnabled(false);
	m_newFolderAction->setEnabled(false);
	m_addFileAction->setEnabled(false);
	m_renameFileAction->setEnabled(false);
	m_deleteFileAction->setEnabled(false);
	m_moveFileAction->setEnabled(false);
	m_checkOutAction->setEnabled(false);
	m_checkInAction->setEnabled(false);
	m_undoChangesAction->setEnabled(false);
	m_historyAction->setEnabled(false);
	m_compareAction->setEnabled(false);
	m_refreshAction->setEnabled(false);

	if (db()->isProjectOpened() == false)
	{
		return;
	}

	// --
	//
	QModelIndexList selectedIndexList = m_testsTreeView->selectedSourceRows();


	// Folder is selected
	//
	bool folderSelected = false;

	// Enable edit only files with several extensions!
	//
	bool editableExtension = false;

	// CheckIn, CheckOut, Undo, Get/set Workcopy
	//
	bool canAnyBeCheckedIn = false;
	bool canAnyBeCheckedOut = false;

	for (const QModelIndex& mi : selectedIndexList)
	{
		if (mi.parent().isValid() == false)
		{
			// Forbid root folders processing
			//
			folderSelected = true;

			continue;
		}

		const FileTreeModelItem* file = m_testsTreeModel->fileItem(mi);
		if (file == nullptr)
		{
			assert(file);
			return;
		}

		if (file->isFolder() == true)
		{
			folderSelected = true;
		}

		if (isEditableExtension(file->fileName()) == true)
		{
			editableExtension = true;
		}

		if (file->state() == VcsState::CheckedOut &&
			(file->userId() == db()->currentUser().userId() || db()->currentUser().isAdminstrator()))
		{
			canAnyBeCheckedIn = true;
		}

		if (file->state() == VcsState::CheckedIn)
		{
			canAnyBeCheckedOut = true;
		}

	}

	// Enable Actions
	//
	m_openFileAction->setEnabled(selectedIndexList.size() == 1 && editableExtension == true);
	m_newFileAction->setEnabled(selectedIndexList.size() == 1);
	m_newFolderAction->setEnabled(selectedIndexList.size() == 1);
	m_addFileAction->setEnabled(selectedIndexList.size() == 1);
	m_renameFileAction->setEnabled(selectedIndexList.size() == 1 && canAnyBeCheckedIn);
	m_moveFileAction->setEnabled(canAnyBeCheckedIn && folderSelected == false);

	// Delete Items action
	//
	m_deleteFileAction->setEnabled(false);

	for (const QModelIndex& mi : selectedIndexList)
	{
		if (mi.parent().isValid() == false)
		{
			// Forbid root items deleting
			//
			continue;
		}

		const FileTreeModelItem* file = m_testsTreeModel->fileItem(mi);
		assert(file);

		if (file->state() == VcsState::CheckedIn/* &&
			file->action() != VcsItemAction::Deleted*/)
		{
			m_deleteFileAction->setEnabled(true);
			break;
		}

		if (file->state() == VcsState::CheckedOut &&
			(file->userId() == db()->currentUser().userId() || db()->currentUser().isAdminstrator())
			/*&& file->action() != VcsItemAction::Deleted*/)
		{
			m_deleteFileAction->setEnabled(true);
			break;
		}
	}

	m_checkInAction->setEnabled(canAnyBeCheckedIn);
	m_checkOutAction->setEnabled(canAnyBeCheckedOut);
	m_undoChangesAction->setEnabled(canAnyBeCheckedIn);
	m_historyAction->setEnabled(selectedIndexList.size() == 1 && folderSelected == false);
	m_compareAction->setEnabled(selectedIndexList.size() == 1 && folderSelected == false);

	m_refreshAction->setEnabled(true);

	setRunTestsActionsState();

	return;
}

void TestsWidget::setCodeEditorActionsState()
{
	m_checkInCurrentDocumentAction->setEnabled(false);
	m_checkOutCurrentDocumentAction->setEnabled(false);
	m_undoChangesCurrentDocumentAction->setEnabled(false);
	m_saveCurrentDocumentAction->setEnabled(false);

	if (documentIsOpen(m_currentFileId) == false)
	{
		return;
	}

	const TestTabPageDocument& document = m_openDocuments.at(m_currentFileId);
	m_saveCurrentDocumentAction->setEnabled(document.modified() == true);

	DbFileInfo fi;
	if (db()->getFileInfo(m_currentFileId, &fi, this) == false)
	{
		QMessageBox::critical(this, "Error", "Get file information error!");
		return;
	}

	if (fi.state() == VcsState::CheckedOut &&
		(fi.userId() == db()->currentUser().userId() || db()->currentUser().isAdminstrator()))
	{
		m_checkInCurrentDocumentAction->setEnabled(true);
		m_undoChangesCurrentDocumentAction->setEnabled(true);
	}

	if (fi.state() == VcsState::CheckedIn)
	{
		m_checkOutCurrentDocumentAction->setEnabled(true);
	}

	return;
}

void TestsWidget::setRunTestsActionsState()
{
	if (db()->isProjectOpened() == false)
	{
		return;
	}

	//

	QModelIndexList selectedIndexList = m_testsTreeView->selectedSourceRows();

	bool folderSelected = false;
	bool editableExtensionSelected = false;

	for (const QModelIndex& mi : selectedIndexList)
	{
		if (mi.parent().isValid() == false)
		{
			// Forbid root folders processing
			//
			folderSelected = true;

			continue;
		}

		const FileTreeModelItem* file = m_testsTreeModel->fileItem(mi);
		if (file == nullptr)
		{
			assert(file);
			return;
		}

		if (file->isFolder() == true)
		{
			folderSelected = true;
		}

		if (isEditableExtension(file->fileName()) == true)
		{
			editableExtensionSelected = true;
		}
	}

	m_runSelectedTestsAction->setEnabled(m_scriptIsRunning == false &&
										 selectedIndexList.isEmpty() == false &&
										 editableExtensionSelected == true &&
										 folderSelected == false);

	//

	bool editableExtentionCurrent = false;

	if (documentIsOpen(m_currentFileId) == true)
	{
		const TestTabPageDocument& document = m_openDocuments.at(m_currentFileId);
		editableExtentionCurrent = isEditableExtension(document.fileName());
	}

	m_runCurrentTestsAction->setEnabled(m_scriptIsRunning == false && editableExtentionCurrent == true);

	//

	m_runAllTestsAction->setEnabled(m_scriptIsRunning == false);
	m_runTestCurrentDocumentAction->setEnabled(m_scriptIsRunning == false);
	m_runTestOpenDocumentAction->setEnabled(m_scriptIsRunning == false);

	m_stopTestsAction->setEnabled(m_scriptIsRunning == true);
}

void TestsWidget::saveSettings()
{
	QSettings s;
	s.setValue("TestsTabPage/dockState", saveState());
	s.setValue("TestsTabPage/leftSplitterState", m_leftSplitter->saveState());
	s.setValue("TestsTabPage/testsHeaderState", m_testsTreeView->header()->saveState());

	return;
}

void TestsWidget::restoreSettings()
{
	// Restore settings

	QSettings s;

	QByteArray data = s.value("TestsTabPage/dockState").toByteArray();
	if (data.isEmpty() == false)
	{
		s.setValue("TestsTabPage/dockState", saveState());
		restoreState(data);
	}

	data = s.value("TestsTabPage/leftSplitterState").toByteArray();
	if (data.isEmpty() == false)
	{
		m_leftSplitter->restoreState(data);
	}

	QByteArray headerState = s.value("TestsTabPage/testsHeaderState").toByteArray();
	if (headerState.isEmpty() == false)
	{
		m_testsTreeView->header()->restoreState(headerState);
	}

	return;
}

void TestsWidget::hideEditor()
{
	m_editorToolBar->setVisible(false);
	// m_editorEmptyLabel->setVisible(true);
	m_currentFileId = -1;

	setRunTestsActionsState();
}

bool TestsWidget::documentIsOpen(int fileId) const
{
	return m_openDocuments.find(fileId) != m_openDocuments.end();
}

void TestsWidget::saveDocument(int fileId)
{
	if (documentIsOpen(fileId) == false)
	{
		Q_ASSERT(false);
		return;
	}

	TestTabPageDocument& document = m_openDocuments.at(fileId);

	if (document.modified() == false)
	{
		// Fils is not modified
		return;
	}

	IdeCodeEditor* codeEditor = document.codeEditor();
	if (codeEditor == nullptr)
	{
		Q_ASSERT(codeEditor);
		return;
	}

	DbFileInfo fi;
	if (db()->getFileInfo(fileId, &fi, this) == false)
	{
		QMessageBox::critical(this, "Error", "Get file information error!");
		return;
	}

	std::shared_ptr<DbFile> dbFile;
	if (db()->getLatestVersion(fi, &dbFile, this) == false)
	{
		return;
	}

	if (dbFile == nullptr)
	{
		Q_ASSERT(dbFile);
		return;
	}

	dbFile->setData(codeEditor->text().toUtf8());

	if (db()->setWorkcopy(dbFile, this) == false)
	{
		return;
	}

	document.setModified(false);

	updateOpenDocumentInfo(fileId);

	return;

}

void TestsWidget::saveAllDocuments()
{
	for (auto& it : m_openDocuments)
	{
		saveDocument(it.first);
	}
}

void TestsWidget::closeDocument(int fileId, bool force)
{
	if (documentIsOpen(fileId) == false)
	{
		Q_ASSERT(false);
		return;
	}

	TestTabPageDocument& document = m_openDocuments.at(fileId);

	IdeCodeEditor* codeEditor = document.codeEditor();
	if (codeEditor == nullptr)
	{
		Q_ASSERT(codeEditor);
		return;
	}

	QTreeWidgetItem* openFilesTreeWidgetItem = document.openFilesTreeWidgetItem();
	if (openFilesTreeWidgetItem == nullptr)
	{
		Q_ASSERT(openFilesTreeWidgetItem);
		return;
	}

	if (force == false)
	{
		if (document.modified() == true)
		{
			QString fileName = document.fileName();

			auto reply = QMessageBox::question(this,
											   qAppName(),
											   tr("Warning! File %1 is modified. Save it now?").arg(fileName),
											   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
			if (reply == QMessageBox::Cancel)
			{
				return;
			}
			if (reply == QMessageBox::Yes)
			{
				saveDocument(fileId);
			}
		}
	}

	// Delete item from opened files list

	int closeIndex = m_openFilesTreeWidget->indexOfTopLevelItem(openFilesTreeWidgetItem);

	// Delete item from combo box

	int comboIndex = m_openDocumentsCombo->findData(fileId, Qt::UserRole);
	if (comboIndex == -1)
	{
		Q_ASSERT(false);
	}
	else
	{
		m_openDocumentsCombo->blockSignals(true);
		m_openDocumentsCombo->removeItem(comboIndex);
		m_openDocumentsCombo->blockSignals(false);
	}

	// Delete item from opened files list

	if (closeIndex == -1)
	{
		Q_ASSERT(false);
	}
	else
	{
		m_openFilesTreeWidget->takeTopLevelItem(closeIndex);
		delete openFilesTreeWidgetItem;
	}

	// Delete item documents list

	codeEditor->deleteLater();
	m_openDocuments.erase(fileId);

	// Open another document

	if (m_openFilesTreeWidget->topLevelItemCount() == 0)
	{
		// No documents left
		Q_ASSERT(m_openDocuments.empty() == true);

		hideEditor();
	}
	else
	{
		if (closeIndex > 0)
		{
			closeIndex--;
		}

		QTreeWidgetItem* anotherItem = m_openFilesTreeWidget->topLevelItem(closeIndex);
		if (anotherItem == nullptr)
		{
			Q_ASSERT(anotherItem);
			return;
		}

		bool ok = false;
		int anotherFileId = anotherItem->data(0, Qt::UserRole).toInt(&ok);

		if (ok == true)
		{
			setCurrentDocument(anotherFileId);
		}
		else
		{
			Q_ASSERT(ok);
		}
	}

	return;
}

void TestsWidget::closeAllDocuments()
{
	for (auto& it : m_openDocuments)
	{
		IdeCodeEditor* codeEditor =  it.second.codeEditor();
		if (codeEditor == nullptr)
		{
			Q_ASSERT(codeEditor);
			return;
		}

		delete codeEditor;
	}

	m_openDocuments.clear();
	m_openDocumentsCombo->clear();

	return;
}

void TestsWidget::updateOpenDocumentInfo(int fileId)
{
	if (documentIsOpen(fileId) == false)
	{
		Q_ASSERT(false);
		return;
	}

	const TestTabPageDocument& document = m_openDocuments.at(fileId);

	QTreeWidgetItem* openFilesTreeWidgetItem = document.openFilesTreeWidgetItem();
	if (openFilesTreeWidgetItem == nullptr)
	{
		Q_ASSERT(openFilesTreeWidgetItem);
		return;
	}

	QString itemName = document.fileName();

	if (document.codeEditor()->readOnly() == true)
	{
		itemName += QObject::tr(" [Read-only]");
	}
	if (document.modified() == true)
	{
		itemName += QObject::tr(" *");
	}

	// Update status on OpenFilesTreeWidget

	openFilesTreeWidgetItem->setText(0, itemName);

	// Update combo box item

	int comboRenameIndex = m_openDocumentsCombo->findData(fileId, Qt::UserRole);
	if (comboRenameIndex != -1)
	{
		m_openDocumentsCombo->blockSignals(true);
		m_openDocumentsCombo->setItemText(comboRenameIndex, itemName);
		m_openDocumentsCombo->model()->sort(0, Qt::AscendingOrder);
		m_openDocumentsCombo->blockSignals(false);
	}
	else
	{
		Q_ASSERT(comboRenameIndex != -1);
	}
}

void TestsWidget::runTests(std::vector<int> fileIds)
{
	if (fileIds.empty() ==  true)
	{
		return;
	}

	if (m_buildPath.isEmpty() == true)
	{
		selectBuild();

		if (m_buildPath.isEmpty() == true)
		{
			return;
		}
	}

	std::vector<DbFileInfo> dbFiles;

	for (int fileId : fileIds)
	{
		if (documentIsOpen(fileId) == true)
		{
			saveDocument(fileId);
		}

		DbFileInfo fi;
		if (db()->getFileInfo(fileId, &fi, this) == false)
		{
			QMessageBox::critical(this, "Error", "Get file information error!");
			return;
		}

		dbFiles.push_back(fi);
	}

	runSimTests(m_buildPath, dbFiles);
	return;
}

void TestsWidget::showEvent(QShowEvent* e)
{
	QMainWindow::showEvent(e);
	e->ignore();

static bool firstEvent = true;

	if (firstEvent == true)
	{
		QList<QDockWidget*> dockWidgets = findChildren<QDockWidget*>();
		for (QDockWidget* dw : dockWidgets)
		{
			restoreDockWidget(dw);
			dw->setVisible(true);
		}
		firstEvent = false;
	}

	return;
}

void TestsWidget::keyPressEvent(QKeyEvent* event)
{
	if (event->modifiers() & Qt::ControlModifier)
	{
		if (event->key() == Qt::Key_S)
		{
			onSaveKeyPressed();
		}

		if (event->key() == Qt::Key_W || event->key() == Qt::Key_F4)
		{
			onCloseKeyPressed();
		}

		if (event->key() == Qt::Key_Tab)
		{
			onCtrlTabKeyPressed();
			return;
		}
	}

	if (event->key() == Qt::Key_Delete &&
		m_testsTreeView->hasFocus() == true &&
		m_deleteFileAction->isEnabled() == true)
	{
		deleteSelectedFiles();
		return;
	}

	QWidget::keyPressEvent(event);
	return;
}

bool TestsWidget::isEditableExtension(const QString& fileName)
{
	for (const QString& ext : m_editableExtensions)
	{
		if (fileName.endsWith(ext) == true)
		{
			return true;
		}
	}

	return false;
}
