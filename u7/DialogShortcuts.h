#ifndef DIALOGSHORTCUTS_H
#define DIALOGSHORTCUTS_H


class DialogShortcuts : public QDialog
{
	Q_OBJECT
public:
	DialogShortcuts(QWidget* parent);

protected:
	virtual void reject() override;

signals:
	void dialogClosed();

private:
	QTreeWidgetItem* addSection(const QString& name);
	void addShortcut(const QString& name, const QString& description, QTreeWidgetItem* sectionItem);

private:
	QTreeWidget* m_treeWidget = nullptr;
};

#endif // DIALOGSHORTCUTS_H
