#include "stdafx.h"
#include "FileSearcher.h"

void ResolveDirInternal(SpaceInfo& Space, const std::filesystem::path& DirPath, SIZE_T Depth, SpaceItem* ParentItem)
{
	auto& CurrentRing = Space[Depth];

	for (auto const& DirEntry : std::filesystem::directory_iterator(DirPath, std::filesystem::directory_options::skip_permission_denied))
	{
		try
		{
			auto FileSize = DirEntry.file_size();
			SpaceItem* Item = nullptr;
			if (DirEntry.is_directory())
				Item = new SpaceItem{ ParentItem, {}, std::move(DirEntry), FileSize, 0, 1 };
			else
				Item = new SpaceItem{ ParentItem, {}, std::move(DirEntry), FileSize, 1, 0 };
			if (ParentItem) ParentItem->BlinkItems.push_back(Item);
			CurrentRing.push_back(Item);

			if (DirEntry.is_directory()) ResolveDirInternal(Space, DirEntry, Depth + 1, Item);
		}
		catch (const std::exception& e)
		{
			spdlog::error("ResolveDirInternal Error!e.what():{}", e.what());
		}
	}
}

void ResolveDir(SpaceInfo& Space, const std::filesystem::path& DirPath)
{
	auto start = std::chrono::steady_clock::now();
	ResolveDirInternal(Space, DirPath, 0, nullptr);
	auto end = std::chrono::steady_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	auto time = (double)duration.count() * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
	spdlog::info(L"DirPath:{},ResolveDirInternal used {} second(s),maximum depth:{}", DirPath.wstring(), time, Space.size());

	uintmax_t TotalCount = 0;
	start = std::chrono::steady_clock::now();
	for (auto RingIter = Space.rbegin(); RingIter != Space.rend(); RingIter++)
	{
		for (auto& Item : RingIter->second)
		{
			if (Item->FlinkItem)
			{
				Item->FlinkItem->Size += Item->Size;
				Item->FlinkItem->FileCount += Item->FileCount;
				Item->FlinkItem->DirCount += Item->DirCount;

				++TotalCount;
			}
		}
	}
	end = std::chrono::steady_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	time = (double)duration.count() * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
	spdlog::info(L"ResolveSpace used {} second(s),maximum loop count:{}", time, TotalCount);
}

void Free(SpaceInfo& Space)
{
	for (auto RingIter = Space.rbegin(); RingIter != Space.rend(); RingIter++)
	{
		for (auto& Item : RingIter->second)
		{
			delete Item;
		}
	}

	Space.clear();
}