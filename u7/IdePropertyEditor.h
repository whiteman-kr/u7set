#ifndef IDEPROPERTYEDITOR_H
#define IDEPROPERTYEDITOR_H

#include "../QScintilla/Qt4Qt5/Qsci/qsciscintilla.h"
#include "../QScintilla/Qt4Qt5/Qsci/qscilexercpp.h"
#include "../QScintilla/Qt4Qt5/Qsci/qscilexerxml.h"

#include "../lib/PropertyEditor.h"

#include "../lib/Tuning/TuningFilterEditor.h"

//
// IdePropertyEditor
//

class IdePropertyEditor : public ExtWidgets::PropertyEditor
{
public:
    IdePropertyEditor(QWidget* parent);
    virtual ~IdePropertyEditor();

    virtual void saveSettings();

    virtual ExtWidgets::PropertyTextEditor* createCodeEditor(Property *property, QWidget* parent) override;
};


//
// DialogFindReplace
//

class DialogFindReplace : public QDialog
{
    Q_OBJECT
public:
    DialogFindReplace(QWidget* parent);

signals:
    void findFirst(QString findText);
    void replace(QString findText, QString text);
    void replaceAll(QString findText, QString replaceText);

private slots:
    void onFind();
    void onReplace();
    void onReplaceAll();

private:
    QLineEdit* m_findEdit = nullptr;
    QLineEdit* m_replaceEdit = nullptr;

    QPushButton* m_findButton = nullptr;
    QPushButton* m_replaceButton = nullptr;
    QPushButton* m_replaceAllButton = nullptr;
};

//
// IdeCodeEditor
//

enum class CodeType
{
    Cpp,
    Xml,
    Unknown
};

class IdeCodeEditor : public ExtWidgets::PropertyTextEditor
{
    Q_OBJECT
public:
    IdeCodeEditor(CodeType codeType, QWidget* parent);
    ~IdeCodeEditor();

    virtual void setText(const QString& text);

    virtual QString text();

    virtual void setReadOnly(bool value);

public slots:
    void findFirst(QString findText);
    void findNext();
    void replace(QString findText, QString replaceText);
    void replaceAll(QString findText, QString replaceText);

protected:
    bool eventFilter(QObject* obj, QEvent* event);

private:
    QsciScintilla* m_textEdit = nullptr;
    QsciLexerCPP m_lexerCpp;
    QsciLexerXML m_lexerXml;

    QWidget* m_parent = nullptr;

    DialogFindReplace* m_findReplace = nullptr;

    QString m_findText;

};

//
// IdeTuningFiltersEditor
//

class IdeTuningFiltersEditor : public ExtWidgets::PropertyTextEditor
{
public:

    explicit IdeTuningFiltersEditor(QWidget* parent);
    virtual ~IdeTuningFiltersEditor();

    void setText(const QString& text) override;

    QString text() override;

    void setReadOnly(bool value) override;

private:
    TuningFilterEditor* m_tuningFilterEditor = nullptr;

    TuningSignalStorage m_signalStorage;
    TuningFilterStorage m_filterStorage;

};



#endif // IDEPROPERTYEDITOR_H

