#include "DialogUpdateFromPreset.h"
#include <cassert>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QListWidget>
#include <QTextEdit>
#include <QDialogButtonBox>

DialogUpdateFromPreset::DialogUpdateFromPreset(bool expertOptions, QStringList& availablePresets, QWidget* parent) :
	QDialog(parent),
	m_availablePresets(availablePresets)
{
	setWindowTitle(tr("Update from preset"));
	setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

	QGridLayout* layout = new QGridLayout;

	QLabel* infoLabel = new QLabel(tr("Update from preset will be performed.<br/>"
									  "<b>All preset objects must be available for chek out by the current user.</b><br/>"
									  "If some objects are checked out by other users, check in these objects first.<br/>"
									  "Not only properties will be updated, if preset has any deleted or removed children the they will be updated accordingly.<br/>"));
	infoLabel->setTextFormat(Qt::RichText);
	//infoLabel->setWordWrap(true);
	layout->addWidget(infoLabel, 0, 0);

	QLabel* expertFunctions = new QLabel(tr("<b>Expert functions:</b>"));

	// Set preset list
	//
	QLabel* presetsLabel = new QLabel(tr("Select presets to update:"));

	m_selectedPresetsList = new QListWidget;
	m_selectedPresetsList->addItems(availablePresets);
	m_selectedPresetsList->sortItems();
	m_selectedPresetsList->setSelectionMode(QAbstractItemView::ExtendedSelection);

	int presetCount = m_selectedPresetsList->count();
	for (int i = 0; i < presetCount; i++)
	{
		QListWidgetItem* listItem = m_selectedPresetsList->item(i);
		assert(listItem);

		listItem->setCheckState(Qt::Checked);
	}

	m_clearPresetButton = new QPushButton(tr("Clear All"));
	m_clearPresetButton->setMaximumSize(m_clearPresetButton->sizeHint());
	connect(m_clearPresetButton, &QPushButton::clicked, this, &DialogUpdateFromPreset::clearPresetBittonClicked);

	m_updateAppSignalsCheckBox = new QCheckBox(tr("Update AppSignlas"));
	m_updateAppSignalsCheckBox->setChecked(true);

	// Set force update properties
	//
	QLabel* forceUpdateLabel = new QLabel(tr("<b>Force</b> update properties, coma separated (e.g. Caption, EquipmentIDTemplate, ...)"));

	m_forceUpdateEdit = new QTextEdit;
	m_forceUpdateEdit->setAcceptRichText(false);
	m_forceUpdateEdit->setAutoFormatting(QTextEdit::AutoNone);

	layout->addWidget(expertFunctions, 1, 0);
	layout->addWidget(presetsLabel, 2, 0);
	layout->addWidget(m_selectedPresetsList, 3, 0);
	layout->addWidget(m_clearPresetButton, 4, 0);
	layout->addWidget(m_updateAppSignalsCheckBox, 5, 0);
	layout->addWidget(forceUpdateLabel, 6, 0);
	layout->addWidget(m_forceUpdateEdit, 7, 0);

	if (expertOptions == false)
	{
		expertFunctions->setVisible(false);
		presetsLabel->setVisible(false);
		m_selectedPresetsList->setVisible(false);
		m_clearPresetButton->setVisible(false);
		m_updateAppSignalsCheckBox->setVisible(false);
		forceUpdateLabel->setVisible(false);
		m_forceUpdateEdit->setVisible(false);
	}

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	layout->addWidget(buttonBox, 8, 0);

	setLayout(layout);

	setMinimumSize(QSize(400, 300));
	resize(sizeHint());

	return;
}

QStringList DialogUpdateFromPreset::forceUpdateProperties() const
{
	QString text = m_forceUpdateEdit->toPlainText().trimmed();
	QStringList splitted = text.split(',', QString::SkipEmptyParts);

	QStringList result;

	for (const QString& s : splitted)
	{
		result.push_back(s.trimmed());
	}

	return result;
}

QStringList DialogUpdateFromPreset::selectedPresets() const
{
	assert(m_selectedPresetsList);

	int itemCount = m_selectedPresetsList->count();

	QStringList result;
	result.reserve(itemCount);

	for (int i = 0; i < itemCount; i++)
	{
		QListWidgetItem* item = m_selectedPresetsList->item(i);
		assert(item);

		if (item->checkState() == Qt::Checked)
		{
			result.push_back(item->text());
		}
	}

	return result;
}

bool DialogUpdateFromPreset::updateAppSignlas() const
{
	assert(m_updateAppSignalsCheckBox);
	return m_updateAppSignalsCheckBox->isChecked();
}

void DialogUpdateFromPreset::clearPresetBittonClicked()
{
	int presetCount = m_selectedPresetsList->count();
	for (int i = 0; i < presetCount; i++)
	{
		QListWidgetItem* listItem = m_selectedPresetsList->item(i);
		assert(listItem);

		listItem->setCheckState(Qt::Unchecked);
	}

	return;
}
