#include <UiLib/WidgetUtils.h>

#include "FindSignalDialog.h"
#include "SignalsTabPage.h"
#include "AppSettings.h"
#include "AppSignalSetProvider.h"

const QString FindSignalDialog::notUniqueMessage("No - not unique");
const QString FindSignalDialog::notEditableMessage("No - checked out by another user");
const QString FindSignalDialog::notCorrectIdMessage("No - uncorrect ID");
const QString FindSignalDialog::cannotCheckoutMessage("No - can't checkout");
const QString FindSignalDialog::replaceableMessage("Yes");
const QString FindSignalDialog::replacedMessage("Yes - replaced");

FindSignalDialog::FindSignalDialog(QTableView* parent) :
	QDialog(parent, Qt::Dialog),
	m_signalTable(parent),
	m_signalSetProvider(AppSignalSetProvider::getInstance()),
	m_propManager(AppSignalPropertyManager::getInstance()),
	m_findString(new QLineEdit(this)),
	m_replaceString(new QLineEdit(this)),
	m_searchInPropertyList(new QComboBox(this)),
	m_caseSensitive(new QCheckBox("Case sensitive", this)),
	m_wholeWords(new QCheckBox("Whole words only", this)),
	m_searchInSelected(new QCheckBox("Search in selected only", this)),
	m_signalsQuantityLabel(new QLabel(this)),
	m_canBeReplacedQuantityLabel(new QLabel(this)),
	m_foundList(new QTableView(this)),
	m_foundListModel(new QStandardItemModel(0, 3, this)),
	m_replaceAllButton(new QPushButton("Replace All", this)),
	m_replaceAndFindNextButton(new QPushButton("Replace and Find", this)),
	m_findPreviousButton(new QPushButton("Find Previous", this)),
	m_findNextButton(new QPushButton("Find Next", this)),
	m_replaceableSignalQuantityBlinkTimer(new QTimer(this)),
	m_regExp4Id(AppSignal::IDENTIFICATORS_VALIDATOR)
{
	m_isExpertMode = theAppSettings.isExpertMode();

	m_currentUserId = m_signalSetProvider->currentUserID();
	m_currentUserIsAdmin = m_signalSetProvider->currentUserIsAdmin();

	setWindowTitle("Find and Replace");

	m_signalProxyModel = dynamic_cast<SignalsProxyModel*>(m_signalTable->model());

	if (m_signalProxyModel == nullptr)
	{
		assert(false);
		deleteLater();
	}

	m_signalModel = dynamic_cast<SignalsModel*>(m_signalProxyModel->sourceModel());

	if (m_signalModel == nullptr)
	{
		assert(false);
		deleteLater();
	}

	m_searchInPropertyList->addItems(QStringList() <<
									 AppSignalPropNames::APP_SIGNAL_ID <<
									 AppSignalPropNames::CUSTOM_APP_SIGNAL_ID <<
									 AppSignalPropNames::CAPTION <<
									 AppSignalPropNames::EQUIPMENT_ID);

	m_foundList->setModel(m_foundListModel);

	m_foundList->verticalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);
	m_foundList->setAlternatingRowColors(false);
	m_foundList->setStyleSheet("QTableView::item:focus{background-color:darkcyan}");
	m_foundList->setEditTriggers(QTableView::NoEditTriggers);
	m_foundList->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	m_foundList->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

	m_foundList->verticalHeader()->setDefaultSectionSize(static_cast<int>(m_foundList->fontMetrics().height() * 1.4));
	m_foundList->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

	m_foundList->horizontalHeader()->setHighlightSections(false);
	m_foundList->horizontalHeader()->setDefaultSectionSize(100);
	m_foundList->horizontalHeader()->setStretchLastSection(true);

	m_foundListModel->setHeaderData(0, Qt::Horizontal, "Found");
	m_foundListModel->setHeaderData(1, Qt::Horizontal, "Replace result");
	m_foundListModel->setHeaderData(2, Qt::Horizontal, "Can be replaced");

	m_replaceAllButton->setAutoDefault(false);
	m_replaceAndFindNextButton->setAutoDefault(false);
	m_findPreviousButton->setAutoDefault(false);
	m_findNextButton->setAutoDefault(false);

	connect(m_findString, &QLineEdit::returnPressed, this, &FindSignalDialog::generateListIfNeededWithWarning);
	connect(m_replaceString, &QLineEdit::returnPressed, this, &FindSignalDialog::generateListIfNeededWithWarning);
	connect(m_searchInPropertyList, &QComboBox::currentTextChanged, this, &FindSignalDialog::generateListIfNeededWithWarning);
	connect(m_caseSensitive, &QCheckBox::checkStateChanged, this, &FindSignalDialog::generateListIfNeededWithWarning);
	connect(m_wholeWords, &QCheckBox::checkStateChanged, this, &FindSignalDialog::generateListIfNeededWithWarning);
	connect(m_searchInSelected, &QCheckBox::checkStateChanged, this, &FindSignalDialog::generateListIfNeededWithWarning);

	connect(m_replaceString, &QLineEdit::textEdited, this, &FindSignalDialog::updateAllReplacement);

	connect(m_foundList->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FindSignalDialog::selectCurrentSignalOnAppSignalsTab);

	connect(m_replaceAllButton, &QPushButton::clicked, this, &FindSignalDialog::replaceAll);
	connect(m_replaceAndFindNextButton, &QPushButton::clicked, this, &FindSignalDialog::replaceAndFindNext);
	connect(m_findPreviousButton, &QPushButton::clicked, this, &FindSignalDialog::findPrevious);
	connect(m_findNextButton, &QPushButton::clicked, this, &FindSignalDialog::findNext);

	// Create completers
	//
	QStringList completerStringList = QSettings{}.value("FindSignalDialog/SearchCompleter").toStringList();
	m_findCompleter = new QCompleter(completerStringList, this);
	m_findCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	m_findString->setCompleter(m_findCompleter);
	connect(m_findString, &QLineEdit::textEdited, this, [this](){m_findCompleter->complete();});
	connect(m_findCompleter, static_cast<void(QCompleter::*)(const QString&)>(&QCompleter::highlighted), m_findString, &QLineEdit::setText);

	completerStringList = QSettings{}.value("FindSignalDialog/ReplaceCompleter").toStringList();
	m_replaceCompleter = new QCompleter(completerStringList, this);
	m_replaceCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	m_replaceString->setCompleter(m_replaceCompleter);
	connect(m_replaceString, &QLineEdit::textEdited, this, [this](){m_replaceCompleter->complete();});
	connect(m_replaceCompleter, static_cast<void(QCompleter::*)(const QString&)>(&QCompleter::highlighted), m_replaceString, &QLineEdit::setText);
	//

	QFormLayout* form = new QFormLayout;

	form->addRow("Find:", m_findString);
	form->addRow("Replace with:", m_replaceString);

	QHBoxLayout* searchSettingsRow = new QHBoxLayout;
	searchSettingsRow->addWidget(m_searchInPropertyList);
	searchSettingsRow->addWidget(m_caseSensitive);
	searchSettingsRow->addWidget(m_wholeWords);
	searchSettingsRow->addWidget(m_searchInSelected);
	searchSettingsRow->addStretch();

	form->addRow("Search in:", searchSettingsRow);

	QHBoxLayout* resultRow = new QHBoxLayout;
	resultRow->addWidget(m_signalsQuantityLabel);
	resultRow->addWidget(m_canBeReplacedQuantityLabel);
	resultRow->addStretch();

	connect(m_replaceableSignalQuantityBlinkTimer, &QTimer::timeout, this, &FindSignalDialog::blinkReplaceableSignalQuantity);

	form->addRow(resultRow);

	QVBoxLayout* vLayout = new QVBoxLayout;
	vLayout->addLayout(form);

	vLayout->addWidget(m_foundList);

	QHBoxLayout* hLayout = new QHBoxLayout;

	hLayout->addWidget(m_replaceAllButton);
	hLayout->addWidget(m_replaceAndFindNextButton);
	hLayout->addWidget(m_findPreviousButton);
	hLayout->addWidget(m_findNextButton);

	vLayout->addLayout(hLayout);

	setLayout(vLayout);

	setWindowPosition(this, "FindSignalDialog");

	new TableDataVisibilityController(m_foundList, "FindSignalDialog", QVector<int>() << 0 << 1 << 2);

	QTimer::singleShot(0, [this](){ m_findString->setFocus(); });
}

