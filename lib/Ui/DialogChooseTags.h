#pragma once
#include <QWidget>
#include <QLayout>
#include <QPushButton>


class ChooseTagsWidget : public QWidget
{
    Q_OBJECT

public:
    ChooseTagsWidget(const QStringList &tags, QWidget* parent);
    ChooseTagsWidget(const std::vector<std::pair<QString, QString>>& tags, QWidget* parent);

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
    QTreeWidget* m_tagsList = nullptr;

    QPushButton* m_okButton = nullptr;
    QPushButton* m_cancelButton = nullptr;

    static QString m_filterText;

    std::vector<std::pair<QString, QString>> m_tags;
};

