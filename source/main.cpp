#include "Papas_Framework.h"
#include <3ds.h>

//===============================================================================
// Framework is constructed as Global
Papas::Framework g_framework;
//===============================================================================


int main(int argc, char* argv[]) {
    
	PapasError ret;

	// Initialize our framework. This encapsulates all systems.
	ret = g_framework.init();
	ASSERT(ret == PAPAS_OK, "");

	while (aptMainLoop()) {
		ret = g_framework.update();

		if (ret == PAPAS_EXIT_REQUESTED)
			break;

		ASSERT(ret == PAPAS_OK, "");
	}

	ret = g_framework.terminate();
	ASSERT(ret == PAPAS_OK, "");

	return 0;
}
