#include <doctest.hpp>
#include <argparse.hpp>
#include <cmath>

DOCTEST_TEST_CASE("Parse positional arguments [positional_arguments]") {
  argparse::ArgumentParser program("test");
  program.add_argument("input");
  program.add_argument("output");
  program.parse_args({ "test", "rocket.mesh", "thrust_profile.csv" });
  REQUIRE(program.get("input") == "rocket.mesh");
  REQUIRE(program.get("output") == "thrust_profile.csv");
}

DOCTEST_TEST_CASE("Parse positional arguments with fixed nargs [positional_arguments]") {
  argparse::ArgumentParser program("test");
  program.add_argument("input");
  program.add_argument("output").nargs(2);
  program.parse_args({ "test", "rocket.mesh", "thrust_profile.csv", "output.mesh" });
  REQUIRE(program.get("input") == "rocket.mesh");
  auto outputs = program.get<std::vector<std::string>>("output");
  REQUIRE(outputs.size() == 2);
  REQUIRE(outputs[0] == "thrust_profile.csv");
  REQUIRE(outputs[1] == "output.mesh");
}

DOCTEST_TEST_CASE("Parse positional arguments with optional arguments [positional_arguments]") {
  argparse::ArgumentParser program("test");
  program.add_argument("input");
  program.add_argument("output").nargs(2);
  program.add_argument("--num_iterations")
    .action([](const std::string& value) { return std::stoi(value); });
  program.parse_args({ "test", "rocket.mesh", "--num_iterations", "15", "thrust_profile.csv", "output.mesh" });
  REQUIRE(program.get<int>("--num_iterations") == 15);
  REQUIRE(program.get("input") == "rocket.mesh");
  auto outputs = program.get<std::vector<std::string>>("output");
  REQUIRE(outputs.size() == 2);
  REQUIRE(outputs[0] == "thrust_profile.csv");
  REQUIRE(outputs[1] == "output.mesh");
}

DOCTEST_TEST_CASE("Parse positional arguments with optional arguments in the middle [positional_arguments]") {
  argparse::ArgumentParser program("test");
  program.add_argument("input");
  program.add_argument("output").nargs(2);
  program.add_argument("--num_iterations")
    .action([](const std::string& value) { return std::stoi(value); });
  REQUIRE_THROWS(program.parse_args({ "test", "rocket.mesh", "thrust_profile.csv", "--num_iterations", "15", "output.mesh" }));
}

DOCTEST_TEST_CASE("Parse remaining arguments deemed positional [positional_arguments]") {
  GIVEN("a program that accepts an optional argument and remaining arguments") {
    argparse::ArgumentParser program("test");
    program.add_argument("-o");
    program.add_argument("input").remaining();

    WHEN("provided no argument") {
      THEN("the program accepts it but gets nothing") {
        REQUIRE_NOTHROW(program.parse_args({"test"}));
        REQUIRE_THROWS_AS(program.get<std::vector<std::string>>("input"),
                          std::logic_error);
      }
    }

    WHEN("provided an optional followed by remaining arguments") {
      program.parse_args({"test", "-o", "a.out", "a.c", "b.c", "main.c"});

      THEN("the optional parameter consumes an argument") {
        using namespace std::literals;
        REQUIRE(program["-o"] == "a.out"s);

        auto inputs = program.get<std::vector<std::string>>("input");
        REQUIRE(inputs.size() == 3);
        REQUIRE(inputs[0] == "a.c");
        REQUIRE(inputs[1] == "b.c");
        REQUIRE(inputs[2] == "main.c");
      }
    }

    WHEN("provided remaining arguments including optional arguments") {
      program.parse_args({"test", "a.c", "b.c", "main.c", "-o", "a.out"});

      THEN("the optional argument is deemed remaining") {
        REQUIRE_THROWS_AS(program.get("-o"), std::logic_error);

        auto inputs = program.get<std::vector<std::string>>("input");
        REQUIRE(inputs.size() == 5);
        REQUIRE(inputs[0] == "a.c");
        REQUIRE(inputs[1] == "b.c");
        REQUIRE(inputs[2] == "main.c");
        REQUIRE(inputs[3] == "-o");
        REQUIRE(inputs[4] == "a.out");
      }
    }
  }
}

DOCTEST_TEST_CASE("Negative nargs is not allowed [positional_arguments]") {
  argparse::ArgumentParser program("test");
  REQUIRE_THROWS_AS(program.add_argument("output").nargs(-1), std::logic_error);
}

DOCTEST_TEST_CASE("Square a number [positional_arguments]") {
  argparse::ArgumentParser program;
  program.add_argument("--verbose", "-v")
    .help("enable verbose logging")
    .default_value(false)
    .implicit_value(true);

  program.add_argument("square")
    .help("display a square of a given number")
    .action([](const std::string& value) { return pow(std::stoi(value), 2); });

  program.parse_args({"./main", "15"});
  REQUIRE(program.get<double>("square") == 225);
}
