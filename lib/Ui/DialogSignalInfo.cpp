#include "DialogSignalInfo.h"
#include "ui_DialogSignalInfo.h"
#include "../CommonLib/Types.h"
#include "../Proto/serialization.pb.h"
#include "../UtilsLib/Ui/UiTools.h"

//
//
//	DialogSetpointDetails
//
//

DialogSetpointDetails::DialogSetpointDetails(QWidget* parent, IAppSignalManager* appSignalManager, std::shared_ptr<Comparator> comparator):
QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
  m_appSignalManager(appSignalManager),
  m_comparator(comparator)
{
	if (m_appSignalManager == nullptr)
	{
		Q_ASSERT(m_appSignalManager);
		return;
	}

	setWindowTitle(tr("Setpoint Details"));

	QVBoxLayout* mainLayout = new QVBoxLayout(this);

	QTextEdit* textEdit = new QTextEdit();
	mainLayout->addWidget(textEdit);

	Comparator* c = m_comparator.get();

	QString s;

	switch (c->cmpType())
	{
		case E::CmpType::Greate:	s += "Type: <b>&gt; (Greater)</b><br>";		break;
		case E::CmpType::Less:		s += "Type: <b>&lt; (Less)</b><br>";		break;
		case E::CmpType::Equal:		s += "Type: <b>= (Equal)</b><br>";			break;
		case E::CmpType::NotEqual:	s += "Type: <b>&lt;&gt; (Not Equal)</b><br>";		break;
	}

	// Input

	bool ok = false;

	AppSignalParam inputParam = m_appSignalManager->signalParam(c->input().appSignalID(), &ok);
	if (ok == false)
	{
		s += tr("Input signal: <b>%1</b><br>").arg(c->input().appSignalID());
	}
	else
	{
		s += tr("Input signal: <b>%1 (%2) - %3</b><br>").arg(inputParam.appSignalId()).arg(inputParam.customSignalId()).arg(inputParam.caption());
	}

	// Compare To

	if (c->compare().isConst() == true)
	{
		s += tr("Compare To: <b>%1</b><br>").arg(QString::number(c->compare().constValue(), 'f', c->precision()));
	}
	else
	{
		AppSignalParam compareParam = m_appSignalManager->signalParam(c->compare().appSignalID(), &ok);
		if (ok == false)
		{
			s += tr("Compare To: <b>%1</b><br>").arg(c->compare().appSignalID());
		}
		else
		{
			s += tr("Compare To: <b>%1 (%2) - %3</b><br>").arg(compareParam.appSignalId()).arg(compareParam.customSignalId()).arg(compareParam.caption());
		}
	}


	// Output

	AppSignalParam outputParam = m_appSignalManager->signalParam(c->output().appSignalID(), &ok);
	if (ok == false)
	{
		s += tr("Output signal: <b>%1</b><br>").arg(c->output().appSignalID());
	}
	else
	{
		s += tr("Output signal: <b>%1 (%2) - %3</b><br>").arg(outputParam.appSignalId()).arg(outputParam.customSignalId()).arg(outputParam.caption());
	}

	// Hysteresis

	if (c->hysteresis().isConst() == true)
	{
		s += tr("Hysteresis: <b>%1</b><br>").arg(QString::number(c->hysteresis().constValue(), 'f', c->precision()));
	}
	else
	{
		AppSignalParam hystParam = m_appSignalManager->signalParam(c->hysteresis().appSignalID(), &ok);
		if (ok == false)
		{
			s += tr("Hysteresis: <b>%1/b<br>").arg(c->hysteresis().appSignalID());
		}
		else
		{
			s += tr("Hysteresis: <b>%1 (%2) - %3</b><br>").arg(hystParam.appSignalId()).arg(hystParam.customSignalId()).arg(hystParam.caption());
		}
	}

	//

	s += "<br>";

	switch(c->inAnalogSignalFormat())
	{
		case E::AnalogAppSignalFormat::Float32:
		{
			s += tr("AnalogSignalFormat: <b>Float32</b><br>");
		}
		break;

		case E::AnalogAppSignalFormat::SignedInt32:
		{
			s += tr("AnalogSignalFormat: <b>SignedInt32</b><br>");
		}
		break;

		default:
			Q_ASSERT(false);
			s += tr("AnalogSignalFormat: <b>Unknown</b><br>");
	}

	s += tr("Precision: <b>%1</b><br>").arg(c->precision());

	s += tr("Schema ID: <b>%1</b><br>").arg(c->schemaID());

	s += tr("Schema Item UUID: <b>%1</b><br>").arg(c->schemaItemUuid().toString());

	s += tr("Label: <b>%1</b><br>").arg(c->label());

	textEdit->setHtml(s);
	textEdit->setReadOnly(true);

	setLayout(mainLayout);

	setMinimumSize(600, 200);
}

//
//
//	SignalFlagsWidget
//
//

SignalFlagsWidget::SignalFlagsWidget(QWidget *parent)
	: QWidget(parent)
{
	setMouseTracking(true);
}

void SignalFlagsWidget::paintEvent(QPaintEvent *)
{

	static const QString flagValueFalse = QStringLiteral(" (0)");
	static const QString flagValueTrue = QStringLiteral(" (1)");
	static const QString flagValueNonValid = QStringLiteral(" (?)");

	QPainter painter(this);

	static QBrush alertBrush(QColor(192, 0, 0));
	static QColor noAlertBrush(Qt::darkGreen);
	static QColor noFlagBrush(Qt::lightGray);

	int flagNo = 0;

	for (int j = 0; j < m_rowCount; j++)
	{
		for (int i = 0; i < m_colCount; i++)
		{
			// Draw the rectangle for flag
			//
			bool flagValid = true;
			bool flagValue = false;
			bool flagAlert = false;

			if (flagState(flagNo, &flagValid, &flagValue, &flagAlert) == true)
			{
				painter.setBrush(flagAlert == true ? alertBrush : noAlertBrush);
			}
			else
			{
				painter.setBrush(noFlagBrush);
			}

			QRect flagRect = flag2Rect(flagNo);

			painter.setPen(Qt::NoPen);
			painter.drawRect(flagRect);

			// Draw the text description for flag
			//

			if (flagNo < static_cast<int>(m_flagNames.size()))
			{
				painter.setPen(Qt::white);

				QString text = m_flagNames[flagNo];

				if (flagValid == true)
				{
					text += flagValue == true ? flagValueTrue : flagValueFalse;
				}
				else
				{
					text += flagValueNonValid;
				}

				painter.drawText(flagRect, Qt::AlignHCenter | Qt::AlignCenter, text);
			}

			flagNo++;
		}
	}
}



void SignalFlagsWidget::mouseMoveEvent(QMouseEvent *event)
{
	if (event == nullptr)
	{
		Q_ASSERT(event);
		return;
	}

	int flagAbove = point2Flag(event->pos());

	if (m_lastFlagAbove != flagAbove)
	{
		m_lastFlagAbove = flagAbove;

		if (flagAbove >= 0 && flagAbove < static_cast<int>(m_flagTooltips.size()))
		{
			QToolTip::showText(mapToGlobal(event->pos()), m_flagTooltips[flagAbove], this);
		}
		else
		{
			QToolTip::hideText();
		}
	}
}

QRect SignalFlagsWidget::flag2Rect(int flagNo)
{
	double colWidth = size().width() / m_colCount;
	double rowHeight = size().height() / m_rowCount;

	int col = flagNo % m_colCount;
	int row = flagNo / m_colCount;

	const int frameWidth = 1;

	double x = (colWidth + frameWidth) * col;
	double y = (rowHeight + frameWidth) * row;

	return QRect(static_cast<int>(x),
				 static_cast<int>(y),
				 static_cast<int>(colWidth - frameWidth * 2),
				 static_cast<int>(rowHeight - frameWidth * 2));
}

int SignalFlagsWidget::point2Flag(const QPoint& pt)
{
	for (int i = 0; i < static_cast<int>(m_flagNames.size()); i++)
	{
		QRect r = flag2Rect(i);

		if (r.contains(pt) == true)
		{
			return i;
		}
	}

	return -1;
}

//
// AppSignalFlagsWidget
//

AppSignalFlagsWidget::AppSignalFlagsWidget(QWidget* parent)
	:SignalFlagsWidget(parent)
{
	m_flagNames =
		{
			QStringLiteral("VALID"),
			QStringLiteral("ST.AVAIL"),
			QStringLiteral("SIM"),
			QStringLiteral("LOCK"),
			QStringLiteral("MISMATCH"),
			QStringLiteral("HIGH"),
			QStringLiteral("LOW")
		};

	m_flagTooltips =
		{
			QStringLiteral("Signal Validity\n\nSet to 1 when validity signal of the signal is set to 1.\nSet to 0 when validity signal of the signal is set to 0.\nIf no validity signal exists - equal to \"ST.AVAIL\" flag."),
			QStringLiteral("Signal State is Available\n\nSet to 1 if application data is received from LM.\nSet to 0 if no application data is received from LM."),
			QStringLiteral("Signal is Simulated\n\nSet to 1 when simulation signal is set to 1 (see AFB sim_lock),\notherwise set to 0. If no simulation signal exists, also set to 0."),
			QStringLiteral("Signal is Locked\n\nSet to 1 when locking signal is set to 1 (see AFB sim_lock),\notherwise set to 0. If no locking signal exists, also set to 0."),
			QStringLiteral("Signal is Mismatched\n\nSet to 1 when mismatch signal is set to 1 (see AFB mismatch),\notherwise set to 0. If no mismatch signal exists, also set to 0."),
			QStringLiteral("Signal Value is High\n\nSet to 1 when signal value is greater than\nHighEngineeringUnits limit, otherwise set to 0."),
			QStringLiteral("Signal Value is Low\n\nSet to 1 when signal value is less than\nLowEngineeringUnits limit, otherwise set to 0."),
		};

}

