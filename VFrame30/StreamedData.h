#ifndef STREAMEDDATA_H
#define STREAMEDDATA_H

namespace Proto
{

	class VFRAME30LIBSHARED_EXPORT StreamedData
	{
	public:
		StreamedData();
		StreamedData(const char* data, size_t size);			// создать объект и проинницализировать data данными из src
		explicit StreamedData(const QByteArray& src);			// создать объект и проинницализировать data данными из src
		explicit StreamedData(size_t capacity);

		virtual ~StreamedData();

	private:
		StreamedData(const StreamedData&);				// Запрещена к использованию
		StreamedData& operator= (const StreamedData&);	// Запрещена к использованию

	public:
		const char* data() const;		// Получить указатель на даннные, указатель не сохранять, данные не изменять.
		QByteArray& mutable_data();		// Получить указатель на даннные
		char* data();					// Получить указатель на даннные, указатель не сохранять
		size_t length() const;			// Получить размер данных (GetData()) в байтах

		void clear();					// clear data

	public:
		QByteArray m_data;
	};

}

#endif // STREAMEDDATA_H
