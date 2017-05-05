#ifndef DIALOGFILEEDITOR_H
#define DIALOGFILEEDITOR_H

#include <QDialog>
#include "IdePropertyEditor.h"
#include "../lib/DbController.h"

class DialogFileEditor : public QDialog
{
    Q_OBJECT

public:
	explicit DialogFileEditor(const QString& fileName, QByteArray *pData, DbController* pDbController, bool readOnly, QWidget *parent);
    ~DialogFileEditor();

private slots:
	void on_DialogFileEditor_finished(int result);

	void on_ok_clicked();
	void on_cancel_clicked();
	void on_validate_clicked();

	void on_validate(QAction* pAction);

private:
	bool saveChanges();

	virtual void closeEvent(QCloseEvent* e);
    virtual void reject();

	bool validate(int schemaFileId);
private:

    QByteArray* m_pData;

    DbController* m_pDbController;
	std::vector<DbFileInfo> m_validateFiles;
	bool m_readOnly;

	CodeEditor* m_editor = nullptr;
	QPushButton* m_buttonValidate = nullptr;
	QPushButton* m_buttonOK = nullptr;
	QPushButton* m_buttonCancel = nullptr;
};

#endif // DIALOGFILEEDITOR_H
