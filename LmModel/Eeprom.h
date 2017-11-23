#ifndef EEPROM_H
#define EEPROM_H
#include <vector>
#include <QByteArray>

namespace LmModel
{
	namespace UartID
	{
		const int Configuration = 258;
		const int ApplicationLogic = 257;
		const int Tuning = 260;
	}

	class Eeprom
	{
	public:
		explicit Eeprom(int uartId = 0);
		virtual ~Eeprom();

		// Access
		//
	public:
		bool init(int frameSize, int frameCount, int fillWith = 0x00);
		bool fill(int fillWith);
		bool reset();					// Set data with FFs
		void clear();					// Clear buffer and reset size

		bool loadData(const QByteArray& fileData, QString* errorMessage);
		bool loadVersion6(const QJsonObject& jConfig, QString* errorMessage);

		bool parseAllocationFrame();

	public:
		quint8 getByte(int frameIndex, int byteOffset);
		quint16 getWord(int frameIndex, int wordOffset);

		qint32 getSint32(int frameIndex, int wordOffset);
		quint32 getUint32(int frameIndex, int wordOffset);

		float getFloat(int frameIndex, int wordOffset);
		double getDouble(int frameIndex, int wordOffset);

		template <typename TYPE>
		TYPE getData(int eepromOffset);

		// Properties
		//
	public:
		int uartId() const;
		int size() const;
		int frameSize() const;
		int frameCount() const;

		int framePayloadSize() const;

		// Parsed allocation frame (1)
		//
		quint16 subsystemKey() const;
		quint16 buildNo() const;
		quint16 configrationsCount() const;
		int configFrameIndex(int configurationNo) const;

		// Data here
		//
	private:
		int m_uartId = 0;
		QByteArray m_data;
		int m_frameSize = 0;
		int m_frameCount = 0;

		// Parsed Data
		//
	private:

		// Data from bitstream file
		//
		int m_framePayloadSize = 0;

		// Data from paresed allocation frame
		//
		quint16	m_subsystemKey = 0;
		quint16	m_buildNo = 0;
		quint16	m_configrationsCount = 0;
		std::vector<int> m_configFrameIndexes;
	};
}

#endif // EEPROM_H
