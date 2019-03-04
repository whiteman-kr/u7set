#include "TestResultDialog.h"


// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

TestResultTable::TestResultTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

TestResultTable::~TestResultTable()
{
	m_mutex.lock();

		m_optionList.clear();

	m_mutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int TestResultTable::columnCount(const QModelIndex&) const
{
	return TEST_RESULT_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int TestResultTable::rowCount(const QModelIndex&) const
{
	return optionCount();
}

// -------------------------------------------------------------------------------------------------------------------

QVariant TestResultTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QVariant result = QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section >= 0 && section < TEST_RESULT_COLUMN_COUNT)
		{
			result = TestResultColumn[section];
		}
	}

	if (orientation == Qt::Vertical)
	{
		result = QString("%1").arg(section + 1);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant TestResultTable::data(const QModelIndex &index, int role) const
{
	if (index.isValid() == false)
	{
		return QVariant();
	}

	int row = index.row();
	if (row < 0 || row >= optionCount())
	{
		return QVariant();
	}

	int column = index.column();
	if (column < 0 || column > TEST_RESULT_COLUMN_COUNT)
	{
		return QVariant();
	}

	SerialPortOption* pOption = option(row);
	if (pOption == nullptr)
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		int result = Qt::AlignLeft;

		switch (column)
		{
			case TEST_RESULT_COLUMN_PORT:				result = Qt::AlignCenter;	break;
			case TEST_RESULT_COLUMN_TYPE:				result = Qt::AlignCenter;	break;
			case TEST_RESULT_COLUMN_PACKETS:			result = Qt::AlignCenter;	break;
			case TEST_RESULT_COLUMN_RECEIVED:			result = Qt::AlignCenter;	break;
			case TEST_RESULT_COLUMN_SKIPPED:			result = Qt::AlignCenter;	break;
			case TEST_RESULT_COLUMN_RESULT:				result = Qt::AlignCenter;	break;
			default:									assert(false);
		}

		return result;
	}

	if (role == Qt::BackgroundColorRole)
	{
		switch (column)
		{
			case TEST_RESULT_COLUMN_RESULT:

				if (pOption->testResult().isOk() == false)
				{
					return QColor(0xFF, 0xA0, 0xA0);
				}
				break;
		}
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(row, column, pOption);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString TestResultTable::text(int row, int column, SerialPortOption* pOption) const
{
	if (row < 0 || row >= optionCount())
	{
		return QString();
	}

	if (column < 0 || column > TEST_RESULT_COLUMN_COUNT)
	{
		return QString();
	}

	if (pOption == nullptr)
	{
		return QString();
	}

	QString result;



	switch (column)
	{
		case TEST_RESULT_COLUMN_PORT:		result = pOption->portName();						break;
		case TEST_RESULT_COLUMN_TYPE:		result = pOption->typeStr();						break;
		case TEST_RESULT_COLUMN_PACKETS:	result = pOption->testResult().packetCountStr();	break;
		case TEST_RESULT_COLUMN_RECEIVED:	result = pOption->testResult().receivedBytesStr();	break;
		case TEST_RESULT_COLUMN_SKIPPED:	result = pOption->testResult().skippedBytesStr();	break;
		case TEST_RESULT_COLUMN_RESULT:		result = pOption->testResult().isOkStr();			break;
		default:							assert(0); break;
	}



	return result;
}

// -------------------------------------------------------------------------------------------------------------------

int TestResultTable::optionCount() const
{
	int count = 0;

	m_mutex.lock();

		count = m_optionList.count();

	m_mutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

SerialPortOption* TestResultTable::option(int index) const
{
	SerialPortOption* pOption = nullptr;

	m_mutex.lock();

		if (index >= 0 && index < m_optionList.count())
		{
			 pOption = m_optionList[index];
		}

	m_mutex.unlock();

	return pOption;
}

// -------------------------------------------------------------------------------------------------------------------

void TestResultTable::set(const QList<SerialPortOption*>& list_add)
{
	int count = list_add.count();
	if (count == 0)
	{
		return;
	}

	beginInsertRows(QModelIndex(), 0, count - 1);

		m_mutex.lock();

			m_optionList = list_add;

		m_mutex.unlock();

	endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void TestResultTable::clear()
{
	int count = optionCount();
	if (count == 0)
	{
		return;
	}

	beginRemoveRows(QModelIndex(), 0, count - 1);

		m_mutex.lock();

			m_optionList.clear();

		m_mutex.unlock();

	endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

TestResultDialog::TestResultDialog( QWidget* parent) :
	QDialog(parent)
{
	createInterface();
	updateTestResult();
}

// -------------------------------------------------------------------------------------------------------------------

TestResultDialog::~TestResultDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool TestResultDialog::createInterface()
{
	setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Options.png"));
	setWindowTitle(tr("Test result options"));
	setFixedSize(700, 270);

	// Module ID
	//
	QHBoxLayout *moduleIDLayout = new QHBoxLayout;

	m_pModuleIDLabel = new QLabel(tr("Module ID"), this);
	m_pModuleIDEdit = new QLineEdit(theOptions.testOption().moduleID(), this);

	moduleIDLayout->addWidget(m_pModuleIDLabel);
	moduleIDLayout->addWidget(m_pModuleIDEdit);
	moduleIDLayout->addStretch();

	// Operator name
	//
	QHBoxLayout *operatorNameLayout = new QHBoxLayout;

	m_pOperatorNameLabel = new QLabel(tr("Operator name"), this);
	m_pOperatorNameEdit = new QLineEdit(theOptions.testOption().operatorName(), this);

	operatorNameLayout->addWidget(m_pOperatorNameLabel);
	operatorNameLayout->addWidget(m_pOperatorNameEdit);
	operatorNameLayout->addStretch();

	// result list
	//
	m_pView = new QTableView(this);
	m_pView->setModel(&m_resultTable);
	m_pView->verticalHeader()->setDefaultSectionSize(22);

	for(int column = 0; column < TEST_RESULT_COLUMN_COUNT; column++)
	{
		m_pView->setColumnWidth(column, TestResultColumnWidth[column]);
	}

	m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);

	// buttons
	//
	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &TestResultDialog::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &TestResultDialog::reject);

	// Main Layout
	//

	QVBoxLayout *optionLayout = new QVBoxLayout;

	optionLayout->addLayout(moduleIDLayout);
	optionLayout->addLayout(operatorNameLayout);

	QGroupBox* group = new QGroupBox();
	group->setLayout(optionLayout);

	QVBoxLayout *mainLayout = new QVBoxLayout;

	mainLayout->addWidget(group);
	mainLayout->addWidget(m_pView);
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void TestResultDialog::updateTestResult()
{
	QList<SerialPortOption*> resultList;

	for(int i = 0; i < SERIAL_PORT_COUNT; i++ )
	{
		SerialPortOption* portOption = theOptions.serialPorts().port(i);
		if (portOption == nullptr)
		{
			continue;
		}

		resultList.append(portOption);
	}

	m_resultTable.set(resultList);
}

// -------------------------------------------------------------------------------------------------------------------

void TestResultDialog::onOk()
{
	m_moduleID = m_pModuleIDEdit->text();
	if (m_moduleID.isEmpty() == true)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, input module ID!"));
		m_pModuleIDEdit->setFocus();
		return;
	}

	m_operatorName = m_pOperatorNameEdit->text();
	if (m_operatorName.isEmpty() == true)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, input operator name!"));
		m_pOperatorNameEdit->setFocus();
		return;
	}

	theOptions.testOption().setModuleID(m_moduleID);
	theOptions.testOption().setOperatorName(m_operatorName);
	theOptions.testOption().save();

	accept();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
