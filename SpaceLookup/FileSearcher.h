#pragma once
#include "stdafx.h"

struct SpaceItem
{
	SpaceItem* FlinkItem;
	std::vector<SpaceItem*> BlinkItems;
	std::wstring Name;
	uintmax_t Size;
};

using SpaceInfo = std::map<SIZE_T, std::vector<SpaceItem*>>;

void ResolveDir(SpaceInfo& Space, const std::filesystem::path& DirPath);
void Free(SpaceInfo& Space);