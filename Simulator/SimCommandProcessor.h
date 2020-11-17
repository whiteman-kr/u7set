#pragma once
#include <map>
#include <functional>
#include "QtCore"
#include "SimDeviceEmulator.h"
#include "SimException.h"

class SimCommandTest_LM5_LM6;

namespace Sim
{
	// Helper union to cast <pointer to member> to <raw data>
	//
	template<typename classT, typename memberT>
	union u_ptm_cast
	{
		memberT classT::*pmember;
		std::array<std::byte, 16> pvoid;
	};


	class CommandProcessor : public QObject
	{
		Q_OBJECT

		friend SimCommandTest_LM5_LM6;

	protected:
		explicit CommandProcessor(DeviceEmulator* device);

	public:
		virtual ~CommandProcessor();
		static CommandProcessor* createInstance(DeviceEmulator* device);

	public:
		// Parse LM command, result written to deviceCommand, can throw SimException
		//
		bool parseFunc(QString parseFunc, DeviceCommand* command);

		virtual void beforeAppLogicParse();
		virtual void afterAppLogicParse(std::vector<DeviceCommand>* commands);

		// Update platform interface, this function is called before work cyle,
		// to update such platform inteface signals as Blink.
		// Update mustb be done directly in RAM
		//
		virtual bool updatePlatformInterfaceState(const QDateTime& currentTime);

		// Run simulation LM command, can throw SimException
		//
		virtual bool runCommand(const DeviceCommand& command);

	protected:
		// Check that such AFB does exist and it has pin  (optional)
		// Raise SimException if fail
		//
		AfbComponent checkAfb(int opCode, int instanceNo, int pinOpCode = -1) const;

		// Check if data in specific range
		// Raise SimException if fail
		//
		void checkParamRange(int paramValue, int minValue, int maxValue, const QString& param) const;

		// Check if data in specific range
		// Raise SimException if fail
		//
		void checkParamExists(const AfbComponentInstance* afbInstance, int paramOpIndex, const QString& paramName) const;

		// Memory sanitizer, checks for reading uninitialized memry
		//
		void sanitizerWrite(quint32 address, quint32 wordCount);
		void sanitizerCheck(quint32 address, quint32 wordCount) const;

		// Generation command string functions (helpers)
		//
		QString strCommand(QString command) const;
		QString strAfbInst(const DeviceCommand* command) const;
		QString strAfbInstPin(const DeviceCommand* command) const;

		QString strAddr(quint16 address) const;
		QString strBitAddr(quint16 address, quint16 bitNo) const;

		QString strBitConst(quint16 data) const;
		QString strWordConst(quint16 data) const;
		QString strDwordConst(quint32 data)const;

	private:
		static const std::map<QString, std::function<CommandProcessor*(DeviceEmulator*)>> m_lmToFactory;

	protected:
		DeviceEmulator* m_device = nullptr;

		std::set<quint32> m_parseMemorySanitizer;	// This set is used only during parsing app logic code
													// It contains already written addresses or addresses wich are
													// written by platform logic (like IO module, receiving data from optic channel, ...)
													// m_parseMemorySanitizer is used to detect reading uninitialized memory
	};
}

