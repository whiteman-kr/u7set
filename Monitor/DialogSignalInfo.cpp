#include "DialogSignalInfo.h"
#include "ui_DialogSignalInfo.h"
#include "../lib/Types.h"
#include "../Proto/serialization.pb.h"
#include "MonitorCentralWidget.h"
#include "Settings.h"
#include "../lib/Ui/UiTools.h"


//
//
//	DialogSetpointDetails
//
//

DialogSetpointDetails::DialogSetpointDetails(QWidget* parent, std::shared_ptr<Comparator> comparator):
QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
  m_comparator(comparator)
{
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

	AppSignalParam inputParam = theSignals.signalParam(c->input().appSignalID(), &ok);
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
		AppSignalParam compareParam = theSignals.signalParam(c->compare().appSignalID(), &ok);
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

	AppSignalParam outputParam = theSignals.signalParam(c->output().appSignalID(), &ok);
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
		AppSignalParam hystParam = theSignals.signalParam(c->hysteresis().appSignalID(), &ok);
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

	switch(c->intAnalogSignalFormat())
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
}

void SignalFlagsWidget::updateControl(AppSignalStateFlags flags)
{
	m_flags = flags;
	update();
}

void SignalFlagsWidget::paintEvent(QPaintEvent *)
{
	const int colCount = 8;
	const int rowCount = 2;

	static const std::array<QString, static_cast<int>(SignalFlagsFields::Count)> flagNames =
		{
			QStringLiteral("VALID"),
			QStringLiteral("STATE"),
			QStringLiteral("SIM"),
			QStringLiteral("LOCK"),
			QStringLiteral("MISMATCH"),
			QStringLiteral("HIGH"),
			QStringLiteral("LOW")
		};

	double colWidth = size().width() / colCount;
	double rowHeight = size().height() / rowCount;

	QPainter painter(this);

	static QBrush alertBrush(QColor(192, 0, 0));
	static QColor noAlertBrush(Qt::darkGreen);
	static QColor noFlagBrush(Qt::lightGray);

	QFont font = painter.font();
	font.setPixelSize(14);
	painter.setFont(font);

	int flagNo = 0;

	double y = 0;

	for (int j = 0; j < rowCount; j++)
	{
		double x = 0;

		for (int i = 0; i < colCount; i++)
		{
			// Draw the rectangle for flag
			//
			QRect flagRect{static_cast<int>(x + 1),
						   static_cast<int>(y + 1),
						   static_cast<int>(colWidth - 2),
						   static_cast<int>(rowHeight - 2)};

			if (flagNo < static_cast<int>(SignalFlagsFields::Count))
			{

				bool value = false;

				switch (static_cast<SignalFlagsFields>(flagNo))
				{
				case SignalFlagsFields::Valid:
					value = !m_flags.valid;
					break;

				case SignalFlagsFields::StateAvailable:
					value = !m_flags.stateAvailable;
					break;

				case SignalFlagsFields::Simulated:
					value = m_flags.simulated;
					break;

				case SignalFlagsFields::Blocked:
					value = m_flags.blocked;
					break;

				case SignalFlagsFields::Mismatch:
					value = m_flags.mismatch;
					break;

				case SignalFlagsFields::AboveHighLimit:
					value = m_flags.aboveHighLimit;
					break;

				case SignalFlagsFields::BelowLowLimit:
					value = m_flags.belowLowLimit;
					break;

				default:
					Q_ASSERT(false);
				}

				painter.setBrush(value == true ? alertBrush : noAlertBrush);
			}
			else
			{
				painter.setBrush(noFlagBrush);
			}

			painter.setPen(Qt::NoPen);
			painter.drawRect(flagRect);

			// Draw the text description for flag

			if (flagNo < static_cast<int>(SignalFlagsFields::Count))
			{
				painter.setPen(Qt::white);
				painter.drawText(flagRect, Qt::AlignHCenter | Qt::AlignCenter, flagNames[flagNo]);
			}

			x += colWidth;

			flagNo++;
		}

		y += rowHeight;
	}
}

//
//
//	DialogSignalInfo
//
//