void FindSignalDialog::reject()
{
	saveDialogGeometry();
	m_shouldReopen = false;

	QDialog::reject();
}

void FindSignalDialog::generateListIfNeeded()
{
	SearchOptions currentOptions = getCurrentSearchOptions();

	if (m_searchOptionsUsedLastTime == currentOptions && m_isMatchToCurrentSignalSet == true)
	{
		return;
	}

	if (currentOptions.findString.isEmpty())
	{
		m_findString->setFocus();
		return;
	}

	m_searchOptionsUsedLastTime = currentOptions;

	QString fieldName = m_searchInPropertyList->currentText();
	m_checkCorrectnessOfId = ((fieldName == AppSignalPropNames::APP_SIGNAL_ID) ||
							  (fieldName == AppSignalPropNames::CUSTOM_APP_SIGNAL_ID));

	reloadCurrentIdsMap();

	int selectedSignalId = getSignalId(getSelectedRow());

	m_foundListModel->removeRows(0, m_foundListModel->rowCount());
	m_totalSignalQuantity = 0;
	m_replaceableSignalQuantity = 0;

	if (currentOptions.searchInSelected == false)
	{
		for (int i = 0; i < m_signalModel->rowCount(); i++)
		{
			addSignalIfNeeded(*m_signalSetProvider->getLoadedSignalByIndex(i, false));
		}
	}
	else
	{
		QModelIndexList selection = m_signalTable->selectionModel()->selectedRows(0);
		if (selection.count() == 0)
		{
			QMessageBox::warning(this, tr("Warning"), tr("No one signal was selected!"));
		}
		for (int i = 0; i < selection.count(); i++)
		{
			int row = m_signalProxyModel->mapToSource(selection[i]).row();
			addSignalIfNeeded(*m_signalSetProvider->getLoadedSignalByIndex(row, false));
		}
	}

	markFistInstancesIfItTheyNotUnique();

	for (int currentIndex = 0; currentIndex < m_foundListModel->rowCount(); currentIndex++)
	{
		if (selectedSignalId == getSignalId(currentIndex))
		{
			m_foundList->setCurrentIndex(m_foundListModel->index(currentIndex, 0));
		}
	}

	updateCounters();

	m_isMatchToCurrentSignalSet = true;
}

