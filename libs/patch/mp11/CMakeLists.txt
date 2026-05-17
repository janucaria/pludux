cmake_minimum_required(VERSION 3.5...3.16)

project(boost_mp11 VERSION 1.92.0 LANGUAGES CXX)

add_library(boost_mp11 INTERFACE)
add_library(Boost::mp11 ALIAS boost_mp11)

target_include_directories(boost_mp11 INTERFACE include)
target_compile_features(boost_mp11 INTERFACE
	cxx_alias_templates
	cxx_variadic_templates
	cxx_decltype
)
