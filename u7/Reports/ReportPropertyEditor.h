#pragma once

#include "../lib/PropertyEditor.h"
#include "../lib/CodeEditor.h"

class ReportPropertyEditor : public ExtWidgets::PropertyTextEditor
{
    Q_OBJECT
public:
    explicit ReportPropertyEditor(QWidget* parent);
    virtual ~ReportPropertyEditor();

    QString text() const override;
    void setText(const QString& text) override;

    bool readOnly() const override;
    void setReadOnly(bool value) override;

    bool externalOkCancelButtons() const override;

private:
    bool isModified() const override;
    void fillObjectsTree();

private slots:
    void onTextChanged();

    void onValidateClicked();
    void onOkClicked();
    void onCancelClicked();


private:
    CodeEditor* m_textEdit = nullptr;
    QTreeWidget* m_treeWidget = nullptr;

    QSplitter* m_topSplitter = nullptr;

    QPushButton* m_okButton = nullptr;
    QPushButton* m_cancelButton = nullptr;

    QWidget* m_parent = nullptr;
};
