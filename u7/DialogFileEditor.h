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

private:
	bool saveChanges();

	virtual void accept();
    virtual void reject();

private:

	QByteArray* m_pData = nullptr;

	DbController* m_pDbController = nullptr;

	std::vector<DbFileInfo> m_validateFiles;

	bool m_readOnly = false;

	IdeCodeEditor* m_editor = nullptr;
	QPushButton* m_buttonOK = nullptr;
	QPushButton* m_buttonCancel = nullptr;
};

#endif // DIALOGFILEEDITOR_H
