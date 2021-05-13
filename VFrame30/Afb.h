#pragma once
#include <QBitArray>
#include "../CommonLib/Types.h"
#include <optional>

class QDomElement;
class QXmlStreamReader;

namespace Proto
{
	class AfbElementXml;
	class AfbElementCollection;
}

namespace Afb
{
	enum class AfbComponentPinType
	{
		Param,
		Input,
		Output
	};

	class AfbComponentPin
	{
	public:
		AfbComponentPin() = default;
		AfbComponentPin(const AfbComponentPin&) = default;
		AfbComponentPin(AfbComponentPin&&) noexcept = default;
		AfbComponentPin(const QString& caption, int opIndex, AfbComponentPinType type);

		AfbComponentPin& operator=(const AfbComponentPin&) = default;
		AfbComponentPin& operator=(AfbComponentPin&&) noexcept = default;

	public:
		bool loadFromXml(const QDomElement& xmlElement, QString* errorMessage);
		bool saveToXml(QDomElement* xmlElement) const;

	public:
		const QString& caption() const noexcept
		{
			return m_caption;
		}
		void setCaption(const QString& value) noexcept
		{
			m_caption = value;
		}

		int opIndex() const noexcept
		{
			return m_opIndex;
		}
		void setOpIndex(int value) noexcept
		{
			m_opIndex = value;
		}

		AfbComponentPinType type() const noexcept
		{
			return m_type;
		}
		void setType(AfbComponentPinType value) noexcept
		{
			m_type = value;
		}

		bool isInputOrParam() const noexcept
		{
			return	m_type == AfbComponentPinType::Input ||
					m_type == AfbComponentPinType::Param;
		}
		bool isOutput() const noexcept
		{
			return	m_type == AfbComponentPinType::Output;
		}

	private:
		QString m_caption;
		int m_opIndex = -1;
		AfbComponentPinType m_type = AfbComponentPinType::Param;
	};

	class AfbComponent : public QObject
	{
		Q_OBJECT

	public:
		AfbComponent();
		AfbComponent(const AfbComponent& that);

		// Serialization
		//
	public:
		bool loadFromXml(const QDomElement& xmlElement, QString* errorMessage);
		bool saveToXml(QDomElement* xmlElement) const;

		// Properties
		//
	public:
		int opCode() const noexcept
		{
			return m_opCode;
		}
		void setOpCode(int value) noexcept
		{
			m_opCode = value;
		}

		bool hasRam() const noexcept
		{
			return m_hasRam;
		}
		void setHasRam(bool value) noexcept
		{
			m_hasRam = value;
		}

		const QString& caption() const noexcept
		{
			return m_caption;
		}
		void setCaption(const QString& value) noexcept
		{
			m_caption = value;
		}

		int impVersion() const noexcept
		{
			return m_impVersion;
		}
		void setImpVersion(int value) noexcept
		{
			m_impVersion = value;
		}

		int versionOpIndex() const noexcept
		{
			return m_versionOpIndex;
		}
		void setVersionOpIndex(int value) noexcept
		{
			m_versionOpIndex = value;
		}

		int maxInstCount() const noexcept
		{
			return m_maxInstCount;
		}
		void setMaxInstCount(int value) noexcept
		{
			m_maxInstCount = value;
		}

		const QString& simulationFunc() const noexcept
		{
			return m_simulationFunc;
		}
		void setSimulationFunc(const QString& value) noexcept
		{
			m_simulationFunc = value;
		}

		const std::unordered_map<int, AfbComponentPin>& pins() const noexcept
		{
			return m_pins;
		}

		bool pinExists(int pinOpIndex) const noexcept
		{
			return (static_cast<size_t>(pinOpIndex) >= m_pinExists.size())
					? false: m_pinExists[pinOpIndex];
		}
		QString pinCaption(int pinOpIndex) const noexcept
		{
			auto it = m_pins.find(pinOpIndex);
			if (it != m_pins.end())
			{
				return it->second.caption();
			}

			return QLatin1String("[UnknownPin ") + QString::number(pinOpIndex) + QLatin1String("]");
		}

	private:
		// Operator= is present, don't forget to add new fields to it
		//
		int m_opCode = -1;
		bool m_hasRam = false;
		QString m_caption;
		int m_impVersion = -1;
        int m_versionOpIndex = -1;
		int m_maxInstCount = 0;
		QString m_simulationFunc;

		std::unordered_map<int, AfbComponentPin> m_pins;		// Key is OpIndex of pin - AfbComponentPin::opIndex()
		std::vector<bool> m_pinExists;									// For fast searching of pin, intensively used in simulator
		// Operator= is present, don't forget to add new fields to it
		//
	};


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
		TCONV = 28,
		INDICATION = 29,
		PULSE_GEN = 30,

