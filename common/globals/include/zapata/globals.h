/*
Copyright (c) 2016
*/

/**
 * @file globals.h
 * @brief Aggregate header for thread-local variable infrastructure.
 *
 * Includes thread-local variable management, cached values with
 * per-thread copies, and the underlying thread-local table.
 *
 * @see zpt::thread_local_variable
 * @see zpt::cached
 */

#pragma once

#include <zapata/globals/cached.h>
#include <zapata/globals/globals.h>
#include <zapata/globals/thread_local_variable.h>
