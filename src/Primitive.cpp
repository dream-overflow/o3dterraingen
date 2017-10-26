/*---------------------------------------------------------------------------------------
  Primitive.cpp
  -------------------------------------------------------------------------------------
  - Objective-3D is under LGPL license (see license.txt)
  - mailto:objective3d@free.fr
  - http://o3d.dreamoverflow.com
  -------------------------------------------------------------------------------------
  Brief: 
---------------------------------------------------------------------------------------*/

#include "Primitive.h"

#include <o3d/Base/Base.h>

/*===================================================
				Classe O3DVertex
===================================================*/

O3D_INT O3DVertex::class_ref_counter = 0;
O3D_INT O3DVertex::class_id_counter = 0;

O3DVertex::O3DVertex():
	m_point(),
	m_geom(NULL),
	m_id(0),
	m_index(0),
	m_type(O3D_VERTEX_NORMAL),
	m_edge(O3D_EDGE_NO),
	m_i(-1),
	m_j(-1)
{
	O3DVertex::class_ref_counter++;
}

O3DVertex::O3DVertex(const O3DVertex & _which):
	m_point(_which.m_point),
	m_geom(_which.m_geom),
	m_id(_which.m_id),
	m_index(_which.m_index),
	m_type(_which.m_type),
	m_edge(_which.m_edge),
	m_i(_which.m_i),
	m_j(_which.m_j)
{
	O3DVertex::class_ref_counter++;
}

O3DVertex::~O3DVertex()
{
	O3DVertex::class_ref_counter--;
}

void O3DVertex::Destroy()
{
	m_geom.Release();
}

O3D_INT O3DVertex::GetNewId()
{
	O3DVertex::class_id_counter++;
	return O3DVertex::class_id_counter;
}

O3D_INT O3DVertex::GetRefCounter()
{
	return O3DVertex::class_ref_counter;
}

/*===================================================
				Classe O3DFace
===================================================*/

O3D_INT O3DFace::class_id_counter = 0;
O3D_INT O3DFace::class_ref_counter = 0;

O3DFace::O3DFace():
	m_id(0),
	m_index(-1),
	m_active(O3D_FALSE)
{
	m_vertices[0] = (O3DVertex*)NULL;
	m_vertices[1] = (O3DVertex*)NULL;
	m_vertices[2] = (O3DVertex*)NULL;

	m_neighbors[0] = (O3DFace*)NULL;
	m_neighbors[1] = (O3DFace*)NULL;
	m_neighbors[2] = (O3DFace*)NULL;

	O3DFace::class_ref_counter++;
}

O3DFace::O3DFace(const O3DFace & _which):
	m_id(_which.m_id),
	m_index(_which.m_index),
	m_active(_which.m_active)
{
	m_vertices[0] = _which.m_vertices[0];
	m_vertices[1] = _which.m_vertices[1];
	m_vertices[2] = _which.m_vertices[2];

	m_neighbors[0] = _which.m_neighbors[0];
	m_neighbors[1] = _which.m_neighbors[1];
	m_neighbors[2] = _which.m_neighbors[2];

	O3DFace::class_ref_counter++;
}

O3DFace::~O3DFace()
{
	O3DFace::class_ref_counter--;
}

void O3DFace::Destroy()
{
	m_vertices[0].Release();
	m_vertices[1].Release();
	m_vertices[2].Release();

	m_neighbors[0].Release();
	m_neighbors[1].Release();
	m_neighbors[2].Release();
}

/*===================================================
				Fonctions de recherche
===================================================*/

O3D_INT O3DFace::FindNeighbor(PO3DFace _find) const
{
	O3D_INT found = -1;

	for (O3D_INT i = 0 ; i < 3 ; ++i)
	{
		if (m_neighbors[i] == _find)
		{
			O3D_ASSERT(found == -1);
			found = i;
		}
	}

	return found;
}

O3D_INT O3DFace::FindVertex(PO3DVertex _find) const
{
	O3D_INT found = -1;

	for (O3D_INT i = 0 ; i < 3 ; ++i)
	{
		if (m_vertices[i] == _find)
		{
			O3D_ASSERT(found == -1);
			found = i;
		}
	}

	return found;
}

/*===================================================
			Fonctions relatives aux voisins
===================================================*/

PO3DFace O3DFace::GetNextNeighbor(O3D_INT _index) const
{
  O3D_ASSERT((_index >= 0) && (_index < 3));
  
  return m_neighbors[(_index+1)%3];
}

PO3DFace O3DFace::GetPrevNeighbor(O3D_INT _index) const
{
  O3D_ASSERT((_index >= 0) && (_index < 3));
  
  return m_neighbors[(_index+2)%3];
}

PO3DFace O3DFace::GetNextNeighbor(PO3DFace _from) const
{
	O3D_INT index = FindNeighbor(_from);

	O3D_ASSERT(index != -1);

	return m_neighbors[(index+1)%3];
}

PO3DFace O3DFace::GetPrevNeighbor(PO3DFace _from) const
{
	O3D_INT index = FindNeighbor(_from);

	O3D_ASSERT(index != -1);

	return m_neighbors[(index+2)%3];
}

PO3DFace O3DFace::GetNextNeighbor(PO3DVertex _from) const
{
	O3D_INT index = FindVertex(_from);

	O3D_ASSERT(index != -1);

	return m_neighbors[(index+1)%3];
}

