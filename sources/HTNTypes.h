#pragma once

#include <cassert>
#include <vector>

namespace HTN
{
	using u8 = unsigned char;
	using u16 = unsigned short;
	using u32 = unsigned int;

	template<typename ENUM_TYPE>
	class BitFieldEnum
	{
	public:
		BitFieldEnum(ENUM_TYPE _type) :
			m_bitField(1u << static_cast<u32>(_type))
		{
		}

		void set() { m_bitField = ~0; }
		void set(ENUM_TYPE pos) { m_bitField |= (1u << static_cast<u32>(pos)); }

		void reset() { m_bitField = 0; }
		void reset(ENUM_TYPE pos) { m_bitField &= ~(1u << static_cast<u32>(pos)); }

		void flip() { m_bitField = ~m_bitField; }
		void flip(ENUM_TYPE pos) { m_bitField ^= (1u << static_cast<u32>(pos)); }

		bool any()  const { return m_bitField != 0; }
		bool none() const { return m_bitField == 0; }
		bool test(ENUM_TYPE pos) const { return (m_bitField & (1u << static_cast<u32>(pos))) != 0; }

	private:
		u32 m_bitField = 0u;
	};

	template<typename POOL_TYPE>
	class SmallPool
	{
	public:
		SmallPool(int _size) :
			m_pool(_size)
		{
		}

		POOL_TYPE* New()
		{
			const int index = _TryFindNextFreeItemIndex();
			if (index < 0)
			{
				assert(false); // No more item in pool
				return nullptr;
			}
			auto& poolItem = m_pool[index];
			poolItem.isFree = false;
			return &poolItem.item;
		}

		void Delete(POOL_TYPE* _item)
		{
			for (auto& poolItem : m_pool)
			{
				if (&poolItem.item == _item)
				{
					poolItem.isFree = true;
					return;
				}
			}
			assert(false); // Item does no belong to pool
		}

	private:

		int _TryFindNextFreeItemIndex() const
		{
			for (size_t i = 0, count = m_pool.size(); i < count; ++i)
			{
				const auto& poolItem = m_pool[i];
				if (poolItem.isFree)
				{
					return static_cast<int>(i);
				}
			}
			return -1;
		}
		struct PoolItem
		{
			POOL_TYPE item;
			bool isFree = true;
		};
		std::vector<PoolItem> m_pool;
	};
}