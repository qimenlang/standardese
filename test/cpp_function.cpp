// Copyright (C) 2016 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <standardese/cpp_function.hpp>

#include <catch.hpp>
#include <standardese/detail/parse_utils.hpp>
#include <standardese/cpp_class.hpp>

#include "test_parser.hpp"

using namespace standardese;

std::size_t no_parameters(const cpp_function_base& base)
{
    std::size_t result = 0;
    for (auto& e : base.get_parameters())
        ++result;
    return result;
}

TEST_CASE("cpp_function", "[cpp]")
{
    parser p;

    auto code = R"(void a(int x, const char *ptr = nullptr);

int b(int c, ...) try
{
    return 0;
}
catch (...)
{
    return 1;
}

int *c(int a = b(0)) = delete;

[[foo]] char & __attribute__((d)) d() noexcept [[bar]];

const int e() noexcept(false);

int (*f(int a))(volatile char&&);

constexpr auto g() -> const char&&;

auto h() noexcept(noexcept(e()))
{
    return 0;
}

auto i() -> decltype(d() = '0');)";

    auto tu = parse(p, "cpp_function", code);

    // no need to check the parameters, same code as for variables
    auto count = 0u;
    for_each(tu.get_file(), [&](const cpp_entity& e) {
        if (e.get_entity_type() == cpp_entity::function_t)
        {
            auto& func = dynamic_cast<const cpp_function&>(e);
            REQUIRE(func.get_name() == func.get_full_name());
            REQUIRE(std::string(func.get_full_name().c_str()) + func.get_signature().c_str()
                    == func.get_unique_name());

            if (func.get_name() == "a")
            {
                ++count;
                REQUIRE(func.get_return_type().get_name() == "void");
                REQUIRE(!func.is_constexpr());
                REQUIRE(!func.is_variadic());
                REQUIRE(func.get_noexcept() == "false");
                REQUIRE(!func.explicit_noexcept());
                REQUIRE(no_parameters(func) == 2u);
                REQUIRE(func.get_signature() == "(int,const char *)");
                REQUIRE(func.get_definition() == cpp_function_declaration);
            }
            else if (func.get_name() == "b")
            {
                ++count;
                REQUIRE(func.get_return_type().get_name() == "int");
                REQUIRE(!func.is_constexpr());
                REQUIRE(func.is_variadic());
                REQUIRE(func.get_noexcept() == "false");
                REQUIRE(!func.explicit_noexcept());
                REQUIRE(no_parameters(func) == 1u);
                REQUIRE(func.get_signature() == "(int,...)");
                REQUIRE(func.get_definition() == cpp_function_definition_normal);
            }
            else if (func.get_name() == "c")
            {
                ++count;
                REQUIRE(func.get_return_type().get_name() == "int*");
                REQUIRE(!func.is_constexpr());
                REQUIRE(!func.is_variadic());
                REQUIRE(func.get_noexcept() == "false");
                REQUIRE(!func.explicit_noexcept());
                REQUIRE(no_parameters(func) == 1u);
                REQUIRE(func.get_signature() == "(int)");
                REQUIRE(func.get_definition() == cpp_function_definition_deleted);
            }
            else if (func.get_name() == "d")
            {
                ++count;
                REQUIRE(func.get_return_type().get_name() == "char&");
                REQUIRE(!func.is_constexpr());
                REQUIRE(!func.is_variadic());
                REQUIRE(func.get_noexcept() == "true");
                REQUIRE(func.explicit_noexcept());
                REQUIRE(no_parameters(func) == 0u);
                REQUIRE(func.get_signature() == "()");
                REQUIRE(func.get_definition() == cpp_function_declaration);
            }
            else if (func.get_name() == "e")
            {
                ++count;
                REQUIRE(func.get_return_type().get_name() == "const int");
                REQUIRE(!func.is_constexpr());
                REQUIRE(!func.is_variadic());
                REQUIRE(func.get_noexcept() == "false");
                REQUIRE(func.explicit_noexcept());
                REQUIRE(no_parameters(func) == 0u);
                REQUIRE(func.get_signature() == "()");
                REQUIRE(func.get_definition() == cpp_function_declaration);
            }
            else if (func.get_name() == "f")
            {
                ++count;
                REQUIRE(func.get_return_type().get_name() == "int(*)(volatile char&&)");
                REQUIRE(!func.is_constexpr());
                REQUIRE(!func.is_variadic());
                REQUIRE(func.get_noexcept() == "false");
                REQUIRE(!func.explicit_noexcept());
                REQUIRE(no_parameters(func) == 1u);
                REQUIRE(func.get_signature() == "(int)");
                REQUIRE(func.get_definition() == cpp_function_declaration);
            }
            else if (func.get_name() == "g")
            {
                ++count;
                REQUIRE(func.get_return_type().get_name() == "const char&&");
                REQUIRE(func.is_constexpr());
                REQUIRE(!func.is_variadic());
                REQUIRE(func.get_noexcept() == "false");
                REQUIRE(!func.explicit_noexcept());
                REQUIRE(no_parameters(func) == 0u);
                REQUIRE(func.get_signature() == "()");
                REQUIRE(func.get_definition() == cpp_function_declaration);
            }
            else if (func.get_name() == "h")
            {
                ++count;
                REQUIRE(func.get_return_type().get_name() == "auto");
                REQUIRE(!func.is_constexpr());
                REQUIRE(!func.is_variadic());
                REQUIRE(func.get_noexcept() == "noexcept(e())");
                REQUIRE(func.explicit_noexcept());
                REQUIRE(no_parameters(func) == 0u);
                REQUIRE(func.get_signature() == "()");
                REQUIRE(func.get_definition() == cpp_function_definition_normal);
            }
            else if (func.get_name() == "i")
            {
                ++count;
                REQUIRE(func.get_return_type().get_name() == "decltype(d()='0')");
                REQUIRE(!func.is_constexpr());
                REQUIRE(!func.is_variadic());
                REQUIRE(func.get_noexcept() == "false");
                REQUIRE(!func.explicit_noexcept());
                REQUIRE(no_parameters(func) == 0u);
                REQUIRE(func.get_signature() == "()");
                REQUIRE(func.get_definition() == cpp_function_declaration);
            }
            else
                REQUIRE(false);
        }
        else
            REQUIRE(false);
    });
    REQUIRE(count == 9u);
}

