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

		const std::shared_ptr<ModuleConfigurationStruct>& data() const;
		void setData(const std::shared_ptr<ModuleConfigurationStruct>& data);

		//Data
		//
	private:
		QString m_name;
		QString m_type;
		int m_offset = -1;
		int m_bit = -1;
		int m_boolSize = -1;				// количество байт, в которых размещается bool
		bool m_userProperty = false;		// This property must be set by user
		QString m_defaultValue;
		QString m_value;

		std::shared_ptr<ModuleConfigurationStruct> m_data;   // указатель на структуру типа Type. Заполняется после загрузки.
	};

	Q_DECLARE_METATYPE(ModuleConfigurationValue*)


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

		QList<ModuleConfigurationValue>& values();

		bool be() const;
		void setBe(bool be);

		// Data
		//
	private:
		QString m_name;				// Straucture name
		int m_dataSize = 0;			// размер структуры в байтах = размер всех данных + размер всех вложенных структур. вычисляется при построении дерева
		int m_size = 0;				// размер структуры, указанный в xml-файле. если не указан - равен по умолчанию 0
		bool m_be = false;			// Big Endian

		QList<ModuleConfigurationValue> m_values;
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

		const std::shared_ptr<ModuleConfigurationStruct>& data() const;
		void setData(const std::shared_ptr<ModuleConfigurationStruct>& data);

		// Data
		//
	private:
		QString m_name;
		QString m_type;
		int m_frameIndex = -1;
		std::shared_ptr<ModuleConfigurationStruct> m_data;		// указатель на структуру типа Type. Заполняется после загрузки.
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
	public:
		bool load(const ::Proto::ModuleConfiguration& message);
		void save(::Proto::ModuleConfiguration* message) const;

		QString lastError() const;

		static void skipUnknownElement(QXmlStreamReader* reader, QString* errorMessage);

	protected:
		bool readStructure(const QString& data);
		void readDeclaration(QXmlStreamReader& reader);
		void readDefinition(QXmlStreamReader& reader);

		void appendVariableItems(const std::shared_ptr<ModuleConfigurationStruct> &pData);
		void createMembers();
		void setVals();

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

		const QString& structDescription() const;
		void setStructDescription(const QString& value);

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

		//QMap<QString, QString> m_valMap;		// Values

		QList<ModuleConfigurationStruct> m_structures;			// Paresed structures	(declarations)
		QList<ModuleConfigurationVariable> m_variables;			// Paresed variables	(definitions)

		QString m_xmlStructDesctription;		// Unparsed XML of Structure Description;
		QString m_lastError;
	};
}
