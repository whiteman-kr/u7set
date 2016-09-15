#include <memory>
#include <vector>
#include <list>
#include <map>
#include <bitset>
#include <cassert>
#include <type_traits>

#define PLAIN_OBJECT_LINE_SIZE			8192

namespace poh
{
	// LineStorage
	//
	template <typename TYPE, int ObjectCount>
	class LineStorage final
	{
	public:
		LineStorage()
		{
			static_assert(ObjectCount > 0, "ObjectCount must be greater than 0");

#ifdef _DEBUG
			//qDebug() << "Aligned Storage size is " << sizeof(TYPE);

			memset(&m_storage, 0xBF, sizeof(m_storage));		// 0xBF -- free space
#endif
		}

		~LineStorage()
		{
			// memory leak, not all memory was release,
			//
			assert(m_freeCount == ObjectCount);
		}

		void* allocateMemory()
		{
			if (m_freeCount == 0)
			{
				assert(m_freeCount > 0);
				return nullptr;
			}

			// Look for the free slot
			//
			for (; m_lastUsefulIndex < m_occupied.size(); m_lastUsefulIndex++)
			{
				if (m_occupied.test(m_lastUsefulIndex) == false)
				{
					void* ptr = alloc(m_lastUsefulIndex);
					return ptr;
				}
			}

			// Slot not found from m_lastUsefulIndex, try to find it from 0
			//
			m_lastUsefulIndex = 0;
			for (; m_lastUsefulIndex < m_occupied.size(); m_lastUsefulIndex++)
			{
				if (m_occupied.test(m_lastUsefulIndex) == false)
				{
					void* ptr = alloc(m_lastUsefulIndex);
					return ptr;
				}
			}

			// All slots are occupied bit m_freeCount is not null, something wrong
			//
			assert(false);
			m_freeCount = 0;		// Try to correct it?

			return nullptr;
		}

		bool freeMemory(void* ptr)
		{
			if (ptr < reinterpret_cast<void*>(&m_storage) ||
				ptr >= reinterpret_cast<void*>(reinterpret_cast<TYPE*>(&m_storage) + ObjectCount))
			{
				return false;
			}

			ptrdiff_t index = reinterpret_cast<TYPE*>(ptr) - reinterpret_cast<TYPE*>(&m_storage);

			m_occupied.reset(index);
			m_lastUsefulIndex = static_cast<int>(index);
			m_freeCount ++;

#ifdef _DEBUG
			memset(ptr, 0xBF, sizeof(TYPE));		// 0xBF -- free space
#endif
			return true;
		}

		bool hasFreeSpace() const
		{
			return m_freeCount > 0;
		}

		bool isEmpty() const
		{
			return m_freeCount == ObjectCount;
		}

	private:
		void* alloc(int index)
		{
			TYPE* ptr = reinterpret_cast<TYPE*>(&m_storage) + index;

			m_occupied.set(index);
			m_freeCount --;
			m_lastUsefulIndex ++;

			//TYPE* ptr0 = reinterpret_cast<TYPE*>(&m_storage) + 0;
			//TYPE* ptr1 = reinterpret_cast<TYPE*>(&m_storage) + 1;
			//qDebug() << "ObjectSize: " << ptrdiff_t(reinterpret_cast<char*>(ptr1) - reinterpret_cast<char*>(ptr0));

			//qDebug() << "Alloc: tr " << ptr << ", index" << index << ", free " << m_freeCount;

#ifdef _DEBUG
			assert(m_freeCount >= 0);

			// check if it has value 0xBF -- debug value for empty space
			//
			assert(*reinterpret_cast<uint8_t*>(ptr) == 0xBF);
#endif
			return ptr;
		}

	private:
		typedef typename std::aligned_storage<sizeof(TYPE) * ObjectCount, alignof(TYPE)>::type AlignedStorage;

		int m_freeCount = ObjectCount;
		int m_lastUsefulIndex = 0;				// !!Probably!! useful index
		AlignedStorage m_storage;
		std::bitset<ObjectCount> m_occupied;	// Constructs a bitset with all bits set to zero.
	};


	// StorageBase
	//
	class StorageBase
	{
	public:
		virtual void* allocateMemory() = 0;
		virtual void freeMemory(void* ptr) = 0;
	};

	// Storage
	//
	template <typename TYPE>
	struct Allocator;

	template <typename TYPE, int ObjectCount>
	class Storage final : public StorageBase
	{
	public:
		Storage()
		{
			static_assert(ObjectCount > 0, "ObjectCount must be greater than 0");
		}

