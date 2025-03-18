#pragma once

class AppSignal;
class AppSignalSetProvider;
class AppSignalPropertyManager;

class SignalsModel;
class SignalsProxyModel;

class FindSignalDialog : public QDialog
{
	Q_OBJECT

	class SearchOptions
	{
	public:
		QString findString;
		int searchedPropertyIndex{};
		bool searchInSelected{};
		bool caseSensitive{};
		bool wholeWords{};

		bool operator==(const SearchOptions &other) const {
			return findString == other.findString &&
					searchedPropertyIndex == other.searchedPropertyIndex &&
					searchInSelected == other.searchInSelected &&
					caseSensitive == other.caseSensitive &&
					wholeWords == other.wholeWords;
		}
	};

	static const QString notUniqueMessage;
	static const QString notEditableMessage;
	static const QString notCorrectIdMessage;
	static const QString cannotCheckoutMessage;
	static const QString replaceableMessage;
	static const QString replacedMessage;

public:
	FindSignalDialog(QTableView* parent = nullptr);

	bool shouldReopen() { return m_shouldReopen; }
	void allowReopen() { m_shouldReopen = true; }

signals:
	void signalSelected(int signalId);

protected:
	virtual void reject() override;

private:
	void addSignalIfNeeded(const AppSignal& signal);
	bool match(QString signalProperty, qsizetype& start, qsizetype& end);
	bool checkForEditableSignal(const AppSignal& signal);
	bool checkForUniqueSignalId(const QString& original, const QString& replaced);
	bool checkForCorrectSignalId(const QString& replaced);
	SearchOptions getCurrentSearchOptions();
	QString getProperty(const AppSignal& signal);
	void setProperty(AppSignal& signal, const QString& value);
	int getSignalId(int row);
	int getSelectedRow();
	void selectRow(int row);
	bool isReplaceable(int row);
	void replace(int row);
	void reloadCurrentIdsMap();
	void markFistInstancesIfItTheyNotUnique();
	void generateListIfNeeded();

	void updateCounters();

	void saveDialogGeometry();
	void saveFindCompleter();
	void saveReplaceCompleter();

private slots:
	void generateListIfNeededWithWarning();
	bool uppercase() const;
	void updateAllReplacement();
	void updateReplacement(int row, bool uppercase);
	void updateReplacement(const AppSignal& signal, int row, bool uppercase);
	void replaceAll();
	void replaceAndFindNext();
	void findPrevious();
	void findNext();
	void selectCurrentSignalOnAppSignalsTab();
	void blinkReplaceableSignalQuantity();

private:
	QTableView* m_signalTable = nullptr;
	SignalsProxyModel* m_signalProxyModel = nullptr;
	SignalsModel* m_signalModel = nullptr;

	AppSignalSetProvider* m_signalSetProvider = nullptr;
	AppSignalPropertyManager* m_propManager = nullptr;

	//

	QLineEdit* m_findString = nullptr;
	QLineEdit* m_replaceString = nullptr;

	QCompleter* m_findCompleter = nullptr;
	QCompleter* m_replaceCompleter = nullptr;

	QComboBox* m_searchInPropertyList = nullptr;

	QCheckBox* m_caseSensitive = nullptr;
	QCheckBox* m_wholeWords = nullptr;
	QCheckBox* m_searchInSelected = nullptr;

	QLabel* m_signalsQuantityLabel = nullptr;
	QLabel* m_canBeReplacedQuantityLabel = nullptr;

	QTableView* m_foundList = nullptr;
	QStandardItemModel* m_foundListModel = nullptr;

	QPushButton* m_replaceAllButton = nullptr;
	QPushButton* m_replaceAndFindNextButton = nullptr;
	QPushButton* m_findPreviousButton = nullptr;
	QPushButton* m_findNextButton = nullptr;

	//

	int m_totalSignalQuantity = 0;
	int m_replaceableSignalQuantity = 0;
	bool m_checkCorrectnessOfId = false;
	QTimer* m_replaceableSignalQuantityBlinkTimer = nullptr;
	bool m_replaceableSignalQuantityBlinkIsOn = false;

	SearchOptions m_searchOptionsUsedLastTime;
	bool m_isMatchToCurrentSignalSet = false;
	QSet<QString> m_signalIds;
	QSet<QString> m_repeatedSignalIds;
	QRegularExpression m_regExp4Id;
	int m_currentUserId = -1;
	bool m_currentUserIsAdmin = false;
	bool m_shouldReopen = true;

	bool m_isExpertMode = false;
};

