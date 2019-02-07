#ifndef PROPERTYEDITOR_H
#define PROPERTYEDITOR_H

#include "../lib/PropertyObject.h"
#include "../qtpropertybrowser/src/qteditorfactory.h"
#include <memory>
#include <QWidget>
#include <QMap>
#include <QVariant>
#include <QSpinBox>
#include <QCheckBox>
#include <QtTreePropertyBrowser>
#include <QtVariantPropertyManager>
#include <QTextEdit>
#include <QDialog>
#include <QSet>
#include <QComboBox>
#include <QStringList>
#include <QPlainTextEdit>

class QtTreePropertyBrowser;
class QtProperty;

Q_DECLARE_METATYPE(std::shared_ptr<Property>)

namespace ExtWidgets
{
	struct FilePathPropertyType
	{
		FilePathPropertyType()
		{
			filter = "*.*";
		}

		QString filePath;
		QString filter;

		static int filePathTypeId();
	};
}

Q_DECLARE_METATYPE(ExtWidgets::FilePathPropertyType)

namespace ExtWidgets
{
	class PropertyEditor;

	const int PropertyEditorTextMaxLength = 32767;

	class PropertyEditorHelp : public QDialog
	{
	public:

		explicit PropertyEditorHelp(const QString &caption, const QString& text, QWidget* parent);
		~PropertyEditorHelp();

	};

	class PropertyTextEditor : public QWidget
	{
		Q_OBJECT

	public:
		PropertyTextEditor(QWidget* parent);

        virtual ~PropertyTextEditor();

		virtual void setText(const QString& text) = 0;

		virtual QString text() = 0;

		virtual void setReadOnly(bool value) = 0;

		void setValidator(const QString& validator);

		bool modified();

		bool hasOkCancelButtons();
	protected:
		void setHasOkCancelButtons(bool value);

	signals:
		void escapePressed();
		void okPressed();
		void cancelPressed();

	public slots:
		void textChanged();

	protected:
		void okButtonPressed();
		void cancelButtonPressed();

	protected:
		QRegExpValidator* m_regExpValidator = nullptr;

	private:
		bool m_modified = false;

		bool m_hasOkCancelButtons = true;
	};

	class PropertyPlainTextEditor : public PropertyTextEditor
	{
		Q_OBJECT

	public:
		PropertyPlainTextEditor(QWidget* parent);

		virtual void setText(const QString& text) override;

		virtual QString text() override;

		virtual void setReadOnly(bool value) override;

	protected:
		bool eventFilter(QObject* obj, QEvent* event);

	private slots:
		void onPlainTextContentsChange(int position, int charsRemoved, int charsAdded);

	private:
		QPlainTextEdit* m_plainTextEdit = nullptr;

		QString m_prevPlainText;

	};

	class MultiFilePathEdit : public QWidget
	{
		Q_OBJECT

	public:
		explicit MultiFilePathEdit(QWidget* parent, bool readOnly);
		void setValue(std::shared_ptr<Property> property, bool readOnly);

	public slots:
		void onEditingFinished();

	signals:
		void valueChanged(QVariant value);

	private slots:
		void onButtonPressed();

	private:
		bool eventFilter(QObject* watched, QEvent* event);

	private:
		QLineEdit* m_lineEdit = nullptr;
        QToolButton* m_button = nullptr;
		bool m_escape = false;
		QVariant m_oldPath;


	};

	class MultiEnumEdit : public QWidget
	{
		Q_OBJECT

	public:
		explicit MultiEnumEdit(QWidget* parent, std::shared_ptr<Property> p, bool readOnly);
		void setValue(std::shared_ptr<Property> property, bool readOnly);

	public slots:
		void indexChanged(int index);

	signals:
		void valueChanged(QVariant value);

		//private slots:
		//void onButtonPressed();

		//private:
		//bool eventFilter(QObject* watched, QEvent* event);