std::map<QString, DialogSignalInfo*> DialogSignalInfo::m_dialogSignalInfoMap;

bool DialogSignalInfo::showDialog(QString appSignalId, MonitorConfigController* configController, TcpSignalClient* tcpSignalClient, MonitorCentralWidget* centralWidget)
{
	auto it = m_dialogSignalInfoMap.find(appSignalId);
	if (it == m_dialogSignalInfoMap.end())
	{
		bool ok = false;
		AppSignalParam signal = theSignals.signalParam(appSignalId, &ok);

		if (ok == true)
		{
			DialogSignalInfo* dsi = new DialogSignalInfo(signal, configController, centralWidget);
			connect(tcpSignalClient, &TcpSignalClient::signalParamAndUnitsArrived, dsi, &DialogSignalInfo::onSignalParamAndUnitsArrived);

			dsi->show();
			dsi->raise();
			dsi->activateWindow();

			m_dialogSignalInfoMap[appSignalId] = dsi;
		}
		else
		{
			QMessageBox::critical(centralWidget, qAppName(), tr("Signal %1 not found.").arg(appSignalId));
			return false;
		}
	}
	else
	{
		DialogSignalInfo* dsi = it->second;
		if (dsi == nullptr)
		{
			Q_ASSERT(dsi);
			return false;
		}

		dsi->raise();
		dsi->activateWindow();

		UiTools::adjustDialogPlacement(dsi);
	}

	return true;
}

void DialogSignalInfo::onSignalParamAndUnitsArrived()
{
	// Refresh signal param inself

	bool ok = false;

	AppSignalParam newSignal = theSignals.signalParam(m_signal.hash(), &ok);

	if (ok == true)
	{
		m_signal = newSignal;
	}
	else
	{
		//Signal was deleted, keep its #appSignalId and Hash
		//
		AppSignalParam oldSignal = m_signal;

		m_signal = AppSignalParam();
		m_signal.setAppSignalId(oldSignal.appSignalId());
		m_signal.setHash(oldSignal.hash());

	}

	// Fill signal information

	fillSignalInfo();

	fillProperties();

	fillSetpoints();

	fillSchemas();
}

DialogSignalInfo::DialogSignalInfo(const AppSignalParam& signal, MonitorConfigController* configController, MonitorCentralWidget* centralWidget) :
	QDialog(centralWidget, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogSignalInfo),
	m_signal(signal),
	m_configController(configController),
	m_centralWidget(centralWidget)
{
	if (m_configController == nullptr || m_centralWidget == nullptr)
	{
		Q_ASSERT(m_configController);
		Q_ASSERT(m_centralWidget);
		return;
	}

	ui->setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);

	// Restore window pos
	//
	if (theSettings.m_signalInfoPos.x() != -1 && theSettings.m_signalInfoPos.y() != -1)
	{
		move(theSettings.m_signalInfoPos);
		restoreGeometry(theSettings.m_signalInfoGeometry);
	}

	//

	ui->tabWidget->tabBar()->setExpanding(true);
	ui->tabWidget->setStyleSheet("QTabBar::tab { min-width: 100px; height: 28;}");

	// signalIdLabel is promoted to QLabelAppSignalDragAndDrop
	// tu support Drag and Drop Operation (drag part)
	//
	ui->signalIdLabel->setAppSignal(signal);
	ui->signalIdLabelIcon->setAppSignal(signal);

	// Fill signal information

	fillSignalInfo();

	fillProperties();

	fillSetpoints();

	fillSchemas();

	ui->tabWidget->setCurrentIndex(0);

	// Remove tuning tab

	if (configController->configuration().tuningEnabled == false || m_signal.enableTuning() == false)
	{
		for (int i = 0; i < ui->tabWidget->count(); i++)
		{
			if (ui->tabWidget->tabText(i) == tr("Tuning"))
			{
				ui->tabWidget->removeTab(i);
				break;
			}
		}
	}

	//

	updateData();

	UiTools::adjustDialogPlacement(this);

	m_updateStateTimerId = startTimer(200);
}