void FindSignalDialog::addSignalIfNeeded(const AppSignal& signal)
{
	QString propertyValue = getProperty(signal);

	qsizetype start = 0;
	qsizetype end = -1;

	if (match(propertyValue, start, end) == true)
	{
		int currentIndex = m_foundListModel->rowCount();
		m_foundListModel->insertRows(currentIndex, 1);

		m_foundListModel->setData(m_foundListModel->index(currentIndex, 0), propertyValue, Qt::DisplayRole);
		m_foundListModel->setData(m_foundListModel->index(currentIndex, 0), signal.ID(), Qt::UserRole);

		m_totalSignalQuantity++;

		updateReplacement(signal, currentIndex, uppercase());
	}
}

bool FindSignalDialog::match(QString signalProperty, qsizetype& start, qsizetype& end)
{
	if (m_findString == nullptr)
	{
		assert(false);
		close();
		return false;
	}
	QString findString = m_searchOptionsUsedLastTime.findString;
	if (findString.isEmpty())
	{
		return false;
	}

	if (m_searchOptionsUsedLastTime.caseSensitive == false)
	{
		signalProperty = signalProperty.toUpper();
	}

	start = signalProperty.indexOf(findString, start);
	if (start != -1)
	{
		end = start + findString.size();

		if (m_searchOptionsUsedLastTime.wholeWords == true)
		{
			bool isPartOfWord = false;

			if (start > 0)
			{
				QChar previousChar = signalProperty[start - 1];
				if (previousChar.isLetterOrNumber() || previousChar == '_')
				{
					isPartOfWord = true;
				}
			}

			if (end < signalProperty.size())
			{
				QChar nextChar = signalProperty[end];
				if (nextChar.isLetterOrNumber() || nextChar == '_')
				{
					isPartOfWord = true;
				}
			}

			if (isPartOfWord == true)
			{
				start = -1;
				end = -1;
				return false;
			}
		}
		return true;
	}
	return false;
}

bool FindSignalDialog::checkForEditableSignal(const AppSignal& signal)
{
	return (signal.checkedOut() == false || signal.userID() == m_currentUserId || m_currentUserIsAdmin == true);
}

bool FindSignalDialog::checkForUniqueSignalId(const QString& original, const QString& replaced)
{
	bool result = true;

	if (m_repeatedSignalIds.contains(original))
	{
		result = false;
	}

	if (m_signalIds.contains(replaced) == true)
	{
		result = false;
		m_repeatedSignalIds.insert(replaced);
	}
	else
	{
		m_signalIds.insert(replaced);
	}

	return result;
}

