#pragma once
#include "SchemaItemControl.h"
#include "PropertyNames.h"

class QLineEdit;

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT SchemaItemLineEdit : public SchemaItemControl
	{
		Q_OBJECT

	public:
		SchemaItemLineEdit(void);
		explicit SchemaItemLineEdit(SchemaUnit unit);
		virtual ~SchemaItemLineEdit(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Methods
	public:
		virtual QWidget* createWidget(QWidget* parent, bool editMode) const override;
		virtual void updateWidgetProperties(QWidget* widget) const override;

	protected slots:
		void afterCreate(QLineEdit* control) const;
		void editingFinished();
		void returnPressed();
		void textChanged(const QString& text);

		void runEventScript(const QString& script, QLineEdit* widget);

		// Text search
		//
	public:
		virtual bool searchText(const QString& text) const override;

		// Properties and Data
		//
	public:
		const QString& text() const;
		void setText(QString value);

		const QString& placeholderText() const;
		void setPlaceholderText(QString value);

		int maxLength() const;
		void setMaxLength(int value);

		E::HorzAlign horzAlign() const;
		void setHorzAlign(E::HorzAlign value);

		E::VertAlign vertAlign() const;
		void setVertAlign(E::VertAlign value);

		bool readOnly() const;
		void setReadOnly(bool value);

		virtual void setStyleSheet(QString value) override;

		QString scriptAfterCreate() const;
		void setScriptAfterCreate(const QString& value);

		QString scriptEditingFinished() const;
		void setScriptEditingFinished(const QString& value);

		QString scriptReturnPressed() const;
		void setScriptReturnPressed(const QString& value);

		QString scriptTextChanged() const;
		void setScriptTextChanged(const QString& value);

	private:
		QJSEngine m_jsEngine;

		QString m_text = {"0.0"};
		QString m_placeholderText;
		int m_maxLength = 32;

		E::HorzAlign m_horzAlign = E::HorzAlign::AlignHCenter;
		E::VertAlign m_vertAlign = E::VertAlign::AlignVCenter;

		bool m_readOnly = false;

		QString m_scriptAfterCreate = PropertyNames::lineEditDefaultEventScript;
		QString m_scriptEditingFinished = PropertyNames::lineEditDefaultEventScript;
		QString m_scriptReturnPressed = PropertyNames::lineEditDefaultEventScript;
		QString m_scriptTextChanged = PropertyNames::lineEditDefaultEventScript;
	};
}