		virtual void* allocateMemory() override
		{
			// Look for line
			//
			for (std::unique_ptr<LineStorage<std::_Ref_count_obj_alloc<TYPE, Allocator<TYPE>>, ObjectCount>>& l : m_lines)
			{
				if (l->hasFreeSpace() == true)
				{
					void* ptr = l->allocateMemory();
					return ptr;
				}
			}

			// All lines are full, create the new one
			//
			m_lines.push_back(std::make_unique<LineStorage<std::_Ref_count_obj_alloc<TYPE, Allocator<TYPE>>, ObjectCount>>());

			LineStorage<std::_Ref_count_obj_alloc<TYPE, Allocator<TYPE>>, ObjectCount>* l = m_lines.back().get();

			void* ptr = l->allocateMemory();
			return ptr;
		}

		virtual void freeMemory(void* ptr) override
		{
			// Look for line for this ptr
			//
			for (auto lineIt = m_lines.begin(); lineIt != m_lines.end(); ++lineIt)
			{
				bool freed = lineIt->get()->freeMemory(ptr);
				if (freed == true)
				{
					// If the line is empty, remove it
					//
					if (lineIt->get()->isEmpty() == true)
					{
						m_lines.erase(lineIt);
					}

					return;
				}
			}

			// ptr is not from this Storage instance, error
			//
			assert(false);
			return;
		}

	private:
		std::list<std::unique_ptr<LineStorage<std::_Ref_count_obj_alloc<TYPE, Allocator<TYPE>>, ObjectCount>>> m_lines;
	};

//	struct Deleter
//	{
//		Deleter(std::shared_ptr<StorageBase> s) :
//			m_storage(s)
//		{
//		}

//		template<typename TYPE>
//		void operator()(TYPE* ptr) const
//		{
//			//ptr->T::~T();
//			ptr->TYPE::~TYPE();
//			m_storage->freeMemory(ptr);
//		}

//		std::shared_ptr<StorageBase> m_storage;
//	};

	template <typename TYPE>
	struct Allocator
	{
		std::shared_ptr<StorageBase> m_storage;
		typedef TYPE value_type;

		Allocator(std::shared_ptr<StorageBase> storage) :
			m_storage(storage)
		{
		}

		template <class U>
		Allocator(const Allocator<U>& other)
		{
			m_storage = other.m_storage;
		}

		TYPE* allocate(std::size_t n)
		{
			assert(n == 1);

			//qDebug() << "Allocator in Allocator::allocate, sizeof(TYPE) " << sizeof(TYPE);			// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

			TYPE* allocatedMemory = static_cast<TYPE*>(m_storage->allocateMemory());

			return allocatedMemory;
		}

		void deallocate(TYPE* ptr, std::size_t /*n*/)
		{
			m_storage->freeMemory(ptr);
		}
	};

	//template <class T, class U>
	//bool operator==(const SimpleAllocator<T>&, const SimpleAllocator<U>&);

	//template <class T, class U>
	//bool operator!=(const SimpleAllocator<T>&, const SimpleAllocator<U>&);


	// PlainObjectHeap
	//
	//template <int LineSize>
	class PlainObjectHeap final
	{
	public:
		template <typename TYPE, typename... Args>
		std::shared_ptr<TYPE> alloc(Args&&... args)
		{
			//qDebug() << "PlainObjectHeap::alloc  sizeof(TYPE) = " << sizeof(TYPE);
			//qDebug() << "PlainObjectHeap::alloc  sizeof(std::_Ref_count_obj<TYPE>) = " << sizeof(std::_Ref_count_obj<TYPE>);
			//qDebug() << "PlainObjectHeap::alloc  sizeof(std::_Ref_count_obj_alloc<TYPE, Allocator<TYPE>>) = " << sizeof(std::_Ref_count_obj_alloc<TYPE, Allocator<TYPE>>);

			std::shared_ptr<StorageBase> storage;

			auto storageIt = m_storage.find(sizeof(std::_Ref_count_obj<TYPE>));

			if (storageIt == m_storage.end())
			{
				storage = std::make_shared<Storage<TYPE, PLAIN_OBJECT_LINE_SIZE>>();
				m_storage[sizeof(std::_Ref_count_obj<TYPE>)] = storage;
			}
			else
			{
				storage = storageIt->second;
			}

			if (storage == nullptr)
			{
				assert(storage);
				throw std::bad_alloc();
			}

			Allocator<TYPE> alloc(storage);

			std::shared_ptr<TYPE> result = std::allocate_shared<TYPE>(alloc, std::forward<Args>(args)...);

			// --
//			void* allocatedMemory = storage->allocateMemory();
//			if (allocatedMemory == nullptr)
//			{
//				throw std::bad_alloc();
//			}

//			std::shared_ptr<TYPE> result(new (allocatedMemory) TYPE(std::forward<Args>(args)...),
//					[storage](TYPE* ptr)
//					{
//						ptr->TYPE::~TYPE();
//						storage->freeMemory(ptr);
//					});

//			std::shared_ptr<TYPE> result(new (allocatedMemory) TYPE(std::forward<Args>(args)...),
//					Deleter(storage));

			return result;
		}

	//private:
		std::map<size_t, std::shared_ptr<StorageBase>> m_storage;
	};

}