bool FindSignalDialog::checkForCorrectSignalId(const QString& replaced)
{
	return m_regExp4Id.match(replaced).hasMatch();
}

FindSignalDialog::SearchOptions FindSignalDialog::getCurrentSearchOptions()
{
	SearchOptions options;
	if (m_findString == nullptr ||
			m_searchInPropertyList == nullptr ||
			m_searchInSelected == nullptr ||
			m_caseSensitive == nullptr ||
			m_wholeWords == nullptr)
	{
		assert(false);
		return options;
	}

	int propertyIndex = m_propManager->propertyIndex(m_searchInPropertyList->currentText());

	if (propertyIndex == -1)
	{
		assert(false);
		return options;
	}

	options.searchedPropertyIndex = propertyIndex;
	options.searchInSelected = m_searchInSelected->isChecked();
	options.caseSensitive = m_caseSensitive->isChecked();
	options.wholeWords = m_wholeWords->isChecked();
	options.findString = m_findString->text();

	if (options.caseSensitive == false)
	{
		options.findString = options.findString.toUpper();
	}

	saveFindCompleter();

	return options;
}

QString FindSignalDialog::getProperty(const AppSignal& signal)
{
	if (m_searchOptionsUsedLastTime.searchedPropertyIndex == -1)
	{
		return QString();
	}

	return m_propManager->value(&signal, m_searchOptionsUsedLastTime.searchedPropertyIndex, m_isExpertMode).toString();
}

void FindSignalDialog::setProperty(AppSignal& signal, const QString& value)
{
	if (m_searchOptionsUsedLastTime.searchedPropertyIndex == -1)
	{
		assert(false);
		return;
	}

	m_propManager->setValue(&signal, m_searchOptionsUsedLastTime.searchedPropertyIndex, value, m_isExpertMode);
}

int FindSignalDialog::getSignalId(int row)
{
	return m_foundListModel->data(m_foundListModel->index(row, 0), Qt::UserRole).toInt();
}

int FindSignalDialog::getSelectedRow()
{
	return m_foundList->currentIndex().row();
}

void FindSignalDialog::selectRow(int row)
{
	m_foundList->setCurrentIndex(m_foundListModel->index(row, 0));
}

bool FindSignalDialog::isReplaceable(int row)
{
	bool replaceable = m_foundListModel->data(m_foundListModel->index(row, 2), Qt::UserRole).toBool();
	if (replaceable == false)
	{
		return false;
	}
	int signalId = getSignalId(row);

	m_signalSetProvider->getLoadedSignalByID(signalId, false);

	int signalIndex = m_signalSetProvider->signalIndex(signalId);
	if (signalIndex == -1)	// Doesn't exist???
	{
		assert(false);
		return false;
	}

	return m_signalSetProvider->isEditableSignal(signalIndex);
}

void FindSignalDialog::replace(int row)
{
	int signalId = getSignalId(row);

	int signalIndex = m_signalSetProvider->signalIndex(signalId);

	if (signalIndex == AppSignalSet::BAD_INDEX)
	{
		Q_ASSERT(false);
		return;
	}

	QString errorMessage;

	bool checkedout = m_signalSetProvider->checkoutSignalByIndex(signalIndex, &errorMessage);

	if (checkedout == false)
	{
		m_foundListModel->setData(m_foundListModel->index(row, 2), cannotCheckoutMessage + ':' + errorMessage, Qt::DisplayRole);
		m_foundListModel->setData(m_foundListModel->index(row, 2), false, Qt::UserRole);
		return;
	}

	const AppSignal* existsSignal = m_signalSetProvider->getSignalByID(signalId);

	if (existsSignal == nullptr)
	{
		Q_ASSERT(false);
		return;
	}

	AppSignal signal(*existsSignal);
	QString newValue = m_foundListModel->data(m_foundListModel->index(row, 1), Qt::DisplayRole).toString();

	setProperty(signal, newValue);

	m_signalSetProvider->saveSignal(&signal, this);

	m_foundListModel->setData(m_foundListModel->index(row, 2), replacedMessage, Qt::DisplayRole);

	saveReplaceCompleter();
}

void FindSignalDialog::reloadCurrentIdsMap()
{
	if (m_checkCorrectnessOfId == false)
	{
		return;
	}

	m_signalIds.clear();
	m_repeatedSignalIds.clear();

	for (int i = 0; i < m_signalModel->rowCount(); i++)
	{
		QString id = getProperty(*m_signalSetProvider->getLoadedSignalByIndex(i, false));
		if (m_signalIds.contains(id))
		{
			m_repeatedSignalIds.insert(id);
		}
		else
		{
			m_signalIds.insert(id);
		}
	}
}

