#ifndef DBOBJECTSTORAGE_H
#define DBOBJECTSTORAGE_H

template <class T>
class DbObjectStorage
{
public:
	DbObjectStorage(DbController* db, int fileTypeId):
		 m_db(db),
		 m_fileTypeId(fileTypeId)
	{
	}

	virtual ~DbObjectStorage()
	{
	}

	void clear()
	{
		m_objectsMap.clear();
		m_objectsVector.clear();
	}

	void add(const QUuid& uuid, T object)
	{
		if (m_objectsMap.find(uuid) != m_objectsMap.end())
		{
			assert(false);
			return;
		}

		m_objectsVector.push_back(object);

		m_objectsMap[uuid] = m_objectsVector.size() - 1;

		assert(m_objectsVector.size() == m_objectsMap.size());
		return;
	}


	void remove(const QUuid& uuid)
	{
		auto it = m_objectsMap.find(uuid);
		if (it == m_objectsMap.end())
		{
			assert(false);
			return;
		}

		// Delete object from vector
		//
		size_t index = it->second;

		if (index < 0 || index >= m_objectsVector.size())
		{
			assert(false);
			return;
		}

		m_objectsVector.erase(m_objectsVector.begin() + index);

		// Delete index from map
		//
		m_objectsMap.erase(it);

		// Decrease all indexes in the map that are bigger than deleted
		//
		for (auto i : m_objectsMap)
		{
			const QUuid& decUuid = i.first;
			size_t decIndex = i.second;

			if (decIndex > index)
			{
				decIndex--;
				m_objectsMap[decUuid] = decIndex;
			}
		}

		assert(m_objectsVector.size() == m_objectsMap.size());
		return;
	}

	bool removeFile(const QUuid& uuid, bool* fileRemoved, QString* errorMessage)
	{
		if (m_db == nullptr ||
			fileRemoved == nullptr ||
			errorMessage == nullptr)
		{
			assert(m_db);
			assert(fileRemoved);
			assert(errorMessage);
			return false;
		}

		*fileRemoved = false;

		DbFileInfo fi = fileInfo(uuid);

		std::vector<DbFileInfo> fileList;
		bool ok = m_db->getFileList(&fileList, m_fileTypeId, fi.fileName(), true, nullptr);

		if (ok == false || fileList.size() != 1)
		{
			*errorMessage = m_db->lastError();
			return false;
		}

		std::shared_ptr<DbFile> file = nullptr;

		ok = m_db->getLatestVersion(fileList[0], &file, nullptr);
		if (ok == false || file == nullptr)
		{
			*errorMessage = m_db->lastError();
			return false;
		}

		if (file->state() != VcsState::CheckedOut)
		{
			if (m_db->checkOut(fileList[0], nullptr) == false)
			{
				*errorMessage = m_db->lastError();
				return false;
			}
		}

		ok = m_db->deleteFiles(&fileList, nullptr);
		if (ok == false)
		{
			*errorMessage = m_db->lastError();
			return false;
		}

		// Checkin file if it exists
		//
		fi = fileList[0];

		if (fi.deleted() == true)
		{
			*fileRemoved = true;
		}

		setFileInfo(uuid, fi);

		return true;
	}

	int count() const
	{
		return static_cast<int>(m_objectsVector.size());
	}

	T get(const QUuid& uuid, bool* ok = nullptr) const
	{
		if (ok)
		{
			*ok = true;
		}

		auto it = m_objectsMap.find(uuid);
		if (it == m_objectsMap.end())
		{
			if (ok)
			{
				*ok = false;
			}

			assert(false);
			return T();
		}

		int index = static_cast<int>(it->second);

		return get(index, ok);
	}

	T get(int index, bool* ok = nullptr) const
	{
		if (ok)
		{
			*ok = true;
		}

		if (index < 0 || index >= static_cast<int>(m_objectsVector.size()))
		{
			if (ok)
			{
				*ok = false;
			}

			assert(false);
			return T();
		}

		return m_objectsVector[index];
	}

	T* getPtr(const QUuid& uuid)
	{
		auto it = m_objectsMap.find(uuid);
		if (it == m_objectsMap.end())
		{
			assert(false);
			return nullptr;
		}

		int index = static_cast<int>(it->second);

		return getPtr(index);
	}

	T* getPtr(int index)
	{
		if (index < 0 || index >= static_cast<int>(m_objectsVector.size()))
		{
			assert(false);
			return nullptr;
		}

		return &m_objectsVector[index];
	}

	bool checkOut(const QUuid& uuid, QString* errorMessage)
	{
		if (m_db == nullptr ||
			errorMessage == nullptr)
		{
			assert(m_db);
			assert(errorMessage);
			return false;
		}

		DbFileInfo fi = fileInfo(uuid);

		if (fi.state() == VcsState::CheckedOut)
		{
			return true;
		}

		bool ok = m_db->checkOut(fi, nullptr);
		if (ok == false)
		{
			*errorMessage = m_db->lastError();
			return false;
		}

		setFileInfo(uuid, fi);

		return true;
	}

	bool checkIn(const QUuid& uuid, const QString& comment, bool* fileRemoved, QString* errorMessage)
	{
		if (m_db == nullptr ||
			fileRemoved == nullptr ||
			errorMessage == nullptr)
		{
			assert(m_db);
			assert(fileRemoved);
			assert(errorMessage);
			return false;
		}

		*fileRemoved = false;

		DbFileInfo fi = fileInfo(uuid);

		if (fi.state() == VcsState::CheckedIn)
		{
			return true;
		}

		if (m_db->checkIn(fi, comment, nullptr) == false)
		{
			*errorMessage = m_db->lastError();
			return false;
		}

		if (fi.deleted() == true)
		{
			*fileRemoved = true;
		}

		setFileInfo(uuid, fi);

		return true;
	}

	bool undo(const QUuid& uuid, bool* fileRemoved, QString* errorMessage)
	{
		if (m_db == nullptr ||
			fileRemoved == nullptr ||
			errorMessage == nullptr)
		{
			assert(m_db);
			assert(fileRemoved);
			assert(errorMessage);
			return false;
		}

		*fileRemoved = false;

		DbFileInfo fi = fileInfo(uuid);

		if (fi.state() != VcsState::CheckedOut)
		{
			return true;
		}

		if (m_db->undoChanges(fi, nullptr) == false)
		{
			*errorMessage = m_db->lastError();
			return false;
		}

		// After undo operation, file can be removed, check this
		//
		setFileInfo(uuid, fi);

		if (fi.deleted() == true)
		{
			*fileRemoved = true;
		}

		return true;
	}

	DbFileInfo fileInfo(const QUuid& uuid) const
	{
		auto it = m_fileInfo.find(uuid);

		if (it == m_fileInfo.end())
		{
			return DbFileInfo();
		}

		return it->second;
	}

	void setFileInfo(const QUuid& uuid, const DbFileInfo& fileInfo)
	{
		m_fileInfo[uuid] = fileInfo;
	}

protected:
	virtual bool load(QString* errorMessage) = 0;
	virtual bool save(const QUuid& uuid, QString* errorMessage) = 0;

protected:
	DbController* m_db = nullptr;
	std::vector<T> m_objectsVector;

private:
	int m_fileTypeId = -1;
	std::map<QUuid, size_t> m_objectsMap;
	std::map<QUuid, DbFileInfo> m_fileInfo;
};

#endif // DBOBJECTSTORAGE_H
