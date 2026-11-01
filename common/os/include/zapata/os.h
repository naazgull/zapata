/*
  This is free and unencumbered software released into the public domain.
*/

/**
 * @file os.h
 * @brief Aggregate header for the os module.
 *
 * Operating-system introspection utilities. Include this single header to
 * access the module's public API.
 *
 * Key functions:
 * - `zpt::ps` - list running processes, optionally filtered by uid and/or gid
 *
 * @see zpt::ps
 */

#pragma once

#include <zapata/os/ps.h>
