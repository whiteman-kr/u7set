#pragma once

#include "SimBasePage.h"
#include <Simulator/SimLogicModule.h>

#include <QAbstractTableModel>
#include <QTreeView>


namespace SimUi
{
	class SimIdeSimulator;

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
		SimCodeModel(SimIdeSimulator* simulator, QString m_lmEquipmentId, QObject* parent = nullptr);

	public:
		virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
		virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

		virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
		virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override final;

		// Data manipulation
		//
	public slots:
		void dataChanged();

	private:
		std::optional<Sim::LogicModule> logicModule();
		std::optional<Sim::LogicModule> logicModule() const;

		// Data
		//
	private:
		SimIdeSimulator* m_simulator = nullptr;
		QString m_lmEquipmentId;

		std::vector<Sim::DeviceCommand> m_commands;
		std::unordered_map<int, size_t>
			m_offsetToCommand; // key: command offset, value: index in m_commands, can be changed to std::vector, memory is not large
	};


	class SimCodeView : public QTreeView
	{
		Q_OBJECT

	public:
		explicit SimCodeView(QWidget* parent = nullptr);
	};

	class SimCodePage : public SimBasePage
	{
		Q_OBJECT

	public:
		SimCodePage(SimIdeSimulator* simulator, QString lmEquipmentId, QWidget* parent = nullptr);

	public:
		QString equipmentId() const;

	private:
		std::optional<Sim::LogicModule> logicModule();
		std::optional<Sim::LogicModule> logicModule() const;

	private:
		QString m_lmEquipmentId;

		SimCodeModel* m_model = nullptr;
		SimCodeView* m_view = nullptr;
	};
} // namespace SimUi
