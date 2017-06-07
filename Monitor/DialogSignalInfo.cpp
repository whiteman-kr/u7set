#include "DialogSignalInfo.h"
#include "ui_DialogSignalInfo.h"
#include "../lib/Types.h"
#include <QPainter>
#include <QClipboard>


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

	static const char* const flagNameStr[] =
	{
		"VAL",
//		"OVRFLW",
//		"UNDRFLW",
	};

	const int FLAG_NAME_COUNT = sizeof(flagNameStr) / sizeof(flagNameStr[0]);


	double colWidth = size().width() / colCount;
	double rowHeight = size().height() / rowCount;


	QPainter painter(this);

	QBrush alertBrush(QColor(192, 0, 0));
	QColor noAlertBrush(Qt::darkGreen);
	QColor noFlagBrush(Qt::lightGray);


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
			QRect flagRect(x + 1, y + 1, colWidth - 2, rowHeight - 2);

			if (flagNo < FLAG_NAME_COUNT)
			{
				bool value = (m_flags.all & (1 << flagNo)) != 0;

				if (flagNo == 0)
				{
					value = !value;	//VAL
				}

				if (value)
				{
					painter.setBrush(alertBrush);
				}
				else
				{
					painter.setBrush(noAlertBrush);
				}
			}
			else
			{
				painter.setBrush(noFlagBrush);
			}

			painter.setPen(Qt::NoPen);
			painter.drawRect(flagRect);

			if (flagNo < FLAG_NAME_COUNT)
			{
				painter.setPen(Qt::white);
				painter.drawText(flagRect, Qt::AlignHCenter | Qt::AlignCenter, flagNameStr[flagNo]);
			}

			flagNo++;
			x += colWidth;
		}

		y += rowHeight;
	}
}


//
//
//	DialogSignalInfo
//
//

DialogSignalInfo::DialogSignalInfo(QWidget *parent, const AppSignalParam& signal) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogSignalInfo),
	m_signal(signal)
{
	ui->setupUi(this);

	ui->tabWidget->tabBar()->setExpanding(true);
	ui->tabWidget->setStyleSheet("QTabBar::tab { min-width: 100px; height: 28;}");

	QString str;

	m_precision = signal.precision();

	// Fill main signal information
	//
	if (m_signal.isAnalog())
	{
		ui->labelStrUnit->setText(m_signal.unit());
	}
	else
	{
		ui->labelStrUnit->setText("");
	}
	ui->labelValue->setText("");

	QFont font = ui->labelValue->font();
	font.setPixelSize(m_currentFontSize);
	ui->labelValue->setFont(font);

	ui->editCustomAppID->setText(m_signal.customSignalId());
	ui->editCaption->setText(m_signal.caption());

	str = E::valueToString<E::SignalType>(m_signal.type());
	if (m_signal.isAnalog())
	{
		str = QString("%1 (%2)").arg(str).arg(E::valueToString<E::AnalogAppSignalFormat>(static_cast<int>(m_signal.analogSignalFormat())));
	}

	str = QString("%1, %2").arg(str).arg(E::valueToString<E::SignalInOutType>(m_signal.inOutType()));

	ui->editSignalType->setText(str);

	ui->editChannel->setText(E::valueToString<E::Channel>(m_signal.channel()));

	// Fill properties list
	//
	QStringList columns;
	columns << "Caption";
	columns << "Value";
	ui->treeProperties->setHeaderLabels(columns);
	ui->treeProperties->setColumnWidth(0, (int)(ui->treeProperties->width() / 2.2));

	QTreeWidgetItem* itemGroup1 = new QTreeWidgetItem(QStringList()<<tr("General"));

	itemGroup1->addChild(new QTreeWidgetItem(QStringList() << tr("AppSignalID") << m_signal.appSignalId()));
	itemGroup1->addChild(new QTreeWidgetItem(QStringList() << tr("EquipmentID") << m_signal.equipmentId()));

	if (m_signal.isAnalog())
	{
		str = QString("%1 - %2").arg(m_signal.unitId()).arg(m_signal.unit());
		itemGroup1->addChild(new QTreeWidgetItem(QStringList() << tr("UnitID") << str));
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


	QTreeWidgetItem* itemGroup3 = new QTreeWidgetItem(QStringList() << tr("Parameters"));

	if (m_signal.isAnalog())
	{
		itemGroup3->addChild(new QTreeWidgetItem(QStringList() << tr("Precision") << QString::number(m_signal.precision())));
		itemGroup3->addChild(new QTreeWidgetItem(QStringList() << tr("Aperture") << QString::number(m_signal.aperture(), 'f', m_signal.precision())));
		itemGroup3->addChild(new QTreeWidgetItem(QStringList() << tr("FilteringTime") << QString::number(m_signal.filteringTime(), 'f', m_signal.precision())));
		itemGroup3->addChild(new QTreeWidgetItem(QStringList() << tr("SpreadTolerance") << QString::number(m_signal.spreadTolerance(), 'f', m_signal.precision())));
	}

	ui->treeProperties->addTopLevelItem(itemGroup3);
	itemGroup3->setExpanded(true);

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

	QTreeWidgetItem* itemGroup7 = new QTreeWidgetItem(QStringList()<<tr("Tuning"));

	itemGroup7->addChild(new QTreeWidgetItem(QStringList()<<tr("EnableTuning")<<(m_signal.enableTuning() ? tr("Yes") : tr("No"))));
	if (m_signal.enableTuning())
	{
		itemGroup7->addChild(new QTreeWidgetItem(QStringList()<<tr("TuningDefaultValue")<<QString::number(m_signal.tuningDefaultValue())));
	}

	ui->treeProperties->addTopLevelItem(itemGroup7);
	itemGroup7->setExpanded(true);

	ui->treeProperties->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->treeProperties, &QTreeWidget::customContextMenuRequested,this, &DialogSignalInfo::prepareContextMenu);


	setWindowTitle(m_signal.customSignalId() + tr(" - ") + m_signal.caption());

	ui->tabWidget->setCurrentIndex(0);

	// Create Flags control
	//
	m_signalFlags = new SignalFlagsWidget(ui->widgetFlags);
	m_signalFlags->move(0, 0);
	m_signalFlags->resize(ui->widgetFlags->size());

	//
	m_hash = ::calcHash(m_signal.appSignalId());

	updateData();

	m_updateStateTimerId = startTimer(500);
}