void AppSignalFlagsWidget::updateControl(AppSignalStateFlags flags)
{
	m_flags = flags;
	update();
}

bool AppSignalFlagsWidget::flagState(int flagNo, bool* const flagValid, bool* const flagValue, bool* const flagAlert) const
{
	if (flagValid == nullptr || flagValue == nullptr || flagAlert == nullptr)
	{
		Q_ASSERT(flagValid);
		Q_ASSERT(flagValue);
		Q_ASSERT(flagAlert);
		return false;
	}

	if (flagNo >= static_cast<int>(m_flagNames.size()))
	{
		return false;
	}

	AppSignalFlagsFields flag = static_cast<AppSignalFlagsFields>(flagNo);

	if (m_flags.stateAvailable == false && flag != AppSignalFlagsFields::StateAvailable)
	{
		// Flag is not valid
		//
		*flagValid = false;
		*flagAlert = true;

		return true;
	}

	*flagValid = true;

	// Flag is valid, get its value
	//
	switch (flag)
	{
	case AppSignalFlagsFields::Valid:
		*flagValue = m_flags.valid;
		break;

	case AppSignalFlagsFields::StateAvailable:
		*flagValue = m_flags.stateAvailable;
		break;

	case AppSignalFlagsFields::Simulated:
		*flagValue = m_flags.simulated;
		break;

	case AppSignalFlagsFields::Blocked:
		*flagValue = m_flags.blocked;
		break;

	case AppSignalFlagsFields::Mismatch:
		*flagValue = m_flags.mismatch;
		break;

	case AppSignalFlagsFields::AboveHighLimit:
		*flagValue = m_flags.aboveHighLimit;
		break;

	case AppSignalFlagsFields::BelowLowLimit:
		*flagValue = m_flags.belowLowLimit;
		break;

	default:
		Q_ASSERT(false);
		*flagValue = false;
	}

	*flagAlert = *flagValue;

	// Valid and StateAvailable flags colors are inverted
	//
	if (flag == AppSignalFlagsFields::Valid ||	flag == AppSignalFlagsFields::StateAvailable)
	{
		*flagAlert = !*flagAlert;
	}

	return true;
}


//
// TuningSignalFlagsWidget
//

TuningSignalFlagsWidget::TuningSignalFlagsWidget(QWidget* parent)
	:SignalFlagsWidget(parent)
{
	m_flagNames =
		{
			QStringLiteral("VALID"),
			QStringLiteral("RANGE"),
			QStringLiteral("WRITING"),
			QStringLiteral("CONTROL"),
			QStringLiteral("ACCESS"),
			QStringLiteral("DEFAULT"),
		};

	m_flagTooltips =
		{
			QStringLiteral("Signal validity\n\nSet to 1 if tuning data is received from LM.\nSet to 0 if no tuning data is received from LM."),
			QStringLiteral("Signal is out of range\n\nSet to 1 when tuning value is out of range, otherwise set to 0."),
			QStringLiteral("Writing in progress\n\nSet to 1 when writing a value is in progress. Resets to 0 after writing was finished "),
			QStringLiteral("Control is enabled\n\nSet to 1 when  tuning control for LM is enabled by TuningService, otherwise set to 0.\n\nNOTE: if SingleLmControl property of TuningService is set to false, this flag is always set to 1."),
			QStringLiteral("Writing is enabled\n\nSet to 1 when LM access key is set, otherwise set to 0.\n\nNOTE: this flag is used only when StatusFlagFunction property\nof TuningClient  is set to 'AccessKey'."),
			QStringLiteral("Default value\n\nSet to 1 tuning signal value is set to default value,\notherwise set to 0."),
		};

}

void TuningSignalFlagsWidget::updateControl(TuningSignalStateFlags flags)
{
	m_flags = flags;
	update();
}

bool TuningSignalFlagsWidget::flagState(int flagNo, bool* const flagValid, bool* const flagValue, bool* const flagAlert) const
{
	if (flagValid == nullptr || flagValue == nullptr || flagAlert == nullptr)
	{
		Q_ASSERT(flagValid);
		Q_ASSERT(flagValue);
		Q_ASSERT(flagAlert);
		return false;
	}

	if (flagNo >= static_cast<int>(m_flagNames.size()))
	{
		return false;
	}

	TuningSignalFlagsFields flag = static_cast<TuningSignalFlagsFields>(flagNo);

	if (m_flags.valid == false && flag != TuningSignalFlagsFields::Valid)
	{
		// Flag is not valid
		//
		*flagValid = false;
		*flagAlert = true;

		return true;
	}

	*flagValid = true;

	// Flag is valid, get its value
	//
	switch (flag)
	{
	case TuningSignalFlagsFields::Valid:
		*flagValue = m_flags.valid;
		break;

	case TuningSignalFlagsFields::OutOfRange:
		*flagValue = m_flags.outOfRange;
		break;

	case TuningSignalFlagsFields::WriteInProgress:
		*flagValue = m_flags.writeInProgress;
		break;

	case TuningSignalFlagsFields::ControlIsEnabled:
		*flagValue = m_flags.controlIsEnabled;
		break;

	case TuningSignalFlagsFields::WritingIsEnabled:
		*flagValue = m_flags.writingIsEnabled;
		break;

	case TuningSignalFlagsFields::TuningDefault:
		*flagValue = m_flags.tuningDefault;
		break;

	default:
		Q_ASSERT(false);
		*flagValue = false;
	}

	*flagAlert = *flagValue;

	// Valid and StateAvailable flags colors are inverted
	//
	if (flag == TuningSignalFlagsFields::Valid ||
		flag == TuningSignalFlagsFields::ControlIsEnabled ||
		flag == TuningSignalFlagsFields::WritingIsEnabled ||
		flag == TuningSignalFlagsFields::TuningDefault)
	{
		*flagAlert = !*flagAlert;
	}

	return true;
}

//
//
//	DialogSignalInfo
//
//

std::map<QString, DialogSignalInfo*> DialogSignalInfo::m_dialogSignalInfoMap;

DialogSignalInfo::DialogSignalInfo(const AppSignalParam& signal,
								   IAppSignalManager* appSignalManager,
								   VFrame30::TuningController* tuningController,
								   bool tuningEnabled,
								   DialogSignalInfo::DialogType dialogType,
								   QWidget* parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogSignalInfo),
	m_signal(signal),
	m_appSignalManager(appSignalManager),
	m_tuningController(tuningController),	// it can be nullptr
	m_tuningEnabled(tuningEnabled)
{
	if (m_appSignalManager == nullptr)
	{
		Q_ASSERT(m_appSignalManager);
		return;
	}

	ui->setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);

	// Restore settings
	//
	QSettings settings;

	QPoint signalInfoPos = settings.value("DialogSignalInfo/pos", QPoint(-1, -1)).toPoint();
	QByteArray signalInfoGeometry = settings.value("DialogSignalInfo/geometry").toByteArray();

	if (signalInfoPos.x() != -1 && signalInfoPos.y() != -1)
	{
		move(signalInfoPos);
		restoreGeometry(signalInfoGeometry);
	}

	//

	ui->tabWidget->tabBar()->setExpanding(true);

	// Setup time labels

	QDateTime time = QDateTime::currentDateTime();
	ui->labelServerTime->setText(time.toString("dd.MM.yyyy hh:mm:ss.zzz"));
	ui->labelPlantTime->setText(time.toString("dd.MM.yyyy hh:mm:ss.zzz"));

	// Setup tree controls

	ui->treeProperties->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->treeProperties, &QTreeWidget::customContextMenuRequested,this, &DialogSignalInfo::preparePropertiesContextMenu);

	ui->treePropertiesExt->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->treePropertiesExt, &QTreeWidget::customContextMenuRequested,this, &DialogSignalInfo::preparePropertiesExtContextMenu);

	ui->treeSetpoints->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->treeSetpoints, &QTreeWidget::customContextMenuRequested,this, &DialogSignalInfo::prepareSetpointsContextMenu);

	ui->treeSchemas->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->treeSchemas, &QTreeWidget::customContextMenuRequested,this, &DialogSignalInfo::prepareSchemaContextMenu);

	ui->tabWidget->setCurrentIndex(0);

	// Remove tuning tab

	if (dialogType == DialogType::Simulator)
	{
		ui->labelPlantTimeHeader->hide();
		ui->labelPlantTime->hide();

		ui->labelServerTimeHeader->setText(tr("Plant Time"));
	}

	if (dialogType == DialogType::Monitor)
	{
		removeTabPage("Extended");
	}

	UiTools::adjustDialogPlacement(this);

	m_updateStateTimerId = startTimer(200);

	setAcceptDrops(true);
}

