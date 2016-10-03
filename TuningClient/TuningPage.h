#ifndef TUNINGPAGE_H
#define TUNINGPAGE_H

#include "Stable.h"
#include "ObjectManager.h"
#include "TuningFilter.h"


class TuningItemModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	TuningItemModel(QObject *parent);
	~TuningItemModel();

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

public:
	void setObjectsIndexes(const std::vector<int> &objectsIndexes);

	std::vector<int> columnsIndexes();
	void setColumnsIndexes(std::vector<int> columnsIndexes);

	//QStringList columnsNames();

	//void update();

	int objectIndex(int index);

	void setFont(const QString& fontName, int fontSize, bool fontBold);

	void addColumn(TuningPageColumns column);

protected:
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &index) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	virtual QBrush backColor(const QModelIndex& index) const;
	virtual QBrush foregroundColor(const QModelIndex& index) const;

private:
	std::vector<int> m_objectsIndexes;
	QStringList m_columnsNames;

	QFont* m_font = nullptr;

protected:
	std::vector<int> m_columnsIndexes;

};

class TuningItemModelMain : public TuningItemModel
{
public:
	TuningItemModelMain(int tuningPageIndex, QObject* parent);

protected:
	virtual QBrush backColor(const QModelIndex& index) const override;
	virtual QBrush foregroundColor(const QModelIndex& index) const override;

};

class FilterButton : public QPushButton
{
	Q_OBJECT
public:
	FilterButton(std::shared_ptr<TuningFilter> filter, const QString& caption, QWidget* parent = nullptr);

	std::shared_ptr<TuningFilter> filter();

private:
	std::shared_ptr<TuningFilter> m_filter;
	QString m_caption;

private slots:
	void slot_toggled(bool checked);

signals:
	void filterButtonClicked(std::shared_ptr<TuningFilter> filter);
};


class TuningPage : public QWidget
{
	Q_OBJECT
public:
	explicit TuningPage(int tuningPageIndex, std::shared_ptr<TuningFilter> tabFilter, QWidget *parent = 0);
	~TuningPage();

	void fillObjectsList();

signals:

private slots:
	void slot_filterButtonClicked(std::shared_ptr<TuningFilter> filter);

public slots:
	void slot_filterTreeChanged(std::shared_ptr<TuningFilter> filter);

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

	TuningItemModelMain *m_model = nullptr;

	std::vector<int> m_objectsIndexes;

	std::shared_ptr<TuningFilter> m_treeFilter = nullptr;

	std::shared_ptr<TuningFilter> m_tabFilter = nullptr;

	std::shared_ptr<TuningFilter> m_buttonFilter = nullptr;

	int m_tuningPageIndex = 0;

};

#endif // TUNINGPAGE_H
