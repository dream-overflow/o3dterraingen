/*---------------------------------------------------------------------------------------
  Infos.h :created on 16/12/2003, 15h54
  -------------------------------------------------------------------------------------
  - Objective-3D is under copyright (c) 2006 Dream Overflow (see license.txt)
  - mailto:objective3d@free.fr
  - http://o3d.dreamoverflow.com
  -------------------------------------------------------------------------------------
  Brief: 
---------------------------------------------------------------------------------------*/

#ifdef _MSC_VER
#pragma once
#endif

#ifndef __O3DTERRAINGEN_INFOS_H
#define __O3DTERRAINGEN_INFOS_H

/*
** API define depend on OS and dll exporting type
*/
#if (defined(__UNIX__) | defined(__APPLE__) | defined(SWIG))
	#define O3DCLM_API
#else /* Windows */
	#define O3DCLM_API
#endif /* _UNIX_ */


#endif // __O3DTERRAINGEN_INFOS_H