DialogSignalInfo::~DialogSignalInfo()
{
	auto it = m_dialogSignalInfoMap.find(m_signal.appSignalId());
	if (it == m_dialogSignalInfoMap.end())
	{

		Q_ASSERT(false);
	}
	else
	{
		m_dialogSignalInfoMap.erase(it);
	}

	// Save window position
	//
	theSettings.m_signalInfoPos = pos();
	theSettings.m_signalInfoGeometry = saveGeometry();

	theSettings.m_signalInfoTreeSetpointsState = ui->treeSetpoints->header()->saveState();

	delete ui;
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

	m_contextMenuSetpointIndex = item->data(static_cast<int>(SetpointsColumns::Index), Qt::UserRole).toInt();

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

void DialogSignalInfo::timerEvent(QTimerEvent* event)
{
	Q_ASSERT(event);

	if  (event->timerId() == m_updateStateTimerId)
	{
		updateData();
	}
}

void DialogSignalInfo::mousePressEvent(QMouseEvent* event)
{
	if ((ui->labelValue->underMouse() || ui->labelValueTuning->underMouse()) && event->button() == Qt::RightButton)
	{
		stateContextMenu(event->pos());
	}
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

		ui->pushButtonSetOne->setVisible(false);
		ui->pushButtonSetZero->setVisible(false);
	}
	else
	{
		ui->labelStrUnit->setText("");
		ui->labelStrUnitTuning->setText("");

		ui->editInputValue->setVisible(false);
		ui->pushButtonSetValue->setVisible(false);
	}
	ui->labelValue->setText("");
	ui->labelValueTuning->setText("");

	QFont font = ui->labelValue->font();
	font.setPixelSize(m_currentFontSize);
	ui->labelValue->setFont(font);
	ui->labelValueTuning->setFont(font);

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
		itemGroup7->addChild(new QTreeWidgetItem(QStringList()<<tr("TuningDefaultValue")<<m_signal.tuningDefaultValue().toString(m_signal.precision())));
		itemGroup7->addChild(new QTreeWidgetItem(QStringList()<<tr("TuningLowBound")<<m_signal.tuningLowBound().toString(m_signal.precision())));
		itemGroup7->addChild(new QTreeWidgetItem(QStringList()<<tr("TuningHighBound")<<m_signal.tuningHighBound().toString(m_signal.precision())));

		ui->treeProperties->addTopLevelItem(itemGroup7);
		itemGroup7->setExpanded(true);
	}


	ui->treeProperties->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->treeProperties, &QTreeWidget::customContextMenuRequested,this, &DialogSignalInfo::preparePropertiesContextMenu);

	ui->treeProperties->resizeColumnToContents(0);
}

void DialogSignalInfo::fillSetpoints()
{
	ui->treeSetpoints->clear();

	QStringList columns;
	columns << tr("#");
	columns << tr("Type");
	columns << tr("Compare To");
	columns << tr("Value");
	columns << tr("Output");
	columns << tr("Value");
	columns << tr("Schema");
	ui->treeSetpoints->setHeaderLabels(columns);

	m_comparators = m_configController->getByInputSignalID(m_signal.appSignalId());

	for (int i = 0; i < m_comparators.size(); i++)
	{
		const std::shared_ptr<Comparator>& c = m_comparators[i];

		QTreeWidgetItem* item = new QTreeWidgetItem();

		item->setText(static_cast<int>(SetpointsColumns::Index), QString::number(i + 1));

		// CompareTo

		if (c->compare().isConst() == true)
		{
			item->setText(static_cast<int>(SetpointsColumns::CompareTo), tr("Const"));
			item->setText(static_cast<int>(SetpointsColumns::CompareToValue), QString::number(c->compare().constValue(), 'f', c->precision()));
		}
		else
		{
			bool ok = false;

			AppSignalParam paramCompareTo = theSignals.signalParam(c->compare().appSignalID(), &ok);
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

		AppSignalParam paramOutput = theSignals.signalParam(c->output().appSignalID(), &ok);
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

		item->setData(static_cast<int>(SetpointsColumns::Index), Qt::UserRole, i);

		ui->treeSetpoints->addTopLevelItem(item);
	}

	ui->treeSetpoints->setSortingEnabled(true);
	ui->treeSetpoints->sortByColumn(static_cast<int>(SetpointsColumns::Index), Qt::AscendingOrder);

	ui->treeSetpoints->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->treeSetpoints, &QTreeWidget::customContextMenuRequested,this, &DialogSignalInfo::prepareSetpointsContextMenu);

	// Adjust column width
	//
	if (theSettings.m_signalInfoTreeSetpointsState.isEmpty() == false)
	{
		ui->treeSetpoints->header()->restoreState(theSettings.m_signalInfoTreeSetpointsState);
	}
	else
	{
		ui->treeSetpoints->resizeColumnToContents(static_cast<int>(SetpointsColumns::Index));
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

	QStringList schemas = m_configController->schemasByAppSignalId(m_signal.appSignalId());

	for (const QString& schema : schemas)
	{

		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setText(0, schema);

		ui->treeSchemas->addTopLevelItem(item);
	}

	ui->treeSchemas->setSortingEnabled(true);
	ui->treeSchemas->sortByColumn(0, Qt::AscendingOrder);

	ui->treeSchemas->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->treeSchemas, &QTreeWidget::customContextMenuRequested,this, &DialogSignalInfo::prepareSchemaContextMenu);
}

