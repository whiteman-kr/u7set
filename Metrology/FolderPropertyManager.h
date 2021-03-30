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
	virtual ~FolderEdit() override;

public:

	QString				folderPath() const { return m_edit->text(); }
	void				setFolderPath(const QString &path) { if (m_edit->text() != path) m_edit->setText(path); }

private:

	QLineEdit*			m_edit;
	QToolButton*		m_button;

protected:

	void				focusInEvent(QFocusEvent* e) override;
	void				focusOutEvent(QFocusEvent* e) override;
	void				keyPressEvent(QKeyEvent* e) override;
	void				keyReleaseEvent(QKeyEvent* e) override;

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
	virtual ~VariantFactory() override;

private:

	QMap<QtProperty*, QList<FolderEdit* > > m_createdEditorsMap;
	QMap<FolderEdit*, QtProperty* > m_editorToPropertyMap;

protected:

	virtual void		connectPropertyManager(QtVariantPropertyManager* manager) override;
	virtual void		disconnectPropertyManager(QtVariantPropertyManager* manager) override;

	virtual QWidget*	createEditor(QtVariantPropertyManager* manager, QtProperty* property, QWidget* parent) override;

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
	virtual ~VariantManager() override {}

public:

	virtual QVariant	value(const QtProperty* property) const override;
	virtual int			valueType(int propertyType) const override;
	virtual bool		isPropertyTypeSupported(int propertyType) const override;

	static int			folerPathTypeId();

private:

	struct Data
	{
		QString value;
	};

	QMap<const QtProperty*, Data> m_valuesMap;

protected:

	virtual QString		valueText(const QtProperty* property) const override;
	virtual void		initializeProperty(QtProperty* property) override;
	virtual void		uninitializeProperty(QtProperty* property) override;

public slots:

	virtual void		setValue(QtProperty* property, const QVariant &val) override;
};

// ==============================================================================================

#endif // FILEPROPERTYMANAGER_H
