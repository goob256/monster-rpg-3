#ifndef STATS_H
#define STATS_H

#include <Nooskewl_Wedge/stats.h>

enum Monster_RPG_3_Status_Condition {
	STATUS_BLIND = wedge::STATUS_SIZE
};

enum Extra_Stats {
	LUCK = 0
};

enum Weak_Strong
{
	WEAK_STRONG_FIRE = wedge::WEAK_STRONG_SIZE,
	WEAK_STRONG_BOLT,
	WEAK_STRONG_ICE,
	WEAK_STRONG_MELEE
};

#endif // STATS_H
