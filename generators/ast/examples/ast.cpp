#include <zapata/ast.h>

auto main(int, char**) -> int {
    auto _operator1 =
      zpt::make_function<zpt::ast::cpp_function>("operator<<", "std::ostream&", zpt::ast::FRIEND);
    _operator1 //
      ->add<zpt::ast::cpp_variable>("_out", "std::ostream&")
      .add<zpt::ast::cpp_variable>("_in", "my_class const&");

    auto _operator2 = zpt::make_function<zpt::ast::cpp_function>(
      "my_module::events::my_class::operator<<", "std::ostream&");
    _operator2 //
      ->add<zpt::ast::cpp_variable>("_out", "std::ostream&")
      .add<zpt::ast::cpp_variable>("_in", "my_class const&");

    auto _if_block = zpt::make_code_block<zpt::ast::cpp_code_block>("if (_in.is_food())");
    _if_block->add<zpt::ast::cpp_instruction>("_out << \"\\t\" << std::endl << std::flush");

    auto _operator2_block = zpt::make_code_block<zpt::ast::cpp_code_block>();
    _operator2_block //
      ->add<zpt::ast::cpp_instruction>("_out << _in.to_string() << std::flush")
      .add(_if_block)
      .add<zpt::ast::cpp_instruction>("return _out");

    _operator2->add(_operator2_block);

    auto _class = zpt::make_class<zpt::ast::cpp_class>("my_class");
    _class //
      ->add<zpt::ast::cpp_function>(zpt::ast::PUBLIC, "my_class", "", zpt::ast::DEFAULT)
      .add<zpt::ast::cpp_function>(zpt::ast::PUBLIC, "~my_class", "", zpt::ast::DEFAULT)
      .add<zpt::ast::cpp_function>(
        zpt::ast::PUBLIC, "print", "void", zpt::ast::VIRTUAL | zpt::ast::ABSTRACT)
      .add<zpt::ast::cpp_function>(
        zpt::ast::PROTECTED, "to_string", "std::string", zpt::ast::CONST | zpt::ast::VIRTUAL)
      .add(_operator1, zpt::ast::PUBLIC);

    auto _namespace1 = zpt::make_code_block<zpt::ast::cpp_code_block>("namespace my_module");
    auto _namespace2 = zpt::make_code_block<zpt::ast::cpp_code_block>("namespace events");
    _namespace2->add(_class);
    _namespace1->add(_namespace2);

    std::cout << "#include <iostream>" << std::endl;
    std::cout << *_namespace1 << std::endl;
    std::cout << *_operator2 << std::endl;
    return 0;
}
