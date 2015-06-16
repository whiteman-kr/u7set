#pragma once

#include "../include/ProtoSerialization.h"
#include "DebugInstCounter.h"

namespace Afbl
{
	// Тип сигнала элемента
	//
	enum AfbSignalType
	{
		Analog,
		Discrete
	};

	// Тип параметра элемента
	//
	enum AfbParamType
	{
		AnalogIntegral,
		AnalogFloatingPoint,
		DiscreteValue
	};

	//
	//
	//	CFblElementSignal	- Сигнал FBL элемента
	//
	//
	class VFRAME30LIBSHARED_EXPORT AfbElementSignal : public QObject
	{
		Q_OBJECT
	public:
		AfbElementSignal(void);
		virtual ~AfbElementSignal(void);

		AfbElementSignal(const AfbElementSignal& that);

		AfbElementSignal& operator=(const AfbElementSignal& that);

		// Serialization
		//
	public:
		bool SaveData(Proto::FblElementSignal* message) const;
		bool LoadData(const Proto::FblElementSignal& message);

		bool saveToXml(QXmlStreamWriter* xmlWriter) const;
		bool loadFromXml(QXmlStreamReader* xmlReader);

		// Properties
		//
	public:
		const QString& opName() const;
		void setOpName(const QString& value);

		const QString& caption() const;
		Q_INVOKABLE QString jsCaption();
		void setCaption(const QString& caption);

		AfbSignalType type() const;
		Q_INVOKABLE int jsType() const;
		void setType(AfbSignalType type);

		Q_INVOKABLE int operandIndex() const;
		void setOperandIndex(int value);

		Q_INVOKABLE int size() const;
        void setSize(int value);

		// Data
		//
private:
		QString m_opName;
		QString m_caption;
		AfbSignalType m_type;
		int m_operandIndex;
        int m_size;
	};


	//
	//
	//	CFblElementParam	- Параметр FBL элемента
	//
	//
	class VFRAME30LIBSHARED_EXPORT AfbElementParam
	{
	public:
		AfbElementParam(void);
		virtual ~AfbElementParam(void);

		// Methods
		//
	public:
		void update(const AfbParamType& type, const QVariant& lowLimit, const QVariant& highLimit);

		// Serialization
		//
	public:
		bool SaveData(Proto::FblElementParam* message) const;
		bool LoadData(const Proto::FblElementParam& message);

		bool loadFromXml(QXmlStreamReader* xmlReader);
		bool saveToXml(QXmlStreamWriter* xmlWriter) const;


		// Properties
		//
	public:
		const QString& caption() const;
		void setCaption(const QString& caption);

		const QString& opName() const;
		void setOpName(const QString& value);

		bool visible() const;
		void setVisible(bool visible);

		AfbParamType type() const;
		void setType(AfbParamType type);

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

        // Data
		//
	private:
		QString m_opName;				// Наименование параметра
		QString m_caption;				// Наименование параметра
		bool m_visible;
		AfbParamType m_type;			// Тип данных параметра
		bool m_instantiator;
		bool m_user;
		QString m_changedScript;

		QVariant m_value;			// Значение параметра
		QVariant m_defaultValue;	// Значение по умолчанию

		QVariant m_lowLimit;		// Нижний предел параметра
		QVariant m_highLimit;		// Верхний предел параметра

		int m_operandIndex;
        int m_size;
    };
	

	//
	//
	//	FblElement	- Application Functioanl Block Description
	//
	//
	class VFRAME30LIBSHARED_EXPORT AfbElement :
		public QObject,
		public Proto::ObjectSerialization<AfbElement>
	{
		Q_OBJECT
	public:

		AfbElement(void);
		virtual ~AfbElement(void);

		AfbElement(const AfbElement& that);

		AfbElement& operator=(const AfbElement& that);

		// Serialization
		//
		friend Proto::ObjectSerialization<AfbElement>;

	public:
		bool loadFromXml(const Proto::AfbElementXml& data);
		bool loadFromXml(const QByteArray& data);
		bool loadFromXml(QXmlStreamReader* xmlReader);

		bool saveToXml(Proto::AfbElementXml* dst) const;
		bool saveToXml(QByteArray* dst) const;
		bool saveToXml(QXmlStreamWriter* xmlWriter) const;

		Q_INVOKABLE QObject* getAfbSignalByOpIndex(int opIndex);

	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

	private:
		// Use this func only in serialization, as while object creation is not fully initialized  and must be read
		//
		static AfbElement* CreateObject(const Proto::Envelope& message);

		// Methods
		//
	public:
		void updateParams(const std::vector<AfbElementParam>& params);

	// Properties and Datas
	//
	public:
		const QString& strID() const;
		void setStrID(const QString& strID);

		QString caption() const;
		void setCaption(const QString& caption);

		unsigned int opcode() const;
		void setOpcode(unsigned int value);

		bool hasRam() const;
		void setHasRam(bool value);

		bool requiredStart() const;
		void setRequiredStart(bool value);

		QString libraryScript() const;
		void setLibraryScript(const QString& value);

		QString afterCreationScript() const;
		void setAfterCreationScript(const QString& value);

		const std::vector<AfbElementSignal>& inputSignals() const;
		void setInputSignals(const std::vector<AfbElementSignal>& inputsignals);

		const std::vector<AfbElementSignal>& outputSignals() const;
		void setOutputSignals(const std::vector<AfbElementSignal>& outputsignals);

		const std::vector<AfbElementParam>& params() const;
		std::vector<AfbElementParam>& params();

		int paramsCount() const;
		void setParams(const std::vector<AfbElementParam>& params);


	private:
		QString m_strID;
		QString m_caption;
		unsigned int m_opcode;
		bool m_hasRam;
		bool m_requiredStart;

		QString m_libraryScript;
		QString m_afterCreationScript;

		std::vector<AfbElementSignal> m_inputSignals;
		std::vector<AfbElementSignal> m_outputSignals;

		std::vector<AfbElementParam> m_params;
	};

	//
	//
	//	FblElementCollection - Коллекция прототипов FBL элементов
	//
	//
	class VFRAME30LIBSHARED_EXPORT AfbElementCollection :
		public VFrame30::DebugInstCounter<AfbElementCollection>
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


