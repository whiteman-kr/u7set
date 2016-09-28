#ifndef TUNINGPAGE_H
#define TUNINGPAGE_H

#include "Stable.h"
#include "ObjectManager.h"
#include "ObjectFilter.h"


class TuningItemModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	void Init();
	TuningItemModel(QObject *parent);
	TuningItemModel(int tuningPageIndex, QObject *parent);
	~TuningItemModel();
public:

	void setObjectsIndexes(const std::vector<int> &objectsIndexes);

	std::vector<int> columnsIndexes();
	void setColumnsIndexes(std::vector<int> columnsIndexes);

	QStringList columnsNames();

	void update();

	int objectIndex(int index);

	void setFont(const QString& fontName, int fontSize, bool fontBold);

public:

	enum TuningPageColumns
	{
		CustomAppSignalID = 0,
		EquipmentID,
		AppSignalID,
		Caption,
		Units,
		Type,

		Value,
		Default,
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

	QFont* m_font = nullptr;

};

class FilterButton : public QPushButton
{
	Q_OBJECT
public:
	FilterButton(std::shared_ptr<ObjectFilter> filter, const QString& caption, QWidget* parent = nullptr);

	std::shared_ptr<ObjectFilter> filter();

private:
	std::shared_ptr<ObjectFilter> m_filter;
	QString m_caption;

private slots:
	void slot_toggled(bool checked);

signals:
	void filterButtonClicked(std::shared_ptr<ObjectFilter> filter);
};


class TuningPage : public QWidget
{
	Q_OBJECT
public:
	explicit TuningPage(int tuningPageIndex, std::shared_ptr<ObjectFilter> tabFilter, QWidget *parent = 0);
	~TuningPage();

	void fillObjectsList();

signals:

private slots:
	void slot_filterButtonClicked(std::shared_ptr<ObjectFilter> filter);

public slots:
	void slot_filterTreeChanged(std::shared_ptr<ObjectFilter> filter);

private:

	QTableView* m_objectList = nullptr;

	QButtonGroup *m_filterButtonGroup = nullptr;

	QVBoxLayout* m_mainLayout = nullptr;

	QHBoxLayout* m_buttonsLayout = nullptr;

	QHBoxLayout* m_bottomLayout = nullptr;

	QPushButton* m_setValueButton = nullptr;

	QPushButton* m_setOnButton = nullptr;

	QPushButton* m_setOffButton = nullptr;

	QPushButton* m_setToDefaultButton = nullptr;

	QPushButton* m_maskButton = nullptr;

	QLineEdit* m_maskEdit = nullptr;

	QComboBox* m_maskTypeCombo = nullptr;

	TuningItemModel *m_model = nullptr;

	std::vector<int> m_objectsIndexes;

	std::shared_ptr<ObjectFilter> m_treeFilter = nullptr;

	std::shared_ptr<ObjectFilter> m_tabFilter = nullptr;

	std::shared_ptr<ObjectFilter> m_buttonFilter = nullptr;

	int m_tuningPageIndex = 0;

};

#endif // TUNINGPAGE_H
