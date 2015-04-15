#pragma once
#include "../include/ProtoSerialization.h"

// ----------------------------------------------------------------------------
//
//						Module Configuration
//
// ----------------------------------------------------------------------------

namespace Hardware
{
	class ModuleConfFirmware : public QObject
	{
		Q_OBJECT

		Q_PROPERTY(int UartID READ uartId)
		Q_PROPERTY(int FrameSize READ frameSize)
		Q_PROPERTY(int FrameCount READ frameCount)

	public:
		ModuleConfFirmware();
		virtual ~ModuleConfFirmware();

		// Methods
		//
	public:
		void init(QString type, QString name, int uartId, int frameSize, int frameCount, const QString &projectName, const QString &userName, int changesetId);
		bool save(QByteArray &dest) const;
        bool load(QString fileName);
        bool isEmpty() const;

		Q_INVOKABLE bool setData8(int frameIndex, int offset, quint8 data);
		Q_INVOKABLE bool setData16(int frameIndex, int offset, quint16 data);
		Q_INVOKABLE bool setData32(int frameIndex, int offset, quint32 data);
        bool setData64(int frameIndex, int offset, quint64 data);

        Q_INVOKABLE bool storeCrc64(int frameIndex, int start, int count, int offset);

        std::vector<quint8> frame(int frameIndex);


		// Properties
		//
	public:
        QString type() const;
		QString name() const;
		int uartId() const;
		int frameSize() const;
		int frameCount() const;
		int changesetId() const;

		// Data
		//
    private:
        QString m_type;
		QString m_name;
		int m_uartId = 0;
		int m_frameSize = 0;
		int m_changesetId = 0;

		QString m_projectName;
		QString m_userName;

        std::vector<std::vector<quint8>> m_frames;
    };

    //Q_DECLARE_METATYPE(Hardware::ModuleConfFirmware);

    class ModuleConfCollection : public QObject
	{
		Q_OBJECT

	public:
		ModuleConfCollection(const QString& projectName, const QString& userName, int changesetId);
		virtual ~ModuleConfCollection();

		// Methods
		//
	public:
        Q_INVOKABLE QObject* jsGet(QString type, QString name, int uartId, int frameSize, int frameCount);

		// Properties
		//
	public:

		// Data
		//
	public:
		const std::map<QString, ModuleConfFirmware>& firmwares() const;

	private:
		std::map<QString, ModuleConfFirmware> m_firmwares;

		QString m_projectName;
		QString m_userName;
		int m_changesetId;
	};



	class ModuleConfigurationStruct;
	class McFirmwareOld;
	struct McDataChunk;


	// ----------------------------------------------------------------------------
	//
	//						ModuleConfigurationValue
	//
	// ----------------------------------------------------------------------------

	class ModuleConfigurationValue
	{
	public:
		ModuleConfigurationValue();
		virtual ~ModuleConfigurationValue();

		// Public methods
		//
	public:
		void readValue(QXmlStreamReader& reader);

		int typeSize() const;  
		int arraySize() const; 

		// Properties
		//
	public:
		const QString& name() const;
		void setName(const QString& name);

		const QString& type() const;
		void setType(const QString& type);

		int offset() const;
		void setOffset(int offset);

		int bit() const;
		void setBit(int bit);

		int boolSize() const;
		void setBoolSize(int boolSize);

		bool userProperty() const;
		void setUserProperty(bool value);

		const QString& defaultValue() const;
		void setDefaultValue(const QString& defaultValue);

		const QString& value() const;
		void setValue(const QString& value);

		// Data
		//
	protected:
		QString m_name;
		QString m_type;
		int m_offset = -1;
		int m_bit = -1;
		int m_boolSize = -1;		
		bool m_userProperty = false;
		QString m_defaultValue;
		QString m_value;
	};

    //Q_DECLARE_METATYPE(ModuleConfigurationValue*)


	// ----------------------------------------------------------------------------
	//
	//						ModulleConfigurationStruct
	//
	// ----------------------------------------------------------------------------

	class ModuleConfigurationStruct
	{
	public:
		ModuleConfigurationStruct();
		ModuleConfigurationStruct(const QString& name, int size, bool be);

		// Methods
		//
	public:
		void readStruct(QXmlStreamReader& reader, QString* errorMessage);

		// Properties
		//
	public:
		const QString& name() const;
		void setName(const QString& name);

		int size() const;
		void setSize(int size);

		const QVector<ModuleConfigurationValue>& values() const;

		bool be() const;
		void setBe(bool be);

		// Data
		//
	private:
		QString m_name;				// Structure name
		int m_size = -1;			// Structure size, entered by user. Optional, if initialized, should be >= actual data size.
		bool m_be = false;			// Big Endian

