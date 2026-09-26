// UnitTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "stdafx.h"
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#undef min
#undef max
#include <catch.hpp>

#include "LinkManager.h"



CExchangeLinkManager* g_pExchangeLinkManager = new CExchangeLinkManager();

//int main() {}