#pragma once

#include "GenericPlatformMath.h"
#include "IdentityFunctor.h"
#include "UnrealTemplate.h"
#include "Invoke.h"
#include "BinaryHeap.h"

namespace AlgoImpl
{
	/**
	 * Implementation of an introspective sort. Starts with quick sort and switches to heap sort when the iteration depth is too big.
	 * The sort is unstable, meaning that the ordering of equal items is not necessarily preserved.
	 * This is the internal sorting function used by IntroSort overrides.
	 *
	 * @param First			pointer to the first element to sort
	 * @param Num			the number of items to sort
	 * @param Projection	The projection to sort by when applied to the element.
	 * @param Predicate		predicate class
	 */
	template <typename T, typename ProjectionType, typename PredicateType>
	void IntroSortInternal(T* First, SIZE_T Num, ProjectionType Projection, PredicateType Predicate)
	{
		struct FStack
		{
			T* Min;
			T* Max;
			uint32 MaxDepth;
		};

		if (Num < 2)
		{
			return;
		}

		FStack RecursionStack[32] = { {First, First + Num - 1, (uint32)(FMath::Loge(Num) * 2.f)} }, Current, Inner;
		for (FStack* StackTop = RecursionStack; StackTop >= RecursionStack; --StackTop) //-V625
		{
			Current = *StackTop;

		Loop:
			int64 Count = Current.Max - Current.Min + 1;

			if (Current.MaxDepth == 0)
			{
				// We're too deep into quick sort, switch to heap sort
				HeapSortInternal(Current.Min, Count, Projection, Predicate);
				continue;
			}

			if (Count <= 8)
			{
				// Use simple bubble-sort.
				while (Current.Max > Current.Min)
				{
					T* Max, * Item;
					for (Max = Current.Min, Item = Current.Min + 1; Item <= Current.Max; Item++)
					{
						if (Predicate(Invoke(Projection, *Max), Invoke(Projection, *Item)))
						{
							Max = Item;
						}
					}
					Swap(*Max, *Current.Max--);
				}
			}
			else
			{
				// Grab middle element so sort doesn't exhibit worst-cast behavior with presorted lists.
				Swap(Current.Min[Count / 2], Current.Min[0]);

				// Divide list into two halves, one with items <=Current.Min, the other with items >Current.Max.
				Inner.Min = Current.Min;
				Inner.Max = Current.Max + 1;
				for (; ; )
				{
					while (++Inner.Min <= Current.Max && !Predicate(Invoke(Projection, *Current.Min), Invoke(Projection, *Inner.Min)));
					while (--Inner.Max > Current.Min && !Predicate(Invoke(Projection, *Inner.Max), Invoke(Projection, *Current.Min)));
					if (Inner.Min > Inner.Max)
					{
						break;
					}
					Swap(*Inner.Min, *Inner.Max);
				}
				Swap(*Current.Min, *Inner.Max);

				--Current.MaxDepth;

				// Save big half and recurse with small half.
				if (Inner.Max - 1 - Current.Min >= Current.Max - Inner.Min)
				{
					if (Current.Min + 1 < Inner.Max)
					{
						StackTop->Min = Current.Min;
						StackTop->Max = Inner.Max - 1;
						StackTop->MaxDepth = Current.MaxDepth;
						StackTop++;
					}
					if (Current.Max > Inner.Min)
					{
						Current.Min = Inner.Min;
						goto Loop;
					}
				}
				else
				{
					if (Current.Max > Inner.Min)
					{
						StackTop->Min = Inner.Min;
						StackTop->Max = Current.Max;
						StackTop->MaxDepth = Current.MaxDepth;
						StackTop++;
					}
					if (Current.Min + 1 < Inner.Max)
					{
						Current.Max = Inner.Max - 1;
						goto Loop;
					}
				}
			}
		}
	}
}

namespace Algo
{
	template <typename RangeType, typename PredicateType>
	FORCEINLINE void IntroSort(RangeType& Range, PredicateType Predicate)
	{
		AlgoImpl::IntroSortInternal(GetData(Range), GetNum(Range), FIdentityFunctor(), MoveTemp(Predicate));
	}
}