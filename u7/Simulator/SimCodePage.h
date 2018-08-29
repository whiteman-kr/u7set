#ifndef SIMCODEPAGE_H
#define SIMCODEPAGE_H

#include <memory>
#include <QAbstractTableModel>
#include <QTreeView>
#include "SimBasePage.h"
#include "SimLogicModule.h"


enum class CodePageColumns
{
	Row = 0,
	Address,
	Code,

	ColumnCount
};


class SimCodeModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	SimCodeModel(SimIdeSimulator* simulator,
				 QString m_lmEquipmentId,
				 QObject* parent = nullptr);

public:
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const final;

	// Data manipultaion
	//
public slots:
	void dataChanged();

private:
	std::shared_ptr<Sim::LogicModule> logicModule();
	std::shared_ptr<Sim::LogicModule> logicModule() const;

	// Data
	//
private:
	SimIdeSimulator* m_simulator = nullptr;
	QString m_lmEquipmentId;

	std::vector<Sim::DeviceCommand> m_commands;
	std::unordered_map<int, size_t> m_offsetToCommand;		// key: command offset, value: index in m_commands, can be changed to std::vector, memory is not large
};


class SimCodeView : public QTreeView
{
	Q_OBJECT

public:
	explicit SimCodeView(QWidget* parent = nullptr);
	virtual ~SimCodeView();

protected:
	//virtual void contextMenuEvent(QContextMenuEvent* event);

protected slots:
//	void headerColumnContextMenuRequested(const QPoint& pos);
//	void headerColumnToggled(bool checked);

	//void copySelection();

signals:
//	void removeAppSignal(QString appSignalId);

//	void requestToShowSignalInfo(QString appSignalId);
//	void requestToRemoveSignal(QString appSignalId);
//	void requestToCopySelection();
//	void requestToSetSignals();

	// Data
	//
private:
	//QAction* copyAction = nullptr;
};

class SimCodePage : public SimBasePage
{
	Q_OBJECT

public:
	SimCodePage(SimIdeSimulator* simulator,
				QString lmEquipmentId,
				QWidget* parent = nullptr);

public:
	QString equipmnetId() const;

private:
	std::shared_ptr<Sim::LogicModule> logicModule();
	std::shared_ptr<Sim::LogicModule> logicModule() const;

private:
	QString m_lmEquipmentId;

	SimCodeModel* m_model = nullptr;
	SimCodeView* m_view = nullptr;
};

#endif // SIMCODEPAGE_H
