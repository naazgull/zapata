/*
  Zapata project <https://github.com/naazgull/zapata>
  Author: n@zgul <n@zgul.me>

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**
 * @file exceptions.h
 * @brief Aggregate header for all Zapata exception types.
 *
 * Include this header to access all exception types and the expect() macro.
 *
 * @par Exception Hierarchy
 * - zpt::exception (base)
 *   - zpt::ExpectationException - Failed expect() assertions
 *   - zpt::SyntaxErrorException - Parser syntax errors
 *   - zpt::NoMoreElementsException - Empty collection access
 *   - zpt::NoSpaceAvailableException - Full collection write access
 *   - zpt::ClosedException - Operations on closed resources
 *   - zpt::CastException - Type conversion failures
 *   - zpt::InterruptedException - Interrupted operations
 *   - zpt::ParserEOF - Unexpected end-of-file
 *   - zpt::NoAttributeNameException - Missing attribute names
 *   - zpt::NoHeaderNameException - Missing HTTP header names
 */

#pragma once

#include <zapata/base/expect.h>
#include <zapata/exceptions/CastException.h>
#include <zapata/exceptions/ClosedException.h>
#include <zapata/exceptions/Exception.h>
#include <zapata/exceptions/InterruptedException.h>
#include <zapata/exceptions/NoAttributeNameException.h>
#include <zapata/exceptions/NoMoreElementsException.h>
#include <zapata/exceptions/NoSpaceAvailableException.h>
#include <zapata/exceptions/ParserEOF.h>
#include <zapata/exceptions/SyntaxErrorException.h>
