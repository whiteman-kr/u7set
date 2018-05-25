#include "SimOverrideWidget.h"
#include <QHBoxLayout>
#include <QSettings>
#include <QHeaderView>
#include <QDragEnterEvent>
#include "../../lib/AppSignal.h"

SimOverrideWidget::SimOverrideWidget(Sim::Simulator* simulator, QWidget* parent) :
	QWidget(parent),
	m_simulator(simulator)
{
	assert(m_simulator);

	m_signalList = new QTreeWidget(this);

	m_signalList->setRootIsDecorated(false);
	m_signalList->setUniformRowHeights(true);

	m_signalList->setColumnCount(static_cast<int>(Columns::ColumnCount));

	QStringList headerLabels;
	headerLabels << "SignalID";
	headerLabels << "Caption";
	headerLabels << "Value";

	m_signalList->setHeaderLabels(headerLabels);

	// --
	//
	QHBoxLayout* layout = new QHBoxLayout;

	layout->insertWidget(0, m_signalList);

	setLayout(layout);

	// --
	//
	QByteArray headerState = QSettings().value("SimulatorWidget/SimOverridenSignals/ListHeader").toByteArray();
	if (headerState.isEmpty() == false)
	{
		m_signalList->header()->restoreState(headerState);
	}

	// Darg and Drop fo rsignals
	//
	setAcceptDrops(true);

	return;
}

SimOverrideWidget::~SimOverrideWidget()
{
	QByteArray headerState = m_signalList->header()->saveState();
	QSettings().setValue("SimulatorWidget/SimOverridenSignals/ListHeader", headerState);

	return;
}

void SimOverrideWidget::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasFormat(AppSignalParamMimeType::value))
	{
		event->acceptProposedAction();
	}

	return;
}

void SimOverrideWidget::dropEvent(QDropEvent* event)
{
	if (event->mimeData()->hasFormat(AppSignalParamMimeType::value) == false)
	{
		assert(event->mimeData()->hasFormat(AppSignalParamMimeType::value) == true);
		event->setDropAction(Qt::DropAction::IgnoreAction);
		event->accept();
		return;
	}

	QByteArray data = event->mimeData()->data(AppSignalParamMimeType::value);

	::Proto::AppSignalSet protoSetMessage;
	bool ok = protoSetMessage.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		event->acceptProposedAction();
		return;
	}

	// Parse data
	//
	QStringList signalIds;

	for (int i = 0; i < protoSetMessage.appsignal_size(); i++)
	{
		const ::Proto::AppSignal& appSignalMessage = protoSetMessage.appsignal(i);

		AppSignalParam appSignalParam;
		ok = appSignalParam.load(appSignalMessage);

		if (ok == true)
		{
			signalIds << appSignalParam.appSignalId();
		}
	}

	if (signalIds.isEmpty() == false)
	{
		m_simulator->overrideSignals().addSignals(signalIds);
	}

	return;
}
