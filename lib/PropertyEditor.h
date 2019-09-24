#ifndef PROPERTYEDITOR_H
#define PROPERTYEDITOR_H

#include "../lib/PropertyObject.h"

#include <QtTreePropertyBrowser>
#include <QtVariantPropertyManager>
#include <QtGroupPropertyManager>

class QtProperty;
class QPlainTextEdit;

namespace ExtWidgets
{
	//
	class PropertyEditor;
	class PropertyEditCellWidget;
	class PropertyTextEditor;

	class PropertyEditorBase
	{
	public:

		PropertyEditorBase();

		virtual PropertyEditor* createChildPropertyEditor(QWidget* parent);
		virtual PropertyTextEditor* createPropertyTextEditor(std::shared_ptr<Property> propertyPtr, QWidget* parent);

		bool expertMode() const;
		void setExpertMode(bool expertMode);

		bool isReadOnly() const;
		void setReadOnly(bool readOnly);

	public:
		//  Base Editor functions used by list and table editors
		//
		static QString propertyVectorText(QVariant& value);
		static QString stringListText(const QVariant& value);
		static QString propertyValueText(Property* p, int row);	// row is used for StringList

		static QIcon drawCheckBox(int state, bool enabled);
		static QIcon drawColorBox(QColor color);
		static QIcon drawImage(const QImage& image);
		static QIcon propertyIcon(Property* p, bool sameValue, bool enabled);

		PropertyEditCellWidget* createCellEditor(std::shared_ptr<Property> propertyPtr, bool sameValue, bool readOnly, QWidget* parent);
		PropertyEditCellWidget* createCellRowEditor(std::shared_ptr<Property> propertyPtr, int row, bool sameValue, bool readOnly, QWidget* parent);

		// Help description functions

		void setScriptHelp(QFile& file);

		void setScriptHelp(const QString& text);
		QString scriptHelp() const;

		QPoint scriptHelpWindowPos() const;
		void setScriptHelpWindowPos(const QPoint& value);

		QByteArray scriptHelpWindowGeometry() const;
		void setScriptHelpWindowGeometry(const QByteArray& value);

	private:
		bool m_expertMode = false;
		bool m_readOnly = false;

		QString m_scriptHelp;
		QPoint m_scriptHelpWindowPos = QPoint(-1, -1);
		QByteArray m_scriptHelpWindowGeometry;
	};

	//
	// -----------------------------------------  Editor dialogs ---------------------------------------
	//

	//
	// PropertyArrayEditorDialog
	//

	class PropertyArrayEditorDialog : public QDialog
	{
		Q_OBJECT

	public:
		PropertyArrayEditorDialog(PropertyEditorBase* propertyEditorBase, QWidget* parent, const QString& propertyName, const QVariant& value);
		~PropertyArrayEditorDialog();

		QVariant value();

	private slots:
		void onMoveUp();
		void onMoveDown();
		void onAdd();
		void onRemove();
		void onSelectionChanged();
		void onPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects);

	private:
		QString getObjectDescription(int objectIndex, PropertyObject* object);
		void updateDescriptions();
		void moveItems(bool forward);

	private:
		QVariant m_value;

		PropertyEditorBase* m_propertyEditorBase = nullptr;

		QTreeWidget* m_treeWidget = nullptr;
		PropertyEditor* m_childPropertyEditor = nullptr;

		QSplitter* m_splitter = nullptr;

