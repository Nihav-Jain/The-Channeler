// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine.h"

#if BUILD_SHIPPING
	#define CHANNELER_SHIP_TEST 0
	#define CHANNELER_SHIP 1
#else
	#define CHANNELER_SHIP_TEST 1
	#define CHANNELER_SHIP 0
#endif
