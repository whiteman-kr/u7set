#pragma once

#include "SchemaItemControl.h"

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT SchemaItemPushButton : public SchemaItemControl
	{
		Q_OBJECT

	public:
		SchemaItemPushButton(void);
		explicit SchemaItemPushButton(SchemaUnit unit);
		virtual ~SchemaItemPushButton(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Methods
	public:
		virtual QWidget* createWidget(QWidget* parent, double zoom) const override;
		virtual void updateWidgetProperties(QWidget* widget, double zoom) const override;

	protected slots:
		void clicked(bool checked);
		void pressed();
		void released();
		void toggled(bool checked);

		// Text search
		//
	public:
		//virtual bool searchText(const QString& text) const override;

		// Properties and Data
		//
	public:
		const QString& text() const;
		void setText(QString value);

		bool isCheckable() const;
		void setCheckable(bool value);

		bool checkedDefault() const;
		void setCheckedDefault(bool value);

		bool autoRepeat() const;
		void setAutoRepeat(bool value);

		int autoRepeatDelay() const;
		void setAutoRepeatDelay(int value);

		int autoRepeatInterval() const;
		void setAutoRepeatInterval(int value);

		const QString& styleSheet() const;
		void setStyleSheet(QString value);

		QString scriptAfterCreate() const;
		void setScriptAfterCreate(const QString& value);

		QString scriptClicked() const;
		void setScriptClicked(const QString& value);

		QString scriptPressed() const;
		void setScriptPressed(const QString& value);

		QString scriptReleased() const;
		void setScriptReleased(const QString& value);

		QString scriptToggled() const;
		void setScriptToggled(const QString& value);

	private:
		QString m_text = {"Button"};

		bool m_checkable = false;
		bool m_checkedDefault = false;

		bool m_autoRepeat = false;
		int m_autoRepeatDelay = 300;
		int m_autoRepeatInterval = 100;

		QString m_styleSheet = PropertyNames::defaultPushButtonStyleSheet;

		QString m_scriptAfterCreate = PropertyNames::defaultPushButtonAfterCreateScript;
		QString m_scriptClicked = PropertyNames::defaultPushButtonEventScript;
		QString m_scriptPressed = PropertyNames::defaultPushButtonEventScript;
		QString m_scriptReleased = PropertyNames::defaultPushButtonEventScript;
		QString m_scriptToggled = PropertyNames::defaultPushButtonEventScript;
	};
}
