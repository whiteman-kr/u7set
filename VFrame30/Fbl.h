#pragma once

#include "../include/ProtoSerialization.h"
#include "DebugInstCounter.h"

namespace Afbl
{
	// ��� ������� ��������
	//
	enum AfbSignalType
	{
		Analog,
		Discrete
	};

	// ��� ��������� ��������
	//
	enum AfbParamType
	{
		AnalogIntegral,
		AnalogFloatingPoint,
		DiscreteValue
	};

	// �������� ��������� ��������
	//
	struct VFRAME30LIBSHARED_EXPORT AfbParamValue
	{
		long long IntegralValue;
		double FloatingPoint;
		bool Discrete;
		AfbParamType Type;	// ��� ������ ���������

		// Serialization
		//
	public:

		AfbParamValue();
		AfbParamValue(long long value, AfbParamType type);
		AfbParamValue(double value, AfbParamType type);
		AfbParamValue(bool value, AfbParamType type);

		bool SaveData(Proto::FblParamValue* message) const;
		bool LoadData(const Proto::FblParamValue& message);

		bool loadFromXml(QXmlStreamReader* xmlReader);
		bool saveToXml(QXmlStreamWriter* xmlWriter) const;
	};

		
	//
	//
	//	CFblElementSignal	- ������ FBL ��������
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

		// Data
		//
private:
		QString m_caption;
		AfbSignalType m_type;
	};


	//
	//
	//	CFblElementParam	- �������� FBL ��������
	//
	//
	class VFRAME30LIBSHARED_EXPORT AfbElementParam
	{
	public:
		AfbElementParam(void);
		virtual ~AfbElementParam(void);

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
						
		// Data
		//
	private:
		QString m_caption;				// ������������ ���������
		AfbParamType m_type;			// ��� ������ ���������

		AfbParamValue m_value;			// �������� ���������
		AfbParamValue m_defaultValue;	// �������� �� ���������

		AfbParamValue m_lowLimit;		// ������ ������ ���������
		AfbParamValue m_highLimit;		// ������� ������ ���������
	};
	

	//
	//
	//	FblElement	- �������� FBL ��������
	//
	//
	class VFRAME30LIBSHARED_EXPORT AfbElement :
		public Proto::ObjectSerialization<AfbElement>,
		public VFrame30::DebugInstCounter<AfbElement>
	{
	public:
		AfbElement(void);
		virtual ~AfbElement(void);

		void Init(void);

		// Serialization
		//
		friend Proto::ObjectSerialization<AfbElement>;

	public:
		bool loadFromXml(QXmlStreamReader* xmlReader);
		bool saveToXml(QXmlStreamWriter* xmlWriter) const;

	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

	private:
		// ������������ ������� ������ ��� ������������, �.�. ��� �������� ������� �� ��������� �� ����������������,
		// � ������ �����������
		static AfbElement* CreateObject(const Proto::Envelope& message);

		// Methods
		//
	public:

	// Properties and Datas
	//
	public:
		const QUuid& guid() const;
		void setGuid(const QUuid& QUuid);

		const QString& strID() const;
		void setStrID(const QString& strID);

		const QString& caption() const;
		void setCaption(const QString& caption);

		unsigned int opcode() const;
		void setOpcode(unsigned int value);

		const std::vector<AfbElementSignal>& inputSignals() const;
		void setInputSignals(const std::vector<AfbElementSignal>& inputsignals);

		const std::vector<AfbElementSignal>& outputSignals() const;
		void setOutputSignals(const std::vector<AfbElementSignal>& outputsignals);

		const std::vector<AfbElementParam>& params() const;
		void setParams(const std::vector<AfbElementParam>& params);

	private:
		QUuid m_guid;
		QString m_strID;
		QString m_caption;
		unsigned int m_opcode;

		std::vector<AfbElementSignal> m_inputSignals;
		std::vector<AfbElementSignal> m_outputSignals;

		std::vector<AfbElementParam> m_params;
	};

	//
	//
	//	FblElementCollection - ��������� ���������� FBL ���������
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


