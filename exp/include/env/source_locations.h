#ifndef EXP_ENV_SOURCE_LOCATIONS_H
#define EXP_ENV_SOURCE_LOCATIONS_H

#include "frontend/source_location.h"

typedef struct LocationRegion {
  u64 begin;
  u64 end;
  SourceLocation location;
} LocationRegion;

typedef struct SourceLocations {
  u64 size;
  u64 capacity;
  LocationRegion *buffer;
} SourceLocations;

SourceLocations source_locations_create();
void source_locations_destroy(SourceLocations *restrict source_locations);

void source_locations_insert(SourceLocations *restrict source_locations,
                             SourceLocation location,
                             u64 Idx);

SourceLocation
source_locations_lookup(SourceLocations *restrict source_locations, u64 Idx);

#endif // EXP_ENV_SOURCE_LOCATIONS_H
