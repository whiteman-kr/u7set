#include "DialogAfbLibraryCheck.h"
#include "../lib/LmDescription.h"
#include "Settings.h"

#include <QSplitter>

DialogAfbLibraryCheck* theDialogAfbLibraryCheck = nullptr;

DialogAfbCheckResult::DialogAfbCheckResult(QString message, QWidget* parent)
	: QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint)
{
	setMinimumSize(960, 600);
	setWindowTitle(tr("AFB Library Check Results"));

	QVBoxLayout* mainLayout = new QVBoxLayout();

	QPlainTextEdit* edit = new QPlainTextEdit();
	edit->setReadOnly(true);
	mainLayout->addWidget(edit);

	edit->setPlainText(message);

	setLayout(mainLayout);
}

DialogAfbLibraryCheck::DialogAfbLibraryCheck(DbController* db, QWidget* parent)
	: QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint),
	  m_db(db)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle(tr("AFB Library Check"));

	QWidget* topWidget = new QWidget();
	QVBoxLayout* topLayout = new QVBoxLayout(topWidget);
	topLayout->setContentsMargins(0, 0, 0, 0);

	// Init combo

	QComboBox* lmDescriptionsCombo = new QComboBox();
	lmDescriptionsCombo->setMinimumWidth(250);

	connect(lmDescriptionsCombo, static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged),
			this, &DialogAfbLibraryCheck::libraryFileChanged);

	QHBoxLayout* comboLayout = new QHBoxLayout();
	comboLayout->addWidget(lmDescriptionsCombo);
	comboLayout->addStretch();

	topLayout->addLayout(comboLayout);

	// Init m_afbComponentTreeWidget

	m_afbComponentTreeWidget = new QTreeWidget(this);

	QStringList columns;
	columns << "Caption";
	columns << "OpCode/OpIndex";
	columns << "HasRam";
	columns << "ImpVersion";
	columns << "VersionOpIndex";
	columns << "PinVersionOpIndex";
	columns << "MaxInstCount";
	columns << "SimulationFunc";

	m_afbComponentTreeWidget->setHeaderLabels(columns);

	m_afbComponentTreeWidget->setColumnWidth(static_cast<int>(AfbComponentColumns::Caption), 150);
	m_afbComponentTreeWidget->setColumnWidth(static_cast<int>(AfbComponentColumns::OpCode_OpIndex), 120);
	m_afbComponentTreeWidget->setColumnWidth(static_cast<int>(AfbComponentColumns::HasRam), 120);
	m_afbComponentTreeWidget->setColumnWidth(static_cast<int>(AfbComponentColumns::ImpVersion), 100);
	m_afbComponentTreeWidget->setColumnWidth(static_cast<int>(AfbComponentColumns::VersionOpIndex), 100);
	m_afbComponentTreeWidget->setColumnWidth(static_cast<int>(AfbComponentColumns::PinVersionOpIndex), 150);
	m_afbComponentTreeWidget->setColumnWidth(static_cast<int>(AfbComponentColumns::MaxInstCount), 100);
	m_afbComponentTreeWidget->setColumnWidth(static_cast<int>(AfbComponentColumns::SimulationFunc), 100);

	m_afbComponentTreeWidget->setSortingEnabled(true);

	QLabel* l = new QLabel(tr("AFB Components"));
	topLayout->addWidget(l);

	topLayout->addWidget(m_afbComponentTreeWidget, 1);

	// Init AFB Element tree widget

	QWidget* bottomWidget = new QWidget();
	QVBoxLayout* bottomLayout = new QVBoxLayout(bottomWidget);
	bottomLayout->setContentsMargins(0, 0, 0, 0);

	l = new QLabel(tr("AFB Items"));
	bottomLayout->addWidget(l);

	m_afbElementTreeWidget = new QTreeWidget(this);

	columns.clear();
	columns << "Caption";
	columns << "StrID";
	columns << "OpCode/OpIndex";
	columns << "Version/Type";
	columns << "HasRam/DataFormat";
	columns << "InternalUse/SignalType";
	columns << "Size";
	columns << "ByteOrder";

	m_afbElementTreeWidget->setHeaderLabels(columns);

	m_afbElementTreeWidget->setColumnWidth(static_cast<int>(AfbElementColumns::Caption), 120);
	m_afbElementTreeWidget->setColumnWidth(static_cast<int>(AfbElementColumns::StrID), 120);
	m_afbElementTreeWidget->setColumnWidth(static_cast<int>(AfbElementColumns::OpCode_OpIndex), 120);
	m_afbElementTreeWidget->setColumnWidth(static_cast<int>(AfbElementColumns::Version_Type), 100);
	m_afbElementTreeWidget->setColumnWidth(static_cast<int>(AfbElementColumns::HasRam_DataFormat), 130);
	m_afbElementTreeWidget->setColumnWidth(static_cast<int>(AfbElementColumns::InternalUse_SignalType), 150);
	m_afbElementTreeWidget->setColumnWidth(static_cast<int>(AfbElementColumns::Size), 50);
	m_afbElementTreeWidget->setColumnWidth(static_cast<int>(AfbElementColumns::ByteOrder), 100);

	m_afbElementTreeWidget->setSortingEnabled(true);

	bottomLayout->addWidget(m_afbElementTreeWidget, 2);

	//
	m_splitter = new QSplitter();
	m_splitter->addWidget(topWidget);
	m_splitter->addWidget(bottomWidget);
	m_splitter->setOrientation(Qt::Vertical);
	m_splitter->restoreState(theSettings.m_afbLibratyCheckSplitterState);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addWidget(m_splitter);
	setLayout(mainLayout);

	setMinimumSize(1024, 768);

	// Fill XML ComboBox

	std::vector<DbFileInfo> files;
	bool ok = db->getFileList(&files, db->afblFileId(), "%.xml", true, this);

	if (ok == true)
	{
		std::sort(files.begin(), files.end(),
				  [](const DbFileInfo& f1, const DbFileInfo& f2) -> bool
		{
			return f1.fileName() < f2.fileName();
		});

		for (const DbFileInfo& fi : files)
		{
			lmDescriptionsCombo->addItem(fi.fileName());
		}
	}
}

