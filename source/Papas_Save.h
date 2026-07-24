#pragma once
//===============================================================================
// name: Papas_Save.h
// desc: Save-file system: three slots stored as small versioned binary files
//       on the SD card (sdmc:/3ds/PapasPizzeria/saveN.sav). The SaveSelect
//       scene picks/creates a slot, Game reads the live data and the manager
//       writes it back at the end of every day.
//===============================================================================

#include "Papas_Constants.h"

namespace Papas {

	// Everything a slot persists. Extending it: append fields, give them a
	// sane default, and bump SAVE_VERSION in the .cpp - older files are read
	// back into the front of the struct and the new fields keep their
	// defaults (see readSlotFile).
	struct SaveData
	{
		s32 day = 1;
		s32 rank = 1;
		s32 lastRankLimit = 0;
		s32 totalTipsCents = 0;
		// Star-customer system, indexed by customer type (1..36)
		u8  customerStars[40] = {};	// 0..5 stars toward the next seal
		u8  customerSeals[40] = {};	// 0..3 gold STAR CUSTOMER! seals
		// Has this type had its NEW CUSTOMER! splash yet (customerdata[t][0]
		// in the original)? The six starters come pre-earned, exactly as
		// GameData.as's blankcustomerdata has them.
		u8  customerMet[40] = {0, 1, 1, 1, 1, 1, 1};
	};

	class SaveManager
	{
	public:
		static const int SLOT_COUNT = 3;

		bool slotExists(int slot);
		bool peekSlot(int slot, SaveData &out);	// read a slot without activating it
		void startNewGame(int slot);			// fresh data, activates + writes the file
		bool loadSlot(int slot);				// activates
		bool save();							// writes the active slot back to SD
		bool eraseSlot(int slot);

		int getActiveSlot() const { return activeSlot; }

		// Live state of the active slot; Game reads this on init and updates
		// it before calling save()
		SaveData data;

		//===============================================================================
		// Singleton Implementations
		static SaveManager& getInstance()
		{
			static SaveManager instance; // Guaranteed to be destroyed.
			return instance;
		}
		SaveManager(SaveManager const&)  = delete;	// Copy constructor
		void operator=(SaveManager const&) = delete;	// Assignment Operator
		//===============================================================================

	private:
		SaveManager() {}

		void slotPath(int slot, char *out, size_t outSize) const;
		int activeSlot = -1;
	};
}
//===============================================================================