DialogSignalInfo::~DialogSignalInfo()
{
	delete ui;
}

void DialogSignalInfo::prepareContextMenu(const QPoint& pos)
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

void DialogSignalInfo::timerEvent(QTimerEvent* event)
{
	assert(event);

	if  (event->timerId() == m_updateStateTimerId)
	{
		updateData();
	}
}

void DialogSignalInfo::mousePressEvent(QMouseEvent* event)
{
	if (ui->labelValue->underMouse() && event->button() == Qt::RightButton)
	{
		contextMenu(event->pos());
	}
}


void DialogSignalInfo::updateData()
{
	bool ok = false;

	AppSignalState state = theSignals.signalState(m_hash, &ok);
	if (ok == false)
	{
		return;
	}

	if (state.m_flags.valid)
	{
		QString strValue;

		if (m_signal.isDiscrete())
		{
			strValue = QString("%1").arg(state.m_value);
		}

		if (m_signal.isAnalog())
		{
			int p = 4;

			switch (m_viewType)
			{
			case ViewType::Dec:
				strValue = QString::number(state.m_value, 'f', m_precision);
				break;
			case ViewType::Hex:
				strValue = /*tr("HEX:") + */QString::number((long)state.m_value, 16) + tr("h");
				break;
			case ViewType::Exp:
				strValue = /*tr("EXP:") + */QString::number(state.m_value, 'e', m_precision);
				break;
			case ViewType::Bin16:
				{
					strValue = QString::number((quint16)state.m_value, 2);
					strValue = strValue.rightJustified(16, '0');
					for (int q = 0; q < 3; q++, p += 5)
					{
						strValue.insert(p, ' ');
					}
				}
				//strValue = tr("BIN16: ") + strValue;
				break;
			case ViewType::Bin32:
				strValue = QString::number((quint32)state.m_value, 2);
				strValue = strValue.rightJustified(32, '0');
				for (int q = 0; q < 7; q++, p += 5)
				{
					strValue.insert(p, ' ');
				}
				//strValue = tr("BIN32: ") + strValue;
				break;
			case ViewType::Bin64:
				strValue = QString::number((quint64)state.m_value, 2);
				strValue = strValue.rightJustified(64, '0');
				for (int q = 0; q < 15; q++, p += 5)
				{
					strValue.insert(p, ' ');
				}
				strValue.insert(40, '\n');
				//strValue = tr("BIN64: ") + strValue;
				break;
			}

		}

		// switch font if needed
		//
		int oldFontSize = m_currentFontSize;
		if (m_signal.isAnalog() && m_viewType == ViewType::Bin64)
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
		}


		if (strValue != ui->labelValue->text())
		{
			ui->labelValue->setText(strValue);
		}
	}
	else
	{
		ui->labelValue->setText("???");
	}

	//QDateTime systemTime = QDateTime::fromMSecsSinceEpoch(state.time.system);
	QDateTime localTime = state.m_time.local.toDateTime();
	QDateTime plaitTime = state.m_time.plant.toDateTime();


	//ui->editSystemTime->setText(systemTime.toString("dd.MM.yyyy hh:mm:ss.zzz"));
	ui->editLocalTime->setText(localTime.toString("dd.MM.yyyy hh:mm:ss.zzz"));
	ui->editPlantTime->setText(plaitTime.toString("dd.MM.yyyy hh:mm:ss.zzz"));

	if (m_signalFlags)
	{
		m_signalFlags->updateControl(state.m_flags);

	}
}


void DialogSignalInfo::contextMenu(QPoint pos)
{
	QMenu menu(this);
	QList<QAction*> actions;


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
					m_precision = i;
				 };

		connect(a, &QAction::triggered, this, f);

		a->setCheckable(true);

		if (i == m_precision)
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

	for (int i = 0; i < static_cast<int>(ViewType::Count); i++)
	{
		QAction* a = new QAction(E::valueToString<DialogSignalInfo::ViewType>(i), &menu);

		auto f = [this, i]() -> void
				 {
					m_viewType = static_cast<ViewType>(i);
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
	QAction* actionCopy = new QAction(tr("Copy"), &menu);

	auto f = [this]() -> void
			 {
				QClipboard *clipboard = QApplication::clipboard();
				clipboard->setText(ui->labelValue->text());
			};

	connect(actionCopy, &QAction::triggered, this, f);

	menu.addAction(actionCopy);

	//
	menu.exec(mapToGlobal(pos));

}