DialogSignalInfo::~DialogSignalInfo()
{
	unregisterDialog(m_signal.appSignalId());

	// Save window position
	//
	QSettings settings;

	QPoint signalInfoPos = pos();
	QByteArray signalInfoGeometry = saveGeometry();
	QByteArray signalInfoTreeSetpointsState = ui->treeSetpoints->header()->saveState();

	settings.setValue("DialogSignalInfo/pos", signalInfoPos);
	settings.setValue("DialogSignalInfo/geometry", signalInfoGeometry);
	settings.setValue("DialogSignalInfo/treeSetpointsState", signalInfoTreeSetpointsState);

	delete ui;
}

DialogSignalInfo* DialogSignalInfo::dialogRegistered(const QString& appSignalId)
{
	auto it = m_dialogSignalInfoMap.find(appSignalId);
	if (it == m_dialogSignalInfoMap.end())
	{
		return nullptr;
	}

	return it->second;
}

void DialogSignalInfo::registerDialog(const QString& appSignalId, DialogSignalInfo* dialog)
{
	m_dialogSignalInfoMap[appSignalId] = dialog;
}

void DialogSignalInfo::unregisterDialog(const QString& appSignalId)
{
	auto it = m_dialogSignalInfoMap.find(appSignalId);
	if (it == m_dialogSignalInfoMap.end())
	{

		Q_ASSERT(false);
	}
	else
	{
		m_dialogSignalInfoMap.erase(it);
	}
}


bool DialogSignalInfo::tuningEnabled() const
{
	return m_tuningEnabled;
}

void DialogSignalInfo::setTuningEnabled(bool enabled)
{
	m_tuningEnabled = enabled;
}

AppSignalParam DialogSignalInfo::signal() const
{
	return m_signal;
}

void DialogSignalInfo::updateSignal(const AppSignalParam& signal)
{
	m_signal = signal;

	fillSignalData();

	return;
}

void DialogSignalInfo::preparePropertiesContextMenu(const QPoint& pos)
{
	Q_UNUSED(pos);

	QMenu menu(this);

	QTreeWidgetItem* item = ui->treeProperties->currentItem();
	if (item == nullptr)
	{
		return;
	}

	// Copy
	QAction* actionCopy = new QAction(tr("Copy"), &menu);

	auto f = [this]() -> void
			 {
				QClipboard *clipboard = QApplication::clipboard();
				QTreeWidgetItem* item = ui->treeProperties->currentItem();
				if (item == nullptr)
				{
					return;
				}
				clipboard->setText(item->text(1));
			};

	connect(actionCopy, &QAction::triggered, this, f);

	menu.addAction(actionCopy);

	//
	menu.exec(QCursor::pos());
}

void DialogSignalInfo::preparePropertiesExtContextMenu(const QPoint& pos)
{
	Q_UNUSED(pos);

	QMenu menu(this);

	QTreeWidgetItem* item = ui->treePropertiesExt->currentItem();
	if (item == nullptr)
	{
		return;
	}

	// Copy
	QAction* actionCopy = new QAction(tr("Copy"), &menu);

	auto f = [this]() -> void
			 {
				QClipboard *clipboard = QApplication::clipboard();
				QTreeWidgetItem* item = ui->treePropertiesExt->currentItem();
				if (item == nullptr)
				{
					return;
				}
				clipboard->setText(item->text(1));
			};

	connect(actionCopy, &QAction::triggered, this, f);

	menu.addAction(actionCopy);

	//
	menu.exec(QCursor::pos());
}

void DialogSignalInfo::prepareSchemaContextMenu(const QPoint& pos)
{
	Q_UNUSED(pos);

	QMenu menu(this);

	QTreeWidgetItem* item = ui->treeSchemas->itemAt(pos);
	if (item == nullptr)
	{
		return;
	}

	m_contextMenuSchemaId = item->text(static_cast<int>(SchemasColumns::SchemaId));

	// Switch

	QAction* actionSwitch = new QAction(tr("Switch to '%1'").arg(m_contextMenuSchemaId), &menu);

	connect(actionSwitch, &QAction::triggered, this, &DialogSignalInfo::switchToSchema);

	menu.addAction(actionSwitch);

	//
	menu.exec(QCursor::pos());
}

void DialogSignalInfo::prepareSetpointsContextMenu(const QPoint& pos)
{
	Q_UNUSED(pos);

	QMenu menu(this);

	QTreeWidgetItem* item = ui->treeSetpoints->itemAt(pos);
	if (item == nullptr)
	{
		return;
	}

	m_contextMenuSchemaId = item->text(static_cast<int>(SetpointsColumns::SchemaId));

	m_contextMenuSetpointIndex = item->data(static_cast<int>(SetpointsColumns::Type), Qt::UserRole).toInt();

	// Switch

	QAction* actionSwitch = new QAction(tr("Switch to '%1'").arg(m_contextMenuSchemaId), &menu);

	connect(actionSwitch, &QAction::triggered, this, &DialogSignalInfo::switchToSchema);

	menu.addAction(actionSwitch);

	// Details

	QAction* actionDetails = new QAction(tr("Details..."), &menu);

	connect(actionDetails, &QAction::triggered, this, &DialogSignalInfo::showSetpointDetails);

	menu.addAction(actionDetails);

	//
	menu.exec(QCursor::pos());
}

void DialogSignalInfo::on_treeSchemas_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	Q_UNUSED(column);

	if (item == nullptr)
	{
		return;
	}

	QString schemaId = item->text(static_cast<int>(SchemasColumns::SchemaId));

	setSchema(schemaId, QStringList() << m_signal.appSignalId());

	return;
}

void DialogSignalInfo::on_treeSetpoints_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	Q_UNUSED(column);

	if (item == nullptr)
	{
		Q_ASSERT(item);
		return;
	}

	m_contextMenuSetpointIndex = item->data(static_cast<int>(SetpointsColumns::Type), Qt::UserRole).toInt();

	showSetpointDetails();

	// Uncomment this to switch to schema

	/*Q_UNUSED(column);

	if (item == nullptr)
	{
		return;
	}

	MonitorSchemaWidget* currentTab = m_centralWidget->currentTab();
	if (currentTab == nullptr)
	{
		Q_ASSERT(currentTab);
		return;
	}

	QString schemaId = item->text(static_cast<int>(SetpointsColumns::SchemaId));

	if (currentTab->schemaId() != schemaId)
	{
		QStringList appSignals;
		appSignals << m_signal.appSignalId();

		currentTab->setSchema(schemaId, appSignals);
	}*/

	return;
}

void DialogSignalInfo::on_pushButtonSetZero_clicked()
{
	if (m_tuningController == nullptr || m_signal.isDiscrete() == false)
	{
		Q_ASSERT(false);
		return;
	}

	int result = QMessageBox::warning(this, qAppName(),
									  tr("Are you sure you want to write value '%1'\n\nto signal '%2' ('%3')?")
									  .arg(0).
									  arg(m_signal.customSignalId()).
									  arg(m_signal.caption()),
									  QMessageBox::Yes|QMessageBox::No,
									  QMessageBox::No);

	if (result != QMessageBox::Yes)
	{
		return;
	}

	m_tuningController->writeValue(m_signal.appSignalId(), false);
}

void DialogSignalInfo::on_pushButtonSetOne_clicked()
{
	if (m_tuningController == nullptr || m_signal.isDiscrete() == false)
	{
		Q_ASSERT(false);
		return;
	}

	int result = QMessageBox::warning(this, qAppName(),
									  tr("Are you sure you want to write value '%1'\n\nto signal '%2' ('%3')?")
									  .arg(1).
									  arg(m_signal.customSignalId()).
									  arg(m_signal.caption()),
									  QMessageBox::Yes|QMessageBox::No,
									  QMessageBox::No);

	if (result != QMessageBox::Yes)
	{
		return;
	}

	m_tuningController->writeValue(m_signal.appSignalId(), true);
}

void DialogSignalInfo::on_pushButtonSetValue_clicked()
{
	if (m_tuningController == nullptr || m_signal.isAnalog() == false)
	{
		Q_ASSERT(false);
		return;
	}

	QString strValue = ui->editInputValue->text();

	if (strValue.isEmpty() == true)
	{
		return;
	}

	bool ok = false;

	double value = strValue.toDouble(&ok);
	if (ok == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Invalid input value!"));
		return;
	}

	int result = QMessageBox::warning(this, qAppName(),
									  tr("Are you sure you want to write value '%1'\n\nto signal '%2' ('%3')?")
									  .arg(strValue).
									  arg(m_signal.customSignalId()).
									  arg(m_signal.caption()),
									  QMessageBox::Yes|QMessageBox::No,
									  QMessageBox::No);

	if (result != QMessageBox::Yes)
	{
		return;
	}

	m_tuningController->writeValue(m_signal.appSignalId(), value);
}


void DialogSignalInfo::switchToSchema()
{
	setSchema(m_contextMenuSchemaId, QStringList() << m_signal.appSignalId());
}

