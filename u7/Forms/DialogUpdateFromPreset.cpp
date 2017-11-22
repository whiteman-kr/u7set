#include "DialogUpdateFromPreset.h"
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QTextEdit>

DialogUpdateFromPreset::DialogUpdateFromPreset(bool expertOptions, QStringList& availablePresets, QWidget* parent) :
	QDialog(parent),
	m_availablePresets(availablePresets)
{
	QGridLayout* layout = new QGridLayout;

	QLabel* infoLabel = new QLabel(tr("Update from preset will be performed.\nAll preset objects must be available for chek out for the current user.\nIf some objects are checked out by other users, check in these objects first."));
	layout->addWidget(infoLabel, 0, 0);

	QLabel* expertFunctions = new QLabel(tr("<b>Expert functions:</b>"), this);

	// Set preset list
	//
	QLabel* presetsLabel = new QLabel(tr("Select presets to update. Note, only properties will not be updated, if preset has any deleted or removed children the they will be updated accordingly."), this);

	m_selectedPresetsList = new QListWidget(this);
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

	m_clearPresetButton = new QPushButton(tr("Clear All"), this);
	m_clearPresetButton->setMaximumSize(m_clearPresetButton->sizeHint());
	connect(m_clearPresetButton, &QPushButton::clicked, this, &DialogUpdateFromPreset::clearPresetBittonClicked);

	// Set force update properties
	//
	QLabel* forceUpdateLabel = new QLabel(tr("<b>Force</b> update properties, coma separated (e.g. Caption, EquipmentIDTemplate, ...)"), this);

	m_forceUpdateEdit = new QTextEdit(this);
	m_forceUpdateEdit->setAcceptRichText(false);
	m_forceUpdateEdit->setAutoFormatting(QTextEdit::AutoNone);

	if (expertOptions == true)
	{
		layout->addWidget(expertFunctions, 1, 0);
		layout->addWidget(presetsLabel, 2, 0);
		layout->addWidget(m_selectedPresetsList, 3, 0);
		layout->addWidget(m_clearPresetButton, 4, 0);
		layout->addWidget(forceUpdateLabel, 5, 0);
		layout->addWidget(m_forceUpdateEdit, 6, 0);
	}

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	layout->addWidget(buttonBox, 7, 0);

	setLayout(layout);

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
