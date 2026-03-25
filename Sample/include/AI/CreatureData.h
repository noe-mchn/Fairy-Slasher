#pragma once

#include <cstdint>

enum class CreatureType : uint8_t
{
    Souls,
    Fairy,
    RareSoul,
    RareFairy
};

struct CreatureData
{
    CreatureType type = CreatureType::Souls;
    float respirationTime = 5.0f;
    float infusionTime = 10.0f;
    const char* lootName = "Souls Essences";
    int lootChance = 60;
    float moveSpeed = 2.0f;
    float detectionRange = 8.0f;
    float attackRange = 1.5f;
    float health = 100.0f;
    float maxHealth = 100.0f;
    bool flees = false;

    static CreatureData MakeSouls()
    {
        CreatureData dataSoul;
        dataSoul.type = CreatureType::Souls;
        dataSoul.respirationTime = 5.0f;
        dataSoul.infusionTime = 10.0f;
        dataSoul.lootName = "Souls Essences";
        dataSoul.lootChance = 60;
        dataSoul.moveSpeed = 3.0f;
        dataSoul.detectionRange = 10.0f;
        dataSoul.attackRange = 1.5f;
        dataSoul.health = 80.0f;
        dataSoul.maxHealth = 80.0f;
        dataSoul.flees = false;
        return dataSoul;
    }

    static CreatureData MakeFairy()
    {
        CreatureData dataFairy;
        dataFairy.type = CreatureType::Fairy;
        dataFairy.respirationTime = 20.0f;
        dataFairy.infusionTime = 20.0f;
        dataFairy.lootName = "Fairy Wings";
        dataFairy.lootChance = 40;
        dataFairy.moveSpeed = 4.0f;
        dataFairy.detectionRange = 12.0f;
        dataFairy.attackRange = 2.0f;
        dataFairy.health = 60.0f;
        dataFairy.maxHealth = 60.0f;
        dataFairy.flees = true;
        return dataFairy;
    }

    static CreatureData MakeRareSoul()
    {
        CreatureData dataSoulR;
        dataSoulR.type = CreatureType::RareSoul;
        dataSoulR.respirationTime = 10.0f;
        dataSoulR.infusionTime = 15.0f;
        dataSoulR.lootName = "Rare Souls Essences";
        dataSoulR.lootChance = 25;
        dataSoulR.moveSpeed = 2.5f;
        dataSoulR.detectionRange = 14.0f;
        dataSoulR.attackRange = 2.0f;
        dataSoulR.health = 150.0f;
        dataSoulR.maxHealth = 150.0f;
        dataSoulR.flees = false;
        return dataSoulR;
    }

    static CreatureData MakeRareFairy()
    {
        CreatureData dataFairyR;
        dataFairyR.type = CreatureType::RareFairy;
        dataFairyR.respirationTime = 30.0f;
        dataFairyR.infusionTime = 30.0f;
        dataFairyR.lootName = "Rare Fairy Wings";
        dataFairyR.lootChance = 10;
        dataFairyR.moveSpeed = 5.0f;
        dataFairyR.detectionRange = 18.0f;
        dataFairyR.attackRange = 2.0f;
        dataFairyR.health = 40.0f;
        dataFairyR.maxHealth = 40.0f;
        dataFairyR.flees = true;
        return dataFairyR;
    }
};
