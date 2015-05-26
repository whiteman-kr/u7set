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

	// Значение параметра элемента
	//
	struct VFRAME30LIBSHARED_EXPORT AfbParamValue
	{
		int32_t IntegralValue;
		double FloatingPoint;
		bool Discrete;
		AfbParamType Type;				// Param data type

		// Serialization
		//
	public:

		AfbParamValue();
		explicit AfbParamValue(int32_t value);
		explicit AfbParamValue(double value);
		explicit AfbParamValue(bool value);

		bool SaveData(Proto::FblParamValue* message) const;
		bool LoadData(const Proto::FblParamValue& message);

		bool loadFromXml(QXmlStreamReader* xmlReader);
		bool saveToXml(QXmlStreamWriter* xmlWriter) const;

		QVariant toQVariant() const;
		static AfbParamValue fromQVariant(QVariant value);
	};

		
	//
	//
	//	CFblElementSignal	- Сигнал FBL элемента
	//
	//
	class VFRAME30LIBSHARED_EXPORT AfbElementSignal
	{
	public:
		AfbElementSignal(void);
		virtual ~AfbElementSignal(void);

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
		const QString& caption() const;
		void setCaption(const QString& caption);

		AfbSignalType type() const;
		void setType(AfbSignalType type);

		int operandIndex() const;
		void setOperandIndex(int value);

        int size() const;
        void setSize(int value);

		// Data
		//
private:
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
		void update(const AfbParamType& type, const AfbParamValue& lowLimit, const AfbParamValue& highLimit);

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

		bool visible() const;
		void setVisible(bool visible);

		AfbParamType type() const;
		void setType(AfbParamType type);

		const AfbParamValue& value() const;
		void setValue(const AfbParamValue& value);

		const AfbParamValue& defaultValue() const;
		void setDefaultValue(const AfbParamValue& defaultValue);

		const AfbParamValue& lowLimit() const;
		void setLowLimit(const AfbParamValue& lowLimit);

		const AfbParamValue& highLimit() const;
		void setHighLimit(const AfbParamValue& highLimit);
						
		int operandIndex() const;
		void setOperandIndex(int value);

        int size() const;
        void setSize(int value);

        // Data
		//
	private:
		QString m_caption;				// Наименование параметра
		bool m_visible;
		AfbParamType m_type;			// Тип данных параметра

		AfbParamValue m_value;			// Значение параметра
		AfbParamValue m_defaultValue;	// Значение по умолчанию

		AfbParamValue m_lowLimit;		// Нижний предел параметра
		AfbParamValue m_highLimit;		// Верхний предел параметра

		int m_operandIndex;
        int m_size;
    };
	

	//
	//
	//	FblElement	- Application Functioanl Block Description
	//
	//
	class VFRAME30LIBSHARED_EXPORT AfbElement :
		public Proto::ObjectSerialization<AfbElement>,
		public VFrame30::DebugInstCounter<AfbElement>
	{
	public:
		AfbElement(void);
		virtual ~AfbElement(void);

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
		const QUuid& guid() const;
		void setGuid(const QUuid& QUuid);

		const QString& strID() const;
		void setStrID(const QString& strID);

		QString caption() const;
		void setCaption(const QString& caption);

		unsigned int opcode() const;
		void setOpcode(unsigned int value);

		bool hasRam() const;
		void setHasRam(bool value);

        const std::vector<AfbElementSignal>& inputSignals() const;
		void setInputSignals(const std::vector<AfbElementSignal>& inputsignals);

		const std::vector<AfbElementSignal>& outputSignals() const;
		void setOutputSignals(const std::vector<AfbElementSignal>& outputsignals);

		const std::vector<AfbElementParam>& params() const;
		std::vector<AfbElementParam>& params();

		int paramsCount() const;
		void setParams(const std::vector<AfbElementParam>& constParams);

		const std::vector<AfbElementParam>& constParams() const;
		int constParamsCount() const;
		void setConstParams(const std::vector<AfbElementParam>& constParams);

	private:
		QUuid m_guid;
		QString m_strID;
		QString m_caption;
		unsigned int m_opcode;
		bool m_hasRam;

		std::vector<AfbElementSignal> m_inputSignals;
		std::vector<AfbElementSignal> m_outputSignals;

		std::vector<AfbElementParam> m_params;
		std::vector<AfbElementParam> m_constParams;
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

		std::shared_ptr<AfbElement> get(const QUuid& QUuid) const;

		// Properties and Datas
		//
	public:
		std::vector<std::shared_ptr<AfbElement>> m_elements;
	};
}


