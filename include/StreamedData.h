#ifndef STREAMEDDATA_H
#define STREAMEDDATA_H

namespace Proto
{
	class VFRAME30LIBSHARED_EXPORT CStreamedData
	{
	public:
		CStreamedData();
		CStreamedData(const char* data, size_t size);			// создать объект и проинницализировать data данными из src
		explicit CStreamedData(const QByteArray& src);			// создать объект и проинницализировать data данными из src
		explicit CStreamedData(size_t capacity);

		virtual ~CStreamedData();

	private:
		CStreamedData(const CStreamedData&);				// Запрещена к использованию
		CStreamedData& operator= (const CStreamedData&);	// Запрещена к использованию

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
