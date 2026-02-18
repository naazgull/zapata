/*
  This is free and unencumbered software released into the public domain.
*/

/**
 * @file transport.h
 * @brief Transport layer abstraction for network protocols.
 *
 * This is the aggregate header for the transport module, providing
 * a unified interface for network communication across different protocols
 * (HTTP, TCP, WebSocket, Unix sockets, etc.).
 *
 * Key types:
 * - `zpt::basic_transport` - Abstract transport interface
 * - `zpt::network::layer` - Transport registry and content negotiation
 *
 * @see zpt::basic_transport
 * @see zpt::network::layer
 */

#pragma once

#include <zapata/transport/transport.h>
