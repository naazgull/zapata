/*
  This is free and unencumbered software released into the public domain.
*/

/**
 * @file ontology.h
 * @brief Message and performative types for protocol-agnostic communication.
 *
 * This is the aggregate header for the ontology module, which provides
 * abstract message types and performatives (HTTP-like methods) that work
 * across different transport protocols.
 *
 * Key types:
 * - `zpt::performative` - Request methods (Get, Post, Put, Delete, etc.)
 * - `zpt::basic_message` - Abstract message interface
 * - `zpt::json_message` - JSON-based message implementation
 *
 * @par Example
 * @code
 * #include <zapata/ontology.h>
 *
 * // Create a JSON message
 * auto msg = zpt::make_message<zpt::json_message>();
 * msg->performative(zpt::Get);
 * msg->uri("/api/users");
 * msg->headers("Accept", "application/json");
 * @endcode
 *
 * @see zpt::basic_message
 * @see zpt::performative
 */

#pragma once

#include <zapata/ontology/message.h>
#include <zapata/ontology/performative.h>
