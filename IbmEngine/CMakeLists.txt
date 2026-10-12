# IBM Engine: this fork's own features, kept apart from the original engine code.
# Each feature is a vertical slice under Features/<Name> with its C++, INI pack and notes.
# The sources compile into the Zero Hour engine library (z_gameengine), like Core's sources do.

add_library(ibm_engine INTERFACE)

target_sources(ibm_engine INTERFACE
    Features/Accuracy/Accuracy.cpp
    Features/Accuracy/Accuracy.h
    Features/Demo/Demo.cpp
    Features/Demo/Demo.h
    Features/Economy/Economy.cpp
    Features/Economy/Economy.h
    Features/FeatureFlags/FeatureFlags.cpp
    Features/FeatureFlags/FeatureFlags.h
    Features/FeatureFlags/FeatureFlagsInternal.h
    Features/FeatureFlags/FeaturePacks.cpp
    Features/FeatureFlags/FeatureSettings.cpp
    Features/FeatureFlags/ObjectCopy.cpp
    Features/Squads/SquadAI.cpp
    Features/Squads/SquadAttack.cpp
    Features/Squads/SquadCommands.cpp
    Features/Squads/SquadReinforce.cpp
    Features/Squads/Squads.h
    Features/Squads/SquadsInternal.h
    Features/Squads/SquadTransport.cpp
    Features/Squads/SquadUpdate.cpp
    Features/Squads/SquadUpdate.h
)

target_include_directories(ibm_engine INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
