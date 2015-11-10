#ifndef SIGNALTABLEMODEL_H
#define SIGNALTABLEMODEL_H

#include <QAbstractTableModel>

class SignalSet;
class DataSource;
class Signal;

class SignalTableModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	SignalTableModel(quint8* buffer, const SignalSet& signalSet, QObject* parent = 0);
	virtual ~SignalTableModel();

	int rowCount(const QModelIndex &parent = QModelIndex()) const ;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	void updateFrame(int frameNo);
	void addDataSource(const DataSource& dataSource);

signals:

public slots:
	void setNeedToSwapBytes(bool value);
	void updateData();

private:
	quint16* m_buffer;
	const SignalSet& m_signalSet;
	QVector<int> m_relatedSignalIndexes;
	std::vector<std::pair<int, int>> m_frameSignalIndexLimits;
	bool needToSwapBytes;

	template <typename TYPE>
	TYPE getAdc(const Signal& signal) const;
};

#endif // SIGNALTABLEMODEL_H
