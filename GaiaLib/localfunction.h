#pragma once
#include "def.h"
#include "gaia.h"
#include <time.h>

namespace Gaia
{
	class Script;
	int gaia_print(Script *gaia);
	int gaia_tostring(Script *gaia);
	int gaia_toInt(Script *gaia);
	int gaia_toNumber(Script *gaia);
	int gaia_getTimer(Script *gaia);
	int gaia_typeof(Script *gaia);
	int gaia_toLowerCase(Script *gaia);
	int gaia_toUpperCase(Script *gaia);
}
