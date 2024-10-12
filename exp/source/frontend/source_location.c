

#include "frontend/source_location.h"
#include "adt/string.h"

SourceLocation source_location_create() {
  SourceLocation location = {
      .file = string_view_create(), .line = 0, .column = 0};
  return location;
}

SourceLocation source_location(StringView file, u64 line, u64 column) {
  SourceLocation location = {.file = file, .line = line, .column = column};
  return location;
}

bool source_location_equality(SourceLocation A, SourceLocation B) {
  if (!string_view_eq(A.file, B.file)) { return false; }
  if (A.line != B.line) { return false; }
  return A.column == B.column;
}

void print_source_location(String *restrict out, SourceLocation location) {
  string_append(out, SV("@"));
  string_append(out, location.file);
  string_append(out, SV("@"));
  string_append_u64(out, location.line);
  string_append(out, SV(":"));
  string_append_u64(out, location.column);
}