DialogAfbLibraryCheck::~DialogAfbLibraryCheck()
{
	theSettings.m_afbLibratyCheckSplitterState = m_splitter->saveState();
	theDialogAfbLibraryCheck = nullptr;
}


void DialogAfbLibraryCheck::libraryFileChanged(const QString& fileName)
{
	std::vector<DbFileInfo> fileList;

	QString LmDescriptionFile = fileName;

	if (LmDescriptionFile.isEmpty() == true)
	{
		return;
	}

	bool result = m_db->getFileList(&fileList, m_db->afblFileId(), LmDescriptionFile, true, this);
	if (result == false)
	{
		return;
	}

	if (fileList.empty() == true)
	{
		QString errorMsg = tr("Cannot find file %1.").arg(LmDescriptionFile);
		QMessageBox::critical(this, qApp->applicationName(), errorMsg);
		return;
	}

	// Get description file from the DB
	//
	std::shared_ptr<DbFile> file;
	result = m_db->getLatestVersion(fileList[0], &file, this);
	if (result == false)
	{
		return;
	}

	// Parse file
	//
	LmDescription lm;
	QString parseErrorMessage;

	result = lm.load(file->data(), &parseErrorMessage);

	if (result == false)
	{
		QString errorMsg = tr("Cannot parse file %1. Error message: %2").arg(LmDescriptionFile).arg(parseErrorMessage);
		QMessageBox::critical(this, qApp->applicationName(), errorMsg);
		return;
	}

	// Get the AFBs and fill tables
	//
	m_afbElementTreeWidget->clear();

	m_afbElements = lm.afbs();

	for (std::shared_ptr<Afb::AfbElement> afb : m_afbElements)
	{
		AfbElementTreeWidgetItem* item = new AfbElementTreeWidgetItem(m_afbElementTreeWidget);

		item->setText(static_cast<int>(AfbElementColumns::Caption), afb->caption());
		item->setText(static_cast<int>(AfbElementColumns::StrID), afb->strID());
		item->setText(static_cast<int>(AfbElementColumns::OpCode_OpIndex), QString::number(afb->opCode()));
		item->setText(static_cast<int>(AfbElementColumns::Version_Type), afb->version());
		if (afb->hasRam().has_value() == true)
		{
			item->setText(static_cast<int>(AfbElementColumns::HasRam_DataFormat), afb->hasRam().value() ? tr("true") : tr("false"));
		}
		else
		{
			item->setText(static_cast<int>(AfbElementColumns::HasRam_DataFormat), tr("inherited"));
		}
		item->setText(static_cast<int>(AfbElementColumns::InternalUse_SignalType), afb->internalUse() ? tr("true") : tr("false"));

		m_afbElementTreeWidget->addTopLevelItem(item);

		// Signals and params

		const std::vector<Afb::AfbSignal>& afbInputSignals = afb->inputSignals();

		for (auto& afbSignal : afbInputSignals)
		{
			AfbElementTreeWidgetItem* pinItem = new AfbElementTreeWidgetItem(item);

			fillAfbSignalItem(pinItem,
							  afbSignal.caption(),
							  afbSignal.opName(),
							  afbSignal.operandIndex(),
							  tr("Input"),
							  afbSignal.dataFormat(),
							  afbSignal.type(),
							  afbSignal.size(),
							  afbSignal.byteOrder());
		}

		const std::vector<Afb::AfbSignal>& afbOutputSignals = afb->outputSignals();

		for (auto& afbSignal : afbOutputSignals)
		{
			AfbElementTreeWidgetItem* pinItem = new AfbElementTreeWidgetItem(item);

			fillAfbSignalItem(pinItem,
							  afbSignal.caption(),
							  afbSignal.opName(),
							  afbSignal.operandIndex(),
							  tr("Output"),
							  afbSignal.dataFormat(),
							  afbSignal.type(),
							  afbSignal.size(),
							  afbSignal.byteOrder());
		}

		const std::vector<Afb::AfbParam>& afbParams = afb->params();

		for (auto& afbParam : afbParams)
		{
			AfbElementTreeWidgetItem* pinItem = new AfbElementTreeWidgetItem(item);

			fillAfbSignalItem(pinItem,
							  afbParam.caption(),
							  afbParam.opName(),
							  afbParam.operandIndex(),
							  tr("Param"),
							  afbParam.dataFormat(),
							  afbParam.type(),
							  afbParam.size(),
							  afbParam.byteOrder());
		}
	}

	m_afbElementTreeWidget->sortByColumn(static_cast<int>(AfbElementColumns::OpCode_OpIndex), Qt::AscendingOrder);

	//

	m_afbComponentTreeWidget->clear();

	m_afbComponents = lm.afbComponents();

	for (auto it = m_afbComponents.begin(); it != m_afbComponents.end(); it++)
	{
		const std::shared_ptr<Afb::AfbComponent> afbComponent = it->second;

		const auto& pins = afbComponent->pins();

		assert(it->first == afbComponent->opCode());

		AfbComponentTreeWidgetItem* item = new AfbComponentTreeWidgetItem(m_afbComponentTreeWidget);

		item->setText(static_cast<int>(AfbComponentColumns::Caption), afbComponent->caption());
		item->setText(static_cast<int>(AfbComponentColumns::OpCode_OpIndex), QString::number(afbComponent->opCode()));
		item->setText(static_cast<int>(AfbComponentColumns::HasRam), afbComponent->hasRam() ? tr("true") : ("false"));
		item->setText(static_cast<int>(AfbComponentColumns::ImpVersion), QString::number(afbComponent->impVersion()));

		item->setText(static_cast<int>(AfbComponentColumns::VersionOpIndex), QString::number(afbComponent->versionOpIndex()));

		if (afbComponent->pinExists(afbComponent->versionOpIndex()) == false)
		{
			item->setText(static_cast<int>(AfbComponentColumns::PinVersionOpIndex), tr("-"));
		}
		else
		{
			const Afb::AfbComponentPin& pin = pins.at(afbComponent->versionOpIndex());
			item->setText(static_cast<int>(AfbComponentColumns::PinVersionOpIndex), tr("%1 = %2").arg(pin.caption()).arg(pin.opIndex()));
		}

		item->setText(static_cast<int>(AfbComponentColumns::MaxInstCount), QString::number(afbComponent->maxInstCount()));
		item->setText(static_cast<int>(AfbComponentColumns::SimulationFunc), afbComponent->simulationFunc());

		m_afbComponentTreeWidget->addTopLevelItem(item);

		// Add components pins
		//
		std::map<int, Afb::AfbComponentPin> afbCombonentPins;
		for (const auto&[k, v] : afbComponent->pins())
		{
			afbCombonentPins[k] = v;
		}

		// --
		//
		for (auto pit = afbCombonentPins.begin(); pit != afbCombonentPins.end(); pit++)
		{
			const Afb::AfbComponentPin& pin = pit->second;

			AfbComponentTreeWidgetItem* pinItem = new AfbComponentTreeWidgetItem(item);

			pinItem->setText(static_cast<int>(AfbComponentColumns::Caption), pin.caption());
			pinItem->setText(static_cast<int>(AfbComponentColumns::OpCode_OpIndex), QString::number(pin.opIndex()));
		}
	}

	m_afbComponentTreeWidget->sortByColumn(static_cast<int>(AfbComponentColumns::OpCode_OpIndex), Qt::AscendingOrder);
}


