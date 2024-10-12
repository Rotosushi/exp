

#include "env/source_locations.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/unreachable.h"

static LocationRegion
location_region(u64 begin, u64 end, SourceLocation location) {
  LocationRegion region = {.begin = begin, .end = end, .location = location};
  return region;
}

SourceLocations source_locations_create() {
  SourceLocations source_locations = {
      .size = 0, .capacity = 0, .buffer = nullptr};
  return source_locations;
}

void source_locations_destroy(SourceLocations *restrict source_locations) {
  source_locations->size     = 0;
  source_locations->capacity = 0;
  deallocate(source_locations->buffer);
  source_locations->buffer = nullptr;
}

static bool source_locations_full(SourceLocations *restrict source_locations) {
  return (source_locations->size + 1) >= source_locations->capacity;
}

static void source_locations_grow(SourceLocations *restrict source_locations) {
  Growth g                 = array_growth_u64(source_locations->capacity,
                              sizeof(*source_locations->buffer));
  source_locations->buffer = reallocate(source_locations->buffer, g.alloc_size);
  source_locations->capacity = g.new_capacity;
}

void source_locations_insert(SourceLocations *restrict source_locations,
                             SourceLocation location,
                             u64 Idx) {
  if (source_locations->buffer == nullptr) {
    source_locations_grow(source_locations);
    source_locations->buffer[0] = location_region(Idx, Idx, location);
    return;
  }

  // we know that the regions grow contiguously and there can be no gaps
  // because each instruction is associated with a source location on creation
  // thus we only have to search the last region in the list to see if this
  // new instruction is associated with the same SourceLocation
  LocationRegion *current =
      source_locations->buffer + (source_locations->size - 1);
  if (source_location_equality(current->location, location)) {
    // add this new instruction to the current region
    current->end = Idx;
    return;
  }

  // create a new region for this instruction to reside in
  if (source_locations_full(source_locations)) {
    source_locations_grow(source_locations);
  }

  source_locations->buffer[source_locations->size++] =
      location_region(Idx, Idx, location);
}

static bool in_region(LocationRegion *restrict region, u64 Idx) {
  return (region->begin <= Idx) && (region->end >= Idx);
}

SourceLocation
source_locations_lookup(SourceLocations *restrict source_locations, u64 Idx) {
  for (u64 i = 0; i < source_locations->size; ++i) {
    LocationRegion *region = source_locations->buffer + i;
    if (in_region(region, Idx)) { return region->location; }
  }
  EXP_UNREACHABLE;
}