void DialogSignalInfo::showSetpointDetails()
{
	if (m_contextMenuSetpointIndex < 0 || m_contextMenuSetpointIndex >= m_setpoints.size())
	{
		Q_ASSERT(false);
		return;
	}

	DialogSetpointDetails* d = new DialogSetpointDetails(this, m_appSignalManager, m_setpoints[m_contextMenuSetpointIndex]);
	d->exec();
}

void DialogSignalInfo::showEvent(QShowEvent* /*e*/)
{
	if (m_firstShow == false)
	{
		return;
	}

	m_firstShow = false;

	// Fill signal information

	fillSignalData();

	updateSignalData();

	return;
}

void DialogSignalInfo::timerEvent(QTimerEvent* event)
{
	Q_ASSERT(event);

	if  (event->timerId() == m_updateStateTimerId)
	{
		updateSignalData();
	}
}

void DialogSignalInfo::mousePressEvent(QMouseEvent* event)
{
	if ((ui->labelValue->underMouse() || ui->labelValueTuning->underMouse()) && event->button() == Qt::RightButton)
	{
		stateContextMenu(event->pos());
	}
}

void DialogSignalInfo::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasFormat(AppSignalParamMimeType::value) == false)
	{
		return;

	}

	// Load data from drag and drop
	//
	QByteArray data = event->mimeData()->data(AppSignalParamMimeType::value);

	::Proto::AppSignalSet protoSetMessage;
	bool ok = protoSetMessage.ParseFromArray(data.constData(), data.size());
	if (ok == false)
	{
		return;
	}

	std::vector<AppSignalParam> appSignals;
	appSignals.reserve(protoSetMessage.appsignal_size());

	// Parse data
	//
	ok = false;
	AppSignalParam appSignalParam;

	for (int i = 0; i < protoSetMessage.appsignal_size(); i++)
	{
		const ::Proto::AppSignal& appSignalMessage = protoSetMessage.appsignal(i);

		ok = appSignalParam.load(appSignalMessage);

		if (ok == true)
		{
			break;
		}
	}

	if (ok == false)
	{
		return;
	}

	// Accept only if signal hash is different
	//
	if (appSignalParam.hash() == m_signal.hash())
	{
		return;
	}

	event->acceptProposedAction();

	return;
}

void DialogSignalInfo::dropEvent(QDropEvent* event)
{
	if (event->mimeData()->hasFormat(AppSignalParamMimeType::value) == false)
	{
		return;
	}

	// Load data from drag and drop
	//
	QByteArray data = event->mimeData()->data(AppSignalParamMimeType::value);

	::Proto::AppSignalSet protoSetMessage;
	bool ok = protoSetMessage.ParseFromArray(data.constData(), data.size());
	if (ok == false)
	{
		event->acceptProposedAction();
		return;
	}

	std::vector<AppSignalParam> appSignals;
	appSignals.reserve(protoSetMessage.appsignal_size());

	// Parse data
	//
	ok = false;
	AppSignalParam newSignal;

	for (int i = 0; i < protoSetMessage.appsignal_size(); i++)
	{
		const ::Proto::AppSignal& appSignalMessage = protoSetMessage.appsignal(i);

		ok = newSignal.load(appSignalMessage);

		if (ok == true)
		{
			break;
		}
	}

	if (ok == false)
	{
		return;
	}

	AppSignalParam oldSignal = m_signal;

	// Unregister current dialog with old id
	//
	unregisterDialog(oldSignal.appSignalId());

	// If there is another dialog with such id - swap it with current and re-register with new id
	//
	DialogSignalInfo* dsi = DialogSignalInfo::dialogRegistered(newSignal.appSignalId());
	if (dsi != nullptr)
	{
		unregisterDialog(newSignal.appSignalId());

		dsi->updateSignal(oldSignal);

		registerDialog(oldSignal.appSignalId(), dsi);
	}

	// Set new signal to current dialog
	//
	updateSignal(newSignal);

	// Register current dialog with new id
	//
	registerDialog(newSignal.appSignalId(), this);

	//qDebug() << "Dialog list:";
	//for (auto it : m_dialogSignalInfoMap)
	//{
	//	qDebug() << it.first << it.second;
	//}

	return;
}

int DialogSignalInfo::tabPageExists(const QString& tabName)
{
	for (int i = 0; i < ui->tabWidget->count(); i++)
	{
		if (ui->tabWidget->tabText(i) == tabName)
		{
			return true;
		}
	}

	return false;
}

QWidget* DialogSignalInfo::tabPageWidget(const QString& tabName)
{
	for (int i = 0; i < ui->tabWidget->count(); i++)
	{
		if (ui->tabWidget->tabText(i) == tabName)
		{
			return ui->tabWidget->widget(i);
		}
	}

	return nullptr;
}

void DialogSignalInfo::addTabPage(const QString& tabName, QWidget* widget)
{
	ui->tabWidget->addTab(widget, tabName);
}

void DialogSignalInfo::removeTabPage(const QString& tabName)
{
	for (int i = 0; i < ui->tabWidget->count(); i++)
	{
		if (ui->tabWidget->tabText(i) == tabName)
		{
			ui->tabWidget->removeTab(i);
			break;
		}
	}
}

void DialogSignalInfo::fillSignalData()
{
	// Fill signal information

	// signalIdLabel is promoted to QLabelAppSignalDragAndDrop
	// tu support Drag and Drop Operation (drag part)
	//

	ui->signalIdLabel->setAppSignal(m_signal);
	ui->signalIdLabelIcon->setAppSignal(m_signal);

	fillSignalInfo();
	fillProperties();
	fillExtProperties();
	fillSetpoints();
	fillSchemas();
	fillTuningTab();

	return;
}

void DialogSignalInfo::fillSignalInfo()
{
	// Fill main signal information
	//
	m_currentPrecision = m_signal.precision();

	setWindowTitle(m_signal.customSignalId() + tr(" - ") + m_signal.caption());

	if (m_signal.isAnalog())
	{
		ui->labelStrUnit->setText(m_signal.unit());
		ui->labelStrUnitTuning->setText(m_signal.unit());
	}
	else
	{
		ui->labelStrUnit->setText("");
		ui->labelStrUnitTuning->setText("");
	}
	ui->labelValue->setText("");
	ui->labelValueTuning->setText("");

	QFont font = ui->labelValue->font();
	font.setPixelSize(m_currentFontSize);
	ui->labelValue->setFont(font);
	ui->labelValueTuning->setFont(font);
	ui->labelStrUnit->setFont(font);
	ui->labelStrUnitTuning->setFont(font);

	ui->editAppSignalID->setText(m_signal.appSignalId());
	ui->editCustomAppSignalID->setText(m_signal.customSignalId());
	ui->editCaption->setPlainText(m_signal.caption());
	ui->editEquipment->setText(m_signal.equipmentId());

	QString str = E::valueToString<E::SignalType>(m_signal.type());
	if (m_signal.isAnalog())
	{
		str = QString("%1 (%2)").arg(str).arg(E::valueToString<E::AnalogAppSignalFormat>(static_cast<int>(m_signal.analogSignalFormat())));
	}

	str = QString("%1, %2").arg(str).arg(E::valueToString<E::SignalInOutType>(m_signal.inOutType()));

	ui->editSignalType->setText(str);

	ui->editChannel->setText(E::valueToString<E::Channel>(m_signal.channel()));
}

