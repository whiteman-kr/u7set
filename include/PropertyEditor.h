#ifndef PROPERTYEDITOR_H
#define PROPERTYEDITOR_H

#include "../include/PropertyObject.h"
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
	class QtMultiFilePathEdit : public QWidget
	{
		Q_OBJECT

	public:
		explicit QtMultiFilePathEdit(QWidget* parent);
        void setValue(std::shared_ptr<Property> property);

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

	class QtMultiEnumEdit : public QWidget
	{
		Q_OBJECT

	public:
		explicit QtMultiEnumEdit(QWidget* parent);
        void setValue(std::shared_ptr<Property> property);

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
	class QtMultiColorEdit : public QWidget
	{
		Q_OBJECT

	public:
		explicit QtMultiColorEdit(QWidget* parent);
        void setValue(std::shared_ptr<Property> property);

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


	class MultiLineEdit : public QDialog
	{
		Q_OBJECT

	public:
		MultiLineEdit(QWidget* parent, const QString& text, const QString& caption);
		QString text();

	private slots:
		void finished(int result);

	private:
		QString m_text;
		QTextEdit* m_textEdit = nullptr;

		virtual void accept();
	};


	class QtMultiCheckBox : public QWidget
	{
		Q_OBJECT

	public:
		explicit QtMultiCheckBox(QWidget* parent);
        void setValue(std::shared_ptr<Property> property, bool sameValue);

	public slots:
		void onStateChanged(int state);

	signals:
		void valueChanged(QVariant value);

	private:
		void updateText();

	private:
		QCheckBox* m_checkBox = nullptr;

	};


	class QtMultiTextEdit : public QWidget
	{
		Q_OBJECT

	public:
		explicit QtMultiTextEdit(QWidget* parent, int userType, const QString& caption);
        void setValue(std::shared_ptr<Property> property);

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
		QVariant m_oldValue;
		int m_userType = 0;
		QString m_caption;
	};


	/*
	class QtMultiDoubleSpinBox : public QWidget
	{
		Q_OBJECT

	public:
		explicit QtMultiDoubleSpinBox(QWidget* parent);
        void setValue(std::shared_ptr<Property> property);

	public slots:
		void onValueChanged(double value);

	signals:
		void valueChanged(QVariant value);

	private:
		bool eventFilter(QObject* watched, QEvent* event);

	private:
		QDoubleSpinBox* m_spinBox = nullptr;
		bool m_escape = false;
	};


	class QtMultiIntSpinBox : public QWidget
	{
		Q_OBJECT
	public:
		explicit QtMultiIntSpinBox(QWidget* parent);
        void setValue(std::shared_ptr<Property> property);

	public slots:
		void onValueChanged(int value);

	signals:
		void valueChanged(QVariant value);

	private:
		bool eventFilter(QObject* watched, QEvent* event);

	private:
		QSpinBox* m_spinBox = nullptr;
		bool m_escape = false;
	};


	class QtMultiUIntSpinBox : public QWidget
	{
		Q_OBJECT
	public:
		explicit QtMultiUIntSpinBox(QWidget* parent);
        void setValue(std::shared_ptr<Property> property);

	public slots:
		void onValueChanged(quint32 value);

	signals:
		void valueChanged(QVariant value);

	private:
		bool eventFilter(QObject* watched, QEvent* event);

	private:
		QSpinBox* m_spinBox = nullptr;
		bool m_escape = false;
	};*/


	class QtMultiVariantPropertyManager : public QtAbstractPropertyManager
	{
		Q_OBJECT

	public:
		explicit QtMultiVariantPropertyManager(QObject* parent);

		QVariant attribute(const QtProperty* property, const QString& attribute) const;
		bool hasAttribute(const QtProperty* property, const QString& attribute) const;

        std::shared_ptr<Property> value(const QtProperty* property) const;
		int valueType(const QtProperty* property) const;

        void setProperty(const QtProperty* property, std::shared_ptr<Property> propertyValue);
        bool sameValue(const QtProperty* property) const;

		QSet<QtProperty*> propertyByName(const QString& propertyName);

		void emitSetValue(QtProperty* property, const QVariant& value);

	private:
		virtual QString displayText(const QtProperty *property) const;

	private:

        struct Data
		{
            std::shared_ptr<Property> p;
			QMap<QString, QVariant> attributes;
		};
        QMap<const QtProperty*, Data> values;

	public slots:
        void setValue(QtProperty* property, const QVariant& value);
		void setAttribute (QtProperty* property, const QString& attribute, const QVariant& value);

	signals:
		void valueChanged(QtProperty* property, QVariant value);

	protected:
		void initializeProperty(QtProperty* property);
		void uninitializeProperty(QtProperty* property);
		QIcon valueIcon(const QtProperty* property) const;
		QString valueText(const QtProperty* property) const;

	};

	class QtMultiVariantFactory : public QtAbstractEditorFactory<QtMultiVariantPropertyManager>
	{
		Q_OBJECT

	public:
		explicit QtMultiVariantFactory(QObject* parent);

		void connectPropertyManager (QtMultiVariantPropertyManager* manager);
		QWidget* createEditor(QtMultiVariantPropertyManager* manager, QtProperty* property, QWidget* parent);
		void disconnectPropertyManager(QtMultiVariantPropertyManager* manager);

	public slots:
		//void slotPropertyChanged(QtProperty* property, QVariant value);
		void slotSetValue(QVariant value);
		void slotEditorDestroyed(QObject* object);

	private:
		QtMultiVariantPropertyManager* m_manager = nullptr;
		QtProperty* m_property = nullptr;
	};

	// -------------------------------------------------------------------------------

	class PropertyEditor : public QtTreePropertyBrowser
	{
		Q_OBJECT

	public:
		explicit PropertyEditor(QWidget* parent);

		// Public functions
		//
	public:
		void updateProperty(const QString& propertyName);

        void setObjects(QList<std::shared_ptr<PropertyObject>>& objects);
		void clearProperties();

		void setExpertMode(bool expertMode);

	protected:
		virtual void valueChanged(QtProperty* property, QVariant value);

	protected slots:
		void updateProperties();

	private slots:
		void onValueChanged(QtProperty* property, QVariant value);
		void onShowErrorMessage (QString message);
		void onCurrentItemChanged(QtBrowserItem* current);

	signals:
		void showErrorMessage(QString message);
        void propertiesChanged(QList<std::shared_ptr<PropertyObject>> objects);

		// Data
		//
	protected:
        QtGroupPropertyManager* m_propertyGroupManager = nullptr;

        QtMultiVariantPropertyManager* m_propertyVariantManager = nullptr;

        QList<std::shared_ptr<PropertyObject>> m_objects;

		// Private Data
		//
	private:
        void createValuesMap(const QSet<QtProperty*>& props, QMap<QtProperty *, std::pair<QVariant, bool> > &values);
        QtProperty* createProperty(QtProperty *parentProperty, const QString& caption, const QString& category, const QString &description, const std::shared_ptr<Property> value, bool sameValue);

	private:
		bool m_expertMode = false;
	};

}

#endif // PROPERTYEDITOR_H