TEST_CASE("cpp_member_function", "[cpp]")
{
    parser p;

    auto code = R"(struct base
{
    static void j();

    virtual int& k() const = 0;

    void l() volatile && = delete;

    virtual void m(int a = (j(), 0)) {}
};

struct derived : base
{
    virtual int& k() const override;

    void m(int a = (j(), 0)) final;

    derived& operator   =(const derived &a) = default;
};)";

    auto tu = parse(p, "cpp_member_function", code);

    // no need to check the parameters, same code as for variables
    auto count = 0u;
    for_each(tu.get_file(), [&](const cpp_entity& e) {
        if (e.get_name() == "base")
        {
            auto& c = dynamic_cast<const cpp_class&>(e);
            for (auto& ent : c)
            {
                auto& func = dynamic_cast<const cpp_member_function&>(ent);
                REQUIRE(!func.is_constexpr());
                REQUIRE(!func.is_variadic());
                REQUIRE(func.get_noexcept() == "false");
                REQUIRE(!func.explicit_noexcept());
                REQUIRE(std::string("base::") + func.get_name().c_str() == func.get_full_name());
                REQUIRE(std::string(func.get_full_name().c_str()) + func.get_signature().c_str()
                        == func.get_unique_name());

                if (func.get_name() == "j")
                {
                    ++count;
                    REQUIRE(func.get_return_type().get_name() == "void");
                    REQUIRE(func.get_virtual() == cpp_virtual_static);
                    REQUIRE(!is_const(func.get_cv()));
                    REQUIRE(!is_volatile(func.get_cv()));
                    REQUIRE(func.get_ref_qualifier() == cpp_ref_none);
                    REQUIRE(func.get_definition() == cpp_function_declaration);
                    REQUIRE(no_parameters(func) == 0u);
                    REQUIRE(func.get_signature() == "()");
                }
                else if (func.get_name() == "k")
                {
                    ++count;
                    REQUIRE(func.get_return_type().get_name() == "int&");
                    REQUIRE(func.get_virtual() == cpp_virtual_pure);
                    REQUIRE(is_const(func.get_cv()));
                    REQUIRE(!is_volatile(func.get_cv()));
                    REQUIRE(func.get_ref_qualifier() == cpp_ref_none);
                    REQUIRE(func.get_definition() == cpp_function_definition_pure);
                    REQUIRE(no_parameters(func) == 0u);
                    REQUIRE(func.get_signature() == "() const");
                }
                else if (func.get_name() == "l")
                {
                    ++count;
                    REQUIRE(func.get_return_type().get_name() == "void");
                    REQUIRE(func.get_virtual() == cpp_virtual_none);
                    REQUIRE(!is_const(func.get_cv()));
                    REQUIRE(is_volatile(func.get_cv()));
                    REQUIRE(func.get_ref_qualifier() == cpp_ref_rvalue);
                    REQUIRE(func.get_definition() == cpp_function_definition_deleted);
                    REQUIRE(no_parameters(func) == 0u);
                    REQUIRE(func.get_signature() == "() volatile &&");
                }
                else if (func.get_name() == "m")
                {
                    ++count;
                    REQUIRE(func.get_return_type().get_name() == "void");
                    REQUIRE(func.get_virtual() == cpp_virtual_new);
                    REQUIRE(!is_const(func.get_cv()));
                    REQUIRE(!is_volatile(func.get_cv()));
                    REQUIRE(func.get_ref_qualifier() == cpp_ref_none);
                    REQUIRE(func.get_definition() == cpp_function_definition_normal);
                    REQUIRE(no_parameters(func) == 1u);
                    REQUIRE(func.get_signature() == "(int)");
                }
                else
                    REQUIRE(false);
            }
        }
        else if (e.get_name() == "derived")
        {
            auto& c = dynamic_cast<const cpp_class&>(e);
            for (auto& ent : c)
            {
                if (ent.get_name() == "base")
                    continue; // skip access specifier

                auto& func = dynamic_cast<const cpp_member_function&>(ent);
                REQUIRE(!func.is_constexpr());
                REQUIRE(!func.is_variadic());
                REQUIRE(func.get_noexcept() == "false");
                REQUIRE(!func.explicit_noexcept());
                REQUIRE(std::string("derived::") + func.get_name().c_str() == func.get_full_name());
                REQUIRE(std::string(func.get_full_name().c_str()) + func.get_signature().c_str()
                        == func.get_unique_name());

                if (func.get_name() == "k")
                {
                    ++count;
                    REQUIRE(func.get_return_type().get_name() == "int&");
                    REQUIRE(func.get_virtual() == cpp_virtual_overriden);
                    REQUIRE(is_const(func.get_cv()));
                    REQUIRE(!is_volatile(func.get_cv()));
                    REQUIRE(func.get_ref_qualifier() == cpp_ref_none);
                    REQUIRE(func.get_definition() == cpp_function_declaration);
                    REQUIRE(no_parameters(func) == 0u);
                    REQUIRE(func.get_signature() == "() const");
                }
                else if (func.get_name() == "m")
                {
                    ++count;
                    REQUIRE(func.get_return_type().get_name() == "void");
                    REQUIRE(func.get_virtual() == cpp_virtual_final);
                    REQUIRE(!is_const(func.get_cv()));
                    REQUIRE(!is_volatile(func.get_cv()));
                    REQUIRE(func.get_ref_qualifier() == cpp_ref_none);
                    REQUIRE(func.get_definition() == cpp_function_declaration);
                    REQUIRE(no_parameters(func) == 1u);
                    REQUIRE(func.get_signature() == "(int)");
                }
                else if (func.get_name() == "operator=")
                {
                    ++count;
                    REQUIRE(func.get_return_type().get_name() == "derived&");
                    REQUIRE(func.get_virtual() == cpp_virtual_none);
                    REQUIRE(!is_const(func.get_cv()));
                    REQUIRE(!is_volatile(func.get_cv()));
                    REQUIRE(func.get_ref_qualifier() == cpp_ref_none);
                    REQUIRE(func.get_definition() == cpp_function_definition_defaulted);
                    REQUIRE(no_parameters(func) == 1u);
                    REQUIRE(func.get_signature() == "(const derived &)");
                }
                else
                    REQUIRE(false);
            }
        }
        else
            REQUIRE(false);
    });
    REQUIRE(count == 7u);
}

