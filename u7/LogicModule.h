#ifndef LOGICMODULE_H
#define LOGICMODULE_H

#include <QObject>
#include <QDomDocument>
#include "../VFrame30/Afb.h"

class LogicModule : public QObject
{
	Q_OBJECT
public:
	explicit LogicModule(QObject* parent = 0);
	virtual ~LogicModule();

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
	void dump() const;

signals:

public slots:

	// Data Structures
	//
public:
	struct FlashMemory
	{
		quint32 appLogicFrameCount = 0xFFFFFFFF;
		quint32 appLogicFrameSize = 0xFFFFFFFF;
		quint32 configFrameCount = 0xFFFFFFFF;
		quint32 configFrameSize = 0xFFFFFFFF;
		quint32 tuningFrameCount = 0xFFFFFFFF;
		quint32 tuningFrameSize = 0xFFFFFFFF;

		bool load(const QDomDocument& document, QString* errorMessage);
	};

	struct Memory
	{
		quint32 appDataOffset = 0xFFFFFFFF;
		quint32 appDataSize = 0xFFFFFFFF;
		quint32 appLogicBitDataOffset = 0xFFFFFFFF;
		quint32 appLogicBitDataSize = 0xFFFFFFFF;
		quint32 appLogicWordDataOffset = 0xFFFFFFFF;
		quint32 appLogicWordDataSize = 0xFFFFFFFF;
		quint32 moduleDataOffset = 0xFFFFFFFF;
		quint32 moduleDataSize = 0xFFFFFFFF;
		quint32 tuningDataOffset = 0xFFFFFFFF;
		quint32 tuningDataSize = 0xFFFFFFFF;
		quint32 txDiagDataOffset = 0xFFFFFFFF;
		quint32 txDiagDataSize = 0xFFFFFFFF;

		bool load(const QDomDocument& document, QString* errorMessage);
	};

	struct LogicUnit
	{
		quint32 alpPhaseTime = 0xFFFFFFFF;
		quint32 clockFrequency = 0xFFFFFFFF;
		quint32 cycleDuration = 0xFFFFFFFF;
		quint32 idrPhaseTime = 0xFFFFFFFF;

		bool load(const QDomDocument& document, QString* errorMessage);
	};

	struct OptoInterface
	{
		quint32 optoPortCount = 0xFFFFFFFF;
		quint32 optoPortAppDataOffset = 0xFFFFFFFF;
		quint32 optoPortAppDataSize = 0xFFFFFFFF;
		quint32 optoInterfaceDataOffset = 0xFFFFFFFF;
		quint32 optoPortDataSize = 0xFFFFFFFF;

		bool load(const QDomDocument& document, QString* errorMessage);
	};

	// Properties
	//
public:
	int descriptionNumber() const;

	const FlashMemory& flashMemory() const;
	const Memory& memory() const;
	const LogicUnit& logicUnit() const;
	const OptoInterface& optoInterface() const;

	const std::vector<std::shared_ptr<Afb::AfbElement>>& afbs() const;

	std::shared_ptr<Afb::AfbComponent> component(int opCode) const;

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
