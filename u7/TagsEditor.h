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

private slots:
	void tagsTextChanged(const QString& text);
	void tagsListItemChanged(QTreeWidgetItem* item, int column);
	void tagsListItemPressed(QTreeWidgetItem* item, int column);

private:
	void updateChecks(const QString& text);
	void updateTags();

private:
	QWidget* m_parent = nullptr;

	QLineEdit* m_textEdit = nullptr;
	QTreeWidget* m_tagsList = nullptr;
};

#endif // TAGSEDITOR_H
