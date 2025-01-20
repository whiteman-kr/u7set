#pragma once

#include <memory>
#include <QObject>

#include "Afb.h"

class QDomDocument;
class SimCommandTest_LM5_LM6;

namespace Hardware
{
	class DeviceModule;
}

using LmCommandCode = quint16;

struct LmCommand
{
	// required parameters
	//
	quint16 code = 0;
	quint16 codeMask = 0;
	QString caption;
	QString simulationFunc;
	QString parseFunc;
	QString description;

	static const int UNDEFINED_PARAM = -1;

	int codeSize = UNDEFINED_PARAM;					// command length in words
	int readTime = UNDEFINED_PARAM;
	bool waitFbExecution = false;

	// optional parameters
	//
	int constRuntime = UNDEFINED_PARAM;
	int writeToBitMemRuntime = UNDEFINED_PARAM;
	int writeToWordMemRuntime = UNDEFINED_PARAM;
	int preFbReadWordTime = UNDEFINED_PARAM;
	int postFbReadWordTime = UNDEFINED_PARAM;
	int preFbReadBitTime = UNDEFINED_PARAM;
	int postFbReadBitTime = UNDEFINED_PARAM;

	// required parameters
	//
	QString checkFunc;				// CodeChecker function name to check command params
	QString getMnemoFunc;			// CodeItem function name to get command mnemonics
	QString calcExecTimeFunc;		// CodeItem function name to calculate command execution time

	bool loadFromXml(const QDomElement& element, QString* errorMessage);

public:

	// Logic Unit Version 2 commands
	//
	static const LmCommandCode NO_COMMAND		= 0x0000;
	static const LmCommandCode NOP				= 0x0040;
	static const LmCommandCode STARTAFB			= 0x0080;
	static const LmCommandCode STOP				= 0x00C0;
	static const LmCommandCode MOV				= 0x0100;
	static const LmCommandCode MOVMEM			= 0x0140;
	static const LmCommandCode MOVC				= 0x0180;
	static const LmCommandCode MOVBC			= 0x01C0;
	static const LmCommandCode WRFB				= 0x0200;
	static const LmCommandCode RDFB				= 0x0240;
	static const LmCommandCode WRFBC			= 0x0280;
	static const LmCommandCode WRFBB			= 0x02C0;
	static const LmCommandCode RDFBB			= 0x0300;
	static const LmCommandCode RDFBCMP			= 0x0340;
	static const LmCommandCode SETMEM			= 0x0380;
	static const LmCommandCode MOVB				= 0x03C0;
	static const LmCommandCode NSTART			= 0x0400;
	static const LmCommandCode APPSTART			= 0x0440;
	static const LmCommandCode MOV32			= 0x0480;
	static const LmCommandCode MOVC32			= 0x04C0;
	static const LmCommandCode WRFB32			= 0x0500;
	static const LmCommandCode RDFB32			= 0x0540;
	static const LmCommandCode WRFBC32			= 0x0580;
	static const LmCommandCode RDFBCMP32		= 0x05C0;
	static const LmCommandCode MOVCMPF			= 0x0600;
	static const LmCommandCode PMOV				= 0x0640;
	static const LmCommandCode PMOV32			= 0x0680;
	static const LmCommandCode FILLB			= 0x06C0;

	// Logic Unit Version 3 additional commands
	//
	static const LmCommandCode RESET			= 0x0060;
	static const LmCommandCode SET				= 0x0050;
	static const LmCommandCode OR				= 0x0070;
	static const LmCommandCode AND				= 0x0048;
	static const LmCommandCode NOT				= 0x0068;
	static const LmCommandCode LSHIFT0			= 0x0058;
	static const LmCommandCode LSHIFT1			= 0x0078;
	static const LmCommandCode MOV_ADDR_ACC		= 0x0120;
	static const LmCommandCode MOV_ACC_ADDR		= 0x0110;
	static const LmCommandCode MOVC_ACC			= 0x01A0;
	static const LmCommandCode MOVB_ACC_ADDR	= 0x03E0;
	static const LmCommandCode MOVB_ADDR_ACC	= 0x03D0;
};

class LmDescription : public QObject
{
	Q_OBJECT

	// Properties needed for accessing the data by configuration script
	//
	Q_PROPERTY(quint32 FlashMemory_ConfigFrameCount READ (m_flashMemory.configFrameCount))
	Q_PROPERTY(quint32 FlashMemory_ConfigFramePayload READ (m_flashMemory.configFramePayload))
	Q_PROPERTY(quint32 FlashMemory_ConfigUartId READ (m_flashMemory.configUartId))
	Q_PROPERTY(quint32 FlashMemory_MaxConfigurationCount READ (m_flashMemory.maxConfigurationCount))
	Q_PROPERTY(quint32 FlashMemory_SingleConfigFirstFrame READ(m_flashMemory.singleConfigFirstFrame))
	Q_PROPERTY(quint32 FlashMemory_SingleConfigFrameCount READ(m_flashMemory.singleConfigFrameCount))
	Q_PROPERTY(quint32 FlashMemory_SingleConfigUniqueIdOffset READ(m_flashMemory.singleConfigUniqueIdOffset))
	Q_PROPERTY(quint32 Memory_TxDiagDataSize READ(m_memory.txDiagDataSize))
	Q_PROPERTY(quint32 OptoInterface_OptoPortCount READ (m_optoInterface.optoPortCount))
	Q_PROPERTY(int Lan_ControllerCount READ (m_lan.lanControllerCount))

