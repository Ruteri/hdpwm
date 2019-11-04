/*

Copyright (C) 2019 Mateusz Morusiewicz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#include <src/crypto/utils.h>

#include <external/catch2/catch.hpp>

using utils::sensitive_string;

TEST_CASE( "constructors are sane", "[sensitive_string_ctrs]" ) {
	{
		sensitive_string dflt;
		REQUIRE( dflt.index == 0 );
		REQUIRE( dflt.max_size == 32 );
	}

	{
		sensitive_string size_ctr(12);
		REQUIRE( size_ctr.index == 0 );
		REQUIRE( size_ctr.max_size == 12 );
	}

	{
		sensitive_string dflt;
		sensitive_string cpy = dflt;
		REQUIRE( cpy.index == 0 );
		REQUIRE( cpy.max_size == 32 );
		REQUIRE( cpy.data != dflt.data );
	}

	{
		sensitive_string dflt;
		auto data_ptr = dflt.data;
		sensitive_string mv = std::move(dflt);
		REQUIRE( mv.index == 0 );
		REQUIRE( mv.max_size == 32 );
		REQUIRE( mv.data == data_ptr );
		REQUIRE( dflt.data == nullptr );
	}

	{
		sensitive_string from_cstr("somestr");
		REQUIRE( from_cstr.index == 7 );
		REQUIRE( from_cstr.max_size == 32 );
	}

	{
		std::string somestr = "somestr";
		sensitive_string from_str(somestr);
		REQUIRE( from_str.index == 7 );
		REQUIRE( from_str.max_size == 32 );
	}
}

TEST_CASE( "destructor clears memory", "[sensitive_string_destrc]" ) {
	char *block = new char[48];

	for (int i = 0; i < 48; ++i) {
		block[i] = 'x';
	}

	sensitive_string dflt;
	dflt.data = block;

	{
		sensitive_string from_str(std::move(dflt));
		REQUIRE( from_str.index == 0 );
		REQUIRE( from_str.max_size == 32 );
	}

	for (int i = 0; i < 32; ++i) REQUIRE( block[i] != 'x' );
	for (int i = 32; i < 48; ++i) REQUIRE( block[i] == 'x' );
}

TEST_CASE( "size is calculated properly", "[sensitive_string_size]" ) {
	sensitive_string from_str("xxxx");
	REQUIRE( from_str.size() == 4 );
	from_str.resize(2);
	REQUIRE( from_str.size() == 1 );
	from_str.push_back('y');
	REQUIRE( from_str.size() == 2 );
	from_str.push_back('y');
}

TEST_CASE( "resize and reserve work as intended", "[sensitive_string_resize_reserve]" ) {
	sensitive_string str(8);
	char *ptr = str.data;
	str.reserve(4);
	REQUIRE( str.data == ptr );
	REQUIRE( str.max_size == 8 );

	str.resize(4);
	REQUIRE( str.data != ptr );
	REQUIRE( str.max_size == 4 );

	ptr = str.data;
	str.reserve(16);
	REQUIRE( str.data != ptr );
	REQUIRE( str.max_size == 16 );
}

TEST_CASE( "push and pop work", "[sensitive_string_push_pop_back]" ) {
	sensitive_string str(2);
	str.pop_back();
	REQUIRE( str.index == 0 );
	REQUIRE( str.data[0] == '\0' );

	str.push_back('x');
	REQUIRE( str.index == 1 );
	REQUIRE( str.size() == 1 );
	REQUIRE( str.max_size == 2 );
	REQUIRE( str.data[0] == 'x' );

	str.push_back('x');
	REQUIRE( str.index == 2 );
	REQUIRE( str.size() == 2 );
	REQUIRE( str.max_size == 2 );
	REQUIRE( str.data[1] == 'x' );

	str.push_back('x');
	REQUIRE( str.index == 3 );
	REQUIRE( str.size() == 3 );
	REQUIRE( str.max_size == 4 );
	for (int i = 0; i < 3; ++i) REQUIRE( str.data[i] == 'x' );

	str.pop_back();
	REQUIRE( str.index == 2 );
	REQUIRE( str.size() == 2 );
	REQUIRE( str.max_size == 4 );
	REQUIRE( str.data[2] == '\0' );
}
