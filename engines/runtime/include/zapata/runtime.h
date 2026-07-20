#pragma once

namespace zpt {
namespace runtime {
auto initialize(int _argc, char** _argv) -> void;
auto shutdown() -> void;
} // namespace runtime
} // namespace zpt