TEST_CASE("cpp_conversion_op", "[cpp]")
{
    parser p;

    auto code = R"(
        template <typename T>
        struct bar {};

        struct foo
        {
            [[noreturn]] operator    int() [[]]; // multiple whitespace

            explicit __attribute__((foo)) operator const char&(void);

            constexpr explicit __attribute__((bar)) operator char() const volatile && noexcept
            {
                return '0';
            }

            operator bar<decltype(int())>();
        };
    )";

    auto tu = parse(p, "cpp_conversion_op", code);

    auto count = 0u;
    for_each(tu.get_file(), [&](const cpp_entity& t) {
        if (t.get_name() != "foo")
            return;

        for (auto& e : dynamic_cast<const cpp_class&>(t))
        {
            auto& op = dynamic_cast<const cpp_conversion_op&>(e);
            INFO(op.get_name().c_str());
            REQUIRE(no_parameters(op) == 0u);
            REQUIRE(std::string("foo::") + op.get_name().c_str() == op.get_full_name());
            REQUIRE(std::string(op.get_full_name().c_str()) + op.get_signature().c_str()
                    == op.get_unique_name());

            if (op.get_name() == "operator int")
            {
                ++count;
                REQUIRE(op.get_target_type().get_name() == "int");
                REQUIRE(!op.is_explicit());
                REQUIRE(!op.is_constexpr());
                REQUIRE(!is_const(op.get_cv()));
                REQUIRE(!is_volatile(op.get_cv()));
                REQUIRE(op.get_ref_qualifier() == cpp_ref_none);
                REQUIRE(op.get_noexcept() == "false");
                REQUIRE(!op.explicit_noexcept());
                REQUIRE(op.get_signature() == "()");
                REQUIRE(op.get_definition() == cpp_function_declaration);
            }
            else if (op.get_name() == "operator const char &")
            {
                ++count;
                REQUIRE(op.get_target_type().get_name() == "const char &");
                REQUIRE(op.is_explicit());
                REQUIRE(!op.is_constexpr());
                REQUIRE(!is_const(op.get_cv()));
                REQUIRE(!is_volatile(op.get_cv()));
                REQUIRE(op.get_ref_qualifier() == cpp_ref_none);
                REQUIRE(op.get_noexcept() == "false");
                REQUIRE(!op.explicit_noexcept());
                REQUIRE(op.get_signature() == "()");
                REQUIRE(op.get_definition() == cpp_function_declaration);
            }
            else if (op.get_name() == "operator char")
            {
                ++count;
                REQUIRE(op.get_target_type().get_name() == "char");
                REQUIRE(op.is_explicit());
                REQUIRE(op.is_constexpr());
                REQUIRE(is_const(op.get_cv()));
                REQUIRE(is_volatile(op.get_cv()));
                REQUIRE(op.get_ref_qualifier() == cpp_ref_rvalue);
                REQUIRE(op.get_noexcept() == "true");
                REQUIRE(op.explicit_noexcept());
                REQUIRE(op.get_signature() == "() const volatile &&");
                REQUIRE(op.get_definition() == cpp_function_definition_normal);
            }
            else if (op.get_name() == "operator bar<decltype(int())>")
            {
                ++count;
                REQUIRE(op.get_target_type().get_name() == "bar<decltype(int())>");
                REQUIRE(!op.is_explicit());
                REQUIRE(!op.is_constexpr());
                REQUIRE(!is_const(op.get_cv()));
                REQUIRE(!is_volatile(op.get_cv()));
                REQUIRE(op.get_ref_qualifier() == cpp_ref_none);
                REQUIRE(op.get_noexcept() == "false");
                REQUIRE(!op.explicit_noexcept());
                REQUIRE(op.get_signature() == "()");
                REQUIRE(op.get_definition() == cpp_function_declaration);
            }
            else
                REQUIRE(false);
        }
    });
    REQUIRE(count == 4u);
}

