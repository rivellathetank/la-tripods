// How to use:
//
// 1. Edit Main():
//    - Specify how many empty slots of each kind you have in the tripod library.
//    - List tripods for your class, with high-priority ones at the top.
//    - Set prio_tripods to the number of high-priority tripods.
//    - List all items with tripods that you have or can buy.
// 2. Run: make && ./la-tripods
//
// The output will tell you which items to store in the library so that the
// following properties are optimized in this order:
//
// 1. The number of stored high-priority tripods (max).
// 2. The number of stored low-priority tripods (max).
// 3. The total cost of bought items (min).

#include <array>
#include <bit>
#include <bitset>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <stack>
#include <tuple>
#include <vector>

namespace {

using namespace std;

// Each page has this many rows: helmet, shoulders, etc.
constexpr uint8_t kRows = 6;
// Each item can have up to this many tripods.
constexpr uint8_t kTripods = 3;

using Book = array<uint8_t, kRows>;

struct Item {
  // The row this item goes to (helmet, shoulders, etc.). In [0, kRows).
  const uint8_t row;
  // How much gold it costs to use this item.
  const uint16_t cost;
  // Tripods that this item provides.
  const uint8_t tripods[kTripods];

  // Whether the item should be used. This field gets set by the optimizer.
  bool used = false;
};

struct Score {
  bool BetterThan(const Score& other, uint64_t prio_mask) const {
    auto score = [&](const Score& x) {
      return make_tuple(x.tripod_count(prio_mask), x.tripod_count(), -x.cost);
    };
    return score(*this) > score(other);
  }

  int tripod_count(uint64_t mask = -1) const { return popcount(tripods & mask); }

