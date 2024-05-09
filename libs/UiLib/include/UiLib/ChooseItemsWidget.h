#pragma once
#include <QWidget>

class QLineEdit;
class QTreeWidget;
class QPushButton;
class QTreeWidgetItem;

namespace UiLib
{
	class ChooseItemsWidget : public QWidget
	{
		Q_OBJECT

	public:
		ChooseItemsWidget(const QStringList& tags, QWidget* parent);
		ChooseItemsWidget(const std::vector<std::pair<QString, QString>>& tagsWithDescriptions, QWidget* parent);

		virtual ~ChooseItemsWidget();

		QString text() const;
		void setText(const QString& text);

		bool readOnly() const;
		void setReadOnly(bool value);

	signals:
		void okPressed();
		void cancelPressed();

	private slots:
		void tagsTextChanged(const QString& text);
		void tagsListItemChanged(QTreeWidgetItem * item, int column);
		void tagsListItemPressed(QTreeWidgetItem * item, int column);
		void filterTextChanged(const QString& text);

	private:
		void setupUi();

		void fillTags();
		void updateChecks(const QString& text);
		void updateTags();

	private:
		QWidget* m_parent = nullptr;
		QLineEdit* m_textEdit = nullptr;
		QLineEdit* m_filterEdit = nullptr;
		QTreeWidget* m_list = nullptr;

		QPushButton* m_okButton = nullptr;
		QPushButton* m_cancelButton = nullptr;

		static QString m_filterText;

		QChar m_separator = QChar::Space;

		std::vector<std::pair<QString, QString>>
			m_tagsWithDescriptions; // First element is tag name, second element name is tag description
	};
} // namespace SchemaClientLib
