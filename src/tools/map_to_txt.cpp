/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/system.h>
#include <engine/shared/datafile.h>
#include <engine/storage.h>
#include <engine/kernel.h>
#include <engine/map.h>
#include <stdio.h>

int main(int argc, const char **argv)
{
	IStorage *pStorage = CreateStorage("Teeworlds", IStorage::STORAGETYPE_BASIC, argc, argv);
	char aInFileName[1024];
	char aOutFileName[1024];
	// IKernel *pKernel = IKernel::Create();
	IEngineMap *pMap = CreateEngineMap();
	dbg_logger_stdout();

	if(!pStorage || argc != 3)
	{
		dbg_msg("error", "usage: <input> <output>");
		return -1;
	}

	str_format(aInFileName, sizeof(aInFileName), "%s", argv[1]);
	str_format(aOutFileName, sizeof(aOutFileName), "%s", argv[2]);

	if(!pMap->Load(aInFileName))
	{
		dbg_msg("error", "map '%s' not found", aInFileName);
		return -1;
	}

	return 0;
}
