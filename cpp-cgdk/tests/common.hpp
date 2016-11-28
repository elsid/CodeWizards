#ifndef STRATEGY_COMMON
#define STRATEGY_COMMON

#include "model/Wizard.h"
#include "model/Game.h"

#include <cmath>

namespace strategy {
namespace tests {

static const model::Wizard SELF(
    1, // Id
    1000, // X
    1000, // Y
    0, // SpeedX
    0, // SpeedY
    M_PI / 4, // Angle
    model::FACTION_ACADEMY, // Faction
    35, // Radius
    100, // Life
    100, // MaxLife
    {}, // Statuses
    1, // OwnerPlayerId
    true, // Me
    100, // Mana
    100, // MaxMana
    600, // VisionRange
    500, // CastRange
    0, // Xp
    0, // Level
    {}, // Skills
    0, // RemainingActionCooldownTicks
    {0, 0, 0, 0, 0, 0, 0}, // RemainingCooldownTicksByAction
    true, // Master
    {} // Messages
);

static const model::Game GAME(
    7018926670356416392, // RandomSeed
    20000, // TickCount
    4000, // MapSize
    0, // SkillsEnabled
    0, // RawMessagesEnabled
    0.25, // FriendlyFireDamageFactor
    0.25, // BuildingDamageScoreFactor
    0.25, // BuildingEliminationScoreFactor
    0, // MinionDamageScoreFactor
    0.25, // MinionEliminationScoreFactor
    0.25, // WizardDamageScoreFactor
    1, // WizardEliminationScoreFactor
    1.67, // TeamWorkingScoreFactor
    1000, // VictoryScore
    600, // ScoreGainRange
    1024, // RawMessageMaxLength
    2, // RawMessageTransmissionSpeed
    35, // WizardRadius
    500, // WizardCastRange
    600, // WizardVisionRange
    4, // WizardForwardSpeed
    3, // WizardBackwardSpeed
    3, // WizardStrafeSpeed
    100, // WizardBaseLife
    10, // WizardLifeGrowthPerLevel
    100, // WizardBaseMana
    10, // WizardManaGrowthPerLevel
    0.05, // WizardBaseLifeRegeneration
    0.005, // WizardLifeRegenerationGrowthPerLevel
    0.2, // WizardBaseManaRegeneration
    0.02, // WizardManaRegenerationGrowthPerLevel
    0.10472, // WizardMaxTurnAngle
    2400, // WizardMaxResurrectionDelayTicks
    1200, // WizardMinResurrectionDelayTicks
    30, // WizardActionCooldownTicks
    60, // StaffCooldownTicks
    60, // MagicMissileCooldownTicks
    90, // FrostBoltCooldownTicks
    120, // FireballCooldownTicks
    120, // HasteCooldownTicks
    120, // ShieldCooldownTicks
    12, // MagicMissileManacost
    36, // FrostBoltManacost
    48, // FireballManacost
    48, // HasteManacost
    48, // ShieldManacost
    12, // StaffDamage
    0.523599, // StaffSector
    70, // StaffRange
    {}, // LevelUpXpValues
    25, // MinionRadius
    400, // MinionVisionRange
    3, // MinionSpeed
    0.10472, // MinionMaxTurnAngle
    100, // MinionLife
    750, // FactionMinionAppearanceIntervalTicks
    60, // OrcWoodcutterActionCooldownTicks
    12, // OrcWoodcutterDamage
    0.523599, // OrcWoodcutterAttackSector
    50, // OrcWoodcutterAttackRange
    30, // FetishBlowdartActionCooldownTicks
    300, // FetishBlowdartAttackRange
    0.523599, // FetishBlowdartAttackSector
    20, // BonusRadius
    2500, // BonusAppearanceIntervalTicks
    200, // BonusScoreAmount
    5, // DartRadius
    50, // DartSpeed
    6, // DartDirectDamage
    10, // MagicMissileRadius
    40, // MagicMissileSpeed
    12, // MagicMissileDirectDamage
    15, // FrostBoltRadius
    35, // FrostBoltSpeed
    24, // FrostBoltDirectDamage
    20, // FireballRadius
    30, // FireballSpeed
    20, // FireballExplosionMaxDamageRange
    100, // FireballExplosionMinDamageRange
    24, // FireballExplosionMaxDamage
    12, // FireballExplosionMinDamage
    50, // GuardianTowerRadius
    600, // GuardianTowerVisionRange
    500, // GuardianTowerLife
    600, // GuardianTowerAttackRange
    36, // GuardianTowerDamage
    240, // GuardianTowerCooldownTicks
    100, // FactionBaseRadius
    800, // FactionBaseVisionRange
    1000, // FactionBaseLife
    800, // FactionBaseAttackRange
    48, // FactionBaseDamage
    240, // FactionBaseCooldownTicks
    240, // BurningDurationTicks
    24, // BurningSummaryDamage
    2400, // EmpoweredDurationTicks
    2, // EmpoweredDamageFactor
    60, // FrozenDurationTicks
    600, // HastenedDurationTicks
    4, // HastenedBonusDurationFactor
    0.3, // HastenedMovementBonusFactor
    0.5, // HastenedRotationBonusFactor
    600, // ShieldedDurationTicks
    4, // ShieldedBonusDurationFactor
    0.25, // ShieldedDirectDamageAbsorptionFactor
    500, // AuraSkillRange
    25, // RangeBonusPerSkillLevel
    1, // MagicalDamageBonusPerSkillLevel
    3, // StaffDamageBonusPerSkillLevel
    0.05, // MovementBonusFactorPerSkillLevel
    1 // MagicalDamageAbsorptionPerSkillLevel
);

}
}

#endif
