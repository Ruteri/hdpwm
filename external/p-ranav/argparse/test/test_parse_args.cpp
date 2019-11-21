#include <doctest.hpp>
#include <argparse.hpp>

DOCTEST_TEST_CASE("Parse a string argument with value [parse_args]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--config");
  program.parse_args({ "test", "--config", "config.yml"});
  REQUIRE(program.get("--config") == "config.yml");
}

DOCTEST_TEST_CASE("Parse a string argument with default value [parse_args]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--config")
    .default_value(std::string("foo.yml"));
  program.parse_args({ "test", "--config" });
  REQUIRE(program.get("--config") == "foo.yml");
}

DOCTEST_TEST_CASE("Parse an int argument with value [parse_args]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--count")
    .action([](const std::string& value) { return std::stoi(value); });
  program.parse_args({ "test", "--count", "5" });
  REQUIRE(program.get<int>("--count") == 5);
}

DOCTEST_TEST_CASE("Parse an int argument with default value [parse_args]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--count")
    .default_value(2)
    .action([](const std::string& value) { return std::stoi(value); });
  program.parse_args({ "test", "--count" });
  REQUIRE(program.get<int>("--count") == 2);
}

DOCTEST_TEST_CASE("Parse a float argument with value [parse_args]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--ratio")
    .action([](const std::string& value) { return std::stof(value); });
  program.parse_args({ "test", "--ratio", "5.6645" });
  REQUIRE(program.get<float>("--ratio") == 5.6645f);
}

DOCTEST_TEST_CASE("Parse a float argument with default value [parse_args]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--ratio")
    .default_value(3.14f)
    .action([](const std::string& value) { return std::stof(value); });
  program.parse_args({ "test", "--ratio" });
  REQUIRE(program.get<float>("--ratio") == 3.14f);
}

DOCTEST_TEST_CASE("Parse a double argument with value [parse_args]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--ratio")
    .action([](const std::string& value) { return std::stod(value); });
  program.parse_args({ "test", "--ratio", "5.6645" });
  REQUIRE(program.get<double>("--ratio") == 5.6645);
}

DOCTEST_TEST_CASE("Parse a double argument with default value [parse_args]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--ratio")
    .default_value(3.14)
    .action([](const std::string& value) { return std::stod(value); });
  program.parse_args({ "test", "--ratio" });
  REQUIRE(program.get<double>("--ratio") == 3.14);
}

DOCTEST_TEST_CASE("Parse a vector of integer arguments [parse_args]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--vector")
    .nargs(5)
    .action([](const std::string& value) { return std::stoi(value); });
  program.parse_args({ "test", "--vector", "1", "2", "3", "4", "5" });
  auto vector = program.get<std::vector<int>>("--vector");
  REQUIRE(vector.size() == 5);
  REQUIRE(vector[0] == 1);
  REQUIRE(vector[1] == 2);
  REQUIRE(vector[2] == 3);
  REQUIRE(vector[3] == 4);
  REQUIRE(vector[4] == 5);
}

DOCTEST_TEST_CASE("Parse a vector of float arguments [parse_args]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--vector")
    .nargs(5)
    .action([](const std::string& value) { return std::stof(value); });
  program.parse_args({ "test", "--vector", "1.1", "2.2", "3.3", "4.4", "5.5" });
  auto vector = program.get<std::vector<float>>("--vector");
  REQUIRE(vector.size() == 5);
  REQUIRE(vector[0] == 1.1f);
  REQUIRE(vector[1] == 2.2f);
  REQUIRE(vector[2] == 3.3f);
  REQUIRE(vector[3] == 4.4f);
  REQUIRE(vector[4] == 5.5f);
}

DOCTEST_TEST_CASE("Parse a vector of double arguments [parse_args]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--vector")
    .nargs(5)
    .action([](const std::string& value) { return std::stod(value); });
  program.parse_args({ "test", "--vector", "1.1", "2.2", "3.3", "4.4", "5.5" });
  auto vector = program.get<std::vector<double>>("--vector");
  REQUIRE(vector.size() == 5);
  REQUIRE(vector[0] == 1.1);
  REQUIRE(vector[1] == 2.2);
  REQUIRE(vector[2] == 3.3);
  REQUIRE(vector[3] == 4.4);
  REQUIRE(vector[4] == 5.5);
}

DOCTEST_TEST_CASE("Parse a vector of string arguments [parse_args]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--vector")
    .nargs(5)
    .action([](const std::string& value) { return value; });
  program.parse_args({ "test", "--vector", "abc", "def", "ghi", "jkl", "mno" });
  auto vector = program.get<std::vector<std::string>>("--vector");
  REQUIRE(vector.size() == 5);
  REQUIRE(vector[0] == "abc");
  REQUIRE(vector[1] == "def");
  REQUIRE(vector[2] == "ghi");
  REQUIRE(vector[3] == "jkl");
  REQUIRE(vector[4] == "mno");
}

DOCTEST_TEST_CASE("Parse a vector of character arguments [parse_args]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--vector")
    .nargs(5)
    .action([](const std::string& value) { return value[0]; });
  program.parse_args({ "test", "--vector", "a", "b", "c", "d", "e" });
  auto vector = program.get<std::vector<char>>("--vector");
  REQUIRE(vector.size() == 5);
  REQUIRE(vector[0] == 'a');
  REQUIRE(vector[1] == 'b');
  REQUIRE(vector[2] == 'c');
  REQUIRE(vector[3] == 'd');
  REQUIRE(vector[4] == 'e');
}

DOCTEST_TEST_CASE("Parse a vector of string arguments and construct objects [parse_args]") {

  class Foo {
  public:
    Foo(const std::string& value) : value(value) {}
    std::string value;
  };

  argparse::ArgumentParser program("test");
  program.add_argument("--vector")
    .nargs(5)
    .action([](const std::string& value) { return Foo(value); });
  program.parse_args({ "test", "--vector", "abc", "def", "ghi", "jkl", "mno" });
  auto vector = program.get<std::vector<Foo>>("--vector");
  REQUIRE(vector.size() == 5);
  REQUIRE(vector[0].value == Foo("abc").value);
  REQUIRE(vector[1].value == Foo("def").value);
  REQUIRE(vector[2].value == Foo("ghi").value);
  REQUIRE(vector[3].value == Foo("jkl").value);
  REQUIRE(vector[4].value == Foo("mno").value);
}