	friend SimCommandTest_LM5_LM6;

public:
	explicit LmDescription(QObject* parent = nullptr);
	explicit LmDescription(const LmDescription& that);
	LmDescription& operator=(const LmDescription& src);
	virtual ~LmDescription();

	// Loading and parsing XML
	//
public:
	bool load(const QByteArray& xml, QString* errorMessage);
	bool load(const QString& xml, QString* errorMessage);
	bool load(const QDomDocument& doc, QString* errorMessage);

	void clear();

protected:
	bool loadCommands(const QDomElement& element, QString* errorMessage);
	bool loadAfbComponents(const QDomElement& element, QString* errorMessage);
	bool loadAfbs(const QDomElement& element, QString* errorMessage);

	// Methods
	//
public:
	static QString lmDescriptionFile(const Hardware::DeviceModule* logicModule);
	void dump() const;

	// Data Structures
	//
public:
	struct FlashMemory
	{
		quint32 m_appLogicFrameCount = 0xFFFFFFFF;
		quint32 m_appLogicFramePayload = 0xFFFFFFFF;
		quint32 m_appLogicFrameSize = 0xFFFFFFFF;
		quint32 m_appLogicUartId = 0;
		bool m_appLogicWriteBitstream = false;

		quint32 m_configFrameCount = 0xFFFFFFFF;
		quint32 m_configFramePayload = 0xFFFFFFFF;
		quint32 m_configFrameSize = 0xFFFFFFFF;
		quint32 m_configUartId = 0;
		bool m_configWriteBitstream = false;

		quint32 m_tuningFrameCount = 0xFFFFFFFF;
		quint32 m_tuningFramePayload = 0xFFFFFFFF;
		quint32 m_tuningFrameSize = 0xFFFFFFFF;
		quint32 m_tuningUartId = 0;
		bool m_tuningWriteBitstream = false;

		quint32 m_maxConfigurationCount = 0;

		quint32 m_singleConfigFirstFrame = 0;
		quint32 m_singleConfigFrameCount = 0;
		quint32 m_singleConfigUniqueIdOffset = 0;

		quint32 configFrameCount() const { return m_configFrameCount; }
		quint32 configFramePayload() const { return m_configFramePayload; }
		quint32 configUartId() const { return m_configUartId; }

		quint32 maxConfigurationCount() const { return m_maxConfigurationCount; }

		quint32 singleConfigFirstFrame() const { return m_singleConfigFirstFrame; }
		quint32 singleConfigFrameCount() const { return m_singleConfigFrameCount; }
		quint32 singleConfigUniqueIdOffset() const { return m_singleConfigUniqueIdOffset; }

		bool load(const QDomDocument& document, QString* errorMessage);
	};

	struct Memory
	{
		quint32 m_codeMemorySize = 0xFFFFFFFF;
		quint32 m_appMemorySize = 0xFFFFFFFF;

		quint32 m_appDataOffset = 0xFFFFFFFF;
		quint32 m_appDataSize = 0xFFFFFFFF;

		quint32 m_appLogicBitDataOffset = 0xFFFFFFFF;
		quint32 m_appLogicBitDataSize = 0xFFFFFFFF;
		quint32 m_appLogicWordDataOffset = 0xFFFFFFFF;
		quint32 m_appLogicWordDataSize = 0xFFFFFFFF;

		quint32 m_moduleDataOffset = 0xFFFFFFFF;
		quint32 m_moduleDataSize = 0xFFFFFFFF;
		quint32 m_moduleCount = 14;

		quint32 m_tuningDataOffset = 0xFFFFFFFF;
		quint32 m_tuningDataSize = 0xFFFFFFFF;
		quint32 m_tuningDataFrameCount = 0xFFFFFFFF;
		quint32 m_tuningDataFramePayload = 0xFFFFFFFF;
		quint32 m_tuningDataFrameSize = 0xFFFFFFFF;

		quint32 m_txDiagDataOffset = 0xFFFFFFFF;
		quint32 m_txDiagDataSize = 0xFFFFFFFF;

		quint32 txDiagDataSize() const { return m_txDiagDataSize; }

		bool load(const QDomDocument& document, QString* errorMessage);

		bool isAppLogicBitData(quint32 address) const;
		bool isAppLogicWordData(quint32 address) const;
	};

