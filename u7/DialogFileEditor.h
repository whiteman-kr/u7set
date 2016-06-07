#ifndef DIALOGFILEEDITOR_H
#define DIALOGFILEEDITOR_H

#include <QDialog>
#include "../lib/DbController.h"

namespace Ui {
class DialogFileEditor;
}

class DialogFileEditor : public QDialog
{
    Q_OBJECT

public:
    explicit DialogFileEditor(const QString& fileName, QByteArray *pData, DbController* pDbController, bool readOnly, QWidget *parent);
    ~DialogFileEditor();


private slots:
	void on_DialogFileEditor_finished(int result);
	void on_btnOk_clicked();
	void on_btnCancel_clicked();
	void on_m_text_textChanged();
	void on_btnValidate_clicked();
	void on_btnLoadFbl_clicked();
	void on_validate(QAction* pAction);

private:
	bool saveChanges();

	virtual void closeEvent(QCloseEvent* e);
    virtual void reject();

	bool validate(int schemaFileId);
private:
    Ui::DialogFileEditor *ui;

    QByteArray* m_pData;

    DbController* m_pDbController;
	std::vector<DbFileInfo> m_validateFiles;
	bool m_readOnly;
	bool m_modified = false;
};

#endif // DIALOGFILEEDITOR_H