		First = LOGIC,
		Last = PULSE_GEN,				// update on adding new AFBs !!!
	};

	//
	// AfbSignal
	//
	class AfbSignal : public QObject
	{
		Q_OBJECT

	public:
		AfbSignal(void);
		AfbSignal(const AfbSignal& that);
		AfbSignal(AfbSignal&& that) noexcept;
		virtual ~AfbSignal();
		AfbSignal& operator=(const AfbSignal& that) noexcept;
		AfbSignal& operator=(AfbSignal&& that) noexcept;

		// Serialization
		//
	public:
		bool loadFromXml(const QDomElement& xmlElement, QString* errorMessage);
		bool saveToXml(QDomElement* element) const;

		// Properties
		//
	public slots:
		const QString& opName() const;
		void setOpName(const QString& value);

		Q_INVOKABLE QString caption() const;
		Q_INVOKABLE QString jsCaption();
		void setCaption(const QString& caption);

		E::SignalType type() const;
		Q_INVOKABLE int jsType() const;
		void setType(E::SignalType type);
		bool setType(const QString& type);

		E::DataFormat dataFormat() const;
		void setDataFormat(E::DataFormat dataFormat);
		bool setDataFormat(const QString& dataFormat);

		Q_INVOKABLE int operandIndex() const;
		void setOperandIndex(int value);

		Q_INVOKABLE int size() const;
		void setSize(int value);

		const std::vector<int>& additionalSizes() const;
		void setAdditionalSizes(std::vector<int> value);

		std::vector<int> allSizes() const;		// Returns size() and additionalSizes() as single vector

		E::ByteOrder byteOrder() const;
		void setByteOrder(E::ByteOrder value);
		bool setByteOrder(const QString& value);

		bool isAnalog() const;
		bool isDiscrete() const;
		bool isBus() const;

		E::BusDataFormat busDataFormat() const;
		void setBusDataFormat(E::BusDataFormat value);
		bool setBusDataFormat(const QString& value);

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
		std::vector<int> m_additionalSizes;
		E::ByteOrder m_byteOrder =  E::ByteOrder::BigEndian;
		E::BusDataFormat m_busDataFormat = E::BusDataFormat::Discrete;

		// WARNING!!!
		// Operator= is present, don't forget to add new fields to it
		//
	};

	//
	// AfbParamValue
	//
	class AfbParamValue
	{
	public:
		AfbParamValue() = default;
		AfbParamValue(E::SignalType _type, E::DataFormat _dataFormat, quint16 _size);
		AfbParamValue(const AfbParamValue&) = default;
		~AfbParamValue() = default;
		AfbParamValue& operator=(const AfbParamValue&) = default;

		bool operator==(const AfbParamValue&) const = default;
		bool operator!=(const AfbParamValue&) const = default;

	public:
		[[nodiscard]] QString toString() const;
		bool fromString(const QString& str);

		[[nodiscard]] int validate(const QString& str) const;

		[[nodiscard]] QVariant toVariant() const;		// Returns AfbParamValue as QVariant
		bool fromVariant(const QVariant& v);			// Set *this to QVariant (AfbParamValue)

	public:
		[[nodiscard]] E::SignalType type() const;
		void setType(E::SignalType type);

		[[nodiscard]] E::DataFormat dataFormat() const;
		void setDataFormat(E::DataFormat dataFormat);

		[[nodiscard]] bool isAnalog() const;
		[[nodiscard]] bool isDiscrete() const;

		[[nodiscard]] int size() const;
		void setSize(int value);

		[[nodiscard]] QVariant value() const;
		bool setValue(const QVariant& v);

		[[nodiscard]] const QString& reference() const;
		void setReference(const QString& value);

		[[nodiscard]] bool checkValue() const;

		// Data
		//
	private:
		E::SignalType m_type = E::SignalType::Analog;
		E::DataFormat m_dataFormat = E::DataFormat::Float;
		quint16 m_size = 0;				// BitWidth

		QVariant m_value;
		QString m_reference;			// Reference to variable e.g. '$(Schema.VarName)'
	};
}	// namespace Afb

Q_DECLARE_METATYPE(Afb::AfbParamValue)

namespace Afb
{
	//
	// AfbParam
	//
	class AfbParam
	{
	public:
		AfbParam(void);

		// Methods
		//
	public:
		void update(const E::SignalType& type, const E::DataFormat dataFormat, E::ByteOrder byteOrder, const QVariant& lowLimit, const QVariant& highLimit);