  uint64_t tripods = 0;
  uint32_t cost = 0;
};

struct Assignment {
  size_t item = -1;
  Score score;
};

void Optimize(vector<Item>& items, int prio_tripods, Book book) {
  const uint64_t prio_mask = (uint64_t{1} << prio_tripods) - 1;

  vector<vector<Item*>> tripods;
  for (Item& item : items) {
    for (uint8_t tripod : item.tripods) {
      if (!tripod) continue;
      if (tripods.size() < tripod) tripods.resize(tripod);
      tripods[tripod - 1].push_back(&item);
    }
  }

  Score best_score = {};
  vector<Assignment> assignments(tripods.size());

  while (!assignments.empty()) {
    uint8_t tripod = assignments.size();
    vector<Item*>& v = tripods[tripod - 1];
    Assignment& a = assignments.back();
    Assignment prev = tripod > 1 ? assignments[tripod - 2] : Assignment();

    if (a.item != static_cast<size_t>(-1)) {
      v[a.item]->used = false;
      ++book[v[a.item]->row];
    } else if (prev.score.tripods & (1 << (tripod - 1))) {
      goto pop;
    } else if ((a.score.tripods & prio_mask) != prio_mask && tripod > prio_tripods) {
      // This is an optimization that works only if there is a solution that obtains
      // all high-priority tripods. If Optimize() doesn't find a solution, try removing
      // this branch.
      goto pop;
    }

    do {
      ++a.item;
      if (a.item == v.size()) goto pop;
    } while (v[a.item]->used || !book[v[a.item]->row]);

    v[a.item]->used = true;
    --book[v[a.item]->row];
    a.score.cost = prev.score.cost + v[a.item]->cost;
    a.score.tripods = prev.score.tripods;
    for (uint8_t t : v[a.item]->tripods) {
      if (t) a.score.tripods |= (uint64_t{1} << (t - 1));
    }

    if (assignments.size() != tripods.size()) {
      assignments.resize(tripods.size(), Assignment{.score = a.score});
    } else if (a.score.BetterThan(best_score, prio_mask)) {
      best_score = a.score;
      cout << "==[ New best assignment: " << a.score.tripod_count(prio_mask) << '/'
           << a.score.tripod_count() << '/' << a.score.cost << ' ' << bitset<64>(a.score.tripods)
           << " ]==\n";
      for (size_t i = 0; i != items.size(); ++i) {
        if (items[i].used)
          cout << "Use item: #" << setfill('0') << setw(2) << i << "\n";
      }
      cout << flush;
    }
    continue;

  pop:
    assignments.pop_back();
  }
}

void Main() {
  // The book has this many empty slots per row.
  // The first page of my book is already sorted out, so
  // there are 4 empty slots left in each row.
  const Book book = {{4, 4, 4, 4, 4, 4}};

  enum Tripod {
    // High-priority tripods: prio_tripods in total.
    kPunishingStrike_MindEnhancement = 1,
    kFrostsCall_EnhancedStrike,
    kPunishingStrike_UnavoidableFate,
    kReverseGravity_LavaArea,
    kSeraphicHail_QuickPrep,
    kEsotericReaction_EnhancedStrike,
    kInferno_FlameArea,
    kSeraphicHail_RedHail,
    kInferno_FirepowerSupplement,
    kIceShower_FrostFragment,
    kLightningVortex_MindEnhancement,
    kSeraphicHail_EvolvedHail,
    kIceShower_AgileCast,
    kFrostsCall_Chill,
    kPunishingStrike_MagickAmplification,
    kEsotericReaction_StabilizedCrystal,
    kExplosion_MagickAmplification,
    kDoomsday_FlameArea,
    kSeraphicHail_Enlightnment,
    kInferno_FirepowerExpansion,

    kLightningVortex_FierceLightning,
    kLightningVortex_QuickPace,
    kRimeArrow_FrostBarrage,
    kLightningBolt_EnchancedStrike,
    kEnergyDischarge_CondensedLightning,
    kDoomsday_Insight,
    kLightningBolt_QuickPrep,
    kExplosion_LawOfTheJungle,
    kSqual_WildfireWhirlwind,
    kEsotericReaction_JunglesLaw,
    kBlaze_BlazingWildfire,
    kLightningBolt_BranchedLightning,
    kPunishingStrike_Destruction,
    kSqual_MindEnchancement,
    kPunishingStrike_Paralize,
    kExplosion_MindEnchancement,
    kFrostsCall_MindEnchancement,
    kIceShower_EnhancedStrike,
    kSqual_QuickPrep,
    kSeraphicHail_MindEnchancement,
    kInferno_Ignite,
    kDoomsday_MindEnhancement,
    kLightningBolt_VitalPointHit,
    kFrostsCall_Enlightnment,
    kReverseGravity_JunglesLaw,
    kIceShower_Enlightnment,
    kElegainsTouch_SwiftFootwork,
    kSeraphicHail_AdditionalExplosion,
    kSqual_AgileCast,
    kInferno_WeakPointDetection,
    kIceShower_FrostZone,
    kReverseGravity_WeakPointDetection,
    kSeraphicHail_WeakPointDetection,
  };

  // This many first tripods listed in Tripod enum are high-priority.
  const int prio_tripods = 20;

  enum Row { kHelmet, kShoulders, kChest, kPants, kGloves, kWeapon };

  // Items that you either have or can buy. Set cost to non-zero for items that
  // you can buy and to zero for those that you own.
  vector<Item> items = {
      /* 00 01:01 */ {kWeapon, 0, {kLightningVortex_QuickPace}},
      /* 01 01:02 */ {kWeapon, 0, {kRimeArrow_FrostBarrage}},
      /* 02 01:03 */ {kWeapon, 0, {kReverseGravity_WeakPointDetection}},
      /* 03 01:04 */ {kWeapon, 0, {kEnergyDischarge_CondensedLightning}},
      /* 04 01:05 */ {kWeapon, 0, {kDoomsday_Insight}},
      /* 05 01:06 */ {kWeapon, 0, {kLightningBolt_EnchancedStrike, kIceShower_FrostZone}},
      /* 06 01:07 */ {kWeapon, 0, {kPunishingStrike_Destruction}},
      /* 07 01:08 */ {kWeapon, 0, {kInferno_WeakPointDetection}},
      /* 08 01:09 */ {kWeapon, 0, {kExplosion_MagickAmplification}},

      /* 09 01:10 */ {kHelmet, 0, {kSeraphicHail_Enlightnment}},
      /* 10 02:01 */ {kHelmet, 0, {kSqual_MindEnchancement}},
      /* 11 02:02 */ {kHelmet, 0, {kPunishingStrike_Paralize}},
      /* 12 02:03 */ {kHelmet, 0, {kExplosion_MindEnchancement}},
      /* 13 02:04 */ {kHelmet, 0, {kFrostsCall_MindEnchancement}},
      /* 14 02:05 */ {kHelmet, 0, {kDoomsday_FlameArea}},
      /* 15 02:06 */ {kHelmet, 0, {kLightningVortex_QuickPace, kIceShower_EnhancedStrike}},
      /* 16 02:07 */ {kHelmet, 0, {kSqual_QuickPrep}},
      /* 17 02:08 */ {kHelmet, 0, {kSeraphicHail_MindEnchancement}},
      /* 18 02:09 */ {kHelmet, 0, {kLightningBolt_QuickPrep, kInferno_Ignite}},

      /* 19 02:10 */ {kChest, 0, {kFrostsCall_MindEnchancement}},
      /* 20 03:01 */ {kChest, 0, {kExplosion_LawOfTheJungle}},
      /* 21 03:02 */ {kChest, 0, {kDoomsday_MindEnhancement}},
      /* 22 03:03 */ {kChest, 0, {kSqual_WildfireWhirlwind}},
      /* 23 03:04 */ {kChest, 0, {kRimeArrow_FrostBarrage}},
      /* 24 03:05 */ {kChest, 0, {kIceShower_FrostZone}},

      /* 25 03:06 */ {kPants, 0, {kEsotericReaction_JunglesLaw}},
      /* 26 03:07 */ {kPants, 0, {kLightningBolt_VitalPointHit}},
      /* 27 03:08 */ {kPants, 0, {kInferno_FirepowerExpansion, kFrostsCall_Enlightnment}},
      /* 28 03:09 */ {kPants, 0, {kBlaze_BlazingWildfire}},
      /* 29 03:10 */ {kPants, 0, {kInferno_Ignite}},
      /* 30 04:01 */ {kPants, 0, {kReverseGravity_JunglesLaw}},
      /* 31 04:02 */ {kPants, 0, {kSqual_MindEnchancement}},
      /* 32 04:03 */ {kPants, 0, {}},
      /* 33 04:04 */ {kPants, 0, {kSeraphicHail_MindEnchancement}},
      /* 34 04:05 */ {kPants, 0, {kLightningBolt_QuickPrep}},
      /* 35 04:06 */ {kPants, 0, {kSeraphicHail_AdditionalExplosion}},
      /* 36 04:07 */ {kPants, 0, {kSqual_WildfireWhirlwind}},
      /* 37 04:08 */ {kPants, 0, {kLightningVortex_FierceLightning}},
      /* 38 04:09 */ {kPants, 0, {kDoomsday_FlameArea}},

      /* 39 04:10 */ {kGloves, 0, {kBlaze_BlazingWildfire}},
      /* 40 05:01 */ {kGloves, 0, {kLightningBolt_QuickPrep}},
      /* 41 05:02 */ {kGloves, 0, {kLightningBolt_VitalPointHit}},
      /* 42 05:03 */ {kGloves, 0, {kSeraphicHail_WeakPointDetection}},
      /* 43 05:04 */ {kGloves, 0, {kLightningVortex_QuickPace}},
      /* 44 05:05 */ {kGloves, 0, {kIceShower_Enlightnment}},

      /* 45 05:06 */ {kShoulders, 0, {kExplosion_MindEnchancement, kEsotericReaction_StabilizedCrystal}},
      /* 46 05:07 */ {kShoulders, 0, {kDoomsday_Insight}},
      /* 47 05:08 */ {kShoulders, 0, {kLightningBolt_BranchedLightning}},
      /* 48 05:09 */ {kShoulders, 0, {kEsotericReaction_JunglesLaw}},
      /* 49 05:10 */ {kShoulders, 0, {kElegainsTouch_SwiftFootwork}},
      /* 50 06:01 */ {kShoulders, 0, {kLightningVortex_FierceLightning}},

      /* 51 08:04 */ {kPants, 0, {kFrostsCall_EnhancedStrike}},
      /* 52 08:06 */ {kShoulders, 0, {kReverseGravity_LavaArea}},
      /* 53 09:01 */ {kWeapon, 0, {kSeraphicHail_QuickPrep}},
      /* 54 09:02 */ {kHelmet, 0, {kPunishingStrike_MindEnhancement}},
      /* 55 09:03 */ {kChest, 0, {kPunishingStrike_UnavoidableFate}},
      /* 56 09:04 */ {kPants, 0, {kSeraphicHail_RedHail, kLightningVortex_MindEnhancement}},
      /* 57 09:05 */ {kGloves, 0, {kEsotericReaction_EnhancedStrike}},
      /* 58 09:06 */ {kShoulders, 0, {kInferno_FlameArea}},
      /* 59 10:01 */ {kWeapon, 0, {kEsotericReaction_StabilizedCrystal}},
      /* 60 10:02 */ {kHelmet, 0, {kInferno_FirepowerSupplement}},
      /* 61 10:03 */ {kChest, 0, {kSeraphicHail_EvolvedHail}},
      /* 62 10:04 */ {kPants, 0, {kIceShower_AgileCast}},
      /* 63 10:05 */ {kGloves, 0, {kFrostsCall_Chill, kPunishingStrike_MagickAmplification}},
      /* 64 10:06 */ {kShoulders, 0, {kIceShower_FrostFragment, kLightningVortex_MindEnhancement}},

      /* 65 07:08 */ {kWeapon, 0, {kEsotericReaction_StabilizedCrystal}},
      /* 66 07:09 */ {kHelmet, 0, {kFrostsCall_Chill}},
      /* 67 07:10 */ {kHelmet, 0, {kIceShower_AgileCast}},
      /* 68 08:08 */ {kHelmet, 0, {kInferno_FirepowerSupplement}},
      /* 69 08:09 */ {kChest, 0, {kFrostsCall_Chill}},
      /* 70 08:10 */ {kPants, 0, {kIceShower_AgileCast, kSqual_AgileCast}},
      /* 71 09:08 */ {kGloves, 0, {kLightningVortex_MindEnhancement}},
      /* 72 09:09 */ {kShoulders, 0, {kSeraphicHail_EvolvedHail}},
      /* 73 09:10 */ {kShoulders, 0, {kFrostsCall_Chill}},
      /* 74 10:08 */ {kShoulders, 0, {kInferno_FirepowerSupplement}},
  };

  Optimize(items, prio_tripods, book);
}

}  // namespace

int main(int argc, char** argv) { Main(); }
