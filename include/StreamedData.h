#ifndef STREAMEDDATA_H
#define STREAMEDDATA_H

namespace Proto
{
	class VFRAME30LIBSHARED_EXPORT CStreamedData
	{
	public:
		CStreamedData();
		CStreamedData(const char* data, size_t size);			// ������� ������ � ������������������� data ������� �� src
		explicit CStreamedData(const QByteArray& src);			// ������� ������ � ������������������� data ������� �� src
		explicit CStreamedData(size_t capacity);

		virtual ~CStreamedData();

	private:
		CStreamedData(const CStreamedData&);				// ��������� � �������������
		CStreamedData& operator= (const CStreamedData&);	// ��������� � �������������

	public:
		const char* data() const;		// �������� ��������� �� �������, ��������� �� ���������, ������ �� ��������.
		QByteArray& mutable_data();		// �������� ��������� �� �������
		char* data();					// �������� ��������� �� �������, ��������� �� ���������
		size_t length() const;			// �������� ������ ������ (GetData()) � ������

		void clear();					// clear data

	public:
		QByteArray m_data;
	};

}

#endif // STREAMEDDATA_H