/*
void DialogAfbLibraryCheck::onCheckPins()
{
	QStringList errorList;

	for (std::shared_ptr<Afb::AfbElement> afb : m_afbElements)
	{
		qDebug() << afb->caption();

		const std::vector<Afb::AfbSignal>& afbInputSignals = afb->inputSignals();
		const std::vector<Afb::AfbSignal>& afbOutputSignals = afb->outputSignals();
		const std::vector<Afb::AfbParam>& afbParams = afb->params();

		for (auto it = m_afbComponents.begin(); it != m_afbComponents.end(); it++)
		{
			const std::shared_ptr<Afb::AfbComponent> afbComponent = it->second;

			if (afb->opCode() != afbComponent->opCode())
			{
				continue;
			}

			qDebug() << afbComponent->caption();


			const std::map<int, Afb::AfbComponentPin>& afbCombonentPins = afbComponent->pins();

			// InputSignals

			bool found = false;

			for (auto afbSignal : afbInputSignals)
			{
				auto itPin = afbCombonentPins.find(afbSignal.operandIndex());
				if (itPin != afbCombonentPins.end())
				{
					const Afb::AfbComponentPin& pin = itPin->second;
					if (pin.caption() == afbSignal.opName())
					{
						found = true;
						break;
					}
				}

				if (found == false)
				{
					QString error = tr("AFB '%1', input '%2' (%3) was not found in component '%4' (opCode %5).")
							.arg(afb->strID())
							.arg(afbSignal.opName())
							.arg(afbSignal.operandIndex())
							.arg(afbComponent->caption())
							.arg(afbComponent->opCode());
					errorList.push_back(error);
				}
			}

			// OutputSignals

			found = false;

			for (auto afbSignal : afbOutputSignals)
			{
				auto itPin = afbCombonentPins.find(afbSignal.operandIndex());
				if (itPin != afbCombonentPins.end())
				{
					const Afb::AfbComponentPin& pin = itPin->second;
					if (pin.caption() == afbSignal.opName())
					{
						found = true;
						break;
					}
				}

				if (found == false)
				{
					QString error = tr("AFB '%1', output '%2' (%3) was not found in component '%4' (opCode %5).")
							.arg(afb->strID())
							.arg(afbSignal.opName())
							.arg(afbSignal.operandIndex())
							.arg(afbComponent->caption())
							.arg(afbComponent->opCode());
					errorList.push_back(error);
				}
			}

			// Params

			found = false;

			for (auto afbParam : afbParams)
			{
				auto itPin = afbCombonentPins.find(afbParam.operandIndex());
				if (itPin != afbCombonentPins.end())
				{
					const Afb::AfbComponentPin& pin = itPin->second;
					if (pin.caption() == afbParam.opName())
					{
						found = true;
						break;
					}
				}

				if (found == false)
				{
					QString error = tr("AFB '%1', output '%2' (%3) was not found in component '%4' (opCode %5).")
							.arg(afb->strID())
							.arg(afbParam.opName())
							.arg(afbParam.operandIndex())
							.arg(afbComponent->caption())
							.arg(afbComponent->opCode());

					errorList.push_back(error);
				}
			}
		}
	}

	if (errorList.isEmpty() == false)
	{
		QString error = errorList.join(QChar::LineFeed);

		DialogAfbCheckResult d(error, this);
		d.exec();
	}
	else
	{
		QMessageBox::information(this, qApp->applicationName(), tr("Pin check OK"));
	}
}
*/