TEST_CASE("cpp_constructor", "[cpp]")
{
    parser p;

    auto code = R"(
        struct foo
        {
            /// a
            [[noreturn]] foo() = delete;

            /// b
            explicit foo(int a = {}) {}

            /// c
            constexpr foo(char c) noexcept;

            /// d
            foo(const foo &other) __attribute__(()) = default;

            /// e
            foo(char *ptr) noexcept(false)
            : foo(0) {}
        };
    )";

    auto tu = parse(p, "cpp_conversion_op", code);

    auto count = 0u;
    for_each(tu.get_file(), [&](const cpp_entity& t) {
        for (auto& e : dynamic_cast<const cpp_class&>(t))
        {
            auto& ctor = dynamic_cast<const cpp_constructor&>(e);
            REQUIRE(!ctor.is_variadic());
            REQUIRE(std::string("foo::") + ctor.get_name().c_str() == ctor.get_full_name());
            REQUIRE(std::string(ctor.get_full_name().c_str()) + ctor.get_signature().c_str()
                    == ctor.get_unique_name());

            if (detail::parse_comment(ctor.get_cursor()) == "/// a")
            {
                ++count;
                REQUIRE(no_parameters(ctor) == 0u);
                REQUIRE(!ctor.is_constexpr());
                REQUIRE(!ctor.is_explicit());
                REQUIRE(ctor.get_noexcept() == "false");
                REQUIRE(!ctor.explicit_noexcept());
                REQUIRE(ctor.get_definition() == cpp_function_definition_deleted);
                REQUIRE(ctor.get_signature() == "()");
            }
            else if (detail::parse_comment(ctor.get_cursor()) == "/// b")
            {
                ++count;
                REQUIRE(no_parameters(ctor) == 1u);
                REQUIRE(!ctor.is_constexpr());
                REQUIRE(ctor.is_explicit());
                REQUIRE(ctor.get_noexcept() == "false");
                REQUIRE(!ctor.explicit_noexcept());
                REQUIRE(ctor.get_definition() == cpp_function_definition_normal);
                REQUIRE(ctor.get_signature() == "(int)");
            }
            else if (detail::parse_comment(ctor.get_cursor()) == "/// c")
            {
                ++count;
                REQUIRE(no_parameters(ctor) == 1u);
                REQUIRE(ctor.is_constexpr());
                REQUIRE(!ctor.is_explicit());
                REQUIRE(ctor.get_noexcept() == "true");
                REQUIRE(ctor.explicit_noexcept());
                REQUIRE(ctor.get_definition() == cpp_function_declaration);
                REQUIRE(ctor.get_signature() == "(char)");
            }
            else if (detail::parse_comment(ctor.get_cursor()) == "/// d")
            {
                ++count;
                REQUIRE(no_parameters(ctor) == 1u);
                REQUIRE(!ctor.is_constexpr());
                REQUIRE(!ctor.is_explicit());
                REQUIRE(ctor.get_noexcept() == "false");
                REQUIRE(!ctor.explicit_noexcept());
                REQUIRE(ctor.get_definition() == cpp_function_definition_defaulted);
                REQUIRE(ctor.get_signature() == "(const foo &)");
            }
            else if (detail::parse_comment(ctor.get_cursor()) == "/// e")
            {
                ++count;
                REQUIRE(no_parameters(ctor) == 1u);
                REQUIRE(!ctor.is_constexpr());
                REQUIRE(!ctor.is_explicit());
                REQUIRE(ctor.get_noexcept() == "false");
                REQUIRE(ctor.explicit_noexcept());
                REQUIRE(ctor.get_definition() == cpp_function_definition_normal);
                REQUIRE(ctor.get_signature() == "(char *)");
            }
            else
                REQUIRE(false);
        }
    });
    REQUIRE(count == 5u);
}