void DialogSignalInfo::updateData()
{
	updateState();
	updateSetpoints();
}

void DialogSignalInfo::updateState()
{
	bool ok = false;

	AppSignalState state = theSignals.signalState(m_signal.hash(), &ok);
	if (ok == false)
	{
		return;
	}

	// switch font if needed
	//
	int oldFontSize = m_currentFontSize;
	if (m_signal.isAnalog() && m_viewType == E::ValueViewType::Bin64)
	{
		m_currentFontSize = 12;
	}
	else
	{
		m_currentFontSize = 20;
	}
	if (oldFontSize != m_currentFontSize)
	{
		QFont font = ui->labelValue->font();
		font.setPixelSize(m_currentFontSize);
		ui->labelValue->setFont(font);
		ui->labelValueTuning->setFont(font);
	}

	// Generate value string even if signal is not valid
	//
	QString strValue = signalStateText(m_signal, state, m_viewType, m_currentPrecision);

	// --
	//
	if (strValue != ui->labelValue->text())
	{
		ui->labelValue->setText(strValue);
		ui->labelValueTuning->setText(strValue);
	}

	//QDateTime systemTime = QDateTime::fromMSecsSinceEpoch(state.time.system);
	QDateTime localTime = state.m_time.local.toDateTime();
	QDateTime plaitTime = state.m_time.plant.toDateTime();


	//ui->editSystemTime->setText(systemTime.toString("dd.MM.yyyy hh:mm:ss.zzz"));
	ui->editLocalTime->setText(localTime.toString("dd.MM.yyyy hh:mm:ss.zzz"));
	ui->editPlantTime->setText(plaitTime.toString("dd.MM.yyyy hh:mm:ss.zzz"));


	ui->widgetFlags->updateControl(state.m_flags);

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

			AppSignalState stateCompare = theSignals.signalState(paramCompare.hash(), &ok);
			if (ok == false)
			{
				item->setText(static_cast<int>(SetpointsColumns::CompareToValue), "?");
			}
			else
			{
				item->setText(static_cast<int>(SetpointsColumns::CompareToValue), signalStateText(paramCompare, stateCompare, E::ValueViewType::Dec, paramCompare.precision()));
			}
		}

		// Output

		QVariant outputData = item->data(static_cast<int>(SetpointsColumns::Output), Qt::UserRole);
		if (outputData.isNull() == false && outputData.canConvert(qMetaTypeId<AppSignalParam>()) == true)
		{
			const AppSignalParam& paramOutput = outputData.value<AppSignalParam>();

			bool ok = false;

			AppSignalState stateOutput = theSignals.signalState(paramOutput.hash(), &ok);
			if (ok == false)
			{
				item->setText(static_cast<int>(SetpointsColumns::OutputValue), "?");
			}
			else
			{
				item->setText(static_cast<int>(SetpointsColumns::OutputValue), signalStateText(paramOutput, stateOutput, E::ValueViewType::Dec, paramOutput.precision()));
			}
		}
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

