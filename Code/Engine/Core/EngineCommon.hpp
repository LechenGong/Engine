#pragma once
#pragma warning( disable : 26451 26812 26495 )

#define UNUSED(x) (void)(x);

#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

extern NamedStrings g_gameConfig; 
extern EventSystem* g_eventSystem;
extern DevConsole* g_devConsole;

typedef Vec2 FloatRange;

bool DoFloatRangeOverlap( FloatRange A, FloatRange B );

