#pragma once

class Level;
class Random;
class Biome;
struct AxoBlockSpawnDef;

void AxoWorldGen_RegisterSpawn(int blockId, const AxoBlockSpawnDef& spawn);
void AxoWorldGen_Decorate(Level* level, Random* random, Biome* biome, int xo, int zo);