	private:
		QComboBox* m_combo = nullptr;
        int m_oldValue;

	};
	class MultiColorEdit : public QWidget
	{
		Q_OBJECT

	public:
		explicit MultiColorEdit(QWidget* parent, bool readOnly);
		void setValue(std::shared_ptr<Property> property, bool readOnly);

	public slots:
		void onEditingFinished();

	signals:
		void valueChanged(QVariant value);

	private slots:
		void onButtonPressed();

	private:
		bool eventFilter(QObject* watched, QEvent* event);
		QColor colorFromText(const QString& t);


	private:
		QLineEdit* m_lineEdit = nullptr;
        QToolButton* m_button = nullptr;
		bool m_escape = false;
		QColor m_oldColor;

	};


	class MultiTextEditorDialog : public QDialog
	{
		Q_OBJECT

	public:
		MultiTextEditorDialog(QWidget* parent, PropertyEditor* propertyEditor, const QString& text, std::shared_ptr<Property> p);
		QString text();

	private slots:
		void finished(int result);

	public slots:
		virtual void accept() override;
		virtual void reject() override;

	private:
		QString m_text;

		PropertyTextEditor* m_editor = nullptr;

		PropertyEditor* m_propertyEditor = nullptr;

		PropertyEditorHelp* m_propertyEditorHelp = nullptr;

		std::shared_ptr<Property> m_property;

	};


	class MultiCheckBox : public QWidget
	{
		Q_OBJECT

	public:
		explicit MultiCheckBox(QWidget* parent);
		void setValue(bool value, bool readOnly);

	public slots:
		void onStateChanged(int state);

	signals:
		void valueChanged(QVariant value);

	private:
		void updateText();

	private:
		QCheckBox* m_checkBox = nullptr;

	};


	class MultiTextEdit : public QWidget
	{
		Q_OBJECT

	public:
		explicit MultiTextEdit(QWidget* parent, std::shared_ptr<Property> p, bool readOnly, PropertyEditor* propertyEditor);
		void setValue(std::shared_ptr<Property> property, bool readOnly);

	public slots:
		void onTextEdited(const QString &text);
		void onEditingFinished();

	signals:
		void valueChanged(QVariant value);

	private slots:
		void onButtonPressed();

	private:
		bool eventFilter(QObject* watched, QEvent* event);

	private:
		QLineEdit* m_lineEdit = nullptr;
        QToolButton* m_button = nullptr;

		bool m_escape = false;
		QVariant m_oldValue;

		bool m_textEdited = false;

		std::shared_ptr<Property> m_property;
		int m_userType = 0;

		PropertyEditor* m_propertyEditor = nullptr;
	};



	class MultiVariantPropertyManager : public QtAbstractPropertyManager
	{
		Q_OBJECT

	public:
		explicit MultiVariantPropertyManager(QObject* parent);

		QVariant attribute(const QtProperty* property, const QString& attribute) const;
		bool hasAttribute(const QtProperty* property, const QString& attribute) const;

        std::shared_ptr<Property> value(const QtProperty* property) const;
		int valueType(const QtProperty* property) const;

        void setProperty(const QtProperty* property, std::shared_ptr<Property> propertyValue);
        bool sameValue(const QtProperty* property) const;

		QSet<QtProperty*> propertyByName(const QString& propertyName);

		void updateProperty(QtProperty* property);

		void emitSetValue(QtProperty* property, const QVariant& value);

	private:
		virtual QString displayText(const QtProperty* property) const;

	private:

        struct Data
		{
            std::shared_ptr<Property> p;
			QMap<QString, QVariant> attributes;
		};
        QMap<const QtProperty*, Data> values;

	public slots:
		void setAttribute (QtProperty* property, const QString& attribute, const QVariant& value);

	signals:
		void valueChanged(QtProperty* property, QVariant value);

	protected:
		void initializeProperty(QtProperty* property);
		void uninitializeProperty(QtProperty* property);
		QIcon valueIcon(const QtProperty* property) const;
		QString valueText(const QtProperty* property) const;

	};

