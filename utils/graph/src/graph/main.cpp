#include <zapata/graph.h>
#include <zapata/json.h>
#include <zapata/uri.h>

auto
main(int argc, char* argv[]) -> int {
    try {
        std::vector<std::string> _services;
        std::vector<std::string> _base{
            "{([^:]+)}://users/{([^/]+)}/password",
            "{([^:]+)}://accounts/{([^/]+)}/info/{([^/]+)}",
            "http://users/{([^/]+)}/login/{([^/]+)}/password/{([^/]+)}",
            "{([^:]+)}://users/{([^/]+)}/documents",
            "{([^:]+)}://users/{([^/]+)}/documents/{([^/]+)}/text/{([^/]+)}/paragraph/{([^/]+)}"
        };

        for (size_t _idx = 0; _idx != 200; ++_idx) {
            for (auto _b : _base) {
                _services.push_back(zpt::r_replace(
                  _b, "://", std::string{ "://" } + std::to_string(_idx) + std::string{ "/" }));
            }
        }

        auto _t1 = std::chrono::high_resolution_clock::now();
        zpt::tree::node<zpt::json, zpt::regex, std::function<bool(zpt::json)>> _root;
        for (auto _s : _services) {
            std::string _p{ zpt::r_replace(zpt::r_replace(_s, "{", ""), "}", "") };
            zpt::regex _r{ _p };
            zpt::json _service = zpt::uri::to_regex(zpt::uri::parse(_s, zpt::JSArray));
            _root.merge(_service.begin(), _service.end(), _r, [](zpt::json _j) -> bool {
                std::cout << static_cast<std::string>(_j) << std::endl << std::flush;
                return true;
            });
        }
        auto _t2 = std::chrono::high_resolution_clock::now();

        std::cout << "finished adding " << _services.size() << " elements in "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(_t2 - _t1).count()
                  << "ms." << std::endl
                  << std::flush;

        auto _t3 = std::chrono::high_resolution_clock::now();
        std::string _u{ "http://100/users/20/documents/10/text/!/paragraph/200" };
        zpt::json _uri = zpt::uri::parse(_u, zpt::JSArray);
        _root.eval(_uri.begin(), _uri.end(), _u, zpt::json{ "body", { "x", 1, "y", 2 } });
        auto _t4 = std::chrono::high_resolution_clock::now();

        std::cout << "finished processing the url in "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(_t4 - _t3).count()
                  << "ms." << std::endl
                  << std::flush;
    }
    catch (zpt::missed_expectation& _e) {
        std::cout << "error: " << _e.what() << std::endl << std::flush;
    }
    return 0;
}