void FindSignalDialog::markFistInstancesIfItTheyNotUnique()
{
	if (m_checkCorrectnessOfId == false)
	{
		return;
	}

	for (int currentIndex = 0; currentIndex < m_foundListModel->rowCount(); currentIndex++)
	{
		bool replaceable = m_foundListModel->data(m_foundListModel->index(currentIndex, 2), Qt::UserRole).toBool();
		QString replacement = m_foundListModel->data(m_foundListModel->index(currentIndex, 1), Qt::DisplayRole).toString();
		if (replaceable == true && m_repeatedSignalIds.contains(replacement) == true)
		{
			m_foundListModel->setData(m_foundListModel->index(currentIndex, 2), false, Qt::UserRole);
			m_foundListModel->setData(m_foundListModel->index(currentIndex, 2), notUniqueMessage, Qt::DisplayRole);
			m_replaceableSignalQuantity--;
		}
	}
}

void FindSignalDialog::updateCounters()
{
	m_signalsQuantityLabel->setText(QString("Found signals (%1) / ").arg(m_totalSignalQuantity, 3, 10, Latin1Char::ZERO));
	m_canBeReplacedQuantityLabel->setText(QString("Can be replaced:(%1)").arg(m_replaceableSignalQuantity, 3, 10, Latin1Char::ZERO));

	if (m_totalSignalQuantity != m_replaceableSignalQuantity)
	{
		blinkReplaceableSignalQuantity();
		m_replaceableSignalQuantityBlinkTimer->start(500);
	}
	else
	{
		m_replaceableSignalQuantityBlinkTimer->stop();
		m_canBeReplacedQuantityLabel->setStyleSheet("");
	}
}

void FindSignalDialog::saveDialogGeometry()
{
	saveWindowPosition(this, "FindSignalDialog");
}

void FindSignalDialog::saveFindCompleter()
{
	QString findText = m_findString->text();
	if (findText.isEmpty() == true)
	{
		return;
	}

	QStringListModel* model = dynamic_cast<QStringListModel*>(m_findCompleter->model());
	if (model == nullptr)
	{
		assert(model != nullptr);
		return;
	}

	QStringList completerStringList = model->stringList();
	if (completerStringList.contains(findText, Qt::CaseInsensitive) == false)
	{
		completerStringList.push_back(findText);
		while (completerStringList.size() > 50)
		{
			completerStringList.pop_front();
		}
		QSettings{}.setValue("FindSignalDialog/SearchCompleter", completerStringList);
		model->setStringList(completerStringList);
	}
}

void FindSignalDialog::saveReplaceCompleter()
{
	QString replaceText = m_replaceString->text();
	if (replaceText.isEmpty() == true)
	{
		return;
	}

	QStringListModel* model = dynamic_cast<QStringListModel*>(m_replaceCompleter->model());
	if (model == nullptr)
	{
		assert(model != nullptr);
		return;
	}

	QStringList completerStringList = model->stringList();

	if (completerStringList.contains(replaceText, Qt::CaseInsensitive) == false)
	{
		completerStringList.push_back(replaceText);
		while (completerStringList.size() > 50)
		{
			completerStringList.pop_front();
		}
		model->setStringList(completerStringList);
		QSettings{}.setValue("FindSignalDialog/ReplaceCompleter", completerStringList);
	}
}

void FindSignalDialog::generateListIfNeededWithWarning()
{
	generateListIfNeeded();
}

bool FindSignalDialog::uppercase() const
{
	return	m_searchInPropertyList->currentText() == AppSignalPropNames::APP_SIGNAL_ID &&
			m_signalSetProvider->projectProperty_uppercaseAppSignalID();
}

void FindSignalDialog::updateAllReplacement()
{
	m_replaceableSignalQuantity = 0;

	reloadCurrentIdsMap();

	bool upc = uppercase();

	for (int i = 0; i < m_foundListModel->rowCount(); i++)
	{
		updateReplacement(i, upc);
	}

	markFistInstancesIfItTheyNotUnique();

	updateCounters();
}

void FindSignalDialog::updateReplacement(int row, bool uppercase)
{
	int signalId = getSignalId(row);

	const AppSignal& signal = *m_signalSetProvider->getLoadedSignalByID(signalId, false);

	updateReplacement(signal, row, uppercase);
}

