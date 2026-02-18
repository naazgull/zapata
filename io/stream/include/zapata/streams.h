/*
  This is free and unencumbered software released into the public domain.
*/

/**
 * @file streams.h
 * @brief I/O stream abstractions and epoll-based polling.
 *
 * This is the aggregate header for the streams module, providing
 * abstract stream types that wrap various I/O sources (sockets, pipes)
 * and an epoll-based polling mechanism for efficient I/O multiplexing.
 *
 * Key types:
 * - `zpt::basic_stream` - Abstract stream wrapper
 * - `zpt::polling` - Epoll-based I/O multiplexer
 *
 * @see zpt::basic_stream
 * @see zpt::polling
 */

#pragma once

#include <zapata/streams/event_stream.h>
#include <zapata/streams/streams.h>
