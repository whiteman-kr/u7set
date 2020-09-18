#ifndef PROPERTYEDITOR_H
#define PROPERTYEDITOR_H

#include <QItemDelegate>
#include "../lib/PropertyObject.h"

class QPlainTextEdit;

namespace ExtWidgets
{
	//
	class PropertyEditor;
	class PropertyEditCellWidget;
	class PropertyTextEditor;

	//
	// PropertyEditorSettings
	//

	struct PropertyEditorSettings
	{
		QByteArray m_arrayPropertyEditorSplitterState;
		QSize m_arrayPropertyEditorSize = QSize(-1, -1);

		QSize m_vectorEditorSize = QSize(-1, -1);

		QPoint m_multiLinePropertyEditorWindowPos;
		QByteArray m_multiLinePropertyEditorGeometry;

		double m_propertyEditorFontScaleFactor = 1.0;

		void restore(QSettings& s);
		void store(QSettings& s);
	};

	//
	// PropertyTools
	//

	class PropertyTools
	{
	public:
		static QString propertyVectorText(QVariant& value);
		static QString stringListText(const QVariant& value);
		static QString colorVectorText(QVariant& value);
		static QString propertyValueText(Property* p, int row);	// row is used for StringList

	};

	//
	// PropertyEditorBase
	//

	class PropertyEditorBase
	{
	public:

		PropertyEditorBase();
		virtual ~PropertyEditorBase();

		virtual PropertyEditor* createChildPropertyEditor(QWidget* parent);
		virtual PropertyTextEditor* createPropertyTextEditor(std::shared_ptr<Property> propertyPtr, QWidget* parent);

		bool expertMode() const;
		void setExpertMode(bool expertMode);

		bool isReadOnly() const;
		void setReadOnly(bool readOnly);

	public:
		//  Base Editor functions used by list and table editors
		//

		static QString colorToText(QColor color);
		static QColor colorFromText(const QString& t);

		static QIcon drawCheckBox(int state, bool enabled);
		static QIcon drawImage(const QImage& image);
		static QIcon propertyIcon(Property* p, bool sameValue, bool enabled);

		PropertyEditCellWidget* createCellEditor(std::shared_ptr<Property> propertyPtr, bool sameValue, bool readOnly, QWidget* parent);
		PropertyEditCellWidget* createCellRowEditor(std::shared_ptr<Property> propertyPtr, int row, bool sameValue, bool readOnly, QWidget* parent);

		// Help description functions

		void setScriptHelpFile(const QString& scriptHelpFile);
		QString scriptHelpFile() const;

	private:
		bool m_expertMode = false;
		bool m_readOnly = false;

		QString m_scriptHelpFile;
		QPoint m_scriptHelpWindowPos = QPoint(-1, -1);
		QByteArray m_scriptHelpWindowGeometry;

	public:
		static QString m_commonCategoryName;	// = "Common"
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
	// VectorEditorDialog
	// Supports:
	//	- QStringList
	//	- QVector<QColor>
	//

	class VectorEditorDialog : public QDialog
	{
		Q_OBJECT

	public:
		VectorEditorDialog(QWidget* parent, const QString& propertyName, const QVariant& value);
		~VectorEditorDialog();

		QVariant value();

	private slots:
		void onMoveUp();
		void onMoveDown();
		void onAdd();
		void onRemove();
		void itemChanged(QTreeWidgetItem *item, int column);

	private:
		void updateVectorData();
		void moveItems(bool forward);

		bool isValueStringList() const;
		bool isValueColorVector() const;

	private:

		int m_valueType = 0;

		QStringList m_strings;

		QVector<QColor> m_colors;

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
		virtual void setInitialText(const QString& text);

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

		void setInitialText(const QString& text) override;

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
	// PropertyEditorObject
	//

	class PropertyTreeWidget;

	enum class PropertyEditorColumns
	{
		Caption,
		Value
	};

	struct PropertyEditorObject
	{
		PropertyEditorObject() = default;

		PropertyEditorObject(std::shared_ptr<Property> property, bool sameValue, bool readOnly)
		{
			this->property = property;
			this->sameValue = sameValue;
			this->readOnly = readOnly;
		}

