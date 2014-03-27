/*
 * main.cpp
 *
 * Created on: Jul 28, 2011
 *     Author: George Stark <george-u@yandex.com>
 *
 * License: GNU GPL v2
 *
 */

#include "application/cc_flasher.h"

//==============================================================================
int main(int argc, char **argv)
{
	CC_Flasher cc_flasher;

	return cc_flasher.execute(argc, argv) ? EXIT_SUCCESS : EXIT_FAILURE;
}