void DialogSignalInfo::fillProperties()
{
	ui->treeProperties->clear();

	// Fill properties list
	//
	QStringList columns;
	columns << "Caption";
	columns << "Value";
	ui->treeProperties->setHeaderLabels(columns);

	QTreeWidgetItem* itemGroup1 = new QTreeWidgetItem(QStringList()<<tr("General"));

	itemGroup1->addChild(new QTreeWidgetItem(QStringList() << tr("AppSignalID") << m_signal.appSignalId()));
	itemGroup1->addChild(new QTreeWidgetItem(QStringList() << tr("EquipmentID") << m_signal.equipmentId()));

	if (m_signal.isAnalog())
	{
		itemGroup1->addChild(new QTreeWidgetItem(QStringList() << tr("Unit") << m_signal.unit()));
	}

	itemGroup1->addChild(new QTreeWidgetItem(QStringList() << tr("Tags") << m_signal.tagStringList().join(' ')));

	ui->treeProperties->addTopLevelItem(itemGroup1);
	itemGroup1->setExpanded(true);

	QTreeWidgetItem* itemGroup2 = new QTreeWidgetItem(QStringList() << tr("Format"));

	//if (m_signal.isAnalog())
	//{
		itemGroup2->addChild(new QTreeWidgetItem(QStringList() << tr("ByteOrder") << E::valueToString<E::ByteOrder>(m_signal.byteOrder())));
	//}
	ui->treeProperties->addTopLevelItem(itemGroup2);
	itemGroup2->setExpanded(true);

	if (m_signal.isAnalog())
	{
		QTreeWidgetItem* itemGroup3 = new QTreeWidgetItem(QStringList() << tr("Parameters"));

		itemGroup3->addChild(new QTreeWidgetItem(QStringList() << tr("Precision") << QString::number(m_signal.precision())));
		itemGroup3->addChild(new QTreeWidgetItem(QStringList() << tr("Aperture") << QString::number(m_signal.aperture(), 'f', m_signal.precision())));
		itemGroup3->addChild(new QTreeWidgetItem(QStringList() << tr("FilteringTime") << QString::number(m_signal.filteringTime(), 'f', m_signal.precision())));
		itemGroup3->addChild(new QTreeWidgetItem(QStringList() << tr("SpreadTolerance") << QString::number(m_signal.spreadTolerance(), 'f', m_signal.precision())));

		ui->treeProperties->addTopLevelItem(itemGroup3);
		itemGroup3->setExpanded(true);
	}

	if (m_signal.isAnalog())
	{
		QTreeWidgetItem* itemGroup4 = new QTreeWidgetItem(QStringList() << tr("Limits"));

		itemGroup4->addChild(new QTreeWidgetItem(QStringList() << tr("LowEngineeringUnits") << QString::number(m_signal.lowEngineeringUnits(), 'f', m_signal.precision())));
		itemGroup4->addChild(new QTreeWidgetItem(QStringList() << tr("HighEngineeringUnits")<<QString::number(m_signal.highEngineeringUnits(), 'f', m_signal.precision())));
		itemGroup4->addChild(new QTreeWidgetItem(QStringList() << tr("LowValidRange")<<QString::number(m_signal.lowValidRange(), 'f', m_signal.precision())));
		itemGroup4->addChild(new QTreeWidgetItem(QStringList() << tr("HighValidRange")<<QString::number(m_signal.highValidRange(), 'f', m_signal.precision())));

		ui->treeProperties->addTopLevelItem(itemGroup4);
		itemGroup4->setExpanded(true);
	}

	if (m_signal.isInput() && m_signal.isAnalog())
	{
		QTreeWidgetItem* itemGroup5 = new QTreeWidgetItem(QStringList() << tr("Input"));

		itemGroup5->addChild(new QTreeWidgetItem(QStringList() << tr("InputLowLimit") << QString::number(m_signal.inputLowLimit(), 'f', m_signal.precision())));
		itemGroup5->addChild(new QTreeWidgetItem(QStringList() << tr("InputHighLimit") << QString::number(m_signal.inputHighLimit(), 'f', m_signal.precision())));
		itemGroup5->addChild(new QTreeWidgetItem(QStringList() << tr("InputUnitID") << QString::number(m_signal.inputUnitId())));
		itemGroup5->addChild(new QTreeWidgetItem(QStringList() << tr("InputSensorID") << E::valueToString<E::SensorType>(m_signal.inputSensorType())));

		ui->treeProperties->addTopLevelItem(itemGroup5);
		itemGroup5->setExpanded(true);
	}

	if (m_signal.isOutput() && m_signal.isAnalog())
	{
		QTreeWidgetItem* itemGroup6 = new QTreeWidgetItem(QStringList()<<tr("Output"));

		itemGroup6->addChild(new QTreeWidgetItem(QStringList() << tr("OutputLowLimit") << QString::number(m_signal.outputLowLimit(), 'f', m_signal.precision())));
		itemGroup6->addChild(new QTreeWidgetItem(QStringList() << tr("OutputHighLimit") << QString::number(m_signal.outputHighLimit(), 'f', m_signal.precision())));
		itemGroup6->addChild(new QTreeWidgetItem(QStringList() << tr("OutputUnitID") << QString::number(m_signal.outputUnitId())));
		itemGroup6->addChild(new QTreeWidgetItem(QStringList() << tr("OutputSensorID") << E::valueToString<E::SensorType>(m_signal.outputSensorType())));
		itemGroup6->addChild(new QTreeWidgetItem(QStringList() << tr("OutputMode") << E::valueToString<E::OutputMode>(m_signal.outputMode())));

		ui->treeProperties->addTopLevelItem(itemGroup6);
		itemGroup6->setExpanded(true);
	}

	if (m_signal.enableTuning())
	{
		QTreeWidgetItem* itemGroup7 = new QTreeWidgetItem(QStringList()<<tr("Tuning"));

		itemGroup7->addChild(new QTreeWidgetItem(QStringList()<<tr("EnableTuning")<<(m_signal.enableTuning() ? tr("Yes") : tr("No"))));
		itemGroup7->addChild(new QTreeWidgetItem(QStringList()<<tr("TuningDefaultValue")<<m_signal.tuningDefaultValue().toString()));
		itemGroup7->addChild(new QTreeWidgetItem(QStringList()<<tr("TuningLowBound")<<m_signal.tuningLowBound().toString()));
		itemGroup7->addChild(new QTreeWidgetItem(QStringList()<<tr("TuningHighBound")<<m_signal.tuningHighBound().toString()));

		ui->treeProperties->addTopLevelItem(itemGroup7);
		itemGroup7->setExpanded(true);
	}

	ui->treeProperties->resizeColumnToContents(0);
}

