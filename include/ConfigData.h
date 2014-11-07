#pragma once

#include "DbStruct.h"

class ConfigValue;

//----------------------------------------------------------------------------------------------------------
//
class ConfigStruct
{
public:
	ConfigStruct();
	ConfigStruct(const QString& name, int size, bool be);

	// Properties
	//
	const QString& name() const;
	void setName(const QString& name);

	int size() const;
	void setSize(const int& size);

	int dataSize() const;
	void setDataSize(const int& dataSize);

	int actualSize() const;

	QList<ConfigValue>& values();

	bool be() const;
	void setBe(const bool& be);

    void readStruct(QXmlStreamReader& reader);
    void writeStruct(QXmlStreamWriter& writer);

	// Data
	//
private:
	QString m_name;
	int m_dataSize;           // размер структуры в байтах = размер всех данных + размер всех вложенных структур. вычисляется при построении дерева
	int m_size;               // размер структуры, указанный в xml-файле. если не указан - равен по умолчанию 0
	bool m_be;                // Big Endian

	QList<ConfigValue> m_values;
};

//----------------------------------------------------------------------------------------------------------
//
class ConfigVariable
{
public:
	ConfigVariable();
	~ConfigVariable();

	// Properties
	//
	const QString& name() const;
	void setName(const QString& name);

	const QString& type() const;
	void setType(const QString& type);

    int frameIndex() const;
	void setFrameIndex(const int& frameIndex);

	const std::shared_ptr<ConfigStruct>& pData() const;
	void setData(const std::shared_ptr<ConfigStruct>& pData);

    void readVariable(QXmlStreamReader& reader);
    void writeVariable(QXmlStreamWriter& writer);
    // Data
	//
private:
	QString m_name;
	QString m_type;
	int m_frameIndex;
	std::shared_ptr<ConfigStruct> m_pData;   // указатель на структуру типа Type. Заполняется после загрузки.

};

//----------------------------------------------------------------------------------------------------------
//
class ConfigValue
{
public:
	ConfigValue();
	virtual ~ConfigValue();

	// Public methods
	//
public:
	int typeSize();     // получить размер типа в битах, или -1 если тип не найден
	int arraySize();    // получить размер массива в []. 1 - если скобок нет, -1 при ошибке (нет закрывающей скобки или некорректное число)

	// Properties
	//
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

	const std::shared_ptr<ConfigStruct>& pData() const;
	void setData(const std::shared_ptr<ConfigStruct>& pData);

    void readValue(QXmlStreamReader& reader);
    void writeValue(QXmlStreamWriter& writer);

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

	std::shared_ptr<ConfigStruct> m_pData;   // указатель на структуру типа Type. Заполняется после загрузки.
};

Q_DECLARE_METATYPE(ConfigValue*)

//----------------------------------------------------------------------------------------------------------
//
class ConfigConfiguration
{
public:
	ConfigConfiguration();

public:
	// Properties
	//
	const QString& name() const;
	void setName(const QString& name);

    int version() const;
	void setVersion(const int& version);

    int uartID() const;
	void setUartID(const int& uartID);

    int minFrameSize() const;
	void setMinFrameSize(const int& minFrameSize);

	QList<ConfigStruct>& structures();
	const QList<ConfigVariable>& variables() const;

    QList<ConfigVariable>& variables();

    void readConfiguration(QXmlStreamReader& reader);
    static void skipUnknownElement(QXmlStreamReader& reader);

    void writeConfiguration(QXmlStreamWriter& writer);
    void writeDeclaration(QXmlStreamWriter& writer);
    void writeDefinition(QXmlStreamWriter& writer);
    void writeData(QXmlStreamWriter& writer);

private:
    // чтение элементов из XML
    //
    void readDeclaration(QXmlStreamReader& reader);
    void readDefinition(QXmlStreamReader& reader);
    void readData(QXmlStreamReader& reader);
    void readVar(QXmlStreamReader& reader);

    void appendVariableItems(const std::shared_ptr<ConfigStruct> &pData);
    int getStructureIndexByType(const QString& type);

    void createMembers();   // после загрузки конфигурации создать в памяти все структуры. вызывается перед чтением данных

    void setVals();      // инициализировать все значения данными из m_valMap
    void setValsStruct(const std::shared_ptr<ConfigStruct>& pData, const QString& valName);

    void getVals();      // инициализировать m_valMap данными из значений
    void getValsStruct(const std::shared_ptr<ConfigStruct>& pData, const QString& valName);
    //Data
	//
private:
	QString m_name;
	int m_version;
	int m_uartID;
	int m_minFrameSize;

