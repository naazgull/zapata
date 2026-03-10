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

#include <zapata/streams/event_stream.h>

constexpr unsigned short FINAL_REPLY_RECEIVED{ 2 };

zpt::event_stream::event_stream() {
    this->__fd = eventfd(0, EFD_SEMAPHORE | EFD_NONBLOCK);
    this->__uri = std::format("self://fd@{}", this->__fd);
}

zpt::event_stream::~event_stream() {}

auto zpt::event_stream::operator=(int) -> zpt::event_stream& { return (*this); }

auto zpt::event_stream::operator<<(ostream_manipulator) -> zpt::event_stream& { return (*this); }

auto zpt::event_stream::read_without_io(std::any& _out) -> zpt::event_stream& {
    _out = this->__content;
    std::uint64_t _val;
    eventfd_read(this->__fd, &_val);
    ++this->__reads;
    return (*this);
}

auto zpt::event_stream::write_without_io(std::any const& _in) -> zpt::event_stream& {
    this->__content = _in;
    eventfd_write(this->__fd, 1);
    return (*this);
}

auto zpt::event_stream::persistent() -> bool { return this->__reads != FINAL_REPLY_RECEIVED; }
