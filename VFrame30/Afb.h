#pragma once

#include "../lib/ProtoSerialization.h"
#include "../lib/DebugInstCounter.h"
#include "../lib/Types.h"
#include "../VFrame30/VFrame30Lib_global.h"
#include <QXmlStreamReader>
#include <QDomElement>


namespace Afb
{
	class VFRAME30LIBSHARED_EXPORT AfbComponent
	{
	public:
		AfbComponent();

		// Serialization
		//
	public:
		bool loadFromXml(const QDomElement& xmlElement, QString* errorMessage);
		bool saveToXml(QDomElement* xmlElement) const;

		// Properties
		//
	public:
		int opCode() const;
		void setOpCode(int value);

		QString caption() const;
		void setCaption(const QString& value);

		int impVersion() const;
		void setImpVersion(int value);

		int opIndex() const;
		void setIndex(int value);

	private:
		int m_opCode = -1;
		QString m_caption;
		int m_impVersion = -1;
		int m_opIndex = -1;
	};


	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// DELETE IT IN FUTURE
	//

	enum class AfbType
	{
		UNKNOWN = 0,
		LOGIC = 1,
		NOT = 2,
		TCT = 3,
		FLIP_FLOP = 4,
		CTUD = 5,
		MAJ = 6,
		SRSST = 7,
		BCOD = 8,
		BDEC = 9,
		BCOMP = 10,
		DAMPER = 11,
		MEM = 12,
		MATH = 13,
		SCALE = 14,
		SCALE_P = 15,
		FUNC = 16,
		INT = 17,
		DPCOMP = 20,
		MUX = 21,
		LATCH = 22,
		LIM = 23,
		DEAD_ZONE = 24,
		POL = 25,
		DER = 26,
		MISMATCH = 27,
		First = LOGIC,
		Last = MISMATCH,
	};

	//
	// AfbSignal
	//
	class VFRAME30LIBSHARED_EXPORT AfbSignal : public QObject
	{
		Q_OBJECT

	public:
		AfbSignal(void);
		AfbSignal(const AfbSignal& that);
		AfbSignal& operator=(const AfbSignal& that);

		// Serialization
		//
	public:
		bool loadFromXml(const QDomElement& xmlElement, QString* errorMessage);
		bool saveToXml(QDomElement* element) const;

		// Properties
		//
	public:
		const QString& opName() const;
		void setOpName(const QString& value);

		Q_INVOKABLE QString caption() const;
		Q_INVOKABLE QString jsCaption();
		void setCaption(const QString& caption);

		E::SignalType type() const;
		Q_INVOKABLE int jsType() const;
		void setType(E::SignalType type);

		E::DataFormat dataFormat() const;
		void setDataFormat(E::DataFormat dataFormat);

		Q_INVOKABLE int operandIndex() const;
		void setOperandIndex(int value);

		Q_INVOKABLE int size() const;
		void setSize(int value);

		E::ByteOrder byteOrder() const;

		bool isAnalog() const;
		bool isDiscrete() const;

		// Data
		//
private:
		// Operator= is present, don't forget to add new fields to it
		//
		QString m_opName;
		QString m_caption;
		E::SignalType m_type = E::SignalType::Analog;
		E::DataFormat m_dataFormat = E::DataFormat::UnsignedInt;
		int m_operandIndex = 0;
		int m_size = 0;
		// Operator= is present, don't forget to add new fields to it
		//
	};

	//
	// AfbParam
	//
	class VFRAME30LIBSHARED_EXPORT AfbParam
	{
	public:
		AfbParam(void);

		// Methods
		//
	public:
		void update(const E::SignalType& type, const E::DataFormat dataFormat, const QVariant& lowLimit, const QVariant& highLimit);

		// Serialization
		//
	public:
		bool deprecatedLoadFromXml(QXmlStreamReader* xmlReader);
		bool loadFromXml(const QDomElement& xmlElement, QString* errorMessage);
		bool saveToXml(QDomElement* xmlElement) const;

		// Properties
		//
	public:
		const QString& caption() const;
		void setCaption(const QString& caption);

		const QString& opName() const;
		void setOpName(const QString& value);

		bool visible() const;
		void setVisible(bool visible);

		E::SignalType type() const;
		void setType(E::SignalType type);

		E::DataFormat dataFormat() const;
		void setDataFormat(E::DataFormat dataFormat);

		bool isAnalog() const;
		bool isDiscrete() const;

		const QVariant& value() const;
		void setValue(const QVariant& value);

		const QVariant& defaultValue() const;
		void setDefaultValue(const QVariant& defaultValue);

		const QVariant& lowLimit() const;
		void setLowLimit(const QVariant& lowLimit);

		const QVariant& highLimit() const;
		void setHighLimit(const QVariant& highLimit);

        int operandIndex() const;
		void setOperandIndex(int value);

		int size() const;
		void setSize(int value);

		bool instantiator() const;
		void setInstantiator(bool value);

		bool user() const;
		void setUser(bool value);

