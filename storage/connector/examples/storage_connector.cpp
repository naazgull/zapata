#include <zapata/connector.h>

auto quote_value(zpt::json const& _to_quote) -> std::string {
    if (_to_quote == "null") { return _to_quote; }
    return std::format("'{}'", static_cast<std::string>(_to_quote));
}

auto quote_name(std::string const& _to_quote) -> std::string {
    return std::format("\"{}\"", _to_quote);
}

auto main(int, char**) -> int {
    zpt::json _find = { "a", "{.gt(+(1,2,*(2,4))).}",
                        "b", "{.lt(double(0.034)).}",
                        "c", "{.boolean(true).}",
                        "d", 0.1,
                        "e", true,
                        "f", "hello",
                        "g", "{.date(\"2022-02-20T16:28:01.000\").}",
                        "h", "{.is(not(null)).}",
                        "i", "{.not(in(1, 2, 3)).}",
                        "j", "{.in(a, b, c).}",
                        "k", "{.not(is(null)).}" };
    std::cout << zpt::storage::extract_find({ quote_value, quote_name }, _find) << std::endl;
}
