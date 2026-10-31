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

#include <assert.h>
#include <errno.h>
#include <malloc.h>
#include <zapata/streams/streams.h>

zpt::stream_set::stream_set(size_t _capacity)
  : __capacity{ _capacity }
  , __by_fd{ std::make_unique<zpt::stream[]>(_capacity) }
  , __by_fd_id{ std::make_unique<std::atomic<__uint128_t>[]>(_capacity) }
  , __available_indexes{ _capacity } {
    for (size_t _idx = 0; _idx != this->__capacity; ++_idx) {
        this->__available_indexes.push(_idx);
    }
}

zpt::stream_set::~stream_set() { this->close(); }

auto zpt::stream_set::insert(zpt::stream _stream) -> bool {
    auto _index = this->__available_indexes.pop();
    if (_index == nullptr) { return false; }
    _stream->__index = *_index;

    if (this->release(_stream)) {
        this->__by_fd[_stream->__index] = _stream;
        std::unique_lock _sentry{ this->__by_mutex };
        this->__by_uuid.insert(std::make_pair(_stream->uuid(), _stream));
        this->__by_uri.insert(std::make_pair(_stream->uri(), _stream));
        return true;
    }

    auto _unavailable = _stream->__index;
    _stream->__index = stream_index_null;
    this->__available_indexes.push(_unavailable);
    return false;
}

auto zpt::stream_set::erase(zpt::stream _stream) -> bool {
    if (this->acquired(_stream)) {
        this->__by_fd[_stream->__index].reset();
        {
            std::unique_lock _sentry{ this->__by_mutex };
            this->__by_uuid.erase(_stream->uuid());
            this->__by_uri.erase(_stream->uri());
        }

        this->__available_indexes.push(_stream->__index);
        _stream->__index = stream_index_null;
        return true;
    }

    return false;
}

auto zpt::stream_set::upgrade(zpt::stream _stream, std::string const& _transport)
  -> zpt::stream_set& {
    std::unique_lock _sentry{ this->__by_mutex };
    if (this->__by_uri.erase(_stream->uri()) == 1) {
        _stream->upgrade(_transport);
        this->__by_uri.insert(std::make_pair(_stream->uri(), _stream));
    }
    return (*this);
}

auto zpt::stream_set::acquire(zpt::stream _stream) -> bool {
    if (_stream->__index == stream_index_null) { return false; }
    return this->acquire(_stream->__index, static_cast<__uint128_t>(_stream->uuid())) != nullptr;
}

auto zpt::stream_set::acquire(zpt::epoll_event_data_t const& _data) -> zpt::stream {
    return this->acquire(_data.__index, _data.__uuid);
}

auto zpt::stream_set::acquired(zpt::stream _stream) const -> bool {
    return this->__by_fd_id[_stream->__index].load() == 0;
}

auto zpt::stream_set::release(zpt::stream _stream) -> bool {
    if (_stream->__index == stream_index_null) { return false; }

    __uint128_t _uuid = static_cast<__uint128_t>(_stream->uuid());
    __uint128_t _empty{ 0 };
    if (this->__by_fd_id[_stream->__index].compare_exchange_strong(
          _empty, _uuid, std::memory_order_release)) {
        return true;
    }

    return false;
}

auto zpt::stream_set::get(zpt::uuid const& _stream_id) const -> zpt::stream {
    std::shared_lock _sentry{ this->__by_mutex };
    auto _found = this->__by_uuid.find(_stream_id);
    if (_found == this->__by_uuid.end()) { return nullptr; }
    return _found->second;
}

auto zpt::stream_set::get(std::string const& _uri) const -> zpt::stream {
    std::shared_lock _sentry{ this->__by_mutex };
    auto _found = this->__by_uri.find(_uri);
    if (_found == this->__by_uri.end()) { return nullptr; }
    return _found->second;
}

auto zpt::stream_set::close() -> zpt::stream_set& {
    std::unique_lock _sentry{ this->__by_mutex };
    for (size_t _idx = 0; _idx != this->__capacity; ++_idx) {
        if (this->__by_fd[_idx] != nullptr) {
            this->__by_fd[_idx]->close();
            this->__by_fd[_idx].reset();
        }
    }
    this->__by_uuid.clear();
    this->__by_uri.clear();
    return (*this);
}

auto zpt::stream_set::acquire(size_t _index, __uint128_t _uuid) -> zpt::stream {
    if (_index == stream_index_null || _uuid == 0) { return nullptr; }

    if (this->__by_fd_id[_index].compare_exchange_strong(_uuid, 0, std::memory_order_acq_rel)) {
        if (static_cast<__uint128_t>(this->__by_fd[_index]->uuid()) == _uuid) {
            auto _stream = this->__by_fd[_index];
            return _stream;
        }
        else { return nullptr; }
    }

    return nullptr;
}