void DialogSignalInfo::fillExtProperties()
{
	ui->treePropertiesExt->clear();

	std::optional<AppSignal> signalExtOpt = getSignalExt(m_signal);

	if (signalExtOpt.has_value() == false)
	{
		return;
	}

	AppSignal& signalExt = signalExtOpt.value();

	// Fill properties list
	//
	QStringList columns;
	columns << "Caption";
	columns << "Value";
	ui->treePropertiesExt->setHeaderLabels(columns);

	QTreeWidgetItem* itemGroup1 = new QTreeWidgetItem(QStringList()<<tr("General"));

	itemGroup1->addChild(new QTreeWidgetItem(QStringList() << tr("AppSignalID") << signalExt.appSignalID()));
	itemGroup1->addChild(new QTreeWidgetItem(QStringList() << tr("EquipmentID") << signalExt.equipmentID()));
	itemGroup1->addChild(new QTreeWidgetItem(QStringList() << tr("LmEquipmentID") << signalExt.lmEquipmentID()));
	itemGroup1->addChild(new QTreeWidgetItem(QStringList() << tr("BusTypeID") << signalExt.busTypeID()));
	itemGroup1->addChild(new QTreeWidgetItem(QStringList() << tr("ExcludeFromBuild") << (signalExt.excludeFromBuild() ? tr("Yes") : tr("No")) ));

	if (m_signal.isAnalog())
	{
		itemGroup1->addChild(new QTreeWidgetItem(QStringList() << tr("Unit") << signalExt.unit()));
	}

	itemGroup1->addChild(new QTreeWidgetItem(QStringList() << tr("Tags") << signalExt.tags().join(' ')));

	ui->treePropertiesExt->addTopLevelItem(itemGroup1);
	itemGroup1->setExpanded(true);

	QTreeWidgetItem* itemGroup2 = new QTreeWidgetItem(QStringList() << tr("Format"));

	if (m_signal.isAnalog())
	{
		itemGroup2->addChild(new QTreeWidgetItem(QStringList() << tr("ByteOrder") << E::valueToString<E::ByteOrder>(signalExt.byteOrder())));
		itemGroup2->addChild(new QTreeWidgetItem(QStringList() << tr("DataSize") << QString::number(signalExt.dataSize())));
		itemGroup2->addChild(new QTreeWidgetItem(QStringList() << tr("DataFormat") << E::valueToString<E::DataFormat>(signalExt.dataFormat())));
	}

	ui->treePropertiesExt->addTopLevelItem(itemGroup2);
	itemGroup2->setExpanded(true);

	if (m_signal.isAnalog())
	{
		QTreeWidgetItem* itemGroup3 = new QTreeWidgetItem(QStringList() << tr("Parameters"));

		itemGroup3->addChild(new QTreeWidgetItem(QStringList() << tr("Precision") << QString::number(signalExt.decimalPlaces())));
		itemGroup3->addChild(new QTreeWidgetItem(QStringList() << tr("Acquire") << (signalExt.acquire() ? tr("Yes") : tr("No")) ));
		itemGroup3->addChild(new QTreeWidgetItem(QStringList() << tr("Archive") << (signalExt.archive() ? tr("Yes") : tr("No")) ));
		itemGroup3->addChild(new QTreeWidgetItem(QStringList() << tr("CoarseAperture") << QString::number(signalExt.coarseAperture(), 'f', signalExt.decimalPlaces())));
		itemGroup3->addChild(new QTreeWidgetItem(QStringList() << tr("FineAperture") << QString::number(signalExt.fineAperture(), 'f', signalExt.decimalPlaces())));
		itemGroup3->addChild(new QTreeWidgetItem(QStringList() << tr("AdaptiveAperture") << (signalExt.adaptiveAperture() ? tr("Yes") : tr("No")) ));

		ui->treePropertiesExt->addTopLevelItem(itemGroup3);
		itemGroup3->setExpanded(true);
	}

	if (m_signal.isAnalog())
	{
		QTreeWidgetItem* itemGroup4 = new QTreeWidgetItem(QStringList() << tr("Limits"));

		itemGroup4->addChild(new QTreeWidgetItem(QStringList() << tr("LowADC") << QString::number(signalExt.lowADC())));
		itemGroup4->addChild(new QTreeWidgetItem(QStringList() << tr("HighADC") << QString::number(signalExt.highADC())));
		itemGroup4->addChild(new QTreeWidgetItem(QStringList() << tr("LowDAC") << QString::number(signalExt.lowDAC())));
		itemGroup4->addChild(new QTreeWidgetItem(QStringList() << tr("HighDAC") << QString::number(signalExt.highDAC())));

		itemGroup4->addChild(new QTreeWidgetItem(QStringList() << tr("LowEngineeringUnits") << QString::number(signalExt.lowEngineeringUnits(), 'f', signalExt.decimalPlaces())));
		itemGroup4->addChild(new QTreeWidgetItem(QStringList() << tr("HighEngineeringUnits")<<QString::number(signalExt.highEngineeringUnits(), 'f', signalExt.decimalPlaces())));
		itemGroup4->addChild(new QTreeWidgetItem(QStringList() << tr("LowValidRange")<<QString::number(signalExt.lowValidRange(), 'f', signalExt.decimalPlaces())));
		itemGroup4->addChild(new QTreeWidgetItem(QStringList() << tr("HighValidRange")<<QString::number(signalExt.highValidRange(), 'f', signalExt.decimalPlaces())));

		ui->treePropertiesExt->addTopLevelItem(itemGroup4);
		itemGroup4->setExpanded(true);
	}

	if (m_signal.isInput() && m_signal.isAnalog())
	{
		QTreeWidgetItem* itemGroup5 = new QTreeWidgetItem(QStringList() << tr("Input"));

		itemGroup5->addChild(new QTreeWidgetItem(QStringList() << tr("InputLowLimit") << QString::number(signalExt.electricLowLimit(), 'f', signalExt.decimalPlaces())));
		itemGroup5->addChild(new QTreeWidgetItem(QStringList() << tr("InputHighLimit") << QString::number(signalExt.electricHighLimit(), 'f', signalExt.decimalPlaces())));
		itemGroup5->addChild(new QTreeWidgetItem(QStringList() << tr("InputUnitID") << QString::number(signalExt.electricUnit())));

		itemGroup5->addChild(new QTreeWidgetItem(QStringList() << tr("rload_Ohm") << QString::number(signalExt.rload_Ohm(), 'f', signalExt.decimalPlaces())));
		itemGroup5->addChild(new QTreeWidgetItem(QStringList() << tr("r0_Ohm") << QString::number(signalExt.r0_Ohm(), 'f', signalExt.decimalPlaces())));

		itemGroup5->addChild(new QTreeWidgetItem(QStringList() << tr("SensorType") << E::valueToString<E::SensorType>(signalExt.sensorType())));
		itemGroup5->addChild(new QTreeWidgetItem(QStringList() << tr("FilteringTime")<<QString::number(signalExt.filteringTime(), 'f', signalExt.decimalPlaces())));
		itemGroup5->addChild(new QTreeWidgetItem(QStringList() << tr("SpreadTolerance")<<QString::number(signalExt.spreadTolerance(), 'f', signalExt.decimalPlaces())));

		ui->treePropertiesExt->addTopLevelItem(itemGroup5);
		itemGroup5->setExpanded(true);
	}

	/*if ((m_signal.isInput() || m_signal.isOutput()) && m_signal.isAnalog())
	{
		QTreeWidgetItem* itemGroup5 = new QTreeWidgetItem(QStringList() << tr("Electric"));

		itemGroup5->addChild(new QTreeWidgetItem(QStringList() << tr("ElectricLowLimit") << QString::number(signalExt.electricLowLimit(), 'f', signalExt.decimalPlaces())));
		itemGroup5->addChild(new QTreeWidgetItem(QStringList() << tr("ElectricHighLimit") << QString::number(signalExt.electricHighLimit(), 'f', signalExt.decimalPlaces())));
		itemGroup5->addChild(new QTreeWidgetItem(QStringList() << tr("ElectricUnit") << E::valueToString<E::ElectricUnit>(signalExt.electricUnit())));

		ui->treePropertiesExt->addTopLevelItem(itemGroup5);
		itemGroup5->setExpanded(true);
	}*/


	if (m_signal.isOutput() && m_signal.isAnalog())
	{
		QTreeWidgetItem* itemGroup6 = new QTreeWidgetItem(QStringList()<<tr("Output"));

		itemGroup6->addChild(new QTreeWidgetItem(QStringList() << tr("OutputLowLimit") << QString::number(signalExt.electricLowLimit(), 'f', signalExt.decimalPlaces())));
		itemGroup6->addChild(new QTreeWidgetItem(QStringList() << tr("OutputHighLimit") << QString::number(signalExt.electricHighLimit(), 'f', signalExt.decimalPlaces())));
		itemGroup6->addChild(new QTreeWidgetItem(QStringList() << tr("OutputUnitID") << QString::number(signalExt.electricUnit())));

		itemGroup6->addChild(new QTreeWidgetItem(QStringList() << tr("SensorType") << E::valueToString<E::SensorType>(signalExt.sensorType())));
		itemGroup6->addChild(new QTreeWidgetItem(QStringList() << tr("OutputMode") << E::valueToString<E::OutputMode>(signalExt.outputMode())));

		ui->treePropertiesExt->addTopLevelItem(itemGroup6);
		itemGroup6->setExpanded(true);
	}

	if (m_signal.enableTuning())
	{
		QTreeWidgetItem* itemGroup7 = new QTreeWidgetItem(QStringList()<<tr("Tuning"));

		itemGroup7->addChild(new QTreeWidgetItem(QStringList()<<tr("EnableTuning")<<(signalExt.enableTuning() ? tr("Yes") : tr("No"))));
		itemGroup7->addChild(new QTreeWidgetItem(QStringList()<<tr("TuningDefaultValue")<<signalExt.tuningDefaultValue().toString()));
		itemGroup7->addChild(new QTreeWidgetItem(QStringList()<<tr("TuningLowBound")<<signalExt.tuningLowBound().toString()));
		itemGroup7->addChild(new QTreeWidgetItem(QStringList()<<tr("TuningHighBound")<<signalExt.tuningHighBound().toString()));

		ui->treePropertiesExt->addTopLevelItem(itemGroup7);
		itemGroup7->setExpanded(true);
	}

	{
		QTreeWidgetItem* itemGroup = new QTreeWidgetItem(QStringList()<<tr("Specific properties"));

		itemGroup->addChild(new QTreeWidgetItem(QStringList() << tr("specPropStruct") << signalExt.specPropStruct()));

		ui->treePropertiesExt->addTopLevelItem(itemGroup);
		itemGroup->setExpanded(true);
	}

	{
		QTreeWidgetItem* itemGroup = new QTreeWidgetItem(QStringList()<<tr("Signal fields from database"));

		itemGroup->addChild(new QTreeWidgetItem(QStringList() << tr("ID") << QString::number(signalExt.ID())));
		itemGroup->addChild(new QTreeWidgetItem(QStringList() << tr("signalGroupID") << QString::number(signalExt.signalGroupID())));
		itemGroup->addChild(new QTreeWidgetItem(QStringList() << tr("signalInstanceID") << QString::number(signalExt.signalInstanceID())));
		itemGroup->addChild(new QTreeWidgetItem(QStringList() << tr("changesetID") << QString::number(signalExt.changesetID())));
		itemGroup->addChild(new QTreeWidgetItem(QStringList() << tr("checkedOut") << (signalExt.checkedOut() ? tr("Yes") : tr("No")) ));
		itemGroup->addChild(new QTreeWidgetItem(QStringList() << tr("userID") << QString::number(signalExt.userID())));

		itemGroup->addChild(new QTreeWidgetItem(QStringList() << tr("created") << signalExt.created().toString("yyyy-MM-dd HH:mm:ss")));
		itemGroup->addChild(new QTreeWidgetItem(QStringList() << tr("deleted") << (signalExt.deleted() ? tr("Yes") : tr("No")) ));
		itemGroup->addChild(new QTreeWidgetItem(QStringList() << tr("instanceCreated") << signalExt.instanceCreated().toString("yyyy-MM-dd HH:mm:ss")));
		itemGroup->addChild(new QTreeWidgetItem(QStringList() << tr("instanceAction") << E::valueToString<E::VcsItemAction>(signalExt.instanceAction())));

		ui->treePropertiesExt->addTopLevelItem(itemGroup);
		itemGroup->setExpanded(true);
	}

	{
		QTreeWidgetItem* itemGroup = new QTreeWidgetItem(QStringList()<<tr("Compile-time Properties"));

		itemGroup->addChild(new QTreeWidgetItem(QStringList() << tr("ioBufAddr") << signalExt.ioBufAddr().toString()));
		itemGroup->addChild(new QTreeWidgetItem(QStringList() << tr("tuningAddr") << signalExt.tuningAddr().toString()));
		itemGroup->addChild(new QTreeWidgetItem(QStringList() << tr("tuningAbsAddr") << signalExt.tuningAbsAddr().toString()));
		itemGroup->addChild(new QTreeWidgetItem(QStringList() << tr("ualAddr") << signalExt.ualAddr().toString()));
		itemGroup->addChild(new QTreeWidgetItem(QStringList() << tr("ualAddrIsValid") << (signalExt.ualAddrIsValid() ? tr("Yes") : tr("No")) ));
		itemGroup->addChild(new QTreeWidgetItem(QStringList() << tr("regBufAddr") << signalExt.regBufAddr().toString()));
		itemGroup->addChild(new QTreeWidgetItem(QStringList() << tr("regValueAddr") << signalExt.regValueAddr().toString()));
		itemGroup->addChild(new QTreeWidgetItem(QStringList() << tr("regValidityAddr") << signalExt.regValidityAddr().toString()));

		itemGroup->addChild(new QTreeWidgetItem(QStringList() << tr("lmRamAccess") << E::valueToString<E::LogicModuleRamAccess>(signalExt.lmRamAccess())));
		itemGroup->addChild(new QTreeWidgetItem(QStringList() << tr("regValueAddrStr") << signalExt.regValueAddrStr()));
		itemGroup->addChild(new QTreeWidgetItem(QStringList() << tr("needConversion") << (signalExt.needConversion() ? tr("Yes") : tr("No")) ));

		itemGroup->addChild(new QTreeWidgetItem(QStringList() << tr("isConst") << (signalExt.isConst() ? tr("Yes") : tr("No")) ));
		itemGroup->addChild(new QTreeWidgetItem(QStringList() << tr("constValue") << QString::number(signalExt.constValue(), 'f', signalExt.decimalPlaces())));

		ui->treePropertiesExt->addTopLevelItem(itemGroup);
		itemGroup->setExpanded(true);
	}

	ui->treePropertiesExt->resizeColumnToContents(0);

}

