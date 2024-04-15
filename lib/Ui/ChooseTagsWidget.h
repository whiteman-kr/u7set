#pragma once
#include <QWidget>
#include "../OnlineLib/MatsUsers.h"

class QLineEdit;
class QTreeWidget;
class QPushButton;
class QTreeWidgetItem;


class ChooseTagsWidget : public QWidget
{
    Q_OBJECT

public:
    ChooseTagsWidget(const QStringList &tags, QChar separator, QWidget* parent);
    
    ChooseTagsWidget(const std::vector<std::pair<QString, QString>>& tags,
					 const std::vector<OnlineLib::MatsUser>& users,
					 QChar separator, 
					 QWidget* parent);

    virtual ~ChooseTagsWidget();

    QString text() const;
    void setText(const QString& text);

    bool readOnly() const;
    void setReadOnly(bool value);

signals:
    void okPressed();
    void cancelPressed();

private slots:
    void tagsTextChanged(const QString& text);
    void tagsListItemChanged(QTreeWidgetItem* item, int column);
    void tagsListItemPressed(QTreeWidgetItem* item, int column);
    void filterTextChanged(const QString& text);

private:
    void setupUi(bool hasDescriptions);

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

    std::vector<std::pair<QString, QString>> m_tags;
    std::vector<OnlineLib::MatsUser> m_users;

    QChar m_separator;
    QString m_objectName = "Tag";
    QString m_objectNames = "tags";
};
