#ifndef MEOW_PARSER_H
#define MEOW_PARSER_H

#include "pufu/trinity.h"

// Load a Meow UI file and create nodes
// Returns the root NodeID (usually a wrapper or the first element)
NodeID trinity_load_meow(const char *filename);

#endif // MEOW_PARSER_H
