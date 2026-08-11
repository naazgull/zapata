/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or distribute
  this software, either in source code form or as a compiled binary, for any
  purpose, commercial or non-commercial, and by any means.

  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright interest in the software to the public
  domain. We make this dedication for the benefit of the public at large and to
  the detriment of our heirs and successors. We intend this dedication to be an
  overt act of relinquishment in perpetuity of all present and future rights to
  this software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/**
 * @file translate.h
 * @brief MongoDB result handling and BSON/JSON translation utilities.
 *
 * Provides utilities for converting between bsoncxx documents and zpt::json,
 * and for building BSON filter, update, projection, and sort documents.
 */

#pragma once

#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/document/value.hpp>
#include <bsoncxx/document/view.hpp>
#include <zapata/json.h>

namespace zpt {
namespace storage {
namespace mongodb {

/** @brief Converts a zpt::json object to a bsoncxx document value.
 * @param _doc JSON object to convert
 * @return bsoncxx document value representing the converted JSON
 */
auto to_bson(zpt::json _doc) -> bsoncxx::document::value;

/** @brief Converts a bsoncxx document view to a zpt::json object.
 * @param _doc BSON document view to convert.
 * @return JSON object representing the BSON document. */
auto from_bson(bsoncxx::document::view _doc) -> zpt::json;

/**
 * @brief Converts a zpt::json filter to a bsoncxx document value.
 * @param _filter Filter JSON object to convert.
 * @return bsoncxx document value representing the filter.
 *
 * An undefined or non-object filter produces an empty document (match all).
 */
auto to_filter(zpt::json _filter) -> bsoncxx::document::value;

/**
 * @brief Converts a zpt::json update object to a MongoDB update document.
 *
 * Defined values go into `$set`, undefined values go into `$unset`.
 * @param _to_update JSON object containing update fields with defined/undefined values
 * @return bsoncxx document value representing the MongoDB update document
 */
auto to_update_doc(zpt::json _to_update) -> bsoncxx::document::value;

/** @brief Converts a zpt::json field list to a MongoDB projection document.
 * @param _fields JSON object or array specifying which fields to include/exclude
 * @return bsoncxx document value representing the MongoDB projection document
 */
auto to_projection(zpt::json _fields) -> bsoncxx::document::value;

/** @brief Converts a zpt::json sort spec to a MongoDB sort document.
 * @param _sort JSON sort specification (field -> direction map).
 * @return BSON document value representing the sort document. */
auto to_sort(zpt::json _sort) -> bsoncxx::document::value;

/** @brief Appends a single zpt::json value into a bsoncxx document builder under key.
 * @param _doc Reference to the BSON document builder.
 * @param _key Key to store the value under.
 * @param _value JSON value to append.
 * @return void */
auto append_value(bsoncxx::builder::basic::document& _doc,
                  std::string const& _key,
                  zpt::json _value) -> void;

} // namespace mongodb
} // namespace storage
} // namespace zpt
