#pragma once
#include <filesystem>
#include <list>

struct SpaceItem
{
	SpaceItem* FlinkItem;
	std::list<SpaceItem*> BlinkItems;
	std::filesystem::directory_entry FileObj;
	uintmax_t Size;
	uintmax_t FileCount;
	uintmax_t DirCount;
};

using SpaceInfo = std::map<SIZE_T, std::list<SpaceItem*>>;

void ResolveDir(SpaceInfo& Space, const std::filesystem::path& DirPath);
void Free(SpaceInfo& Space);