#pragma once

#include "../../Simulator/SimProfiles.h"
#include "../lib/DbController.h"
#include "../QScintilla/Qt4Qt5/Qsci/qsciscintilla.h"
#include "../lib/QScintillaLexers/LexerJavaScript.h"

class SimProfileEditor : public QDialog
{
	Q_OBJECT
public:
static void run(DbController* dbController, QWidget* parent);

private:
	SimProfileEditor(DbController* dbController, QWidget* parent);
	virtual ~SimProfileEditor();

protected:
	virtual void closeEvent(QCloseEvent* e) override;

private:
	bool askForSaveChanged();

private slots:
	bool saveChanges();

	void checkProfiles();
	void example();

	void textChanged();

	void accept() override;
	void reject() override;

	void projectClosed();

private:
	bool m_modified = false;

	QString m_startText;

	DbController* m_db = nullptr;

	LexerJavaScript m_lexer;
	QsciScintilla* m_textEdit = nullptr;

	static SimProfileEditor* m_simProfileEditor;
	static const QString m_exampleText;
};