void FindSignalDialog::updateReplacement(const AppSignal& signal, int row, bool uppercase)
{
	QString propertyValue = getProperty(signal);

	qsizetype start = 0;
	qsizetype end = -1;

	QString replaced = propertyValue;
	while (match(replaced, start, end) == true)
	{
		replaced = replaced.left(start) + m_replaceString->text() + replaced.mid(end);
		start += m_replaceString->text().length();
	}

	if (uppercase)
	{
		replaced = replaced.toUpper();
	}

	m_foundListModel->setData(m_foundListModel->index(row, 1), replaced, Qt::DisplayRole);

	bool replaceable = true;

	if (replaceable == true && checkForEditableSignal(signal) == false)
	{
		replaceable = false;
		m_foundListModel->setData(m_foundListModel->index(row, 2), notEditableMessage, Qt::DisplayRole);
	}

	if (replaceable == true && m_checkCorrectnessOfId == true && checkForCorrectSignalId(replaced) == false)
	{
		replaceable = false;
		m_foundListModel->setData(m_foundListModel->index(row, 2), notCorrectIdMessage, Qt::DisplayRole);
	}

	if (replaceable == true && m_checkCorrectnessOfId == true && checkForUniqueSignalId(propertyValue, replaced) == false)
	{
		replaceable = false;
		m_foundListModel->setData(m_foundListModel->index(row, 2), notUniqueMessage, Qt::DisplayRole);
	}

	m_foundListModel->setData(m_foundListModel->index(row, 2), replaceable, Qt::UserRole);
	if (replaceable == true)
	{
		m_replaceableSignalQuantity++;
		m_foundListModel->setData(m_foundListModel->index(row, 2), replaceableMessage, Qt::DisplayRole);
	}
}

void FindSignalDialog::replaceAll()
{
	generateListIfNeeded();

	bool allAreReplaceable = true;

	for (int row = 0; row < m_foundListModel->rowCount(); row++)
	{
		if (isReplaceable(row) == false)
		{
			allAreReplaceable = false;
			break;
		}
	}

	if (allAreReplaceable == false)
	{
		QMessageBox msgBox(
					QMessageBox::Warning,
					"Warning",
					"Some signals have not replaceable values",
					QMessageBox::Yes | QMessageBox::Cancel,
					this);

		msgBox.button(QMessageBox::Yes)->setText("Replace all possible");

		if (msgBox.exec() == QMessageBox::Cancel)
		{
			return;
		}
	}

	for (int row = 0; row < m_foundListModel->rowCount(); row++)
	{
		if (isReplaceable(row) == true)
		{
			replace(row);
		}
	}
}

void FindSignalDialog::replaceAndFindNext()
{
	generateListIfNeeded();

	int row = getSelectedRow();

	if (isReplaceable(row) == true)
	{
		replace(row);
		selectRow(row + 1);
	}
	else
	{
		QMessageBox msgBox(
					QMessageBox::Warning,
					"Warning",
					"Value could not be replaced",
					QMessageBox::Yes | QMessageBox::Cancel,
					this);

		msgBox.button(QMessageBox::Yes)->setText("Skip and goto next");

		if (msgBox.exec() == QMessageBox::Yes)
		{
			selectRow(row + 1);
		}
	}
}

void FindSignalDialog::findPrevious()
{
	generateListIfNeeded();

	int row = getSelectedRow();

	if (row > 0)
	{
		selectRow(row - 1);
	}
}

void FindSignalDialog::findNext()
{
	generateListIfNeeded();

	int row = getSelectedRow();

	if (row < m_foundListModel->rowCount() - 1)
	{
		selectRow(row + 1);
	}
}

void FindSignalDialog::selectCurrentSignalOnAppSignalsTab()
{
	QModelIndex index = m_foundList->currentIndex();
	if (index.isValid())
	{
		m_signalTable->clearSelection();
		int signalId = m_foundListModel->data(m_foundListModel->index(index.row(), 0), Qt::UserRole).toInt();
		emit signalSelected(signalId);
	}
}

void FindSignalDialog::blinkReplaceableSignalQuantity()
{
	if (m_replaceableSignalQuantityBlinkIsOn)
	{
		m_canBeReplacedQuantityLabel->setStyleSheet("QLabel {background-color : red; color : yellow;}");
	}
	else
	{
		m_canBeReplacedQuantityLabel->setStyleSheet("QLabel {background-color : yellow; color : red;}");
	}

	m_replaceableSignalQuantityBlinkIsOn = !m_replaceableSignalQuantityBlinkIsOn;
}