    QMap<QString, QString> m_valMap;

	QList<ConfigStruct> m_structures;
	QList<ConfigVariable> m_variables;
};

//----------------------------------------------------------------------------------------------------------
//
class ConfigDataReader
{
public:
    ConfigDataReader();

    // Public methods
    //
public:
    bool isLoaded() const;
    bool load(const QString& fileName); // загрузка из формата QSettings
    bool load(const QByteArray& data);  // загрузка из буфера в формате JSON

    // Properties
    //
    const QString& name() const;
    const QString& fileName() const;
    int version() const;
    int uartID() const;
    int minFrameSize() const;
    int framesCount() const;
    int changeset() const;

    std::vector<int> getFramesNo() const;
    const std::vector<uint8_t>& frameData(int frameIndex) const;

    // Some daclarations
    //
private:
    class ConfigDataItem
    {
    public:
        int m_index;
        std::vector<uint8_t> m_data;
    };

    // Data
    //
private:
    QString m_name;
    int m_version;
    int m_uartID;
    int m_minFrameSize;
    QString m_fileName;
    int m_changeset;

    std::vector<ConfigDataItem> m_frameData;
};

//----------------------------------------------------------------------------------------------------------
//
class ConfigData : public QObject
{
	// Public methods
	//
public:

    bool load(const DbFile& file);
    bool save(DbFile* file);

    //------------------------------------
    //obsolete functions, must be removed in future
    //bool saveData(const QString& fileName) const;		// Save to file in QSettings format
    //bool saveData(QByteArray &data) const;              // Save to byte buffer as JSON

    //bool loadData(const QString& fileName);         	// Load from file in QSettings fromat
    //bool loadData(const QByteArray& data);          	// Load from the buffer in JSON fromat

    //------------------------------------
    //bool compile(const QString& fileName) const;		// Compile and save to the file in QSettings format
    bool compile(QByteArray& data) const;				// Complie and save to the buffer as JSON
    bool compile(ConfigDataReader* reader) const; 		// Complie and save to the buffer as JSON

private:
    bool saveConfig(QByteArray& data);
    bool readConfig(const QByteArray& data);

    //void saveStructure(const std::shared_ptr<ConfigStruct> &pData, QSettings& settings) const;
    //void saveStructure(const std::shared_ptr<ConfigStruct> &pData, QJsonObject &jVar) const;

    //void loadStructure(const std::shared_ptr<ConfigStruct>& pData, QSettings& settings);
    //void loadStructure(const std::shared_ptr<ConfigStruct>& pData, QJsonObject jVar);

    bool compileStructure(const std::shared_ptr<ConfigStruct> &pData, QByteArray &array, int baseAddress) const;

	// Properties
	//
public:
	QList<ConfigConfiguration>& configurations();

	void setFileName(const QString& fileName);

	const DbFileInfo& fileInfo() const;

	// Data
	//
private:
	QList<ConfigConfiguration> m_configurations;
	DbFileInfo m_fileInfo;
};

class ConfigDataModelNode
{
public:
    ConfigDataModelNode();
    ConfigDataModelNode(const std::shared_ptr<ConfigConfiguration>& config, ConfigDataModelNode* parent);
    ConfigDataModelNode(const std::shared_ptr<ConfigVariable>& variable, ConfigDataModelNode* parent);
    ConfigDataModelNode(const std::shared_ptr<ConfigValue>& value, ConfigDataModelNode* parent);
    ~ConfigDataModelNode();

    enum Type{Root, Configuration, Struct, Variable, Value} type;

    // Data
    //
    std::shared_ptr<void> object;

    ConfigDataModelNode* parent;
    QList <ConfigDataModelNode*> children;

};

class ConfigDataModel: public QAbstractItemModel
{
public:
    ConfigDataModel(QObject* parent = 0);
    ~ConfigDataModel();

    // Public methods
    //
public:
    QModelIndex	index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
    QModelIndex	parent(const QModelIndex & index) const;
    int	rowCount(const QModelIndex & parent = QModelIndex()) const;
    int	columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    void setRootNode(ConfigDataModelNode* rootNode);

    ConfigDataModelNode* nodeFromIndex(const QModelIndex& index) const;

    // Data
    //
private:
    ConfigDataModelNode* rootNode;
};

Q_DECLARE_METATYPE(ConfigDataReader)

