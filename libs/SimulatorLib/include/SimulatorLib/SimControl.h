#pragma once

#include "SimControlStatus.h"

namespace Sim
{
	class ControlImpl;

	//
	// Sim::Control
	//
	class Control : public QObject
	{
		Q_OBJECT

	private:
		friend class SimulatorPrivate;
		explicit Control(ControlImpl& impl, QObject* parent = nullptr);

	public:
		void stopThread();
		void reset();

		int setRunList(QStringList equipmentIds);

		bool startSimulation(std::chrono::microseconds duration = std::chrono::microseconds{-1});
		void pause();
		void stop();

		ControlStatus status() const;

		SimControlState state() const;
		bool isRunning() const;

		std::chrono::microseconds duration() const;
		std::chrono::microseconds leftTime() const;

		double speedFactor() const;
		void setSpeedFactor(double value);

	signals:
		void stateChanged(SimControlState state);
		void statusUpdate(ControlStatus state);

	private:
		ControlImpl& m_impl;
	};

} // namespace Sim
