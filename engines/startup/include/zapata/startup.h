/*
  This is free and unencumbered software released into the public domain.
*/

/**
 * @file startup.h
 * @brief Application startup and plugin management.
 *
 * Provides application lifecycle management, configuration loading,
 * and dynamic plugin loading.
 *
 * Key types:
 * - `zpt::plugin` - Plugin wrapper
 * - `zpt::startup::boot` - Application boot manager
 *
 * @see zpt::startup::boot
 * @see zpt::BOOT
 */

#pragma once

#include <zapata/globals.h>
#include <zapata/startup/configuration.h>
#include <zapata/startup/startup.h>