void DialogSignalInfo::fillSetpoints()
{
	ui->treeSetpoints->clear();

	QStringList columns;
	columns << tr("Type");
	columns << tr("Compare To");
	columns << tr("Value");
	columns << tr("Output");
	columns << tr("Value");
	columns << tr("Schema");
	ui->treeSetpoints->setHeaderLabels(columns);

	m_setpoints = m_appSignalManager->setpointsByInputSignalId(m_signal.appSignalId());

	for (int i = 0; i < m_setpoints.size(); i++)
	{
		const std::shared_ptr<Comparator>& c = m_setpoints[i];

		QTreeWidgetItem* item = new QTreeWidgetItem();

		// CompareTo

		if (c->compare().isConst() == true)
		{
			item->setText(static_cast<int>(SetpointsColumns::CompareTo), tr("Const"));
			item->setText(static_cast<int>(SetpointsColumns::CompareToValue), QString::number(c->compare().constValue(), 'f', c->precision()));
		}
		else
		{
			bool ok = false;

			AppSignalParam paramCompareTo = m_appSignalManager->signalParam(c->compare().appSignalID(), &ok);
			if (ok == true)
			{
				item->setText(static_cast<int>(SetpointsColumns::CompareTo), paramCompareTo.customSignalId());

				if (c->compare().isAcquired() == true)
				{
					item->setData(static_cast<int>(SetpointsColumns::CompareTo), Qt::UserRole, QVariant::fromValue(paramCompareTo));
				}
				else
				{
					item->setText(static_cast<int>(SetpointsColumns::CompareToValue), tr("Not acquired"));
				}
			}
			else
			{
				item->setText(static_cast<int>(SetpointsColumns::CompareTo), c->compare().appSignalID());
			}

		}

		// Type

		switch (c->cmpType())
		{
			case E::CmpType::Greate:	item->setText(static_cast<int>(SetpointsColumns::Type), ">");		break;
			case E::CmpType::Equal:		item->setText(static_cast<int>(SetpointsColumns::Type), "=");		break;
			case E::CmpType::Less:		item->setText(static_cast<int>(SetpointsColumns::Type), "<");		break;
			case E::CmpType::NotEqual:	item->setText(static_cast<int>(SetpointsColumns::Type), "<>");		break;
		}

		// Output

		bool ok = false;

		AppSignalParam paramOutput = m_appSignalManager->signalParam(c->output().appSignalID(), &ok);
		if (ok == true)
		{
			item->setText(static_cast<int>(SetpointsColumns::Output), paramOutput.customSignalId());

			if (c->output().isAcquired() == true)
			{
				item->setData(static_cast<int>(SetpointsColumns::Output), Qt::UserRole, QVariant::fromValue(paramOutput));
			}
			else
			{
				item->setText(static_cast<int>(SetpointsColumns::OutputValue), tr("Not acquired"));
			}
		}
		else
		{
			item->setText(static_cast<int>(SetpointsColumns::Output), c->output().appSignalID());
		}

		// Schema

		item->setText(static_cast<int>(SetpointsColumns::SchemaId), c->schemaID());

		item->setData(static_cast<int>(SetpointsColumns::Type), Qt::UserRole, i);

		ui->treeSetpoints->addTopLevelItem(item);
	}

	ui->treeSetpoints->setSortingEnabled(true);
	ui->treeSetpoints->sortByColumn(static_cast<int>(SetpointsColumns::CompareToValue), Qt::AscendingOrder);

	// Adjust column width
	//
	QSettings settings;

	QByteArray signalInfoTreeSetpointsState = settings.value("DialogSignalInfo/treeSetpointsState").toByteArray();

	if (signalInfoTreeSetpointsState.isEmpty() == false)
	{
		ui->treeSetpoints->header()->restoreState(signalInfoTreeSetpointsState);
	}
	else
	{
		ui->treeSetpoints->resizeColumnToContents(static_cast<int>(SetpointsColumns::CompareTo));
		ui->treeSetpoints->resizeColumnToContents(static_cast<int>(SetpointsColumns::Output));
		ui->treeSetpoints->resizeColumnToContents(static_cast<int>(SetpointsColumns::SchemaId));
	}
}

void DialogSignalInfo::fillSchemas()
{
	ui->treeSchemas->clear();

	QStringList columns;
	columns << "Schema";
	ui->treeSchemas->setHeaderLabels(columns);

	QStringList schemas = schemasByAppSignalId(m_signal.appSignalId());

	for (const QString& schema : schemas)
	{

		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setText(0, schema);

		ui->treeSchemas->addTopLevelItem(item);
	}

	ui->treeSchemas->setSortingEnabled(true);
	ui->treeSchemas->sortByColumn(0, Qt::AscendingOrder);
}

void DialogSignalInfo::fillTuningTab()
{
	const QString tuningTabPageName = QObject::tr("Tuning");

	bool tuningEnabled = m_signal.enableTuning() == true && m_tuningEnabled == true;
	bool tuningTabVisible = tabPageExists(tuningTabPageName);

	if (tuningEnabled == false && tuningTabVisible == true)
	{
		// Hide Tuning tab page
		//
		m_tuningTabWidget = tabPageWidget(tuningTabPageName);

		removeTabPage(tr("Tuning"));
	}
	else
	{
		if (tuningEnabled == true && tuningTabVisible == false)
		{
			// Show Tuning tab page
			//
			if (m_tuningTabWidget == nullptr)
			{
				Q_ASSERT(m_tuningTabWidget);
				return;
			}

			addTabPage(tuningTabPageName, m_tuningTabWidget);
		}
	}

	// Update tuning tab page controls
	//
	if (tuningEnabled == true)
	{
		ui->editInputValue->setVisible(m_signal.isAnalog());
		ui->pushButtonSetValue->setVisible(m_signal.isAnalog());

		ui->pushButtonSetZero->setVisible(m_signal.isDiscrete());
		ui->pushButtonSetOne->setVisible(m_signal.isDiscrete());
	}

}

void DialogSignalInfo::updateSignalData()
{
	updateAppSignalState();

	updateSetpoints();

	if (m_signal.enableTuning() == true && tuningEnabled() == true)
	{
		updateTuningSignalState();
	}
}

void DialogSignalInfo::updateAppSignalState()
{
	bool ok = false;

	AppSignalState appSignalState = m_appSignalManager->signalState(m_signal.hash(), &ok);
	if (ok == false)
	{
		return;
	}

	// switch font if needed
	//
	int labelHeight = ui->labelValue->height();

	if (ui->labelValue->maximumHeight() != labelHeight)
	{
		ui->labelValue->setMaximumHeight(labelHeight);
		ui->labelValueTuning->setMaximumHeight(labelHeight);

		ui->labelStrUnit->setMaximumHeight(labelHeight);
		ui->labelStrUnitTuning->setMaximumHeight(labelHeight);
	}

	// Generate value string even if signal is not valid
	//
	QString strValue = appSignalStateText(m_signal, appSignalState, m_viewType, m_currentPrecision);

	// Calculate word wrap
	//
	bool multiLineText = strValue.contains(QChar::LineFeed);

	// Calculate font size
	//
	int oldFontSize = m_currentFontSize;

	if (m_signal.isAnalog() &&
		(m_viewType == E::ValueViewType::Bin32 || m_viewType == E::ValueViewType::Bin64 || multiLineText == true))
	{
		m_currentFontSize = static_cast<int>(labelHeight * 0.33);
	}
	else
	{
		m_currentFontSize = static_cast<int>(labelHeight * 0.4);
	}

	if (oldFontSize != m_currentFontSize)
	{
		QFont font = ui->labelValue->font();
		font.setPixelSize(m_currentFontSize);

		ui->labelValue->setFont(font);
		ui->labelValueTuning->setFont(font);

		ui->labelStrUnit->setFont(font);
		ui->labelStrUnitTuning->setFont(font);
	}

	// Set text
	//
	if (strValue != ui->labelValue->text())
	{
		ui->labelValue->setText(strValue);
	}

	QDateTime localTime = appSignalState.m_time.local.toDateTime();
	QDateTime plaitTime = appSignalState.m_time.plant.toDateTime();

	ui->labelServerTime->setText(localTime.toString("dd.MM.yyyy hh:mm:ss.zzz"));
	ui->labelPlantTime->setText(plaitTime.toString("dd.MM.yyyy hh:mm:ss.zzz"));

	ui->widgetFlags->updateControl(appSignalState.m_flags);

	return;
}