TEST_CASE("cpp_destructor", "[cpp]")
{
    parser p;

    auto code = R"(
        struct a
        {
            [[deprecated]] ~a() = default;
        };

        struct b
        {
            ~b() = delete;
        };

        struct c
        {
            ~c() noexcept(false);
        };

        struct d
        {
            virtual [[]] ~d() noexcept = 0;
        };

        struct e : d
        {
            ~e() __attribute__(()) override {}
        };
    )";

    auto tu = parse(p, "cpp_conversion_op", code);

    auto count = 0u;
    for_each(tu.get_file(), [&](const cpp_entity& t) {
        auto& c = dynamic_cast<const cpp_class&>(t);
        if (c.get_name() == "a")
        {
            for (auto& e : c)
            {
                auto& dtor = dynamic_cast<const cpp_destructor&>(e);
                REQUIRE(no_parameters(dtor) == 0);
                REQUIRE(!dtor.is_constexpr());
                REQUIRE(dtor.get_noexcept() == "true");
                REQUIRE(dtor.get_virtual() == cpp_virtual_none);
                REQUIRE(!dtor.explicit_noexcept());
                REQUIRE(dtor.get_definition() == cpp_function_definition_defaulted);
                REQUIRE(dtor.get_signature() == "()");
                REQUIRE(dtor.get_full_name() == "a::~a");
                REQUIRE(dtor.get_unique_name() == "a::~a()");
                ++count;
            }
        }
        else if (c.get_name() == "b")
        {
            for (auto& e : c)
            {
                auto& dtor = dynamic_cast<const cpp_destructor&>(e);
                REQUIRE(no_parameters(dtor) == 0);
                REQUIRE(!dtor.is_constexpr());
                REQUIRE(dtor.get_noexcept() == "true");
                REQUIRE(!dtor.explicit_noexcept());
                REQUIRE(dtor.get_virtual() == cpp_virtual_none);
                REQUIRE(dtor.get_definition() == cpp_function_definition_deleted);
                REQUIRE(dtor.get_signature() == "()");
                REQUIRE(dtor.get_full_name() == "b::~b");
                REQUIRE(dtor.get_unique_name() == "b::~b()");
                ++count;
            }
        }
        else if (c.get_name() == "c")
        {
            for (auto& e : c)
            {
                auto& dtor = dynamic_cast<const cpp_destructor&>(e);
                REQUIRE(no_parameters(dtor) == 0);
                REQUIRE(!dtor.is_constexpr());
                REQUIRE(dtor.get_noexcept() == "false");
                REQUIRE(dtor.explicit_noexcept());
                REQUIRE(dtor.get_virtual() == cpp_virtual_none);
                REQUIRE(dtor.get_definition() == cpp_function_declaration);
                REQUIRE(dtor.get_signature() == "()");
                REQUIRE(dtor.get_full_name() == "c::~c");
                REQUIRE(dtor.get_unique_name() == "c::~c()");
                ++count;
            }
        }
        else if (c.get_name() == "d")
        {
            for (auto& e : c)
            {
                auto& dtor = dynamic_cast<const cpp_destructor&>(e);
                REQUIRE(no_parameters(dtor) == 0);
                REQUIRE(!dtor.is_constexpr());
                REQUIRE(dtor.get_noexcept() == "true");
                REQUIRE(dtor.explicit_noexcept());
                REQUIRE(dtor.get_virtual() == cpp_virtual_pure);
                REQUIRE(dtor.get_definition() == cpp_function_definition_pure);
                REQUIRE(dtor.get_signature() == "()");
                REQUIRE(dtor.get_full_name() == "d::~d");
                REQUIRE(dtor.get_unique_name() == "d::~d()");
                ++count;
            }
        }
        else if (c.get_name() == "e")
        {
            for (auto& e : c)
            {
                if (e.get_name() == "d")
                    continue; // skip base

                auto& dtor = dynamic_cast<const cpp_destructor&>(e);
                REQUIRE(no_parameters(dtor) == 0);
                REQUIRE(!dtor.is_constexpr());
                REQUIRE(dtor.get_noexcept() == "true");
                REQUIRE(!dtor.explicit_noexcept());
                REQUIRE(dtor.get_virtual() == cpp_virtual_overriden);
                REQUIRE(dtor.get_definition() == cpp_function_definition_normal);
                REQUIRE(dtor.get_signature() == "()");
                REQUIRE(dtor.get_full_name() == "e::~e");
                REQUIRE(dtor.get_unique_name() == "e::~e()");
                ++count;
            }
        }
        else
            REQUIRE(false);
    });
    REQUIRE(count == 5u);
}

