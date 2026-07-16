/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or distribute
  this software, either in source code form or as a compiled binary, for any
  purpose, commercial or non-commercial, and by any means.

  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright interest in the software to the public
  domain. We make this dedication for the benefit of the public at large, and to
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

#include <SWI-Prolog.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

/* Helper: create an atom from a C string. */
static inline atom_t mk_atom(const char* s) { return PL_new_atom(s); }

/* Build a Prolog list from strings, tail-to-head, and unify it with `out`.
 * Every PL_new_term_ref() is matched with a PL_free_term_ref() once its
 * value has been embedded in the result — the same allocate/free symmetry
 * expected of any other resource. Returns false if construction fails
 * (e.g. a resource exception), leaving `out` untouched. */
static bool build_atom_list(term_t out, std::vector<std::string> const& items) {
    term_t cur = PL_new_term_ref();
    PL_put_nil(cur);
    for (auto it = items.rbegin(); it != items.rend(); ++it) {
        term_t cell = PL_new_term_ref();
        term_t elt = PL_new_term_ref();
        PL_put_atom_chars(elt, it->c_str());
        bool ok = PL_cons_list(cell, elt, cur);
        PL_free_term_ref(elt);
        PL_free_term_ref(cur);
        if (!ok) {
            PL_free_term_ref(cell);
            return false;
        }
        cur = cell;
    }
    bool ok = PL_put_term(out, cur);
    PL_free_term_ref(cur);
    return ok;
}

/* C callback for PL_for_dict — receives a term_t pair via void*.
 * PL_for_dict continues iterating while the callback returns FALSE;
 * a non-zero return stops iteration early and becomes its result. */
static int dict_visit_cb(term_t key, term_t value, void* ctx_raw) {
    auto* ctx = static_cast<std::pair<term_t, term_t>*>(ctx_raw);
    if (!PL_put_term(ctx->first, key) || !PL_put_term(ctx->second, value)) {
        std::cerr << "  <failed to copy dict entry>\n";
        return TRUE; /* stop iteration on error */
    }
    char* kbuf = nullptr;
    if (PL_get_atom_chars(ctx->first, &kbuf)) { std::cout << "  " << kbuf; }
    char* vbuf = nullptr;
    if (PL_get_atom_chars(ctx->second, &vbuf)) { std::cout << ": " << vbuf << "\n"; }
    else {
        int val;
        if (PL_get_integer(ctx->second, &val)) { std::cout << ": " << val << "\n"; }
    }
    return FALSE;
}

