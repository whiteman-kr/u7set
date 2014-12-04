#pragma once
#include "../include/ProtoSerialization.h"

// ----------------------------------------------------------------------------
//
//						Module Configuration
//
// ----------------------------------------------------------------------------

namespace Hardware
{
	class ModuleConfigurationStruct;
	class McFirmware;
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

		int typeSize();     // получить размер типа в битах, или -1 если тип не найден
		int arraySize();    // получить размер массива в []. 1 - если скобок нет, -1 при ошибке (нет закрывающей скобки или некорректное число)

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
		int m_boolSize = -1;				// количество байт, в которых размещается bool
		bool m_userProperty = false;		// This property must be set by user
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

		int dataSize() const;
		void setDataSize(int dataSize);

		int actualSize() const;

		const QVector<ModuleConfigurationValue>& values() const;

		bool be() const;
		void setBe(bool be);

		// Data
		//
	private:
		QString m_name;				// Straucture name
		int m_dataSize = 0;			// размер структуры в байтах = размер всех данных + размер всех вложенных структур. вычисляется при построении дерева
		int m_size = 0;				// размер структуры, указанный в xml-файле. если не указан - равен по умолчанию 0
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
		//std::shared_ptr<ModuleConfigurationStruct> m_data;		// указатель на структуру типа Type. Заполняется после загрузки.
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

		bool compile(McFirmware* dest, const QString& deviceStrId, int changeset, QString* errorString) const;

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
	class McFirmware
	{
	public:
		McFirmware();
		~McFirmware();

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
