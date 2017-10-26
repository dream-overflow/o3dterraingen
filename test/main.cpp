#include "../TerrainGen/TerrainGen.h"

#include <iostream>
#include <map>

int main()
{
	O3DZoneManip * pManip = new O3DZoneManip();

	/* Initialisation of the elevation buffer */
	O3D_INT * pSource = new O3D_INT[17*17];
	memset(pSource, 0, 17*17*sizeof(O3D_INT));

	/* Initialisation of the rules */
	O3D_T_LevelRules levelRules;
	O3DRule rule;

	rule.type = O3D_RULE_FACE_NUMBER_LIMIT;
	rule.value = 100;
	rule.useMaterial = O3D_FALSE;
	rule.useEnergetic = O3D_FALSE;

	levelRules.push_back(rule);

	O3D_T_LevelRulesArray zoneRules;
	zoneRules.push_back(levelRules);
	zoneRules.push_back(levelRules);

	/* Construction of the instructions */
	O3DZoneManipInfo info;
	info.pSource = pSource;
	info.pMaterial = NULL;
	info.pEnergetic = NULL;
	info.layerWidth = 17;
	info.layerHeight = 17;
	info.dx = 1.0f;
	info.dy = 1.0f;
	info.dz = 1.0f;
	info.rules = zoneRules;
	info.ce = 0.001f;
	info.cn = 0.001f;
	info.ck = 0.001f;

	O3DZoneManipResult result;

	pManip->Init(info);
	pManip->Start(result);

	std::cout << "Compteur de vertex : " << O3DVertex::GetRefCounter() << std::endl;
	std::cout << "Compteur de faces : " << O3DFace::GetRefCounter() << std::endl;

	PO3DVertex vertex;

	O3D_DELETE(pManip);

	std::cout << "Compteur de vertex : " << O3DVertex::GetRefCounter() << std::endl;
	std::cout << "Compteur de faces : " << O3DFace::GetRefCounter() << std::endl;

	system("PAUSE");

	return 0;
};