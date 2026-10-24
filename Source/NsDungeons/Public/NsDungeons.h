// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/** The log of the dungeon generation */
NSDUNGEONS_API DECLARE_LOG_CATEGORY_EXTERN(LogNsDungeons, Log, All);

/** The runtime module of NsDungeons: the graph generation and the cell layout. No game content */
class FNsDungeonsModule : public IModuleInterface
{
};
