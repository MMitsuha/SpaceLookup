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
			spdlog::error("ResolveDirInternal Error!e.what():", e.what());
		}
	}
}

void ResolveDir(SpaceInfo& Space, const std::filesystem::path& DirPath)
{
	ResolveDirInternal(Space, DirPath, 0, nullptr);

	for (auto RingIter = Space.rbegin(); RingIter != Space.rend(); RingIter++)
	{
		for (auto& Item : RingIter->second)
		{
			if (Item->FlinkItem)
			{
				Item->FlinkItem->Size += Item->Size;
				Item->FlinkItem->FileCount += Item->FileCount;
				Item->FlinkItem->DirCount += Item->DirCount;
			}
		}
	}
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