		std::shared_ptr<Property> m_property;
	};

	//
	// StringListEditorDialog
	//

	class StringListEditorDialog : public QDialog
	{
		Q_OBJECT

	public:
		StringListEditorDialog(QWidget* parent, const QString& propertyName, const QVariant& value);
		~StringListEditorDialog();

		QVariant value();

	private slots:
		void onMoveUp();
		void onMoveDown();
		void onAdd();
		void onRemove();
		void itemChanged(QTreeWidgetItem *item, int column);

	private:
		void updateStrings();
		void moveItems(bool forward);

	private:
		QStringList m_strings;

		QTreeWidget* m_treeWidget = nullptr;

		std::shared_ptr<Property> m_property;
	};

	//
	// PropertyEditorHelp
	//

	class PropertyEditorHelpDialog : public QDialog
	{
	public:
		explicit PropertyEditorHelpDialog(const QString &caption, const QString& text, QWidget* parent);
		~PropertyEditorHelpDialog();

	};

	//
	// PropertyTextEditor
	//

	const int PropertyEditorTextMaxLength = 32767;

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

	//
	// PropertyPlainTextEditor
	//

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


	//
	// -----------------------------------------  Editor widgets ---------------------------------------
	//

	//
	// PropertyEditWidget - a base class for Multi*Edit classes
	//

	class PropertyEditCellWidget : public QWidget
	{
		Q_OBJECT

	public:
		PropertyEditCellWidget(QWidget* parent);
		~PropertyEditCellWidget();

		virtual void setValue(std::shared_ptr<Property> property, bool readOnly);

	signals:
		void valueChanged(QVariant value);

	};

	//
	// MultiFilePathEdit
	//

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

	class MultiFilePathEdit : public PropertyEditCellWidget
	{
		Q_OBJECT

	public:
		explicit MultiFilePathEdit(QWidget* parent, bool readOnly);
		void setValue(std::shared_ptr<Property> property, bool readOnly) override;

	public slots:
		void onEditingFinished();

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

	//
	// MultiEnumEdit
	//

	class MultiEnumEdit : public PropertyEditCellWidget
	{
		Q_OBJECT

	public:
		explicit MultiEnumEdit(QWidget* parent, std::shared_ptr<Property> p, bool readOnly);
		void setValue(std::shared_ptr<Property> property, bool readOnly) override;

	public slots:
		void indexChanged(int index);

	private:
		QComboBox* m_combo = nullptr;

	};

	//
	// MultiColorEdit
	//

	class MultiColorEdit : public PropertyEditCellWidget
	{
		Q_OBJECT

	public:
		explicit MultiColorEdit(QWidget* parent, bool readOnly);
		void setValue(std::shared_ptr<Property> property, bool readOnly) override;

	public slots:
		void onEditingFinished();

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

	//
	// MultiTextEditorDialog
	//

	class MultiTextEditorDialog : public QDialog
	{
		Q_OBJECT

	public:
		MultiTextEditorDialog(PropertyEditorBase* propertyEditorBase, QWidget* parent, const QString& text, std::shared_ptr<Property> p);
		QString text();

	private slots:
		void finished(int result);

	public slots:
		virtual void accept() override;
		virtual void reject() override;

	private:
		QString m_text;

		PropertyTextEditor* m_editor = nullptr;

		PropertyEditorBase* m_propertyEditorBase = nullptr;

		PropertyEditorHelpDialog* m_propertyEditorHelp = nullptr;

		std::shared_ptr<Property> m_property;

	};

	//
	// PropertyEditorCheckBox
	//

	class PropertyEditorCheckBox : public QCheckBox
	{
	public:
		PropertyEditorCheckBox(QWidget* parent) : QCheckBox(parent)
		{
		}

		bool hitOnButton(const QPoint& pos)
		{
			return hitButton(pos);
		}

	private:
		virtual void paintEvent(QPaintEvent *e) override;
	};


	//
	// MultiCheckBox
	//

	class MultiCheckBox : public PropertyEditCellWidget
	{
		Q_OBJECT

	public:
		explicit MultiCheckBox(QWidget* parent, bool readOnly);
		void setValue(std::shared_ptr<Property> propertyPtr, bool readOnly);

	public slots:
		void changeValueOnButtonClick();
		void onStateChanged(int state);

	private:
		void updateText();

	private:
		PropertyEditorCheckBox* m_checkBox = nullptr;

	};

	//
	// MultiTextEdit
	//


	class MultiTextEdit : public PropertyEditCellWidget
	{
		Q_OBJECT

	public:
		explicit MultiTextEdit(PropertyEditorBase* propertyEditorBase, std::shared_ptr<Property> p, bool readOnly, QWidget* parent);

		// Row parameter is used for QStringList property type. In this case, valueChanged signal returns QString type, NOT QStringList!
		//
		explicit MultiTextEdit(PropertyEditorBase* propertyEditorBase, std::shared_ptr<Property> p, int row, bool readOnly, QWidget* parent);

		void setValue(std::shared_ptr<Property> property, bool readOnly) override;

	public slots:
		void onTextEdited(const QString &text);
		void onEditingFinished();

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
		int m_row = -1;
		int m_userType = 0;

		PropertyEditorBase* m_propertyEditorBase = nullptr;
	};

	//
	// MultiArrayEdit
	//

	class MultiArrayEdit : public PropertyEditCellWidget
	{
		Q_OBJECT

	public:
		explicit MultiArrayEdit(PropertyEditorBase* propertyEditorBase, QWidget* parent, std::shared_ptr<Property> p, bool readOnly);
		void setValue(std::shared_ptr<Property> property, bool readOnly) override;

	private slots:
		void onButtonPressed();

	private:
		QLineEdit* m_lineEdit = nullptr;
		QToolButton* m_button = nullptr;

		QVariant m_currentValue;
		std::shared_ptr<Property> m_property;

		PropertyEditorBase* m_propertyEditorBase = nullptr;
	};

	//
	// MultiVariantPropertyManager
	//

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

	//
	// MultiVariantFactory
	//

	class MultiVariantFactory : public QtAbstractEditorFactory<MultiVariantPropertyManager>
	{
		Q_OBJECT

	public:
		explicit MultiVariantFactory(PropertyEditor* propertyEditor);

		void connectPropertyManager (MultiVariantPropertyManager* manager);
		QWidget* createEditor(MultiVariantPropertyManager* manager, QtProperty* property, QWidget* parent);
		void disconnectPropertyManager(MultiVariantPropertyManager* manager);

	public slots:

		void slotSetValue(QVariant value);		// sets value from argument

		void slotEditorDestroyed(QObject* object);

	private:
		MultiVariantPropertyManager* m_manager = nullptr;
		QtProperty* m_property = nullptr;

		PropertyEditor* m_propertyEditor = nullptr;

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

	struct PropertyEditorSettings
	{
		QByteArray m_arrayPropertyEditorSplitterState;
		QSize m_arrayPropertyEditorSize = QSize(-1, -1);

		QSize m_stringListEditorSize = QSize(-1, -1);

		QPoint m_multiLinePropertyEditorWindowPos;
		QByteArray m_multiLinePropertyEditorGeometry;

		// Ide Property Editor Options
		//
		double m_propertyEditorFontScaleFactor = 1.0;

		QPoint m_scriptHelpWindowPos;
		QByteArray m_scriptHelpWindowGeometry;

		void restore(QSettings& s);
		void store(QSettings& s);
	};

	//
	// PropertyEditor
	//

	class PropertyEditor : public QtTreePropertyBrowser, public PropertyEditorBase
	{
		friend class MultiTextEditorDialog;

		Q_OBJECT

	public:
		explicit PropertyEditor(QWidget* parent);

		// Public functions
		//
	public:
		void updatePropertyValues(const QString& propertyName);

		void setObjects(const std::vector<std::shared_ptr<PropertyObject>>& objects);
		void setObjects(const QList<std::shared_ptr<PropertyObject>>& objects);

		const QList<std::shared_ptr<PropertyObject>>& objects() const;

	protected:
		virtual void valueChanged(QString propertyName, QVariant value);

	protected slots:
		void updatePropertiesList();
		void updatePropertiesValues();

	private slots:
		void onValueChanged(QtProperty* property, QVariant value);
		void onShowErrorMessage (QString message);

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

	};
}

Q_DECLARE_METATYPE(std::shared_ptr<Property>)
Q_DECLARE_METATYPE(ExtWidgets::FilePathPropertyType)

extern ExtWidgets::PropertyEditorSettings thePropertyEditorSettings;

#endif // PROPERTYEDITOR_H
