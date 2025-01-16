#pragma once

#include "ArchiveData.h"

enum class ArchiveColumns
{
	Row = 0,
	AppSignalId,
	CustomSignalId,
	Caption,
	State,

	// Next are hidden by default in MonitorArchiveWidget::MonitorArchiveWidget!!!, cannot do it in ArchiveView constructor, don't know why(((
	//
	Valid,
	StateAvailable,
	Simulated,
	Blocked,
	Mismatch,
	OutOfLimits,
	ArchivingReason,
	Duration,

	// Next are visible by default
	//
	Time,
	Server,

	ColumnCount
};

Q_DECLARE_METATYPE(ArchiveColumns);

struct ArchiveSignalParam : public ArchiveSignal
{
	ArchiveSignalParam() = default;

	explicit ArchiveSignalParam(const ArchiveSignal& archiveSignal) :
		ArchiveSignal(archiveSignal),
		precision(archiveSignal.signalParam.precision()),
		analogAppSignalParam(archiveSignal.signalParam.analogSignalFormat())
	{
	}

	E::ValueViewType viewType = E::ValueViewType::Dec;
	int precision = 2;
	E::AnalogAppSignalFormat analogAppSignalParam = E::AnalogAppSignalFormat::SignedInt32;
};

//
//
//		ArchiveModel
//
//
class ArchiveModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	explicit ArchiveModel(QObject* parent = nullptr);

public:
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QVariant data(int row, int column, int role) const;

private:
	QString getValueString(const AppSignalState& state, const ArchiveSignalParam& signalParam) const;
	void updateCachedState(int row) const;

	// Data manipultaion
	//
public:
	void setParams(const std::vector<ArchiveSignal>& archiveSignals, E::TimeType timeType);
	void addData(ArchiveRequestResult&& chunk);

	void clear();
	void removeSignal(QString appSignalId, QString archiveServiceId);

	std::vector<ArchiveSignal> archiveSignals();
	std::vector<ArchiveSignalParam> appSignals();

	const ArchiveSignalParam& signalParam(int row) const;

	bool setShowParams(Hash signalHash, E::ValueViewType viewType, int precision);

	// Data
	//
private:
	std::map<Hash, ArchiveSignalParam> m_archiveSignalsMap;
	std::vector<ArchiveSignal> m_archiveSignalsVector;

	E::TimeType m_timeType = E::TimeType::Local;
	ArchiveData m_archive;

	mutable int m_cachedStateIndex = -1;
	mutable ArchiveSignalState m_cachedSignalState;

	inline static const QString NonValidString{"LowLimit"};
	inline static const ArchiveSignalParam InvalidSignalParam{};
};


//
//
//		ArchiveView
//
//
class ArchiveView : public QTableView
{
	Q_OBJECT

public:
	explicit ArchiveView(QWidget* parent = nullptr);
	virtual ~ArchiveView();

protected:
	virtual void contextMenuEvent(QContextMenuEvent* event);

protected slots:
	void headerColumnContextMenuRequested(const QPoint& pos);
	void headerColumnToggled(bool checked);

	void copySelection();

signals:
	void removeAppSignal(QString appSignalId);

	void requestToShowSignalInfo(QString appSignalId);
	void requestToRemoveSignal(QString appSignalId, QString archiveServiceId);
	void requestToCopySelection();
	void requestToSetSignals();

	// Data
	//
private:
	QMenu m_columnMenu;

	QAction* copyAction = nullptr;
};

