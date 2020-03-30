#include "../lib/WUtils.h"
#include <QString>
#include <cassert>


void swapBytes(const char* src, char* dest, int size)
{
	assert(src != dest);

	dest += size - 1;

	for(int i = 0; i < size; i++)
	{
		*dest-- = *src++;
	}
}

bool partitionOfInteger(int number, const std::vector<int>& availableParts, std::vector<int>* resultPartition)
{
	if (resultPartition == nullptr)
	{
		assert(false);
		return false;
	}

	resultPartition->clear();

	if (availableParts.size() == 0)
	{
		assert(false);
		return false;
	}

	// special case fast processing when parts count == 1
	//
	if (availableParts.size() == 1)
	{
		int part0 = availableParts[0];

		if (part0 == 0)
		{
			assert(false);		//
			return false;
		}

		if ((number % part0) == 0)
		{
			int n = number / part0;

			for(int i = 0; i < n; i++)
			{
				resultPartition->push_back(part0);
			}

			return true;
		}

		return false;
	}

	//

	std::vector<int> parts = availableParts;

	// sort available parts in DESCENDING order
	//
	std::sort(parts.begin(), parts.end());
	std::reverse(parts.begin(), parts.end());

	std::vector<std::pair<int, int>> tmp;

	int startPartIndex = 0;
	int curPartIndex = 0;
	int removeFromIndex = -1;

	int iterationsCounter = 0;

	do
	{
		do
		{
			iterationsCounter++;

			if (iterationsCounter >= 500)
			{
				assert(false);			// difficult solution or looping???
				return false;
			}

			tmp.push_back(std::pair<int, int>(parts[curPartIndex], curPartIndex));

			int tmpSum = 0;

			for(auto& pr : tmp)
			{
				tmpSum += pr.first;
			}

			// debug output

//			QString resultStr = QString("%1 = ").arg(tmpSum);

//			for(auto& pr : tmp)
//			{
//				resultStr += QString().setNum(pr.first) + " ";
//			}

//			qDebug() << resultStr.toStdString().c_str();

			// debug output

			if (tmpSum == number)
			{
				for(auto& p : tmp)
				{
					resultPartition->push_back(p.first);
				}

				return true;
			}

			if (tmpSum < number)
			{
				continue;
			}

			// here if tmpSum > number

			tmp.pop_back();			// remove last and try next part

			curPartIndex++;

			if (curPartIndex >= static_cast<int>(parts.size()))
			{
				curPartIndex--;
				break;
			}

			continue;
		}
		while(true);

		if (startPartIndex + 1 == static_cast<int>(parts.size()))
		{
			return false;
		}

		if (tmp.size() == 0)
		{
			startPartIndex++;

			if (startPartIndex >=  static_cast<int>(parts.size()))
			{
				return false;
			}

			curPartIndex = startPartIndex;
			removeFromIndex = -1;

			continue;
		}

		// here if tmp.size() > 0
		//
		if (removeFromIndex == -1)
		{
			removeFromIndex = static_cast<int>(tmp.size() - 1);
		}
		else
		{
			// here removeFromIndex can be == 0
			// after decrement it will be -1
			//
			removeFromIndex--;
		}

		if (removeFromIndex <= 0)
		{
			startPartIndex++;

			if (startPartIndex >=  static_cast<int>(parts.size()))
			{
				return false;
			}

			curPartIndex = startPartIndex;
			removeFromIndex = -1;

			tmp.clear();

			continue;
		}

		if (removeFromIndex > 0)
		{
			if (removeFromIndex >= static_cast<int>(tmp.size()))
			{
				assert(false);
				return false;
			}

			tmp.erase(tmp.begin() + removeFromIndex, tmp.end());

			if (tmp.size() != 0)
			{
				int lastItemPartIndex = tmp.back().second;

				if (lastItemPartIndex < static_cast<int>(parts.size() - 1))
				{
					curPartIndex = lastItemPartIndex + 1;
				}
				else
				{
					curPartIndex = lastItemPartIndex;
				}
			}
			else
			{
				assert(false);
			}
		}
		else
		{
			return false;
		}
	}
	while(true);

	return false;
}

bool partitionOfInteger(int number, const QVector<int>& availableParts, QVector<int>* partition)
{
	if (partition == nullptr)
	{
		assert(false);
		return false;
	}

	std::vector<int> parts;

	for(int p : availableParts)
	{
		parts.push_back(p);
	}

	std::vector<int> resultPartition;

	bool result = partitionOfInteger(number, parts, &resultPartition);

	partition->clear();

	for(int p : resultPartition)
	{
		partition->append(p);
	}

	return result;
}