QString DialogSignalInfo::signalStateText(const AppSignalParam& param, const AppSignalState& state, E::ValueViewType viewType, int precision)
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
		strValue = AppSignalState::toString(state.m_value, viewType, precision);
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
	if (event->button() == Qt::LeftButton)
	{
		m_dragStartPosition = event->pos();
	}

	return;
}

void QLabelAppSignalDragAndDrop::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons().testFlag(Qt::LeftButton) == false)
	{
		return;
	}

	if ((event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance())
	{
		return;
	}

	// Save signals to protobufer
	//
	::Proto::AppSignalSet protoSetMessage;
	::Proto::AppSignal* protoSignalMessage = protoSetMessage.add_appsignal();
	m_appSignalParam.save(protoSignalMessage);

	QByteArray data;
	data.resize(protoSetMessage.ByteSize());

	protoSetMessage.SerializeToArray(data.data(), protoSetMessage.ByteSize());

	// --
	//
	if (data.isEmpty() == false)
	{
		QDrag* drag = new QDrag(this);
		QMimeData* mimeData = new QMimeData;

		mimeData->setData(AppSignalParamMimeType::value, data);
		drag->setMimeData(mimeData);

		drag->exec(Qt::CopyAction);

		qDebug() << "Start drag for " << m_appSignalParam.appSignalId();
		qDebug() << "Drag and drop data buffer size " << data.size();
	}

	return;
}

void DialogSignalInfo::on_treeSchemas_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	Q_UNUSED(column);

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

	QString schemaId = item->text(static_cast<int>(SchemasColumns::SchemaId));

	if (currentTab->schemaId() != schemaId)
	{
		QStringList appSignals;
		appSignals << m_signal.appSignalId();

		currentTab->setSchema(schemaId, appSignals);
	}

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

	m_contextMenuSetpointIndex = item->data(static_cast<int>(SetpointsColumns::Index), Qt::UserRole).toInt();

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
	if (m_signal.isDiscrete() == false)
	{
		Q_ASSERT(false);
		return;
	}

	m_centralWidget->tuningController()->writeValue(m_signal.appSignalId(), false);
}

void DialogSignalInfo::on_pushButtonSetOne_clicked()
{
	if (m_signal.isDiscrete() == false)
	{
		Q_ASSERT(false);
		return;
	}

	m_centralWidget->tuningController()->writeValue(m_signal.appSignalId(), true);
}

void DialogSignalInfo::on_pushButtonSetValue_clicked()
{
	if (m_signal.isAnalog() == false)
	{
		Q_ASSERT(false);
		return;
	}

	QString strValue = ui->editInputValue->text();

	bool ok = false;

	double value = strValue.toDouble(&ok);
	if (ok == false)
	{
		QMessageBox::critical(m_centralWidget, qAppName(), tr("Invalid input value!"));
		return;
	}

	m_centralWidget->tuningController()->writeValue(m_signal.appSignalId(), value);
}


void DialogSignalInfo::switchToSchema()
{
	MonitorSchemaWidget* currentTab = m_centralWidget->currentTab();
	if (currentTab == nullptr)
	{
		Q_ASSERT(currentTab);
		return;
	}

	if (currentTab->schemaId() != m_contextMenuSchemaId)
	{
		QStringList appSignals;
		appSignals << m_signal.appSignalId();

		currentTab->setSchema(m_contextMenuSchemaId, appSignals);
	}
}

void DialogSignalInfo::showSetpointDetails()
{
	if (m_contextMenuSetpointIndex < 0 || m_contextMenuSetpointIndex >= m_comparators.size())
	{
		Q_ASSERT(false);
		return;
	}

	DialogSetpointDetails* d = new DialogSetpointDetails(this, m_comparators[m_contextMenuSetpointIndex]);
	d->exec();
}
