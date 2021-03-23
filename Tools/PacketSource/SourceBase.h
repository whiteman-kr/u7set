#ifndef SOURCEBASE_H
#define SOURCEBASE_H

#include "BuildOption.h"

#include "SourceWorker.h"
#include "SignalBase.h"
#include "FrameBase.h"

#include "../../lib/HostAddressPort.h"

// ==============================================================================================

namespace PS
{
	//
	//

	#define PS_SUPPORT_VERSION		5

	const int SUPPORT_VERSION		= PS_SUPPORT_VERSION;		// last known version of Rup::VERSION

	const int SIM_FRAME_VERSION		= 1;						// last version of SimFrame

	const int UDP_PORT				= 10000;

	const int SEND_TIMEOUT			= 5;						// 5 ms

	//
	//

	struct SourceInfo
	{
	public:

		SourceInfo();

		void clear();

		int index = -1;

		QString caption;
		QString equipmentID;

		int moduleType = 0;
		QString subSystem;
		int frameCount = 0;
		quint32 dataID = 0;

		HostAddressPort lmIP;				// get from CfgSrv from xml file
		HostAddressPort appDataSrvIP;		// get from CfgSrv for send UDP (this is AppDataReceivingIP and AppDataReceivingPort of AppDataSrv)

		int signalCount = 0;
	};

	//
	//

	class Source : public QObject
	{
		Q_OBJECT

	public:

		Source();
		Source(const Source& from);
		Source(const PS::SourceInfo& si);
		virtual ~Source() override;

	public:

		void clear();

		//

		PS::SourceInfo& info() { return m_si; }
		QStringList& associatedSignalList()  { return m_associatedSignalList; }
		std::vector<PS::Signal>& signalList()  { return m_signalList; }
		FrameBase& frameBase() { return m_frameBase; }

		//
		//
		bool run();
		bool stop();

		bool isRunning();
		int sentFrames();

		//
		//
		void loadSignals(const SignalBase& signalBase);
		void initSignalsState();

		bool createWorker();
		void deleteWorker();

		//
		//
		Source& operator=(const Source& from);

	private:

		mutable QMutex m_sourceMutex;

		QThread* m_pThread = nullptr;
		SourceWorker* m_pWorker = nullptr;

		PS::SourceInfo m_si;
		QStringList m_associatedSignalList;
		std::vector<PS::Signal> m_signalList;
		FrameBase m_frameBase;
	};
}

// ==============================================================================================

class SourceBase : public QObject
{
	Q_OBJECT

public:

	explicit SourceBase(QObject *parent = nullptr);
	virtual ~SourceBase() override;

public:

	void clear();
	int count() const;

	int append(const PS::Source &source);
	void remove(int index);

	PS::Source source(int index) const;
	PS::Source* sourcePtr(int index);
	PS::Source* sourcePtr(const QString& equipmentID);

	void setSource(int index, const PS::Source& source);

	SourceBase& operator=(const SourceBase& from);

	// run stop send udp trhread
	//
	void runSourece(int index);
	void stopSourece(int index);

	void runAllSoureces();
	void stopAllSoureces();

	// source id for run
	//
	void runSources(const QStringList& sourceIDList);

private:

	mutable QMutex m_sourceMutex;
	std::vector<PS::Source> m_sourceList;
};

// ==============================================================================================

#endif // SOURCEBASE_H
