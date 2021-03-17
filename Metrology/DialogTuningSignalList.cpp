#include "DialogTuningSignalList.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

QVariant TuningSignalTable::data(const QModelIndex &index, int role) const
{
	if (index.isValid() == false)
	{
		return QVariant();
	}

	int row = index.row();
	if (row < 0 || row >= count())
	{
		return QVariant();
	}

	int column = index.column();
	if (column < 0 || column > m_columnCount)
	{
		return QVariant();
	}

	Metrology::Signal* pSignal = at(row);
	if (pSignal == nullptr || pSignal->param().isValid() == false)
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		int result = Qt::AlignLeft;

		switch (column)
		{
			case TUN_SIGNAL_LIST_COLUMN_RACK:			result = Qt::AlignCenter;	break;
			case TUN_SIGNAL_LIST_COLUMN_APP_ID:			result = Qt::AlignLeft;		break;
			case TUN_SIGNAL_LIST_COLUMN_CUSTOM_ID:		result = Qt::AlignLeft;		break;
			case TUN_SIGNAL_LIST_COLUMN_EQUIPMENT_ID:	result = Qt::AlignLeft;		break;
			case TUN_SIGNAL_LIST_COLUMN_CAPTION:		result = Qt::AlignLeft;		break;
			case TUN_SIGNAL_LIST_COLUMN_STATE:			result = Qt::AlignCenter;	break;
			case TUN_SIGNAL_LIST_COLUMN_DEFAULT:		result = Qt::AlignCenter;	break;
			case TUN_SIGNAL_LIST_COLUMN_RANGE:			result = Qt::AlignCenter;	break;
			default:									assert(0);
		}

		return result;
	}

	if (role == Qt::ForegroundRole)
	{
		if (column == TUN_SIGNAL_LIST_COLUMN_DEFAULT)
		{
			return QColor(Qt::darkGray);
		}

		return QVariant();
	}


	if (role == Qt::BackgroundRole)
	{
		if (column == TUN_SIGNAL_LIST_COLUMN_STATE)
		{
			if (pSignal->state().valid() == false)
			{
				return theOptions.signalInfo().colorFlagValid();
			}
		}

		return QVariant();
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(row, column, pSignal);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString TuningSignalTable::text(int row, int column, Metrology::Signal* pSignal) const
{
	if (row < 0 || row >= count())
	{
		return QString();
	}

	if (column < 0 || column > m_columnCount)
	{
		return QString();
	}

	if (pSignal == nullptr)
	{
		return QString();
	}

	Metrology::SignalParam& param = pSignal->param();
	if (param.isValid() == false)
	{
		return QString();
	}

	QString result;

	switch (column)
	{
		case TUN_SIGNAL_LIST_COLUMN_RACK:			result = param.location().rack().caption();	break;
		case TUN_SIGNAL_LIST_COLUMN_APP_ID:			result = param.appSignalID();				break;
		case TUN_SIGNAL_LIST_COLUMN_CUSTOM_ID:		result = param.customAppSignalID();			break;
		case TUN_SIGNAL_LIST_COLUMN_EQUIPMENT_ID:	result = param.equipmentID();				break;
		case TUN_SIGNAL_LIST_COLUMN_CAPTION:		result = param.caption();					break;
		case TUN_SIGNAL_LIST_COLUMN_STATE:			result = signalStateStr(pSignal);			break;
		case TUN_SIGNAL_LIST_COLUMN_DEFAULT:		result = qApp->translate("MetrologySignal", param.tuningDefaultValueStr().toUtf8());break;
		case TUN_SIGNAL_LIST_COLUMN_RANGE:			result = param.tuningRangeStr();			break;
		default:									assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString TuningSignalTable::signalStateStr(Metrology::Signal* pSignal) const
{
	if (pSignal == nullptr)
	{
		return QString();
	}

	Metrology::SignalParam& param = pSignal->param();
	if (param.isValid() == false)
	{
		return QString();
	}

	if (pSignal->state().valid() == false)
	{
		return qApp->translate("MetrologySignal", Metrology::SignalNoValid);
	}

	QString stateStr, formatStr;

	switch (param.signalType())
	{
		case E::SignalType::Analog:

			formatStr = QString::asprintf("%%.%df", param.decimalPlaces());

			stateStr = QString::asprintf(formatStr.toAscii(), pSignal->state().value());

			break;

		case E::SignalType::Discrete:

			stateStr = pSignal->state().value() == 0.0 ? tr("No") : tr("Yes");

			break;

		case E::SignalType::Bus:

			stateStr.clear();

			break;

		default:
			assert(0);
	}

	return stateStr;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

E::SignalType	DialogTuningSignalList::m_typeAD = E::SignalType::Analog;

// -------------------------------------------------------------------------------------------------------------------

DialogTuningSignalList::DialogTuningSignalList(QWidget* parent) :
	DialogList(0.5, 0.4, false, parent)
{
	createInterface();
	DialogTuningSignalList::updateList();

	startSignalStateTimer();
}

// -------------------------------------------------------------------------------------------------------------------

DialogTuningSignalList::~DialogTuningSignalList()
{
	stopSignalStateTimer();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogTuningSignalList::createInterface()
{
	setWindowTitle(tr("Tuning signals"));

	// menu
	//
	m_pSignalMenu = new QMenu(tr("&Signal"), this);
	m_pEditMenu = new QMenu(tr("&Edit"), this);
	m_pViewMenu = new QMenu(tr("&View"), this);
	m_pViewTypeADMenu = new QMenu(tr("Type A/D"), this);

	// action
	//
	m_pSetValueAction = m_pSignalMenu->addAction(tr("&Set value ..."));
	m_pSetValueAction->setIcon(QIcon(":/icons/ChangeState.png"));

	m_pSignalMenu->addSeparator();
	m_pSignalMenu->addAction(m_pExportAction);

	m_pEditMenu->addAction(m_pFindAction);
	m_pEditMenu->addSeparator();
	m_pEditMenu->addAction(m_pCopyAction);
	m_pEditMenu->addAction(m_pSelectAllAction);
	m_pEditMenu->addSeparator();

	m_pTypeAnalogAction = m_pViewTypeADMenu->addAction(tr("Analog"));
	m_pTypeAnalogAction->setCheckable(true);
	m_pTypeAnalogAction->setChecked(m_typeAD == E::SignalType::Analog);
	m_pTypeDiscreteAction = m_pViewTypeADMenu->addAction(tr("Discrete"));
	m_pTypeDiscreteAction->setCheckable(true);
	m_pTypeDiscreteAction->setChecked(m_typeAD == E::SignalType::Discrete);
	m_pTypeBusAction = m_pViewTypeADMenu->addAction(tr("Bus"));
	m_pTypeBusAction->setCheckable(true);
	m_pTypeBusAction->setChecked(m_typeAD == E::SignalType::Bus);

	m_pViewMenu->addMenu(m_pViewTypeADMenu);

	//
	//
	addMenu(m_pSignalMenu);
	addMenu(m_pEditMenu);
	addMenu(m_pViewMenu);

	//
	//
	connect(m_pSetValueAction, &QAction::triggered, this, &DialogTuningSignalList::onProperties);

	connect(m_pTypeAnalogAction, &QAction::triggered, this, &DialogTuningSignalList::showTypeAnalog);
	connect(m_pTypeDiscreteAction, &QAction::triggered, this, &DialogTuningSignalList::showTypeDiscrete);
	connect(m_pTypeBusAction, &QAction::triggered, this, &DialogTuningSignalList::showTypeBus);

	//
	//
	m_signalTable.setColumnCaption(DialogTuningSignalList::metaObject()->className(), TUN_SIGNAL_LIST_COLUMN_COUNT, TuningSignalColumn);
	setModel(&m_signalTable);

	//
	//
	DialogList::createHeaderContexMenu(TUN_SIGNAL_LIST_COLUMN_COUNT, TuningSignalColumn, TuningSignalColumnWidth);
	createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogTuningSignalList::createContextMenu()
{
	addContextAction(m_pSetValueAction);
	addContextSeparator();
	addContextMenu(m_pViewTypeADMenu);
	addContextSeparator();
	addContextAction(m_pCopyAction);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogTuningSignalList::updateVisibleColunm()
{
	m_pTypeAnalogAction->setChecked(m_typeAD == E::SignalType::Analog);
	m_pTypeDiscreteAction->setChecked(m_typeAD == E::SignalType::Discrete);
	m_pTypeBusAction->setChecked(m_typeAD == E::SignalType::Bus);

	for(int c = 0; c < TUN_SIGNAL_LIST_COLUMN_COUNT; c++)
	{
		hideColumn(c, false);
	}

	hideColumn(TUN_SIGNAL_LIST_COLUMN_CUSTOM_ID, true);
	hideColumn(TUN_SIGNAL_LIST_COLUMN_EQUIPMENT_ID, true);

	if (m_typeAD == E::SignalType::Discrete)
	{
		hideColumn(TUN_SIGNAL_LIST_COLUMN_RANGE, true);
	}

	if (m_typeAD == E::SignalType::Bus)
	{
		hideColumn(TUN_SIGNAL_LIST_COLUMN_STATE, true);
		hideColumn(TUN_SIGNAL_LIST_COLUMN_DEFAULT, true);
		hideColumn(TUN_SIGNAL_LIST_COLUMN_RANGE, true);
	}
}


// -------------------------------------------------------------------------------------------------------------------

void DialogTuningSignalList::updateList()
{
	updateVisibleColunm();

	// update signal list

	m_signalTable.clear();

	QVector<Metrology::Signal*> signalList;

	int signalCount = theSignalBase.tuning().signalBase().count();
	for(int i = 0; i < signalCount; i++)
	{
		Metrology::Signal* pSignal = theSignalBase.tuning().signalBase().signal(i);
		if (pSignal == nullptr)
		{
			continue;
		}

		Metrology::SignalParam& param = pSignal->param();
		if (param.isValid() == false)
		{
			continue;
		}

		if (param.signalType() != m_typeAD)
		{
			continue;
		}

		signalList.append(pSignal);
	}

	m_signalTable.set(signalList);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogTuningSignalList::updateState()
{
	m_signalTable.updateColumn(TUN_SIGNAL_LIST_COLUMN_STATE);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogTuningSignalList::startSignalStateTimer()
{
	if (m_updateSignalStateTimer == nullptr)
	{
		m_updateSignalStateTimer = new QTimer(this);
		connect(m_updateSignalStateTimer, &QTimer::timeout, this, &DialogTuningSignalList::updateState);
	}

	m_updateSignalStateTimer->start(250); // 250 ms
}

// -------------------------------------------------------------------------------------------------------------------

void DialogTuningSignalList::stopSignalStateTimer()
{
	if (m_updateSignalStateTimer != nullptr)
	{
		m_updateSignalStateTimer->stop();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogTuningSignalList::onProperties()
{
	QTableView* pView = view();
	if (pView == nullptr)
	{
		return;
	}

	int index = pView->currentIndex().row();
	if (index < 0 || index >= m_signalTable.count())
	{
		return;
	}

	Metrology::Signal* pSignal = m_signalTable.at(index);
	if (pSignal == nullptr)
	{
		return;
	}

	Metrology::SignalParam& param = pSignal->param();
	if (param.isValid() == false)
	{
		return;
	}

	if (param.isBus() == true || param.isInternal() == false)
	{
		return;
	}

	DialogTuningSignalState* dialog = new DialogTuningSignalState(param);
	dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogTuningSignalList::showTypeAnalog()
{
	m_typeAD = E::SignalType::Analog;

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogTuningSignalList::showTypeDiscrete()
{
	m_typeAD = E::SignalType::Discrete;

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogTuningSignalList::showTypeBus()
{
	m_typeAD = E::SignalType::Bus;

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

DialogTuningSignalState::DialogTuningSignalState(const Metrology::SignalParam& param, QWidget* parent) :
	QDialog(parent),
	m_param(param)
{
	createInterface();
}

// -------------------------------------------------------------------------------------------------------------------

DialogTuningSignalState::~DialogTuningSignalState()
{
}

// -------------------------------------------------------------------------------------------------------------------

void DialogTuningSignalState::createInterface()
{
	setWindowFlags(Qt::Window  | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Tuning.png"));
	setWindowTitle(tr("Signal state"));

	if (m_param.isValid() == false)
	{
		QMessageBox::critical(this, windowTitle(), tr("It is not possible to change signal state!"));
		return;
	}

	// main Layout
	//
	QVBoxLayout* mainLayout = new QVBoxLayout;

	switch(m_param.signalType())
	{
		case E::SignalType::Analog:
			{
				QLabel* stateLabel = new QLabel(tr("Please, input new state of analog signal:"));
				stateLabel->setAlignment(Qt::AlignHCenter);

				QRegExp rx("^[-]{0,1}[0-9]*[.]{1}[0-9]*$");
				QValidator* validator = new QRegExpValidator(rx, this);

				m_stateEdit = new QLineEdit(QString::number(theSignalBase.signalState(m_param.hash()).value() ));
				m_stateEdit->setAlignment(Qt::AlignHCenter);
				m_stateEdit->setValidator(validator);

				QLabel* rangeLabel = new QLabel(m_param.tuningRangeStr());
				rangeLabel->setAlignment(Qt::AlignHCenter);

				// buttons
				//
				QHBoxLayout* buttonLayout = new QHBoxLayout ;

				QPushButton* okButton = new QPushButton(tr("Ok"));
				QPushButton* cancelButton = new QPushButton(tr("Cancel"));

				connect(okButton, &QPushButton::clicked, this, &DialogTuningSignalState::onOk);
				connect(cancelButton, &QPushButton::clicked, this, &DialogTuningSignalState::reject);

				buttonLayout->addWidget(okButton);
				buttonLayout->addWidget(cancelButton);

				// main Layout
				//
				mainLayout->addWidget(stateLabel);
				mainLayout->addWidget(m_stateEdit);
				mainLayout->addWidget(rangeLabel);
				mainLayout->addStretch();
				mainLayout->addLayout(buttonLayout);
			}
			break;

		case E::SignalType::Discrete:
			{
				QLabel* stateLabel = new QLabel(tr("Please, select new state of discrete signal:"));

				// buttons
				//
				QHBoxLayout* buttonLayout = new QHBoxLayout ;

				QPushButton* yesButton = new QPushButton(tr("Yes"));
				QPushButton* noButton = new QPushButton(tr("No"));

				connect(yesButton, &QPushButton::clicked, this, &DialogTuningSignalState::onYes);
				connect(noButton, &QPushButton::clicked, this, &DialogTuningSignalState::onNo);

				buttonLayout->addWidget(yesButton);
				buttonLayout->addWidget(noButton);

				// main Layout
				//
				mainLayout->addWidget(stateLabel);
				mainLayout->addLayout(buttonLayout);
			}
			break;

		default:
			assert(0);
	}

	setLayout(mainLayout);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogTuningSignalState::onOk()
{
	double state = m_stateEdit->text().toDouble();

	if (state < m_param.tuningLowBound().toDouble()|| state > m_param.tuningHighBound().toDouble())
	{
		QString str, formatStr;

		formatStr = QString::asprintf("%%.%df", m_param.decimalPlaces());

		str = tr("Failed input value: %1").arg(QString::asprintf(formatStr.toAscii(), state));
		str += tr("\nRange of signal: %1").arg(m_param.tuningRangeStr());

		QMessageBox::critical(this, windowTitle(), str);
		return;
	}

	theSignalBase.tuning().appendCmdFowWrite(m_param.hash(), m_param.tuningValueType(), state);

	accept();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogTuningSignalState::onYes()
{
	theSignalBase.tuning().appendCmdFowWrite(m_param.hash(), m_param.tuningValueType(), 1);

	accept();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogTuningSignalState::onNo()
{
	theSignalBase.tuning().appendCmdFowWrite(m_param.hash(), m_param.tuningValueType(), 0);

	accept();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
