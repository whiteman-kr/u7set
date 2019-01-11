#ifndef LOGICMODULE_H
#define LOGICMODULE_H

#include <memory>
#include <QObject>
#include <QDomDocument>
#include "../VFrame30/Afb.h"

namespace Hardware
{
	class DeviceModule;
}

struct LmCommand
{
	quint16 code = 0;
	quint16 codeMask = 0;
	QString caption;
	QString simulationFunc;
	QString parseFunc;
	QString description;

	bool loadFromXml(const QDomElement& element, QString* errorMessage);
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
	Q_PROPERTY(quint32 Memory_TxDiagDataSize READ (m_memory.txDiagDataSize))
	Q_PROPERTY(quint32 OptoInterface_OptoPortCount READ (m_optoInterface.optoPortCount))

public:
	explicit LmDescription(QObject* parent = 0);
	explicit LmDescription(const LmDescription& that);
	LmDescription& operator=(const LmDescription& src);
	virtual ~LmDescription();

	// Loading and parsing XML
	//
public:
	bool load(const QByteArray& xml, QString* errorMessage);
	bool load(const QString& xml, QString* errorMessage);
	bool load(QDomDocument doc, QString* errorMessage);

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

		quint32 configFrameCount() const { return m_configFrameCount; }
		quint32 configFramePayload() const { return m_configFramePayload; }
		quint32 configUartId() const { return m_configUartId; }

		quint32 maxConfigurationCount() const { return m_maxConfigurationCount; }

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
	};

	struct LogicUnit
	{
		quint32 m_alpPhaseTime = 0xFFFFFFFF;
		quint32 m_clockFrequency = 0xFFFFFFFF;
		quint32 m_cycleDuration = 0xFFFFFFFF;
		quint32 m_idrPhaseTime = 0xFFFFFFFF;

		bool load(const QDomDocument& document, QString* errorMessage);
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

	const std::vector<std::shared_ptr<Afb::AfbElement>>& afbs() const;

	std::shared_ptr<Afb::AfbComponent> component(int opCode) const;
	const std::map<int, std::shared_ptr<Afb::AfbComponent>>& afbComponents() const;

	LmCommand command(int commandCode) const;
	const std::map<int, LmCommand>& commands() const;
	std::vector<LmCommand> commandsAsVector() const;

	// Data
	//
private:
	// !!! Copy constructor is defined, don't forget to add new memers copy to it
	//
	QString m_name;
	int m_descriptionNumber = -1;
    QString m_configurationScriptFile;
    QString m_version;

	FlashMemory m_flashMemory;
	Memory m_memory;
	LogicUnit m_logicUnit;
	OptoInterface m_optoInterface;

	// Possible commands
	//
	std::map<int, LmCommand> m_commands;		// Key is command.code

	// AFBs
	//
	std::map<int, std::shared_ptr<Afb::AfbComponent>> m_afbComponents;		// Key is OpCode of AFBComponent
	std::vector<std::shared_ptr<Afb::AfbElement>> m_afbs;

	// !!! Copy constructor is defined, don't forget to add new memers copy to it
	//
};


#endif // LOGICMODULE_H
