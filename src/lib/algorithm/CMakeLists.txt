set(LIBRARY_NAME ${ALGORITHM_LIB})

include_directories(${CMAKE_SOURCE_DIR}/src/lib
					${OpenCV_INCLUDE_DIRS}
					${Boost_INCLUDE_DIR})
					
set(SOURCE_FILES    extractboundary/NaiveBoundary.cpp
					skinning/Filling.cpp
					evaluation/ShapeError.cpp)
# make the library
add_library(
    ${LIBRARY_NAME}
    ${INCLUDE_FILES}
    ${SOURCE_FILES}
	)

SET_TARGET_PROPERTIES(${LIBRARY_NAME} PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(${LIBRARY_NAME} ${MATHTOOLS_LIB}
									  ${SHAPE_LIB}
									  ${SKELETON_LIB}
									  ${BOUNDARY_LIB})

