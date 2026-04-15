#include <vector>

namespace Core
{
	template <typename T>
	class SwapbackArray
	{
	public:
		SwapbackArray<T>() { m_Data = std::vector<T>(); }

		std::vector<T>& Data() { return m_Data; }
		const std::vector<T>& Data() const { return m_Data; }
		 
		T& operator[](size_t index) { return m_Data[index];	}
		const T& operator[](size_t index) const { return m_Data[index];	}

		inline void RemoveAt(size_t index)
		{
			m_Data[index] = std::move(m_Data.back());
			m_Data.pop_back();
		}

		inline void RemoveAt(std::vector<T>::iterator it)
		{
			size_t index = it - m_Data.begin();
			RemoveAt(index);
		}

		inline auto begin() { return m_Data.begin(); }
		inline auto end() { return m_Data.end(); }
		inline auto begin() const { return m_Data.begin(); }
		inline auto end() const { return m_Data.end(); }

		inline size_t Size() const { return m_Data.size(); }
		inline bool Empty() const { return m_Data.empty(); }
		inline void Pushback(T& val) { return m_Data.push_back(val); }
		inline void Popback() { return m_Data.pop_back(); }

	private:
		std::vector<T> m_Data;
	};
}