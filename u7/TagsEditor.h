#ifndef TAGSEDITOR_H
#define TAGSEDITOR_H

#include "../lib/PropertyEditor.h"
#include "../lib/DbController.h"

class TagsEditor : public ExtWidgets::PropertyTextEditor
{
	Q_OBJECT
public:
	explicit TagsEditor(DbController* dbController, QWidget* parent);
	virtual ~TagsEditor();

	QString text() const override;
	void setText(const QString& text) override;

	bool readOnly() const override;
	void setReadOnly(bool value) override;

	bool externalOkCancelButtons() const override;

private slots:
	void tagsTextChanged(const QString& text);
	void tagsListItemChanged(QTreeWidgetItem* item, int column);
	void tagsListItemPressed(QTreeWidgetItem* item, int column);
	void filterTextChanged(const QString& text);

private:
	void fillDbTags();
	void updateChecks(const QString& text);
	void updateTags();

private:
	QWidget* m_parent = nullptr;
	QLineEdit* m_textEdit = nullptr;
	QLineEdit* m_filterEdit = nullptr;
	QTreeWidget* m_tagsList = nullptr;

	QPushButton* m_okButton = nullptr;
	QPushButton* m_cancelButton = nullptr;

	static QString m_filterText;

	std::vector<DbTag> m_dbTags;
};

#endif // TAGSEDITOR_H
