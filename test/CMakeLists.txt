#
# mht/tests CMakeLists.txt
#

#Enable warnings
add_definitions("-Wall")

#The targets included
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/test/*.cc")

#Compile the code
add_executable(unit_tests ${SOURCES})
target_link_libraries(unit_tests mht_base emdw ${GLINEAR_LIBRARIES} ${PATRECII_LIBRARIES} ${BOOST_LIBRARIES} ${GTEST_LIBRARIES} pthread)
