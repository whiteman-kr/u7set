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

class LmDescription : public QObject
{
	Q_OBJECT

	// Properties needed for accessing the data by configuration script
	//
	Q_PROPERTY(quint32 FlashMemory_ConfigFrameCount READ (m_flashMemory.configFrameCount))
	Q_PROPERTY(quint32 FlashMemory_ConfigFrameSize	READ (m_flashMemory.configFrameSize))
	Q_PROPERTY(quint32 FlashMemory_ConfigUartId		READ (m_flashMemory.configUartId))
	Q_PROPERTY(quint32 Memory_TxDiagDataSize		READ (m_memory.txDiagDataSize))
	Q_PROPERTY(quint32 OptoInterface_OptoPortCount	READ (m_optoInterface.optoPortCount))

public:
	explicit LmDescription(QObject* parent = 0);
	virtual ~LmDescription();

	// Loading and parsing XML
	//
public:
	bool load(const QByteArray& file, QString* errorMessage);
	bool load(const QString& file, QString* errorMessage);
	bool load(QDomDocument doc, QString* errorMessage);

protected:
	bool loadAfbComponents(const QDomElement& element, QString* errorMessage);
	bool loadAfbs(const QDomElement& element, QString* errorMessage);

	// Methods
	//
public:
	static QString lmDescriptionFile(const Hardware::DeviceModule* logicModule);
	void dump() const;

signals:

public slots:

	// Data Structures
	//
public:
	struct FlashMemory
	{
		quint32 m_appLogicFrameCount = 0xFFFFFFFF;
		quint32 m_appLogicFrameSize = 0xFFFFFFFF;
		quint32 m_appLogicUartId = 0;
		bool m_appLogicWriteBitstream = false;

		quint32 m_configFrameCount = 0xFFFFFFFF;
		quint32 m_configFrameSize = 0xFFFFFFFF;
		quint32 m_configUartId = 0;
		bool m_configWriteBitstream = false;

		quint32 m_tuningFrameCount = 0xFFFFFFFF;
		quint32 m_tuningFrameSize = 0xFFFFFFFF;
		quint32 m_tuningUartId = 0;
		bool m_tuningWriteBitstream = false;

		quint32 configFrameCount() const { return m_configFrameCount; }
		quint32 configFrameSize() const { return m_configFrameSize; }
		quint32 configUartId() const { return m_configUartId; }

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
		quint32 m_tuningDataOffset = 0xFFFFFFFF;
		quint32 m_tuningDataSize = 0xFFFFFFFF;
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

		quint32 optoPortCount() const { return m_optoPortCount; }

		bool load(const QDomDocument& document, QString* errorMessage);
	};

	// Properties
	//
public:
	int descriptionNumber() const;
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

	// Data
	//
private:
	int m_descriptionNumber = -1;
    QString m_configurationScriptFile;
    QString m_version;

	FlashMemory m_flashMemory;
	Memory m_memory;
	LogicUnit m_logicUnit;
	OptoInterface m_optoInterface;

	// AFBs
	//
	std::map<int, std::shared_ptr<Afb::AfbComponent>> m_afbComponents;		// Key is OpCode of AFBComponent
	std::vector<std::shared_ptr<Afb::AfbElement>> m_afbs;
};

#endif // LOGICMODULE_H