		QString changedScript() const;
		void setChangedScript(const QString& value);

		QString units() const;
		void setUnits(const QString& value);

		// Data
		//
	private:
		QString m_opName;			// Param name
		QString m_caption;			// Param caption
		bool m_visible;
		E::SignalType m_type;		// Param type
		E::DataFormat m_dataFormat;
		bool m_instantiator;
		bool m_user;
		QString m_changedScript;

		QVariant m_value;			// Param value
		QVariant m_defaultValue;	// Param default value

		QVariant m_lowLimit;		// Low limit for param
		QVariant m_highLimit;		// High limit for param

		int m_operandIndex;
		int m_size;

		QString m_units;
	};


	//
	// FblElement
	//
	class VFRAME30LIBSHARED_EXPORT AfbElement :
		public QObject
	{
		Q_OBJECT

	public:
		AfbElement(void);
		AfbElement(const AfbElement& that);
		AfbElement& operator=(const AfbElement& that);

		// Serialization
		//
	public:
		bool loadFromXml(const Proto::AfbElementXml& data, QString* errorMsg);
//		bool loadFromXml(const QByteArray& data, QString &errorMsg);
		bool loadFromXml(const QDomElement& xmlElement, QString* errorMessage);
		bool deprecatedFormatLoad(const Proto::AfbElementXml& data, QString& errorMsg);

		bool saveToXml(Proto::AfbElementXml* dst) const;
//		bool saveToXml(QByteArray* dst) const;
		bool saveToXml(QDomElement* xmlElement) const;

		Q_INVOKABLE QObject* getAfbSignalByOpIndex(int opIndex);
		Q_INVOKABLE QObject* getAfbSignalByCaption(QString caption);

//	protected:
//		virtual bool SaveData(Proto::Envelope* message) const override;
//		virtual bool LoadData(const Proto::Envelope& message) override;

//	private:
//		// Use this func only in serialization, as while object creation is not fully initialized  and must be read
//		//
//		static std::shared_ptr<AfbElement> CreateObject(const Proto::Envelope& message);

		// Methods
		//
	public:
		void updateParams(const std::vector<AfbParam>& params);

	// Properties and Datas
	//
	public:
		const QString& strID() const;
		void setStrID(const QString& strID);

		QString caption() const;
		void setCaption(const QString& caption);

		QString description() const;
		void setDescription(const QString& value);

		QString version() const;
		void setVersion(const QString& value);

		QString category() const;
		void setCategory(const QString& value);

		int opCode() const;
		void setOpCode(int value);

		bool hasRam() const;
		void setHasRam(bool value);

		bool internalUse() const;
		void setInternalUse(bool value);

		QString libraryScript() const;
		void setLibraryScript(const QString& value);

		QString afterCreationScript() const;
		void setAfterCreationScript(const QString& value);

		const std::vector<AfbSignal>& inputSignals() const;
		void setInputSignals(const std::vector<AfbSignal>& inputsignals);

		const std::vector<AfbSignal>& outputSignals() const;
		void setOutputSignals(const std::vector<AfbSignal>& outputsignals);

		const std::vector<AfbParam>& params() const;
		std::vector<AfbParam>& params();

		int paramsCount() const;
		void setParams(const std::vector<AfbParam>& params);

		std::shared_ptr<Afb::AfbComponent> component();
		std::shared_ptr<Afb::AfbComponent> component() const;
		void setComponent(std::shared_ptr<Afb::AfbComponent> value);

		QString componentCaption() const;

	private:
		// ATTENTION!!! AfbElement has operator =, add copy of any new member to it!!!!
		//
		QString m_strID;
		QString m_caption;
		QString m_description;
		QString m_version = "0.0000";
		QString m_category;
		int m_opCode = -1;
		bool m_hasRam = false;
		bool m_internalUse = false;

		QString m_libraryScript;
		QString m_afterCreationScript;

		std::vector<AfbSignal> m_inputSignals;
		std::vector<AfbSignal> m_outputSignals;

		std::vector<AfbParam> m_params;

		std::shared_ptr<Afb::AfbComponent> m_component;
	};

	//
	//	AfbElementCollection
	//
	class VFRAME30LIBSHARED_EXPORT AfbElementCollection :
		public DebugInstCounter<AfbElementCollection>
	{
	public:
		AfbElementCollection(void);
		virtual ~AfbElementCollection(void);

		void Init(void);

		// Serialization
		//
	public:
		bool SaveData(Proto::AfbElementCollection* message) const;
		bool LoadData(const Proto::AfbElementCollection& message);

		// Methods
		//
	public:
		void setElements(const std::vector<std::shared_ptr<AfbElement>>& elements);

		const std::vector<std::shared_ptr<AfbElement>>& elements() const;
		std::vector<std::shared_ptr<AfbElement>>* mutable_elements();

		std::shared_ptr<AfbElement> get(const QString& strID) const;

		// Properties and Datas
		//
	public:
		std::vector<std::shared_ptr<AfbElement>> m_elements;
	};
}


