# CoH: Company of Heroes ideas for Zero Hour. See README.md and Docs/PLAN.md.
#
# zi_coh is the one CoH target. Its sources compile into the Zero Hour engine (z_gameengine links it),
# and every slice's Tests/*.cpp compiles into z_googletest. A feature adds itself with one
# coh_add_feature line below; the Kernel needs no change for that.

add_library(zi_coh INTERFACE)
target_include_directories(zi_coh INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Only the hub's header, for the engine's device library: it calls a few hooks (the radar) but the
# CoH sources compile into z_gameengine alone.
add_library(zi_coh_hooks INTERFACE)
target_include_directories(zi_coh_hooks INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Adds a slice folder: its sources to the engine, its tests to z_googletest.
function(coh_add_slice folder)
    file(GLOB_RECURSE sources CONFIGURE_DEPENDS ${folder}/*.cpp ${folder}/*.h)
    file(GLOB_RECURSE tests CONFIGURE_DEPENDS ${folder}/Tests/*.cpp ${folder}/Tests/*.h)
    if(tests)
        list(REMOVE_ITEM sources ${tests})
    endif()
    target_sources(zi_coh INTERFACE ${sources})
    set_property(GLOBAL APPEND PROPERTY COH_TEST_SOURCES ${tests})
endfunction()

# Adds the slice in Features/<name>. The name is also its flag and its COH_FEATURE name.
function(coh_add_feature name)
    coh_add_slice(Features/${name})
    set_property(GLOBAL APPEND PROPERTY COH_FEATURES ${name})
endfunction()

coh_add_slice(Kernel)

# ---- Features, in the order their INI packs load, for example: coh_add_feature(Sectors) ----

coh_add_feature(Sectors)
coh_add_feature(GeneralsArsenal)

# ---- The list of features the Kernel registers ----

get_property(features GLOBAL PROPERTY COH_FEATURES)
set(COH_FEATURE_DECLARATIONS "")
set(COH_FEATURE_REGISTRATIONS "")
foreach(feature IN LISTS features)
    string(APPEND COH_FEATURE_DECLARATIONS "CoH::FeatureInfo coh_feature_${feature}();\n")
    string(APPEND COH_FEATURE_REGISTRATIONS "\tregistry.add(coh_feature_${feature}());\n")
endforeach()
configure_file(Kernel/Engine/FeatureList.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/FeatureList.cpp @ONLY)
target_sources(zi_coh INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/FeatureList.cpp)

# ---- Tests ----

if(RTS_BUILD_OPTION_TESTS AND TARGET z_googletest)
    get_property(tests GLOBAL PROPERTY COH_TEST_SOURCES)
    target_sources(z_googletest PRIVATE ${tests})
    target_include_directories(z_googletest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endif()
