#ifndef DBTAGSEDITOR_H
#define DBTAGSEDITOR_H

#include "../lib/PropertyEditor.h"
#include "../lib/Ui/ChooseTagsWidget.h"

class DbTagsEditor : public ExtWidgets::PropertyTextEditor
{
	Q_OBJECT

public:
	static DbTagsEditor* tagsEditor(DbController* dbController, QWidget* parent);
    static DbTagsEditor* matsUsersEditor(DbController* dbController, QWidget* parent);

private:
    DbTagsEditor(DbController* dbController, bool showTags, bool showUsers, QWidget* parent);

public:
    virtual ~DbTagsEditor();

    QString text() const override;
    void setText(const QString& text) override;

    bool readOnly() const override;
    void setReadOnly(bool value) override;

protected:
    bool externalOkCancelButtons() const override;

private:
	bool isModified() const override;

private:
     ChooseTagsWidget* m_tagsWidget = nullptr;
};


#endif // DBTAGSEDITOR_H
