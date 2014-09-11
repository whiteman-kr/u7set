#pragma once

#include "VFrame30.pb.h"
#include "DebugInstCounter.h"

namespace Fbl
{
	// ��� ������� ��������
	//
	enum FblSignalType			
	{
		Analog,
		Discrete
	};

	// ��� ��������� ��������
	//
	enum FblParamType
	{
		AnalogIntegral,
		AnalogFloatingPoint,
		DiscreteValue
	};

	// �������� ��������� ��������
	//
	struct FblParamValue
	{
		long long IntegralValue;
		double FloatingPoint;
		bool Discrete;

		// Serialization
		//
	public:
		bool SaveData(::Proto::FblParamValue* message) const;
		bool LoadData(const ::Proto::FblParamValue& message);
	};

		
	//
	//
	//	CFblElementSignal	- ������ FBL ��������
	//
	//
	class VFRAME30LIBSHARED_EXPORT FblElementSignal
	{
	public:
		FblElementSignal(void);
		virtual ~FblElementSignal(void);

		// Serialization
		//
	public:
		bool SaveData(::Proto::FblElementSignal* message) const;
		bool LoadData(const ::Proto::FblElementSignal& message);

		// Properties
		//
	public:
		const QString& caption() const;
		void setCaption(const QString& caption);

		FblSignalType type() const;
		void setType(FblSignalType type);

		// Data
		//
private:
		QString m_caption;
		FblSignalType m_type;
	};


	//
	//
	//	CFblElementParam	- �������� FBL ��������
	//
	//
	class VFRAME30LIBSHARED_EXPORT FblElementParam
	{
	public:
		FblElementParam(void);
		virtual ~FblElementParam(void);

		// Serialization
		//
	public:
		bool SaveData(::Proto::FblElementParam* message) const;
		bool LoadData(const ::Proto::FblElementParam& message);

		// Properties
		//
	public:
		const QString& caption() const;
		void setCaption(const QString& caption);

		FblParamType type() const;
		void setType(FblParamType type);

		const FblParamValue& value() const;
		void setValue(const FblParamValue& value);

		const FblParamValue& defaultValue() const;
		void setDefaultValue(const FblParamValue& defaultValue);

		const FblParamValue& lowLimit() const;
		void setLowLimit(const FblParamValue& lowLimit);

		const FblParamValue& highLimit() const;
		void setHighLimit(const FblParamValue& highLimit);
						
		// Data
		//
	private:
		QString m_caption;				// ������������ ���������
		FblParamType m_type;			// ��� ������ ���������

		FblParamValue m_value;			// �������� ���������
		FblParamValue m_defaultValue;	// �������� �� ���������

		FblParamValue m_lowLimit;		// ������ ������ ���������
		FblParamValue m_highLimit;		// ������� ������ ���������
	};
	

	//
	//
	//	FblElement	- �������� FBL ��������
	//
	//
	class VFRAME30LIBSHARED_EXPORT FblElement :
		public VFrame30::Proto::CVFrameObjectSerialization<FblElement>,
		public VFrame30::DebugInstCounter<FblElement>
	{
	public:
		FblElement(void);
		virtual ~FblElement(void);

		void Init(void);

		// Serialization
		//
		friend VFrame30::Proto::CVFrameObjectSerialization<FblElement>;

	protected:
		virtual bool SaveData(::Proto::Envelope* message) const override;
		virtual bool LoadData(const ::Proto::Envelope& message) override;

	private:
		// ������������ ������� ������ ��� ������������, �.�. ��� �������� ������� �� ��������� �� ����������������,
		// � ������ �����������
		static FblElement* CreateObject(const ::Proto::Envelope& message);

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

		const std::vector<FblElementSignal>& inputSignals() const;
		void setInputSignals(const std::vector<FblElementSignal>& inputsignals);

		const std::vector<FblElementSignal>& outputSignals() const;
		void setOutputSignals(const std::vector<FblElementSignal>& outputsignals);

		const std::vector<FblElementParam>& params() const;
		void setParams(const std::vector<FblElementParam>& params);

	private:
		QUuid m_guid;
		QString m_strID;
		QString m_caption;
		unsigned int m_opcode;

		std::vector<FblElementSignal> m_inputSignals;
		std::vector<FblElementSignal> m_outputSignals;

		std::vector<FblElementParam> m_params;
	};

	//
	//
	//	FblElementCollection - ��������� ���������� FBL ���������
	//
	//
	class VFRAME30LIBSHARED_EXPORT FblElementCollection :
		public VFrame30::DebugInstCounter<FblElementCollection>
	{
	public:
		FblElementCollection(void);
		virtual ~FblElementCollection(void);

		void Init(void);

		// Serialization
		//
	public:

		// Methods
		//
	public:
		std::shared_ptr<FblElement> Get(const QUuid& QUuid) const;

		// Properties and Datas
		//
	public:
		std::vector<std::shared_ptr<FblElement>> elements;
	};
}