		QVector<ModuleConfigurationValue> m_values;
	};

	// ----------------------------------------------------------------------------
	//
	//						ModuleConfigurationVariable
	//
	// ----------------------------------------------------------------------------

	class ModuleConfigurationVariable
	{
	public:
		ModuleConfigurationVariable();
		~ModuleConfigurationVariable();

		// Methods
		//
	public:
		void readVariable(QXmlStreamReader& reader);

		// Properties
		//
	public:
		const QString& name() const;
		void setName(const QString& name);

		const QString& type() const;
		void setType(const QString& type);

		int frameIndex() const;
		void setFrameIndex(int frameIndex);

		//const std::shared_ptr<ModuleConfigurationStruct>& data() const;
		//void setData(const std::shared_ptr<ModuleConfigurationStruct>& data);

		// Data
		//
	private:
		QString m_name;
		QString m_type;
		int m_frameIndex = -1;
		//std::shared_ptr<ModuleConfigurationStruct> m_data;		
	};

	// ----------------------------------------------------------------------------
	//
	//						ModuleConfiguration
	//
	// ----------------------------------------------------------------------------

	class ModuleConfiguration : public QObject
	{
		Q_OBJECT

	public:
		ModuleConfiguration();
		virtual ~ModuleConfiguration();

		// Methods
		//
	public:
		bool load(const ::Proto::ModuleConfiguration& message);
		void save(::Proto::ModuleConfiguration* message) const;

		bool compile(McFirmwareOld* dest, const QString& deviceStrId, int changeset, QString* errorString) const;

		void addUserPropertiesToObject(QObject* object) const;
		bool setUserProperty(const QString& name, const QVariant& value);

		QString lastError() const;

		static void skipUnknownElement(QXmlStreamReader* reader, QString* errorMessage);

		bool readStructure(const char* data);


	protected:
		void readDeclaration(QXmlStreamReader& reader);
		void readDefinition(QXmlStreamReader& reader);

		void createUserProperties(QString* errorMessage);
		void parseUserProperties(const ModuleConfigurationStruct& structure, const QString& parentVariableName, QString* errorMessage);

		bool compileVariable(const ModuleConfigurationVariable& var, McDataChunk* chunk, QString* errorString) const;
		bool compileStructure(const ModuleConfigurationStruct& compileStruct, McDataChunk* chunk, int baseAddress, QMap<QString, int>& structSizeMap, QString* errorString) const;
		bool countStructureSize(const ModuleConfigurationStruct& compileStruct, QMap<QString, int>& structSizeMap) const;
		int getStructureSize(const ModuleConfigurationStruct& compileStruct, QMap<QString, int>& structSizeMap) const;

		// Properties
		//
	public:
		bool hasConfiguration() const;
		void setHasConfiguration(bool value);

		const QString& name() const;
		void setName(const QString& value);

		int version() const;
		void setVersion(int value);

		int uartId() const;
		void setUartId(int value);

		int minFrameSize() const;
		void setMinFrameSize(int value);

		const std::string& structDescription() const;
		void setStructDescription(const std::string& value);

		// Data
		//
	private:
		bool m_hasConfiguration = false;

		// Configuartion attributes
		//
		QString m_name;
		int m_version = -1;
		int m_uartID = -1;						// Module UART ID, for this configuration
		int m_minFrameSize = -1;				// Flash memory frame size

		QHash<QString, ModuleConfigurationStruct> m_structures;		// Paresed structures	(declarations)
		QVector<ModuleConfigurationVariable> m_variables;			// Paresed variables	(definitions)

		std::string m_xmlStructDesctription;						// Unparsed XML of Structure Description;
		QString m_lastError;

		QHash<QString, ModuleConfigurationValue> m_userProperties;
	};

	// Compiled chunk of module configuration
	//
	struct McDataChunk
	{
		McDataChunk(const QString deviceStrId, int deviceChangeset, int frameIndex);

		QString deviceStrId;
		int deviceChangeset;
		int frameIndex;
		QByteArray data;
	};

	// Compiled Module Configuration Firmware
	//
	class McFirmwareOld
	{
	public:
		McFirmwareOld();
		~McFirmwareOld();

		// Public methods
	public:
		//bool save() const;
		//bool load();

		//bool addDataChunk();

		// Properties
		//
	public:
		QString name() const;
		void setName(const QString& value);

		int uartId() const;
		void setUartId(int value);

		int frameSize() const;
		void setFrameSize(int value);

		void addChunk(const McDataChunk& chunk);

		// Data
		//
	private:
		QString m_name;

		int m_uartID = -1;						// Module UART ID, for this configuration
		int m_frameSize = -1;					// Flash memory frame size

		std::list<McDataChunk> m_data;
	};
}