		// Serialization
		//
	public:
		bool deprecatedLoadFromXml(QXmlStreamReader* xmlReader);
		bool loadFromXml(const QDomElement& xmlElement, QString* errorMessage);
		bool saveToXml(QDomElement* xmlElement) const;

		// Properties
		//
	public:
		[[nodiscard]] const QString& caption() const;
		void setCaption(const QString& caption);

		[[nodiscard]] const QString& opName() const;
		void setOpName(const QString& value);

		[[nodiscard]] bool visible() const;
		void setVisible(bool visible);

		[[nodiscard]] E::SignalType type() const;
		void setType(E::SignalType type);

		[[nodiscard]] E::DataFormat dataFormat() const;
		void setDataFormat(E::DataFormat dataFormat);

		[[nodiscard]] bool isAnalog() const;
		[[nodiscard]] bool isDiscrete() const;

		[[nodiscard]] const AfbParamValue& afbParamValue() const;
		[[nodiscard]] AfbParamValue& afbParamValue();
		void setAfbParamValue(const AfbParamValue& v);

		[[nodiscard]] const QVariant& defaultValue() const;
		void setDefaultValue(const QVariant& defaultValue);

		[[nodiscard]] const QVariant& lowLimit() const;
		void setLowLimit(const QVariant& lowLimit);

		[[nodiscard]] const QVariant& highLimit() const;
		void setHighLimit(const QVariant& highLimit);

		[[nodiscard]] int operandIndex() const;
		void setOperandIndex(int value);

		[[nodiscard]] int size() const;
		void setSize(int value);

		[[nodiscard]] E::ByteOrder byteOrder() const;
		void setByteOrder(E::ByteOrder value);

		[[nodiscard]] bool instantiator() const;
		void setInstantiator(bool value);

		[[nodiscard]] bool user() const;
		void setUser(bool value);

		[[nodiscard]] QString changedScript() const;
		void setChangedScript(const QString& value);

		[[nodiscard]] const QString& units() const;
		void setUnits(const QString& value);

		// Data
		//
	private:
		QString m_opName;			// Param name
		QString m_caption;			// Param caption
		bool m_visible;
		E::ByteOrder m_byteOrder;
		bool m_instantiator;
		bool m_user;
		QString m_changedScript;

		AfbParamValue m_afbParamValue;	// Param value
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
	class AfbElement :
		public QObject
	{
		Q_OBJECT

	public:
		AfbElement(void);
		AfbElement(const AfbElement& that);
		AfbElement& operator=(const AfbElement& that) noexcept;

		// Serialization
		//
	public:
		bool loadFromXml(const Proto::AfbElementXml& data, QString* errorMsg);
		bool loadFromXml(const QDomElement& xmlElement, QString* errorMessage);
		bool deprecatedFormatLoad(const Proto::AfbElementXml& data, QString& errorMsg);

		bool saveToXml(Proto::AfbElementXml* dst) const;
		bool saveToXml(QDomElement* xmlElement) const;

		Q_INVOKABLE QObject* getAfbSignalByOpIndex(int opIndex);
		Q_INVOKABLE QObject* getAfbSignalByCaption(QString caption);

		// Methods
		//
	public:
		void updateParams(const std::vector<AfbParam>& params);

	// Properties and Datas
	//
	public:
		const QString& strID() const;
		void setStrID(const QString& strID);

		const QString& caption() const;
		void setCaption(const QString& caption);

		const QString& description() const;
		void setDescription(const QString& value);

		const QString& version() const;
		void setVersion(const QString& value);

		const QString& category() const;
		void setCategory(const QString& value);

		int opCode() const;
		void setOpCode(int value);

		std::optional<bool> hasRam() const;
		void setHasRam(bool value);

		bool internalUse() const;
		void setInternalUse(bool value);

		int minWidth() const;
		void setMinWidth(int value);

		int minHeight() const;
		void setMinHeight(int value);

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
		QString m_strId;
		QString m_caption;
		QString m_description;
		QString m_version = "0.0000";
		QString m_category;
		int m_opCode = -1;
		std::optional<bool> m_hasRam;
		bool m_internalUse = false;
		int m_minWidth = 10;			// Min width in GridSize, so read minwidth is m_minWidth * GridSize
		int m_minHeight = 0;

		QString m_libraryScript;
		QString m_afterCreationScript;

		std::vector<AfbSignal> m_inputSignals;
		std::vector<AfbSignal> m_outputSignals;

		std::vector<AfbParam> m_params;

		std::shared_ptr<Afb::AfbComponent> m_component;

		// ATTENTION!!! AfbElement has operator =, add copy of any new member to it!!!!
		//
	};

	//
	//	AfbElementCollection
	//
	class AfbElementCollection
	{
	public:
		AfbElementCollection(void);

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