void DialogAfbLibraryCheck::fillAfbSignalItem(AfbElementTreeWidgetItem* pinItem,
											  const QString& caption,
											  const QString& opName,
											  int operandIndex,
											  const QString& type,
											  E::DataFormat dataFormat,
											  E::SignalType signalType,
											  int size,
											  E::ByteOrder byteOrder)
{
	pinItem->setText(static_cast<int>(AfbElementColumns::Caption), caption);
	pinItem->setText(static_cast<int>(AfbElementColumns::StrID), opName);
	pinItem->setText(static_cast<int>(AfbElementColumns::OpCode_OpIndex), QString::number(operandIndex));
	pinItem->setText(static_cast<int>(AfbElementColumns::Version_Type), type);

	switch (dataFormat)
	{
	case E::DataFormat::UnsignedInt:		pinItem->setText(static_cast<int>(AfbElementColumns::HasRam_DataFormat), tr("UnsignedInt"));			break;
	case E::DataFormat::SignedInt:			pinItem->setText(static_cast<int>(AfbElementColumns::HasRam_DataFormat), tr("SignedInt"));			break;
	case E::DataFormat::Float:				pinItem->setText(static_cast<int>(AfbElementColumns::HasRam_DataFormat), tr("Float"));				break;
	default:
		assert(false);
	}

	switch (signalType)
	{
	case E::SignalType::Analog:				pinItem->setText(static_cast<int>(AfbElementColumns::InternalUse_SignalType), tr("Analog"));			break;
	case E::SignalType::Discrete:			pinItem->setText(static_cast<int>(AfbElementColumns::InternalUse_SignalType), tr("Discrete"));			break;
	case E::SignalType::Bus:				pinItem->setText(static_cast<int>(AfbElementColumns::InternalUse_SignalType), tr("Bus"));				break;
	default:
		assert(false);
	}

	pinItem->setText(static_cast<int>(AfbElementColumns::Size), QString::number(size));

	switch (byteOrder)
	{
	case E::ByteOrder::LittleEndian:		pinItem->setText(static_cast<int>(AfbElementColumns::ByteOrder), tr("LittleEndian"));			break;
	case E::ByteOrder::BigEndian:			pinItem->setText(static_cast<int>(AfbElementColumns::ByteOrder), tr("BigEndian"));			break;
	default:
		assert(false);
	}
}
