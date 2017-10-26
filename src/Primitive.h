/*---------------------------------------------------------------------------------------
  Primitive.h
  -------------------------------------------------------------------------------------
  - Objective-3D is under LGPL license (see license.txt)
  - mailto:objective3d@free.fr
  - http://o3d.dreamoverflow.com
  -------------------------------------------------------------------------------------
  Brief: 
---------------------------------------------------------------------------------------*/

#ifdef _MSC_VER
#pragma once
#endif

#ifndef __O3DPrimitive_H
#define __O3DPrimitive_H

#include <o3d/Math/Vector3.h>

#include "Tools.h"
#include "Infos.h"

#ifdef _MSC_VER
#pragma warning(disable:4290)
#pragma warning(disable:4275)
#endif

#define ACCESS_DEFINITION(TYPE, NAME) \
    void Set_##NAME(TYPE _value) { m_##NAME = _value; } \
    const TYPE Get_##NAME() const { return m_##NAME; }

#define ACCESS_DEFINITION_REFERENCE(TYPE, NAME) \
    TYPE & Get_##NAME() { return m_##NAME; } \
    const TYPE & Get_##NAME() const { return m_##NAME; }

class O3DVertex;
class O3DFace;

typedef CLMSmartPointer<O3DVertex> PO3DVertex;
typedef CLMSmartPointer<O3DFace> PO3DFace;

enum O3DVertexType { O3D_VERTEX_NORMAL, O3D_VERTEX_CORNER, O3D_VERTEX_EDGE, O3D_VERTEX_LOCKED };
enum O3DEdgeType { O3D_EDGE_0, O3D_EDGE_1, O3D_EDGE_2, O3D_EDGE_3, O3D_EDGE_NO };

/*===================================================
				Classe O3DVertex
===================================================*/


class O3DCLM_API O3DVertex
{
public:

	O3DVertex();
	O3DVertex(const O3DVertex & _which);
	~O3DVertex();

	void Destroy();

	ACCESS_DEFINITION(O3DVector3, point);
	ACCESS_DEFINITION(PO3DVertex, geom);

	ACCESS_DEFINITION(O3D_UINT, id);
	ACCESS_DEFINITION(O3D_UINT, index);

	ACCESS_DEFINITION(O3DVertexType, type);
	ACCESS_DEFINITION(O3DEdgeType, edge);

	ACCESS_DEFINITION_REFERENCE(O3D_INT, i);
	ACCESS_DEFINITION_REFERENCE(O3D_INT, j);

	static O3D_INT GetNewId();

	static O3D_INT GetRefCounter();

private:

	O3DVector3 m_point;
	PO3DVertex m_geom;

	O3D_UINT m_id;
	O3D_UINT m_index;				///< Index du vertex dans le vector de O3DTerrainGenerator

	O3DVertexType m_type;
	O3DEdgeType m_edge;

	O3D_INT m_i;				///< Initial coordinate X on the heightmap
	O3D_INT m_j;

	static O3D_INT class_ref_counter;
	static O3D_INT class_id_counter;
};



/*===================================================
				Classe O3DFace
===================================================*/

class O3DCLM_API O3DFace
{
public:

/*===================================================
					Constructors
===================================================*/

	O3DFace();
	O3DFace(const O3DFace & _which);
	~O3DFace();

	void Destroy();

/*===================================================
					Member functions
===================================================*/

	O3D_INT FindNeighbor(PO3DFace _find) const;
	O3D_INT FindVertex(PO3DVertex _find) const;

	PO3DFace GetNextNeighbor(O3D_INT _index) const;
	PO3DFace GetPrevNeighbor(O3D_INT _index) const;

	PO3DFace GetNextNeighbor(PO3DFace _from) const;
	PO3DFace GetPrevNeighbor(PO3DFace _from) const;

	PO3DFace GetNextNeighbor(PO3DVertex _from) const;
	PO3DFace GetPrevNeighbor(PO3DVertex _from) const;

	PO3DVertex GetNextVertex(O3D_INT _index) const;
	PO3DVertex GetPrevVertex(O3D_INT _index) const;

	PO3DVertex GetNextVertex(PO3DVertex _from) const;
	PO3DVertex GetPrevVertex(PO3DVertex _from) const;

	void SetVertex(O3D_INT _index, PO3DVertex _pointer);
	PO3DVertex GetVertex(O3D_INT _index) const;
	PO3DVertex GetVertex(PO3DFace _face) const;

	void SetNeighbor(O3D_INT _index, PO3DFace _pointer);
	PO3DFace GetNeighbor(O3D_INT _index) const;
	PO3DFace GetNeighbor(PO3DVertex _vertex) const;

	void Check() const;

	ACCESS_DEFINITION(O3D_UINT, id);
	ACCESS_DEFINITION(O3D_UINT, index);
	ACCESS_DEFINITION(O3D_BOOL, active);

	static O3D_INT GetNewId();

	static O3D_INT GetRefCounter();

private:

/*===================================================
					Member variables
===================================================*/

	PO3DVertex m_vertices[3]; // indices des vertices dans le vector
	PO3DFace m_neighbors[3]; // indices des faces

	O3D_UINT m_id; // correspond à l'indice dans le tableau du heightmap <= non
	O3D_INT m_index;

	O3D_BOOL m_active;

/*===================================================
					Static variables
===================================================*/

	static O3D_INT class_ref_counter;
	static O3D_INT class_id_counter;
};

#undef ACCESS_DEFINITION
#undef ACCESS_DEFINITION_REFERENCE

#endif // __O3DPrimitive_H
