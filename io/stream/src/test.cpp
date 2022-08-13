#include <zapata/streams.h>
#include <fstream>

auto
main(int argc, char* argv[]) -> int {
    if (argc > 1) {
        auto _file = zpt::make_stream<std::fstream>(
          argv[1], std::ios_base::in | std::ios_base::out | std::ios_base::app);
        if (stream_cast<std::fstream>(_file).is_open()) {
            std::string _s;
            *_file >> _s;
            *_file << std::endl << _s << std::flush;
            stream_cast<std::fstream>(_file).close();
        }
    }
    return 0;
}
