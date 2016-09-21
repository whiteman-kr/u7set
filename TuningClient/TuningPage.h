#ifndef TUNINGPAGE_H
#define TUNINGPAGE_H

#include "Stable.h"
#include "ObjectManager.h"
#include "ObjectFilter.h"


class TuningItemModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	TuningItemModel(int tuningPageIndex, QObject *parent);
public:


	void setObjectsIndexes(const std::vector<int> &objectsIndexes);

	std::vector<int> columnsIndexes();
	void setColumnsIndexes(std::vector<int> columnsIndexes);

	QStringList columnsNames();

	void update();

	int objectIndex(int index);

public:

	enum TuningPageColumns
	{
		SignalID = 0,
		EquipmentID,
		AppSignalID,
		Caption,
		Units,
		Type,

		Value,
		Valid,
		Underflow,
		Overflow,
	};


protected:
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &index) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;


private:
	std::vector<int> m_objectsIndexes;
	QStringList m_columnsNames;
	std::vector<int> m_columnsIndexes;

};

class FilterButton : public QPushButton
{
public:
	FilterButton(const QString& filterId, const QString& caption, QWidget* parent = nullptr);

private:
	QString m_filterId;
	QString m_caption;
};


class TuningPage : public QWidget
{
	Q_OBJECT
public:
	explicit TuningPage(int tuningPageIndex, ObjectFilter* tabFilter, QWidget *parent = 0);
	~TuningPage();

signals:

public slots:

private:

	QTableView* m_objectList = nullptr;

	std::vector<FilterButton*> m_buttons;

	QVBoxLayout* m_mainLayout = nullptr;

	QHBoxLayout* m_buttonsLayout = nullptr;

	QHBoxLayout* m_bottomLayout = nullptr;

	QPushButton* m_applyButton = nullptr;

	QPushButton* m_restoreButton = nullptr;

	QPushButton* m_maskButton = nullptr;

	QLineEdit* m_maskEdit = nullptr;

	QComboBox* m_maskTypeCombo = nullptr;

	TuningItemModel *m_model = nullptr;

	std::vector<int> m_objectsIndexes;

	ObjectFilter* m_tabFilter = nullptr;

	int m_tuningPageIndex = 0;

};

#endif // TUNINGPAGE_H
