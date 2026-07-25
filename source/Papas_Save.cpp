#include "Papas_Save.h"
#include <cstdio>
#include <cstring>
#include <sys/stat.h>

namespace {
	// "PPZS" little-endian; rejects garbage files that happen to be the right size
	const u32 SAVE_MAGIC   = 0x535A5050;
	const u32 SAVE_VERSION = 3;	// v3: added customerMet (v2: stars + seals split)
	const u32 SAVE_OLDEST  = 2;	// versions below this are unreadable
	const char *SAVE_DIR   = "sdmc:/3ds/PapasPizzeria";

	struct SaveFileHeader
	{
		u32 magic;
		u32 version;
		u32 dataSize;
	};

	// A fresh real or virtual SD may not have either directory yet.
	void ensureSaveDir()
	{
		mkdir("sdmc:/3ds", 0777);
		mkdir(SAVE_DIR, 0777);
	}

	// Older saves fill the struct's front; reject newer or oversized layouts.
	bool readSlotFile(const char *path, Papas::SaveData &out)
	{
		FILE *f = fopen(path, "rb");
		if (f == nullptr) return false;

		out = Papas::SaveData();	// the tail of a short file stays at defaults
		SaveFileHeader header;
		bool ok = fread(&header, sizeof(header), 1, f) == 1
			&& header.magic == SAVE_MAGIC
			&& header.version >= SAVE_OLDEST
			&& header.version <= SAVE_VERSION
			&& header.dataSize <= sizeof(Papas::SaveData)
			&& fread(&out, header.dataSize, 1, f) == 1;
		fclose(f);
		if (!ok) return false;

		// Mark old saves' unlocked customers as met so their splash doesn't replay.
		if (header.version < 3)
		{
			int unlocked = 6 + (out.rank - 1);
			if (unlocked > 35) unlocked = 35;
			for (int type = 1; type <= unlocked; type++)
				out.customerMet[type] = 1;
		}
		return true;
	}
}

void Papas::SaveManager::slotPath(int slot, char *out, size_t outSize) const
{
	snprintf(out, outSize, "%s/save%d.sav", SAVE_DIR, slot + 1);
}

bool Papas::SaveManager::slotExists(int slot)
{
	SaveData scratch;
	return peekSlot(slot, scratch);
}

bool Papas::SaveManager::peekSlot(int slot, SaveData &out)
{
	if (slot < 0 || slot >= SLOT_COUNT) return false;
	char path[64];
	slotPath(slot, path, sizeof(path));
	return readSlotFile(path, out);
}

void Papas::SaveManager::startNewGame(int slot)
{
	if (slot < 0 || slot >= SLOT_COUNT) return;
	data = SaveData();	// day 1, rank 1, no tips, no stars
	activeSlot = slot;
	save();	// write immediately so the slot shows up as used right away
}

bool Papas::SaveManager::loadSlot(int slot)
{
	SaveData loaded;
	if (!peekSlot(slot, loaded)) return false;
	data = loaded;
	activeSlot = slot;
	return true;
}

bool Papas::SaveManager::save()
{
	if (activeSlot < 0 || activeSlot >= SLOT_COUNT) return false;

	ensureSaveDir();
	char path[64];
	slotPath(activeSlot, path, sizeof(path));
	FILE *f = fopen(path, "wb");
	if (f == nullptr) return false;

	SaveFileHeader header = {SAVE_MAGIC, SAVE_VERSION, sizeof(SaveData)};
	bool ok = fwrite(&header, sizeof(header), 1, f) == 1
		&& fwrite(&data, sizeof(data), 1, f) == 1;
	fclose(f);
	return ok;
}

bool Papas::SaveManager::eraseSlot(int slot)
{
	if (slot < 0 || slot >= SLOT_COUNT) return false;
	char path[64];
	slotPath(slot, path, sizeof(path));
	if (activeSlot == slot) activeSlot = -1;
	return remove(path) == 0;
}
