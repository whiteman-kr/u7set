#ifndef STREAMEDDATA_H
#define STREAMEDDATA_H

namespace Proto
{

	class VFRAME30LIBSHARED_EXPORT StreamedData
	{
	public:
		StreamedData();
		StreamedData(const char* data, size_t size);			// ������� ������ � ������������������� data ������� �� src
		explicit StreamedData(const QByteArray& src);			// ������� ������ � ������������������� data ������� �� src
		explicit StreamedData(size_t capacity);

		virtual ~StreamedData();

	private:
		StreamedData(const StreamedData&);				// ��������� � �������������
		StreamedData& operator= (const StreamedData&);	// ��������� � �������������

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