auto test_pl_integration(char* _e_arg) -> int {
    /* 0. Initialize the engine. */
    if (!PL_initialise(1, &_e_arg)) {
        std::cerr << "ERROR: PL_initialise failed\n";
        return 1;
    }

    int rc = 0;
    predicate_t call1 = PL_predicate("call", 1, "user");

    /* ================================================================
     * 1. Define facts: assertz(parent(tom, bob)) etc.
     * ================================================================ */
    struct {
        const char* x;
        const char* y;
    } parents[] = {
        { "tom", "bob" },
        { "tom", "liz" },
        { "bob", "ann" },
        { "bob", "pat" },
    };
    for (auto&& p : parents) {
        /* Build parent(x, y): PL_new_term_refs() gives a contiguous
         * block of refs, which is what PL_cons_functor_v() expects. */
        term_t args = PL_new_term_refs(2);
        PL_put_atom_chars(args + 0, p.x);
        PL_put_atom_chars(args + 1, p.y);

        term_t parent = PL_new_term_ref();
        if (!PL_cons_functor_v(parent, PL_new_functor(mk_atom("parent"), 2), args)) {
            std::cerr << "cons_functor_v(parent) failed\n";
            PL_free_term_ref(args + 1);
            PL_free_term_ref(args + 0);
            PL_free_term_ref(parent);
            rc = 1;
            goto cleanup;
        }
        PL_free_term_ref(args + 1);
        PL_free_term_ref(args + 0);

        /* assertz(parent(x, y)) */
        term_t goal = PL_new_term_ref();
        if (!PL_cons_functor_v(goal, PL_new_functor(mk_atom("assertz"), 1), parent)) {
            std::cerr << "cons_functor_v(assertz) failed\n";
            PL_free_term_ref(parent);
            PL_free_term_ref(goal);
            rc = 1;
            goto cleanup;
        }
        PL_free_term_ref(parent);

        qid_t qid = PL_open_query(nullptr, PL_Q_NORMAL, call1, goal);
        int status = PL_next_solution(qid);
        if (status == PL_S_FALSE) {
            std::cerr << "assertz(parent(" << p.x << "," << p.y << ")) failed\n";
            term_t exc = PL_exception(qid);
            if (exc) {
                char* buf = nullptr;
                if (PL_get_atom_chars(exc, &buf)) { std::cerr << "  exception: " << buf << "\n"; }
            }
            PL_close_query(qid);
            PL_free_term_ref(goal);
            rc = 1;
            goto cleanup;
        }
        PL_close_query(qid);
        PL_free_term_ref(goal);
    }

    /* ================================================================
     * 2b. Define fact with a list argument: owns(bob, [cat,dog,fish]).
     *     Also demonstrates list construction and iteration via C API.
     * ================================================================ */
    {
        term_t pets_list = PL_new_term_ref();
        if (!build_atom_list(pets_list, { "cat", "dog", "fish" })) {
            std::cerr << "failed to build pets list\n";
            PL_free_term_ref(pets_list);
            rc = 1;
            goto cleanup;
        }

        /* Iterate the list using PL_get_list */
        term_t head = PL_new_term_ref();
        term_t iter_tail = PL_new_term_ref();

        std::cout << "  [";
        if (PL_get_list(pets_list, head, iter_tail)) {
            bool first = true;
            do {
                char* buf = nullptr;
                if (PL_get_atom_chars(head, &buf)) {
                    if (!first) std::cout << ",";
                    std::cout << buf;
                    first = false;
                }
            } while (PL_get_list(iter_tail, head, iter_tail));
        }
        std::cout << "]" << std::endl;
        PL_free_term_ref(iter_tail);
        PL_free_term_ref(head);

        /* owns(bob, pets_list) */
        term_t owns_args = PL_new_term_refs(2);
        PL_put_atom_chars(owns_args + 0, "bob");
        bool owns_args_ok = PL_put_term(owns_args + 1, pets_list);
        PL_free_term_ref(pets_list);
        if (!owns_args_ok) {
            std::cerr << "failed to build owns/2 args\n";
            PL_free_term_ref(owns_args + 1);
            PL_free_term_ref(owns_args + 0);
            rc = 1;
            goto cleanup;
        }

        term_t owns = PL_new_term_ref();
        if (!PL_cons_functor_v(owns, PL_new_functor(mk_atom("owns"), 2), owns_args)) {
            std::cerr << "cons_functor_v(owns) failed\n";
            PL_free_term_ref(owns_args + 1);
            PL_free_term_ref(owns_args + 0);
            PL_free_term_ref(owns);
            rc = 1;
            goto cleanup;
        }
        PL_free_term_ref(owns_args + 1);
        PL_free_term_ref(owns_args + 0);

        /* assertz(owns(bob, [cat,dog,fish])) */
        term_t goal = PL_new_term_ref();
        if (!PL_cons_functor_v(goal, PL_new_functor(mk_atom("assertz"), 1), owns)) {
            std::cerr << "cons_functor_v(assertz) failed\n";
            PL_free_term_ref(owns);
            PL_free_term_ref(goal);
            rc = 1;
            goto cleanup;
        }
        PL_free_term_ref(owns);

        qid_t qid = PL_open_query(nullptr, PL_Q_NORMAL, call1, goal);
        if (PL_next_solution(qid) == PL_S_FALSE) {
            std::cerr << "assertz(owns(bob, [cat,dog,fish])) failed\n";
            PL_close_query(qid);
            PL_free_term_ref(goal);
            rc = 1;
            goto cleanup;
        }
        PL_close_query(qid);
        PL_free_term_ref(goal);
    }

    /* ================================================================
     * 2c. Define a fact with a dictionary argument.
     *     Demonstrates PL_put_dict — something the C++2 API cannot do.
     * ================================================================ */
    {
        /* {person| name: alice, age: 30} */
        term_t dict_term = PL_new_term_ref();
        atom_t dict_keys[2] = { mk_atom("age"), mk_atom("name") };
        term_t dict_vals = PL_new_term_refs(2);

        /* PL_put_dict requires keys sorted by the standard order of terms. */
        bool dict_ok = PL_put_integer(dict_vals + 0, 30) &&
                       PL_put_atom_chars(dict_vals + 1, "alice") &&
                       PL_put_dict(dict_term, mk_atom("person"), 2, dict_keys, dict_vals);
        PL_free_term_ref(dict_vals + 1);
        PL_free_term_ref(dict_vals + 0);
        if (!dict_ok) {
            std::cerr << "failed to build dict\n";
            PL_free_term_ref(dict_term);
            rc = 1;
            goto cleanup;
        }

        /* assertz(person(alice, 30, Dict)) */
        term_t fact_args = PL_new_term_refs(3);
        bool fact_args_ok = PL_put_atom_chars(fact_args + 0, "alice") &&
                            PL_put_integer(fact_args + 1, 30) &&
                            PL_put_term(fact_args + 2, dict_term);

        term_t fact = PL_new_term_ref();
        bool fact_ok =
          fact_args_ok && PL_cons_functor_v(fact, PL_new_functor(mk_atom("person"), 3), fact_args);
        PL_free_term_ref(fact_args + 2);
        PL_free_term_ref(fact_args + 1);
        PL_free_term_ref(fact_args + 0);
        if (!fact_ok) {
            std::cerr << "failed to build person fact\n";
            PL_free_term_ref(fact);
            PL_free_term_ref(dict_term);
            rc = 1;
            goto cleanup;
        }

        term_t goal = PL_new_term_ref();
        if (!PL_cons_functor_v(goal, PL_new_functor(mk_atom("assertz"), 1), fact)) {
            std::cerr << "cons_functor_v(assertz) failed\n";
            PL_free_term_ref(fact);
            PL_free_term_ref(goal);
            PL_free_term_ref(dict_term);
            rc = 1;
            goto cleanup;
        }
        PL_free_term_ref(fact);

        qid_t qid = PL_open_query(nullptr, PL_Q_NORMAL, call1, goal);
        if (PL_next_solution(qid) == PL_S_FALSE) {
            std::cerr << "assertz(person(alice, 30, <dict>)) failed\n";
            PL_close_query(qid);
            PL_free_term_ref(goal);
            PL_free_term_ref(dict_term);
            rc = 1;
            goto cleanup;
        }
        PL_close_query(qid);
        PL_free_term_ref(goal);

        /* Iterate the dict with PL_for_dict. */
        term_t dk = PL_new_term_ref();
        term_t dv = PL_new_term_ref();
        std::pair<term_t, term_t> dict_ctx{ dk, dv };
        PL_for_dict(dict_term, dict_visit_cb, &dict_ctx, 0);
        PL_free_term_ref(dv);
        PL_free_term_ref(dk);
        PL_free_term_ref(dict_term);
    }

    /* ================================================================
     * 3. Collect children of tom.
     * ================================================================ */
    {
        std::vector<std::string> children;

        term_t q_args = PL_new_term_refs(2);
        PL_put_atom_chars(q_args + 0, "tom");

        predicate_t pred = PL_predicate("parent", 2, "user");
        qid_t qid = PL_open_query(nullptr, PL_Q_NORMAL, pred, q_args);
        while (PL_next_solution(qid) != PL_S_FALSE) {
            char* buf = nullptr;
            if (PL_get_atom_chars(q_args + 1, &buf)) { children.push_back(buf); }
        }
        PL_close_query(qid);
        PL_free_term_ref(q_args + 1);
        PL_free_term_ref(q_args + 0);

        term_t child_list = PL_new_term_ref();
        if (!build_atom_list(child_list, children)) {
            std::cerr << "failed to build children list\n";
            PL_free_term_ref(child_list);
            rc = 1;
            goto cleanup;
        }

        /* Iterate using PL_get_list */
        term_t h = PL_new_term_ref();
        term_t t = PL_new_term_ref();
        if (PL_get_list(child_list, h, t)) {
            do {
                char* buf = nullptr;
                if (PL_get_atom_chars(h, &buf)) { std::cout << "  parent(tom, " << buf << ")\n"; }
            } while (PL_get_list(t, h, t));
        }
        PL_free_term_ref(t);
        PL_free_term_ref(h);
        PL_free_term_ref(child_list);
    }

    /* ================================================================
     * 4. Arithmetic: X is 42 + 10
     * ================================================================ */
    {
        term_t plus_args = PL_new_term_refs(2);
        bool plus_args_ok = PL_put_integer(plus_args + 0, 42) && PL_put_integer(plus_args + 1, 10);

        term_t plus_term = PL_new_term_ref();
        bool plus_ok =
          plus_args_ok && PL_cons_functor_v(plus_term, PL_new_functor(mk_atom("+"), 2), plus_args);
        PL_free_term_ref(plus_args + 1);
        PL_free_term_ref(plus_args + 0);
        if (!plus_ok) {
            std::cerr << "failed to build 42+10\n";
            PL_free_term_ref(plus_term);
            rc = 1;
            goto cleanup;
        }

        term_t x = PL_new_term_ref();
        term_t is_args = PL_new_term_refs(2);
        bool is_args_ok = PL_put_term(is_args + 0, x) && PL_put_term(is_args + 1, plus_term);
        PL_free_term_ref(plus_term);
        if (!is_args_ok) {
            std::cerr << "failed to build is/2 args\n";
            PL_free_term_ref(is_args + 1);
            PL_free_term_ref(is_args + 0);
            PL_free_term_ref(x);
            rc = 1;
            goto cleanup;
        }

        term_t goal = PL_new_term_ref();
        if (!PL_cons_functor_v(goal, PL_new_functor(mk_atom("is"), 2), is_args)) {
            std::cerr << "cons_functor_v(is) failed\n";
            PL_free_term_ref(is_args + 1);
            PL_free_term_ref(is_args + 0);
            PL_free_term_ref(goal);
            PL_free_term_ref(x);
            rc = 1;
            goto cleanup;
        }
        PL_free_term_ref(is_args + 1);
        PL_free_term_ref(is_args + 0);

        qid_t qid = PL_open_query(nullptr, PL_Q_NORMAL, call1, goal);
        if (PL_next_solution(qid) != PL_S_FALSE) {
            int val;
            if (PL_get_integer(x, &val)) { std::cout << "  42 + 10 = " << val << "\n"; }
        }
        PL_close_query(qid);
        PL_free_term_ref(goal);
        PL_free_term_ref(x);
    }

    /* ================================================================
     * 5. member/2 — check if bob is in a list
     * ================================================================ */
    {
        term_t bob_list = PL_new_term_ref();
        if (!build_atom_list(bob_list, { "tom", "bob", "ann", "pat" })) {
            std::cerr << "failed to build bob list\n";
            PL_free_term_ref(bob_list);
            rc = 1;
            goto cleanup;
        }

        term_t m_args = PL_new_term_refs(2);
        PL_put_atom_chars(m_args + 0, "bob");
        bool m_args_ok = PL_put_term(m_args + 1, bob_list);
        PL_free_term_ref(bob_list);
        if (!m_args_ok) {
            std::cerr << "failed to build member/2 args\n";
            PL_free_term_ref(m_args + 1);
            PL_free_term_ref(m_args + 0);
            rc = 1;
            goto cleanup;
        }

        term_t goal = PL_new_term_ref();
        if (!PL_cons_functor_v(goal, PL_new_functor(mk_atom("member"), 2), m_args)) {
            std::cerr << "cons_functor_v(member) failed\n";
            PL_free_term_ref(m_args + 1);
            PL_free_term_ref(m_args + 0);
            PL_free_term_ref(goal);
            rc = 1;
            goto cleanup;
        }
        PL_free_term_ref(m_args + 1);
        PL_free_term_ref(m_args + 0);

        qid_t qid = PL_open_query(nullptr, PL_Q_NORMAL, call1, goal);
        if (PL_next_solution(qid) != PL_S_FALSE) {
            std::cout << "  member(bob, [tom,bob,ann,pat]): true\n";
        }
        PL_close_query(qid);
        PL_free_term_ref(goal);
    }

    /* ================================================================
     * 6. consult/1 — load a Prolog source file.
     *    There is no PL_consult() in the C API: loading a file is a
     *    normal Prolog goal, called the same way as any other predicate.
     * ================================================================ */
    {
        /* Write a small source file to consult, so the example is
         * self-contained (no external .pl file to ship/locate). */
        char const* tmp_path = "/tmp/zapata_prolog_integration_example.pl";
        std::ofstream out(tmp_path);
        out << ":- dynamic(likes/2).\n"
            << "likes(mary, wine).\n"
            << "likes(john, beer).\n";
        out.close();

        term_t file = PL_new_term_ref();
        PL_put_atom_chars(file, tmp_path);

        predicate_t consult_pred = PL_predicate("consult", 1, "user");
        qid_t qid = PL_open_query(nullptr, PL_Q_NORMAL, consult_pred, file);
        if (PL_next_solution(qid) == PL_S_FALSE) {
            std::cerr << "consult(" << tmp_path << ") failed\n";
            PL_close_query(qid);
            PL_free_term_ref(file);
            rc = 1;
            goto cleanup;
        }
        PL_close_query(qid);
        PL_free_term_ref(file);

        term_t q_args = PL_new_term_refs(2);
        PL_put_atom_chars(q_args + 0, "mary");
        predicate_t likes_pred = PL_predicate("likes", 2, "user");
        qid = PL_open_query(nullptr, PL_Q_NORMAL, likes_pred, q_args);
        if (PL_next_solution(qid) != PL_S_FALSE) {
            char* buf = nullptr;
            if (PL_get_atom_chars(q_args + 1, &buf)) {
                std::cout << "  likes(mary, " << buf << ") — loaded from consulted file\n";
            }
        }
        PL_close_query(qid);
        PL_free_term_ref(q_args + 1);
        PL_free_term_ref(q_args + 0);

        std::remove(tmp_path);
    }

    /* ================================================================
     * 7. Parse a string holding a Prolog clause with PL_chars_to_term,
     *    then assert the resulting term with assertz/1.
     * ================================================================ */
    {
        term_t clause = PL_new_term_ref();
        if (!PL_chars_to_term("likes(susan, tea).", clause)) {
            std::cerr << "PL_chars_to_term failed to parse the clause\n";
            PL_free_term_ref(clause);
            rc = 1;
            goto cleanup;
        }

        term_t goal = PL_new_term_ref();
        if (!PL_cons_functor_v(goal, PL_new_functor(mk_atom("assertz"), 1), clause)) {
            std::cerr << "cons_functor_v(assertz) failed\n";
            PL_free_term_ref(clause);
            PL_free_term_ref(goal);
            rc = 1;
            goto cleanup;
        }
        PL_free_term_ref(clause);

        qid_t qid = PL_open_query(nullptr, PL_Q_NORMAL, call1, goal);
        if (PL_next_solution(qid) == PL_S_FALSE) {
            std::cerr << "assertz(parsed clause) failed\n";
            PL_close_query(qid);
            PL_free_term_ref(goal);
            rc = 1;
            goto cleanup;
        }
        PL_close_query(qid);
        PL_free_term_ref(goal);

        term_t q_args = PL_new_term_refs(2);
        PL_put_atom_chars(q_args + 0, "susan");
        predicate_t likes_pred = PL_predicate("likes", 2, "user");
        qid = PL_open_query(nullptr, PL_Q_NORMAL, likes_pred, q_args);
        if (PL_next_solution(qid) != PL_S_FALSE) {
            char* buf = nullptr;
            if (PL_get_atom_chars(q_args + 1, &buf)) {
                std::cout << "  likes(susan, " << buf << ") — parsed from a string clause\n";
            }
        }
        PL_close_query(qid);
        PL_free_term_ref(q_args + 1);
        PL_free_term_ref(q_args + 0);
    }

cleanup:
    PL_cleanup(rc == 0 ? 0 : 1);
    return rc;
}

auto main(int /*argc*/, char** argv) -> int { return test_pl_integration(argv[0]); }
