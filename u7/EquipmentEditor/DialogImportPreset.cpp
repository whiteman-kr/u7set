#include "DialogImportPreset.h"
#include "ui_DialogImportPreset.h"

DialogImportPreset::DialogImportPreset(const Proto::ExportedDevicePreset *message, QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogImportPreset),
	m_message(message)
{
	ui->setupUi(this);

	setWindowTitle(tr("Import Presets"));

	QStringList headerLabels;
	headerLabels << tr("Preset ID");
	headerLabels << tr("Caption");
	headerLabels << tr("Version");
	ui->presetsTree->setHeaderLabels(headerLabels);

	for (const auto& it : m_message->items().items())
	{
		if (it.has_deviceobject() == false)
		{
			continue;
		}

		QStringList l;
		l.push_back(QString::fromStdString(it.deviceobject().presetname().text()));
		l.push_back(QString::fromStdString(it.deviceobject().caption().text()));
		l.push_back(QString::number(it.deviceobject().presetversion()));

		QTreeWidgetItem* item = new QTreeWidgetItem(l);
		item->setCheckState(0, Qt::Checked);

		QByteArray uuid(it.deviceobject().uuid().uuid().data());
		item->setData(0, Qt::UserRole, uuid);

		ui->presetsTree->addTopLevelItem(item);
	}

	for (int i = 0; i < ui->presetsTree->columnCount(); i++)
	{
		ui->presetsTree->resizeColumnToContents(i);
	}

	ui->projectNameLabel->setText("Project Name: " + (message->description().has_projectname() == true ?
															  QString::fromStdString(message->description().projectname()) :
															  tr("Unknown")));

	ui->userNameLabel->setText("User Name: " + (message->description().has_username() == true ?
														  QString::fromStdString(message->description().username()) :
														  tr("Unknown")));

	ui->exportTimeLabel->setText("Created: " + (message->description().has_exporttime() == true ?
														  QDateTime::fromSecsSinceEpoch(message->description().exporttime()).toString("dd/MM/yyyy HH:mm:ss") :
														  tr("Unknown")));
}

DialogImportPreset::~DialogImportPreset()
{
	delete ui;
}

::Proto::EnvelopeSet DialogImportPreset::chosenItems() const
{
	return m_chosenItems;
}

void DialogImportPreset::accept()
{
	for (const auto& it : m_message->items().items())
	{
		if (it.has_deviceobject() == false)
		{
			continue;
		}

		QByteArray uuid(it.deviceobject().uuid().uuid().data());

		for (int i = 0; i < ui->presetsTree->topLevelItemCount(); i++)
		{
			if (ui->presetsTree->topLevelItem(i)->checkState(0) == Qt::Checked &&
				ui->presetsTree->topLevelItem(i)->data(0, Qt::UserRole).toByteArray() == uuid)
			{
				::Proto::Envelope* ni = m_chosenItems.add_items();
				*ni = it;
				break;
			}
		}
	}

	if (m_chosenItems.items_size() == 0)
	{
		QMessageBox::warning(this, qAppName(), tr("Please choose at least one preset!"));
		return;
	}

	QDialog::accept();
}