		void setTreeWidgetItem(QTreeWidgetItem* item)
		{
			this->item = item;
		}

		std::shared_ptr<Property> property;
		bool sameValue = true;
		bool readOnly = false;

		QTreeWidgetItem* item = nullptr;
	};

	//
	// PropertyEditorDelegate
	//

	class PropertyEditorDelegate : public QItemDelegate
	{
		Q_OBJECT
	public:
		explicit PropertyEditorDelegate(PropertyTreeWidget* treeWidget, PropertyEditor* propretyEditor);

	private:
		QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
		void destroyEditor(QWidget *editor, const QModelIndex &index) const override;
		void setEditorData(QWidget *editor, const QModelIndex &index) const override;
		void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
		void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

		QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
		void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

	private slots:
		void onValueChanged(QVariant value);

	signals:
		void valueChanged(QVariant value);

	private:
		mutable PropertyEditCellWidget *m_cellEditor = nullptr;
		PropertyEditor* m_propertyEditor = nullptr;
		PropertyTreeWidget* m_treeWidget = nullptr;
		mutable QModelIndex m_editIndex;
	};

	//
	// PropertyTreeWidget
	//

	class PropertyTreeWidget : public QTreeWidget
	{
		Q_OBJECT

	public:
		QString propertyCaption(const QModelIndex& mi);
		void closeCurrentEditorIfOpen();

	protected:
		virtual void mousePressEvent(QMouseEvent *event) override;
		virtual void keyPressEvent(QKeyEvent *event) override;
		virtual void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

	signals:
		void mousePressed();
		void editKeyPressed();
		void spaceKeyPressed();
	};

	//
	// PropertyEditor
	//

	class PropertyEditor : public QWidget, public PropertyEditorBase
	{
		Q_OBJECT

		friend class PropertyEditorDelegate;

	public:
		explicit PropertyEditor(QWidget* parent);

		// Objects operations
		//
		void clear();

		const QList<std::shared_ptr<PropertyObject>>& objects() const;
		void setObjects(const std::vector<std::shared_ptr<PropertyObject>>& objects);
		void setObjects(const QList<std::shared_ptr<PropertyObject>>& objects);

		// Properties and support
		//
		int splitterPosition() const;
		void setSplitterPosition(int pos);

		void autoAdjustSplitterPosition();

		bool isPropertyExists(const QString& propertyName) const;

	public slots:
		void updatePropertiesValues();
		void updatePropertyValue(const QString& propertyName);

	protected:
		virtual void valueChanged(QString propertyName, QVariant value);
		virtual void showEvent(QShowEvent* event) override;
		virtual void hideEvent(QHideEvent* event) override;

	protected slots:
		void updatePropertiesList();

	private slots:
		void onCellClicked();
		void onCellEditKeyPressed();
		void onCellToggleKeyPressed();
		void onCellEditorClosed(QWidget *editor, QAbstractItemDelegate::EndEditHint hint);
		void onShowErrorMessage (QString message);
		void onValueChanged(QVariant value);

	signals:
		void showErrorMessage(QString message);
		void propertiesChanged(QList<std::shared_ptr<PropertyObject>> objects);

	private:
		void fillProperties();
		void createProperty(const PropertyEditorObject& poe);
		std::shared_ptr<Property> propertyFromIndex(QModelIndex index) const;
		int getSelectionType();	// returns -1 if no type is selected or they are different
		void startEditing();
		void toggleSelected();

	private:
		// Private Data
		//
		PropertyTreeWidget* m_treeWidget = nullptr;
		std::map<QString, PropertyEditorObject> m_treeObjects;
		QList<std::shared_ptr<PropertyObject>> m_objects;
		PropertyEditorDelegate* m_itemDelegate = nullptr;
	};

	extern PropertyEditorSettings thePropertyEditorSettings;
}


Q_DECLARE_METATYPE(std::shared_ptr<Property>)
Q_DECLARE_METATYPE(ExtWidgets::FilePathPropertyType)
Q_DECLARE_METATYPE(QVector<QColor>)

#endif // PROPERTYEDITOR_H
