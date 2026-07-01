#pragma once

namespace VFrame30
{
	class SchemaItem;
} // namespace VFrame30

using SchemaItemPtr = std::shared_ptr<VFrame30::SchemaItem>;

class SchemaFindDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SchemaFindDialog(bool replaceEnabled, QWidget* parent);
	virtual ~SchemaFindDialog();

	QString findText() const;
	void setFocusToEditLine();

	void ensureVisible();

	bool replaceEnabled() const;

signals:
	void findPrev(Qt::CaseSensitivity cs);
	void findNext(Qt::CaseSensitivity cs);

	void replaceAndFind(QString findText, QString replaceWith, Qt::CaseSensitivity cs);
	void replaceAll(QString findText, QString replaceWith, Qt::CaseSensitivity cs);

protected slots:
	void replaceAndFindPressed();
	void replaceAllPressed();

public slots:
	void updateCompleter();
	void updateFoundInformation(SchemaItemPtr item,
								const std::list<std::pair<QString, QString>>& foundProps,
								QString searchText,
								Qt::CaseSensitivity cs);

private:
	virtual void closeEvent(QCloseEvent* e);
	virtual void done(int r);

	void saveSettings();
	void saveFindCompleter();
	void saveReplaceCompleter();

private:
	QLineEdit* m_findTextEdit = nullptr;
	QLineEdit* m_replaceTextEdit = nullptr;

	QCompleter* m_findCompleter = nullptr;
	QCompleter* m_replaceCompleter = nullptr;

	QCheckBox* m_caseSensitiveCheckBox = nullptr;
	QTextEdit* m_findResult = nullptr;

	QPushButton* m_prevButton = nullptr;
	QPushButton* m_nextButton = nullptr;

	QPushButton* m_replaceButton = nullptr;
	QPushButton* m_replaceAllButton = nullptr;

	bool m_replaceEnabled = false;
};