PO3DFace O3DFace::GetPrevNeighbor(PO3DVertex _from) const
{
	O3D_INT index = FindVertex(_from);

	O3D_ASSERT(index != -1);

	return m_neighbors[(index+2)%3];
}

/*===================================================
			Fonctions relatives aux vertex
===================================================*/

PO3DVertex O3DFace::GetNextVertex(O3D_INT _index) const
{
	O3D_ASSERT((_index >= 0) && (_index < 3));
  
	return m_vertices[(_index+1)%3];
}

PO3DVertex O3DFace::GetPrevVertex(O3D_INT _index) const
{
	O3D_ASSERT((_index >= 0) && (_index < 3));
  
	return m_vertices[(_index+2)%3];
}

PO3DVertex O3DFace::GetNextVertex(PO3DVertex _from) const
{
	O3D_INT index = FindVertex(_from);

	O3D_ASSERT(index != -1);

	return m_vertices[(index+1)%3];
}

PO3DVertex O3DFace::GetPrevVertex(PO3DVertex _from) const
{
	O3D_INT index = FindVertex(_from);

	O3D_ASSERT(index != -1);

	return m_vertices[(index+2)%3];
}

void O3DFace::SetVertex(O3D_INT _index, PO3DVertex _pointer)
{
	O3D_ASSERT((_index >= 0) && (_index < 3));

	m_vertices[_index] = _pointer;
}

PO3DVertex O3DFace::GetVertex(O3D_INT _index) const
{
	O3D_ASSERT((_index >= 0) && (_index < 3));

	return m_vertices[_index];
}

PO3DVertex O3DFace::GetVertex(PO3DFace _face) const
{
	O3D_INT found = FindNeighbor(_face);

	O3D_ASSERT(found != -1);

	return m_vertices[found];
}

void O3DFace::SetNeighbor(O3D_INT _index, PO3DFace _pointer)
{
	O3D_ASSERT((_index >= 0) && (_index < 3));

	m_neighbors[_index] = _pointer;
}

PO3DFace O3DFace::GetNeighbor(O3D_INT _index) const
{
	O3D_ASSERT((_index >= 0) && (_index < 3));

	return m_neighbors[_index];
}

PO3DFace O3DFace::GetNeighbor(PO3DVertex _vertex) const
{
	O3D_INT found = FindVertex(_vertex);

	O3D_ASSERT(found != -1);

	return m_neighbors[found];
}

void O3DFace::Check() const
{
	PO3DFace neighbor = (O3DFace*)NULL;
	PO3DVertex vertex1 = (O3DVertex*)NULL;
	PO3DVertex vertex2 = (O3DVertex*)NULL;

	for (O3D_INT i=0 ; i<3 ; ++i)
	{
		// aucun des vertices n'est NULL
		if (!m_vertices[i]) { O3D_ASSERT(0); } //Raise(E_not_valid, "Vertex == NULL");

		// qu'il n'y a pas deux fois le meme
		for (O3D_INT j=0 ; j<3 ; ++j)
			if ((i != j) && (m_vertices[i] == m_vertices[j])) { O3D_ASSERT(0); } //Raise(E_not_valid, "Deux vertex identiques");

		if ((m_vertices[i]->Get_type() != O3D_VERTEX_NORMAL) && (m_vertices[(i+1)%3]->Get_type() != O3D_VERTEX_NORMAL) && (m_vertices[(i+1)%3]->Get_edge() == m_vertices[i]->Get_edge()))
			if (m_neighbors[i])
				//Raise(E_not_valid, O3DString("Deux vertex non normal avec un voisin au milieu"));
			{ O3D_ASSERT(0); }

		// un voisin est nul ssi les deux vertices sont soit corner, soit edge
		if (!m_neighbors[i])
			if (!((m_vertices[i]->Get_type() > 0) && (m_vertices[(i+1)%3]->Get_type() > 0))) { O3D_ASSERT(0); } //Raise (E_not_valid, "Voisin NULL alors que un ou plus de vertex est normal");

		// correspondance avec les voisins
		neighbor = m_neighbors[i];
		if (neighbor)
		{
			vertex1 = m_vertices[i];
			vertex2 = m_vertices[(i+1)%3];
	  
			if (neighbor->FindVertex(vertex1) == -1) { O3D_ASSERT(0); } //Raise(E_not_valid, O3DString("Le vertex1 avec un voisin de correspond pas ") << i);
			if (neighbor->FindVertex(vertex2) == -1) { O3D_ASSERT(0); } //Raise(E_not_valid, O3DString("Le vertex2 avec un voisin de correspond pas ") << i);
			if (neighbor->GetNextVertex(vertex2) != vertex1) { O3D_ASSERT(0); } //Raise(E_not_valid, O3DString("Les deux vertices n'ont pas un ordre correct ") << i);
			if (neighbor->GetNeighbor(vertex2).Get() != this) { O3D_ASSERT(0); } //Raise(E_not_valid, O3DString("Erreur de lien avec un voisin ") << i);
		}
	}
}

O3D_INT O3DFace::GetNewId()
{
	O3DFace::class_id_counter++;
	return O3DFace::class_id_counter;
}


O3D_INT O3DFace::GetRefCounter()
{
	return O3DFace::class_ref_counter;
}