	struct LogicUnit
	{
		quint32 m_alpPhaseTime = 0xFFFFFFFF;			// in microseconds
		quint32 m_clockFrequency = 0xFFFFFFFF;			// in Hz
		quint32 m_cycleDuration = 0xFFFFFFFF;			// in microseconds
		quint32 m_idrPhaseTime = 0xFFFFFFFF;			// in microseconds

		bool load(const QDomDocument& document, QString* errorMessage);

		double clockTimeSecs() const;
		int idrPhaseClocks() const;
		int alpPhaseClocks() const;
	};

	struct OptoInterface
	{
		quint32 m_optoPortCount = 0xFFFFFFFF;
		quint32 m_optoPortAppDataOffset = 0xFFFFFFFF;
		quint32 m_optoPortAppDataSize = 0xFFFFFFFF;
		quint32 m_optoInterfaceDataOffset = 0xFFFFFFFF;
		quint32 m_optoPortDataSize = 0xFFFFFFFF;
		bool m_sharedBuffer = false;

		quint32 optoPortCount() const { return m_optoPortCount; }

		bool load(const QDomDocument& document, QString* errorMessage);
	};

	struct LanController
	{
		E::LanControllerType m_type = E::LanControllerType::Tuning;
		int m_place = 0;

		bool isProvideTuning() const;
		bool isProvideAppData() const;
		bool isProvideDiagData() const;
	};

	struct Lan
	{
		int m_rupVersion = 0;
		int m_fotipVersion = 0;

		std::vector<LanController> m_lanControllers;

		int lanControllerCount() const;
		E::LanControllerType lanControllerType(int index, bool* ok = nullptr) const;
		int lanControllerPlace(int index, bool* ok = nullptr) const;

		LanController lanController(int index, bool* ok) const;

		bool load(const QDomDocument& document, QString* errorMessage);
	};

	struct Other
	{
		int ocmTxDataSizeLimit = 0;
		int ocmRxDataSizeLimit = 0;

		bool load(const QDomDocument& document, QString* errorMessage);
	};

	// Properties
	//
public:
	QString name() const;

	Q_INVOKABLE int descriptionNumber() const;

	const QString& configurationStringFile() const;
	Q_INVOKABLE QString jsConfigurationStringFile() const;

	const QString& version() const;

	const FlashMemory& flashMemory() const;
	const Memory& memory() const;
	const LogicUnit& logicUnit() const;
	const OptoInterface& optoInterface() const;
	const Lan& lan() const;
	const Other& other() const;

	Q_INVOKABLE int jsLanControllerType(int index);
	Q_INVOKABLE int jsLanControllerPlace(int index);

	bool checkAfbVersions() const;
	quint32 checkAfbVersionsOffset(bool absoluteValue) const;

	const std::vector<std::shared_ptr<Afb::AfbElement>>& afbElements() const;
	std::vector<std::shared_ptr<Afb::AfbElement>> afbElements(int opCode) const;
	std::vector<std::shared_ptr<Afb::AfbElement>> afbElements(const QString& componentCaption) const;
	const std::shared_ptr<Afb::AfbElement> afbElement(const QString& elementCaption) const;

	std::shared_ptr<Afb::AfbComponent> component(int opCode) const;
	std::shared_ptr<Afb::AfbComponent> component(const QString& caption) const;
	const std::map<int, std::shared_ptr<Afb::AfbComponent>>& afbComponents() const;

	LmCommand command(int commandCode) const;
	const LmCommand* commandPtr(int commandCode) const;
	const std::map<int, LmCommand>& commands() const;
	std::vector<LmCommand> commandsAsVector() const;
	int logicUnitCommandsVersion() const;
	bool isCommandsAvailable(const std::vector<LmCommandCode>& commandsCodes) const;
	bool isBitAccAvailable() const;

	// Data
	//
private:
	// !!! Copy constructor is defined, don't forget to add new members copy to it
	//
	QString m_name;
	int m_descriptionNumber = -1;
    QString m_configurationScriptFile;
    QString m_version;

	FlashMemory m_flashMemory;
	Memory m_memory;
	LogicUnit m_logicUnit;
	OptoInterface m_optoInterface;
	Lan m_lan;
	Other m_other;

	// Possible commands
	//
	std::map<int, LmCommand> m_commands;		// Key is command.code
	int m_logicUnitCommandsVersion = 0;

	mutable std::optional<bool> m_bitAccAvailable;

	// AFBs
	//
	bool m_checkAfbVersions = false;			// Generate code for checking AFB versions
	quint32 m_checkAfbVersionsOffset = 0;		// Result offset to genarate checking AFB versions

	std::map<int, std::shared_ptr<Afb::AfbComponent>> m_afbComponents;		// Key is OpCode of AFBComponent
	std::vector<std::shared_ptr<Afb::AfbElement>> m_afbElements;

	// !!! Copy constructor is defined, don't forget to add new memers copy to it
	//
};

using LmDescriptionShared = std::shared_ptr<LmDescription>;
using LmDescriptionConstShared = std::shared_ptr<const LmDescription>;