void DialogSignalInfo::updateSetpoints()
{
	/*if (ui->treeSetpoints->isVisible() == false)
	{
		return;
	}*/

	int count = ui->treeSetpoints->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item = ui->treeSetpoints->topLevelItem(i);
		if (item == nullptr)
		{
			Q_ASSERT(item);
			return;
		}

		// CompareTo

		QVariant compareToData = item->data(static_cast<int>(SetpointsColumns::CompareTo), Qt::UserRole);
		if (compareToData.isNull() == false && compareToData.canConvert(qMetaTypeId<AppSignalParam>()) == true)
		{
			const AppSignalParam& paramCompare = compareToData.value<AppSignalParam>();

			bool ok = false;

			AppSignalState stateCompare = m_appSignalManager->signalState(paramCompare.hash(), &ok);
			if (ok == false)
			{
				item->setText(static_cast<int>(SetpointsColumns::CompareToValue), "?");
			}
			else
			{
				item->setText(static_cast<int>(SetpointsColumns::CompareToValue), appSignalStateText(paramCompare, stateCompare, E::ValueViewType::Dec, paramCompare.precision()));
			}
		}

		// Output

		QVariant outputData = item->data(static_cast<int>(SetpointsColumns::Output), Qt::UserRole);
		if (outputData.isNull() == false && outputData.canConvert(qMetaTypeId<AppSignalParam>()) == true)
		{
			const AppSignalParam& paramOutput = outputData.value<AppSignalParam>();

			bool ok = false;

			AppSignalState stateOutput = m_appSignalManager->signalState(paramOutput.hash(), &ok);
			if (ok == false)
			{
				item->setText(static_cast<int>(SetpointsColumns::OutputValue), "?");
			}
			else
			{
				item->setText(static_cast<int>(SetpointsColumns::OutputValue), appSignalStateText(paramOutput, stateOutput, E::ValueViewType::Dec, paramOutput.precision()));
			}
		}
	}

	return;
}

void DialogSignalInfo::updateTuningSignalState()
{
	// Tuning information

	TuningSignalState tuningSignalState = m_tuningController->signalState(m_signal.appSignalId()).value<TuningSignalState>();

	QString strValue = tuningSignalStateText(m_signal, tuningSignalState, m_viewType, m_currentPrecision);

	if (strValue != ui->labelValueTuning->text())
	{
		ui->labelValueTuning->setText(strValue);
	}

	ui->labelSuccessfulReadTime->setText(tuningSignalState.successfulReadTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
	ui->labelWriteRequestTime->setText(tuningSignalState.writeRequestTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
	ui->labelSuccessfulWriteTime->setText(tuningSignalState.successfulWriteTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
	ui->labelUnsuccessfulWriteTime->setText(tuningSignalState.unsuccessfulWriteTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));

	ui->widgetTuningFlags->updateControl(tuningSignalState.m_flags);

	// Enable/disable controls

	bool controlEnabled = tuningSignalState.valid() == true &&
						  tuningSignalState.controlIsEnabled() == true;// &&
						  //tuningSignalState.writingIsEnabled() == true;	// This flag is not always used! ???

	if (ui->pushButtonSetOne->isEnabled() != controlEnabled)
	{
		ui->pushButtonSetOne->setEnabled(controlEnabled);
	}
	if (ui->pushButtonSetZero->isEnabled() != controlEnabled)
	{
		ui->pushButtonSetZero->setEnabled(controlEnabled);
	}
	if (ui->pushButtonSetValue->isEnabled() != controlEnabled)
	{
		ui->pushButtonSetValue->setEnabled(controlEnabled);
	}
	if (ui->editInputValue->isEnabled() != controlEnabled)
	{
		ui->editInputValue->setEnabled(controlEnabled);
	}

	return;
}


void DialogSignalInfo::stateContextMenu(QPoint pos)
{
	QMenu menu(this);

	// Precision
	//
	QString strPrecision = ".";

	QActionGroup *precisionGroup = new QActionGroup(this);
	precisionGroup->setExclusive(true);

	for (int i = 0; i < 10; i++)
	{
		QAction* a = new QAction(strPrecision, &menu);

		auto f = [this, i]() -> void
				 {
					m_currentPrecision = i;
				 };

		connect(a, &QAction::triggered, this, f);

		a->setCheckable(true);

		if (i == m_currentPrecision)
		{
			a->setChecked(true);
		}

		precisionGroup->addAction(a);

		strPrecision += "0";
	}

	menu.addActions(precisionGroup->actions());

	//
	QAction* separator = new QAction(&menu);
	separator->setSeparator(true);
	menu.addAction(separator);

	// View type
	//
	QActionGroup *viewGroup = new QActionGroup(this);
	viewGroup->setExclusive(true);

	for (int i = 0; i < static_cast<int>(E::ValueViewType::Count); i++)
	{
		QAction* a = new QAction(E::valueToString<E::ValueViewType>(i), &menu);

		auto f = [this, i]() -> void
				 {
					m_viewType = static_cast<E::ValueViewType>(i);
				 };

		connect(a, &QAction::triggered, this, f);

		a->setCheckable(true);

		if (i == static_cast<int>(m_viewType))
		{
			a->setChecked(true);
		}

		viewGroup->addAction(a);
	}

	menu.addActions(viewGroup->actions());

	//
	QAction* separator2 = new QAction(&menu);
	separator2->setSeparator(true);
	menu.addAction(separator2);

	// Copy
	//
	QAction* actionCopy = new QAction(tr("Copy"), &menu);

	auto f = [this]() -> void
			 {
				QClipboard *clipboard = QApplication::clipboard();
				clipboard->setText(ui->labelValue->text());
			};

	connect(actionCopy, &QAction::triggered, this, f);

	menu.addAction(actionCopy);

	// --
	//
	menu.exec(mapToGlobal(pos));

	return;
}

QString DialogSignalInfo::appSignalStateText(const AppSignalParam& param, const AppSignalState& state, E::ValueViewType viewType, int precision)
{
	// Generate value string even if signal is not valid
	//

	QString strValue;

	if (param.isDiscrete() == true)
	{
		strValue = QString("%1").arg(state.m_value);
	}

	if (param.isAnalog() == true)
	{
		double ipart = 0;
		double fpart = std::modf(state.m_value, &ipart);

		if((viewType == E::ValueViewType::Bin16 || viewType == E::ValueViewType::Bin32 || viewType == E::ValueViewType::Bin64 || viewType == E::ValueViewType::Hex) &&
		   precision > 0 &&
		   fpart != 0)
		{
			strValue = tr("Only integer value is displayed in HEX or BIN mode.\nSet view precision to zero to see the value.");
		}
		else
		{
			strValue = AppSignalState::toString(state.m_value, viewType, E::AnalogFormat::f_9, precision);
		}
	}

	// Generate non valid string
	//
	if (state.m_flags.valid == false)
	{
		if (state.m_flags.stateAvailable == true)
		{
			// Even state is not valid in some reason LM has value for this signal, show it
			//
			strValue = QString("? (%1)").arg(strValue);
		}
		else
		{
			strValue = QStringLiteral("?");
		}
	}

	return strValue;
}

QString DialogSignalInfo::tuningSignalStateText(const AppSignalParam& param, const TuningSignalState& state, E::ValueViewType viewType, int precision)
{
	// Generate value string even if signal is not valid
	//

	QString strValue;

	if (param.isDiscrete() == true)
	{
		strValue = QString("%1").arg(state.m_value.discreteValue());
	}

	if (param.isAnalog() == true)
	{
		double ipart = 0;
		double fpart = std::modf(state.m_value.toDouble(), &ipart);

		if((viewType == E::ValueViewType::Bin16 || viewType == E::ValueViewType::Bin32 || viewType == E::ValueViewType::Bin64 || viewType == E::ValueViewType::Hex) &&
		   precision > 0 &&
		   fpart != 0)
		{
			strValue = tr("Only integer value is displayed in HEX or BIN mode.\nSet view precision to zero to see the value.");
		}
		else
		{
			strValue = AppSignalState::toString(state.m_value.toDouble(), viewType, E::AnalogFormat::f_9, precision);
		}
	}

	// Generate non valid string
	//
	if (state.m_flags.valid == false)
	{
		strValue = QStringLiteral("?");
	}

	return strValue;
}

//
// QLabelAppSignalDragAndDrop
//

QLabelAppSignalDragAndDrop::QLabelAppSignalDragAndDrop(QWidget* parent) :
	QLabel(parent)
{
}

void QLabelAppSignalDragAndDrop::setAppSignal(const AppSignalParam& signal)
{
	m_appSignalParam = signal;
}

void QLabelAppSignalDragAndDrop::mousePressEvent(QMouseEvent* event)
{
	m_dragDrop.onMousePress(event, QList<AppSignalParam>() << m_appSignalParam);

	return;
}

void QLabelAppSignalDragAndDrop::mouseMoveEvent(QMouseEvent* event)
{
	m_dragDrop.onMouseMove(event, this);

	return;
}


