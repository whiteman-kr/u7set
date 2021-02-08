#ifndef FILEPROPERTYMANAGER_H
#define FILEPROPERTYMANAGER_H

#include <QMap>
#include <QLineEdit>

#include "../qtpropertybrowser/src/qtpropertymanager.h"
#include "../qtpropertybrowser/src/qtvariantproperty.h"
#include "../qtpropertybrowser/src/qttreepropertybrowser.h"

// ==============================================================================================

class FolderEdit : public QWidget
{
	Q_OBJECT

public:

	explicit FolderEdit(QWidget* parent = nullptr);
	virtual ~FolderEdit();

public:

	QString				folderPath() const { return m_edit->text(); }
	void				setFolderPath(const QString &path) { if (m_edit->text() != path) m_edit->setText(path); }

private:

	QLineEdit*			m_edit;
	QToolButton*		m_button;

protected:

	void				focusInEvent(QFocusEvent* e);
	void				focusOutEvent(QFocusEvent* e);
	void				keyPressEvent(QKeyEvent* e);
	void				keyReleaseEvent(QKeyEvent* e);

signals:

	void				folderPathChanged(const QString &folderPath);

private slots:

	void				buttonClicked();
};

// ==============================================================================================

class VariantFactory : public QtVariantEditorFactory
{
	Q_OBJECT

public:

	explicit VariantFactory(QObject* parent = nullptr) : QtVariantEditorFactory(parent) {}
	virtual ~VariantFactory();

private:

	QMap<QtProperty*, QList<FolderEdit* > > m_createdEditorsMap;
	QMap<FolderEdit*, QtProperty* > m_editorToPropertyMap;

protected:

	virtual void		connectPropertyManager(QtVariantPropertyManager* manager);
	virtual void		disconnectPropertyManager(QtVariantPropertyManager* manager);

	virtual QWidget*	createEditor(QtVariantPropertyManager* manager, QtProperty* property, QWidget* parent);

private slots:

	void				slotPropertyChanged(QtProperty* property, const QVariant &value);

	void				slotSetValue(const QString &value);
	void				slotEditorDestroyed(QObject* object);
};

// ==============================================================================================

class VariantManager : public QtVariantPropertyManager
{
	Q_OBJECT

public:
	explicit VariantManager(QObject* parent = nullptr) : QtVariantPropertyManager(parent) {}
	virtual ~VariantManager() {}

public:

	virtual QVariant	value(const QtProperty* property) const;
	virtual int			valueType(int propertyType) const;
	virtual bool		isPropertyTypeSupported(int propertyType) const;

	static int			folerPathTypeId();

private:

	struct Data
	{
		QString value;
	};

	QMap<const QtProperty*, Data> m_valuesMap;

protected:

	virtual QString		valueText(const QtProperty* property) const;
	virtual void		initializeProperty(QtProperty* property);
	virtual void		uninitializeProperty(QtProperty* property);

public slots:

	virtual void		setValue(QtProperty* property, const QVariant &val);
};

// ==============================================================================================

#endif // FILEPROPERTYMANAGER_H