TEST_CASE("implicit virtual", "[cpp]")
{
    parser p;

    auto code = R"(
            struct base_a
            {
                virtual void a(int c) const noexcept = 0;
                virtual int& b(char c, ...) volatile;

                virtual operator int() &&;
            };

            struct base_b
            {
                void a() const;

                virtual void c() const;

                virtual ~base_b();
            };

            struct mid : base_b {};

            struct derived
            : base_a, mid
            {
                /// a
                void a(int c) const noexcept;

                /// b
                void a() const;

                /// c
                int& b(char c, ...) volatile;

                /// d
                int* b(char c);

                /// e
                virtual void c() const;

                /// f
                operator int() &&;

                /// g
                ~derived();
            };
        )";

    auto tu = parse(p, "implicit_virtual", code);

    auto count = 0u;
    for_each(tu.get_file(), [&](const cpp_entity& e) {
        if (e.get_name() == "derived")
        {
            for (auto& member : dynamic_cast<const cpp_class&>(e))
            {
                if (detail::parse_comment(member.get_cursor()) == "/// a")
                {
                    ++count;
                    REQUIRE(dynamic_cast<const cpp_member_function&>(member).get_virtual()
                            == cpp_virtual_overriden);
                }
                else if (detail::parse_comment(member.get_cursor()) == "/// b")
                {
                    ++count;
                    REQUIRE(dynamic_cast<const cpp_member_function&>(member).get_virtual()
                            == cpp_virtual_none);
                }
                else if (detail::parse_comment(member.get_cursor()) == "/// c")
                {
                    ++count;
                    REQUIRE(dynamic_cast<const cpp_member_function&>(member).get_virtual()
                            == cpp_virtual_overriden);
                }
                else if (detail::parse_comment(member.get_cursor()) == "/// d")
                {
                    ++count;
                    REQUIRE(dynamic_cast<const cpp_member_function&>(member).get_virtual()
                            == cpp_virtual_none);
                }
                else if (detail::parse_comment(member.get_cursor()) == "/// e")
                {
                    ++count;
                    REQUIRE(dynamic_cast<const cpp_member_function&>(member).get_virtual()
                            == cpp_virtual_overriden);
                }
                else if (detail::parse_comment(member.get_cursor()) == "/// f")
                {
                    ++count;
                    REQUIRE(dynamic_cast<const cpp_conversion_op&>(member).get_virtual()
                            == cpp_virtual_overriden);
                }
                else if (detail::parse_comment(member.get_cursor()) == "/// g")
                {
                    ++count;
                    REQUIRE(dynamic_cast<const cpp_destructor&>(member).get_virtual()
                            == cpp_virtual_overriden);
                }
                else
                    REQUIRE(false);
            }
        }
    });
    REQUIRE(count == 7u);
}
