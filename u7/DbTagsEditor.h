#ifndef DBTAGSEDITOR_H
#define DBTAGSEDITOR_H

#include "../lib/PropertyEditor.h"
#include "../lib/Ui/DialogChooseTags.h"

class DbTagsEditor : public ExtWidgets::PropertyTextEditor
{
	Q_OBJECT
public:
    explicit DbTagsEditor(DbController* dbController, QWidget* parent);
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
     ChooseTagsWidget* m_cw = nullptr;
};

#endif // DBTAGSEDITOR_H
