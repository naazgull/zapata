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

#include <SWI-cpp2.h>
#include <iostream>
#include <string>
#include <vector>
#include <zapata/prolog.h>

auto test_pl_integration(char* _e_arg) -> int {
    try {
        // 1. Initialize the engine
        PlEngine e(_e_arg);

        // Helper: create an atom from a C string
        auto a = [](const char* s) { return PlTerm_atom(s); };

        // 2. Define facts: assertz(parent(tom, bob)) etc.
        for (auto&& [x, y] : {
               std::pair{ "tom", "bob" },
               std::pair{ "tom", "liz" },
               std::pair{ "bob", "ann" },
               std::pair{ "bob", "pat" },
             }) {
            PlCompound parent("parent", PlTermv{ a(x), a(y) });
            std::cout << "!!!!! " << zpt::prolog::to_json(parent) << std::endl;
            PlTermv args{ parent };
            PlQuery q("assertz", args);
            if (!q.next_solution()) {
                std::cerr << "assertz(parent(" << x << "," << y << ")) failed\n";
                return 1;
            }
        }

        // 2b. Define fact with a list argument
        {
            PlTerm_var pets_list;
            PlTerm_tail l(pets_list);
            (void)l.append(a("cat"));
            (void)l.append(a("dog"));
            (void)l.append(a("fish"));
            (void)l.close();

            std::cout << "!!!!! " << zpt::prolog::to_json(pets_list) << std::endl;

            PlTerm_tail t{ pets_list };
            PlTerm_var v;
            std::cout << "  [";
            while (t.next(v)) { std::cout << v.as_string() << ","; }
            (void)t.close();
            std::cout << "]" << std::endl;

            PlTermv args{ PlCompound("owns", PlTermv{ a("bob"), pets_list }) };
            PlQuery q("assertz", args);
            if (!q.next_solution()) {
                std::cerr << "assertz(owns(bob, [cat,dog,fish])) failed\n";
                return 1;
            }
        }

        // 3. Collect children of tom.
        //    PL_close_query() rolls back any bindings made inside the query frame
        //    (trail unwind), so we must save results as C++ values while the
        //    query is still open, then build the Prolog list afterwards.
        {
            std::vector<std::string> children;

            PlTerm_var X;
            PlTermv args{ a("tom"), X };
            PlQuery q("parent", args);
            while (q.next_solution()) { children.push_back(args[1].as_string()); }

            // Build the result list and iterate it — all outside the query frame
            PlTerm_var child_list;
            PlTerm_tail l(child_list);
            for (auto const& child : children) { (void)l.append(PlTerm_atom(child)); }
            (void)l.close();

            // Iterate using PlTerm_tail
            PlTerm_tail tail(child_list);
            PlTerm_var e;
            while (tail.next(e)) { std::cout << "  parent(tom, " << e.as_string() << ")\n"; }
        }

        // 4. Arithmetic: X is 42 + 10
        {
            PlTerm_var x;
            PlCompound plus("+", PlTermv{ PlTerm_integer(42), PlTerm_integer(10) });
            PlTermv args{ x, plus };
            PlQuery q("is", args);
            if (q.next_solution()) { std::cout << "  42 + 10 = " << x.as_string() << "\n"; }
        }

        // 5. member/2 — check if bob is in a list
        {
            PlTerm_var bob_list;
            PlTerm_tail l(bob_list);
            for (auto&& name : { "tom", "bob", "ann", "pat" }) { (void)l.append(a(name)); }
            (void)l.close();

            PlTermv args{ a("bob"), bob_list };
            PlQuery q("member", args);
            if (q.next_solution()) { std::cout << "  member(bob, [tom,bob,ann,pat]): true\n"; }
        }
    }
    catch (PlFail const&) {
        // PlFail is normal control flow in C++2 — thrown by next_solution()/next()
        // on no more solutions. Only unexpected here.
        std::cerr << "ERROR: unexpected PlFail\n";
        return 1;
    }
    catch (PlException const& ex) {
        std::cerr << "ERROR: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}

auto main(int /*argc*/, char** argv) -> int { test_pl_integration(argv[0]); }
