#pragma once

#include <UiLib/PropertyEditor.h>
#include <UiLib/ChooseItemsWidget.h>

class DbChooseItemsDialog : public ExtWidgets::PropertyTextEditor
{
	Q_OBJECT

public:
	static DbChooseItemsDialog* tagsEditor(DbController* dbController, QWidget* parent);
    static DbChooseItemsDialog* matsUsersEditor(DbController* dbController, QWidget* parent);

private:
    DbChooseItemsDialog(const std::vector<std::pair<QString, QString>>& tagsWithDescriptions, QWidget* parent);

public:
    virtual ~DbChooseItemsDialog();

    QString text() const override;
    void setText(const QString& text) override;

    bool readOnly() const override;
    void setReadOnly(bool value) override;

protected:
    bool externalOkCancelButtons() const override;

private:
	bool isModified() const override;

private:
     UiLib::ChooseItemsWidget* m_tagsWidget = nullptr;
};
