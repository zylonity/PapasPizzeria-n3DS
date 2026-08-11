#pragma once
// Three versioned save slots under sdmc:/3ds/PapasPizzeria/.

#include "Papas_Constants.h"

namespace Papas {

	// Add new fields at the end with defaults, then bump SAVE_VERSION.
	struct SaveData
	{
		s32 day = 1;
		s32 rank = 1;
		s32 lastRankLimit = 0;
		s32 totalTipsCents = 0;
		// Star-customer system, indexed by customer type (1..36)
		u8  customerStars[40] = {};	// 0..5 stars toward the next seal
		u8  customerSeals[40] = {};	// 0..3 gold STAR CUSTOMER! seals
		// Tracks each NEW CUSTOMER! splash; the six starters begin as seen.
		u8  customerMet[40] = {0, 1, 1, 1, 1, 1, 1};
		// What the customer file prints under each photo.
		u16 customerFirstDay[40] = {};	// day they first walked in, 0 = never have
		u16 customerServed[40] = {};	// orders we've taken off them
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

		// Live slot data, updated by Game before save().
		SaveData data;

		// Shared instance.
		static SaveManager& getInstance()
		{
			static SaveManager instance; // Guaranteed to be destroyed.
			return instance;
		}
		SaveManager(SaveManager const&)  = delete;	// Copy constructor
		void operator=(SaveManager const&) = delete;	// Assignment Operator

	private:
		SaveManager() {}

		void slotPath(int slot, char *out, size_t outSize) const;
		int activeSlot = -1;
	};
}
//===============================================================================