	class MultiVariantFactory : public QtAbstractEditorFactory<MultiVariantPropertyManager>
	{
		Q_OBJECT

	public:
		explicit MultiVariantFactory(PropertyEditor* propertyEditor);

		void connectPropertyManager (MultiVariantPropertyManager* manager);
		QWidget* createEditor(MultiVariantPropertyManager* manager, QtProperty* property, QWidget* parent);
		void disconnectPropertyManager(MultiVariantPropertyManager* manager);

	public slots:
		//void slotPropertyChanged(QtProperty* property, QVariant value);

		void slotSetValue(QVariant value);		// sets value from argument
		void slotSetValueTimer();				// sets value m_valueSetOnTimer, needed for QTimer::singleShot

		void slotEditorDestroyed(QObject* object);

	private:
		MultiVariantPropertyManager* m_manager = nullptr;
		QtProperty* m_property = nullptr;

		PropertyEditor* m_propertyEditor = nullptr;

		QVariant m_valueSetOnTimer;
	};

	// -------------------------------------------------------------------------------

	struct CreatePropertyStruct
	{
		std::shared_ptr<Property> property;
		QString caption;
		QString description;
		QString category;
		bool sameValue = false;
		bool readOnly = false;

	};

	class PropertyEditor : public QtTreePropertyBrowser
	{
		friend class MultiTextEditorDialog;

		Q_OBJECT

	public:
		explicit PropertyEditor(QWidget* parent);

		virtual void saveSettings();

		// Public functions
		//
	public:
		void updatePropertyValues(const QString& propertyName);

		void setObjects(const std::vector<std::shared_ptr<PropertyObject>>& objects);
		void setObjects(const QList<std::shared_ptr<PropertyObject>>& objects);

		const QList<std::shared_ptr<PropertyObject>>& objects() const;

		void setExpertMode(bool expertMode);

		bool readOnly() const;
		void setReadOnly(bool readOnly);

		// Help description functions

		void setScriptHelp(const QString& text);
		QString scriptHelp() const;

		QPoint scriptHelpWindowPos() const;
		void setScriptHelpWindowPos(const QPoint& value);

		QByteArray scriptHelpWindowGeometry() const;
		void setScriptHelpWindowGeometry(const QByteArray& value);

	protected:
		virtual void valueChanged(QtProperty* property, QVariant value);

		virtual PropertyTextEditor* createPropertyTextEditor(Property *property, QWidget* parent);

	protected slots:
		void updatePropertiesList();
		void updatePropertiesValues();

	private slots:
		void onValueChanged(QtProperty* property, QVariant value);
		void onShowErrorMessage (QString message);
		void onCurrentItemChanged(QtBrowserItem* current);

	signals:
		void showErrorMessage(QString message);
        void propertiesChanged(QList<std::shared_ptr<PropertyObject>> objects);

	private:
		void fillProperties();
		void clearProperties();

		void createValuesMap(const QSet<QtProperty*>& props, QMap<QtProperty*, std::pair<QVariant, bool> > &values);
		QtProperty* createProperty(QtProperty* parentProperty, const QString& caption, const QString& category, const QString &description, const std::shared_ptr<Property> value, bool sameValue, bool readOnly);

		static bool createPropertyStructsSortFunc(const CreatePropertyStruct& cs1, const CreatePropertyStruct& cs2);

	private:
		// Private Data
		//
		QtGroupPropertyManager* m_propertyGroupManager = nullptr;

		MultiVariantPropertyManager* m_propertyVariantManager = nullptr;

		QList<std::shared_ptr<PropertyObject>> m_objects;

		bool m_expertMode = false;
		bool m_readOnly = false;

		QString m_scriptHelp;
		QPoint m_scriptHelpWindowPos;
		QByteArray m_scriptHelpWindowGeometry;

	};

}

#endif // PROPERTYEDITOR_H
