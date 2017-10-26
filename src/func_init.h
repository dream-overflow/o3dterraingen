/*---------------------------------------------------------------------------------------
  Func_BuildFromAssembly.h :created on 16/12/2003, 15h54
  -------------------------------------------------------------------------------------
  - Objective-3D is under copyright (c) 2006 Dream Overflow (see license.txt)
  - mailto:objective3d@free.fr
  - http://o3d.dreamoverflow.com
  -------------------------------------------------------------------------------------
  Brief: 
---------------------------------------------------------------------------------------*/

/* Initialize the object with 4 other zoneManip by merging them.
 * Function used if the number of levels in instruction.rules is greater than one */
void O3DZoneManip::Init(const O3D_T_ZoneManipArray & _array)
{
	O3D_ASSERT((_array.Width() == 2) && (_array.Height() == 2));

    // The base zones are indexed with the following scheme
    // | 2 3 |
    // | 0 1 |

	// Next, we must verify that each object sent in parameter use the same heightmap size
	O3D_UINT child_heightmap_x = _array(0,0)->Width();
	O3D_UINT child_heightmap_y = _array(0,0)->Height();
	
	O3D_ASSERT((_array(1,0)->Width() == child_heightmap_x));
	O3D_ASSERT((_array(1,0)->Height() == child_heightmap_y));
	O3D_ASSERT((_array(0,1)->Width() == child_heightmap_x));
	O3D_ASSERT((_array(0,1)->Height() == child_heightmap_y));
	O3D_ASSERT((_array(1,1)->Width() == child_heightmap_x));
	O3D_ASSERT((_array(1,1)->Height() == child_heightmap_y));

	// One important point, we must assemble the heightmaps
	m_heightmap.SetSize(2 * child_heightmap_x - 1, 2 * child_heightmap_y - 1);

	O3D_BOOL ret = O3D_FALSE;

	ret = m_heightmap.Insert(_array(0,0)->Heightmap(), 0, 0);
	O3D_ASSERT(ret);
	ret = m_heightmap.Insert(_array(1,0)->Heightmap(), child_heightmap_x - 1, 0);
	O3D_ASSERT(ret);
	ret = m_heightmap.Insert(_array(0,1)->Heightmap(), 0, child_heightmap_y - 1);
	O3D_ASSERT(ret);
	ret = m_heightmap.Insert(_array(1,1)->Heightmap(), child_heightmap_x - 1, child_heightmap_y - 1);
	O3D_ASSERT(ret);

	// At this level, each vertex from each previous O3DTerrainGenerator object own a unique pair
	// of coordinates (i,j) which define the initial vertex position on each heightmap.
	// Those coordinates will change by an offset equal to the size of each heightmap.
    for (O3D_UINT y = 0 ; y < 2 ; ++y)
		for (O3D_UINT x = 0 ; x < 2 ; ++x)
		{
			for (O3D_UINT i = 0 ; i < _array(x,y)->Vertices().size() ; ++i)
			{
				_array(x,y)->Vertices()[i]->Get_i() += x * (child_heightmap_x - 1);
				_array(x,y)->Vertices()[i]->Get_j() += y * (child_heightmap_y - 1);
				_array(x,y)->Vertices()[i]->Set_point(O3DVector3(float(_array(x,y)->Vertices()[i]->Get_i()) * Dx(),
																 float(_array(x,y)->Vertices()[i]->Get_j()) * Dy(),
																 _array(x,y)->Vertices()[i]->Get_point()[Z]));
			}
		}

	// Declaration of zone pointers
	O3DZoneManip * zone00 = _array(0,0),
				 * zone10 = _array(1,0),
				 * zone01 = _array(0,1),
				 * zone11 = _array(1,1);

	// Next, we copy "zone00" into "this"
	m_vertices.insert(m_vertices.end(), zone00->m_vertices.begin(), zone00->m_vertices.end());
	m_faces.insert(m_faces.end(), zone00->m_faces.begin(), zone00->m_faces.end());

	zone00->m_faces.clear();
	zone00->m_vertices.clear();

/*==============================================================
Connection of the zone 00 and the zone 10
==============================================================*/

    PO3DVertex v1, v2, f1, f2;
    PO3DFace cur, init;
    O3D_INT found = 0;

    for (O3D_UINT i = 0 ; i < child_heightmap_y-1 ; ++i)
    {
    // Case of zone 1
		// We now that there are _size_x vertex along the edge between 0 and 1.
		// So we first find them, and next we find the faces which use them.
		found = zone10->FindVertexWithPosIJ(child_heightmap_x - 1, i);
        O3D_ASSERT(found != -1);
		v1 = zone10->Vertices()[found];
        
        found = zone10->FindVertexWithPosIJ(child_heightmap_x - 1, i+1);
        O3D_ASSERT(found != -1);
        v2 = zone10->Vertices()[found];

		// We look for the correct face which use v1 and v1 in the clock-wise direction.
		found = zone10->FindFacesByVertices(v1, v2);
        O3D_ASSERT(found != -1);
        cur = zone10->Faces()[found];

    // Case of 0
		// The same sceme is reproducted here since we need to link the faces of different zones.
        found = FindVertexWithPosIJ(child_heightmap_x - 1, i);
        O3D_ASSERT(found != -1);
        f2 = Vertices()[found];

        found = FindVertexWithPosIJ(child_heightmap_x - 1, i+1);
        O3D_ASSERT(found != -1);
        f1 = Vertices()[found];
        
        found = FindFacesByVertices(f1, f2);
        O3D_ASSERT(found != -1);
        init = Faces()[found];
        
    // Refresh of neighbors
        PO3DFace neighbor = cur->GetNeighbor(v2);
        PO3DFace prev = neighbor;
        
		// Next, we modify the vertex used by all faces using the vertex v2.
		// We turn around the vertex v2 until we reach the face on the edge.
        while (neighbor && (neighbor->GetNextVertex(v2)->Get_i() != O3D_INT(child_heightmap_x - 1)))
        {
            prev = neighbor;
            neighbor = prev->GetNeighbor(v2);
            prev->SetVertex(prev->FindVertex(v2), f1);
        }
        
    // Connection of 2 faces: cur (zone10) and init (zone00)
        cur->SetVertex(cur->FindVertex(v1), f2);
        cur->SetVertex(cur->FindVertex(v2), f1);
        cur->SetNeighbor(cur->FindVertex(f2), init);
        
        init->SetNeighbor(init->FindVertex(f1), cur);
        
    // The vertices shared by the zone 00 and the zone 10 are no more on a edge,
	// and so we must change their type.
        if (i < child_heightmap_y - 2)
        {
            f1->Set_type(O3D_VERTEX_NORMAL);
            f1->Set_edge(O3D_EDGE_NO);
        }
        else if (i == child_heightmap_y - 2)
        {
            f1->Set_type(O3D_VERTEX_EDGE);
            f1->Set_edge(O3D_EDGE_1);
        }
        
        if (i > 0)
        {
            f2->Set_type(O3D_VERTEX_NORMAL);
            f2->Set_edge(O3D_EDGE_NO);
        }
        else
        {
            f2->Set_type(O3D_VERTEX_EDGE);
            f2->Set_edge(O3D_EDGE_3);
        }
    }

    // On copie toutes les faces de 1 dans 0
	for (O3D_UINT i = 0 ; i < (O3D_UINT)zone10->Faces().size() ; ++i)
    {
        m_faces.push_back(zone10->Faces()[i]);
        m_faces.back()->Set_index(O3D_UINT(m_faces.size()) - 1);
    }

	zone10->m_faces.clear();
    
    // On copie la plupart des vertex de 1 dans 0
    for (O3D_UINT i = 0 ; i < O3D_UINT(zone10->Vertices().size()) ; ++i)
    {
		O3D_ASSERT(zone10->Vertices()[i]->Get_i() >= 0);

        if ((O3D_UINT)zone10->Vertices()[i]->Get_i() > child_heightmap_x - 1)
        {
            m_vertices.push_back(zone10->Vertices()[i]);
            m_vertices.back()->Set_index(O3D_UINT(m_vertices.size()) - 1);
        }
    }

	zone10->m_vertices.clear();

#ifdef _DEBUG

	for (O3D_UINT i = 0 ; i < (O3D_UINT)m_faces.size() ; ++i)
    {
        try
        {
            m_faces[i]->Check();
        }
        catch(...)
        {
            O3D_ASSERT(0);
        }
    }

#endif

/*==============================================================
Connection of the zone 01 and the zone 11
==============================================================*/

    for (O3D_UINT i = 0 ; i < child_heightmap_x-1 ; ++i)
    {
    // Case of zone 11
		found = zone11->FindVertexWithPosIJ(child_heightmap_x - 1, i + child_heightmap_y - 1);
        O3D_ASSERT(found != -1);
        v1 = zone11->Vertices()[found];
        
        found = zone11->FindVertexWithPosIJ(child_heightmap_x - 1, i+1  + child_heightmap_y - 1);
        O3D_ASSERT(found != -1);
        v2 = zone11->Vertices()[found];

		found = zone11->FindFacesByVertices(v1, v2);
        O3D_ASSERT(found != -1);
        cur = zone11->Faces()[found];

    // A propos de 01
        found = zone01->FindVertexWithPosIJ(child_heightmap_x - 1, i + child_heightmap_y - 1);
        O3D_ASSERT(found != -1);
        f2 = zone01->Vertices()[found];

        found = zone01->FindVertexWithPosIJ(child_heightmap_x - 1, i+1 + child_heightmap_y - 1);
        O3D_ASSERT(found != -1);
        f1 = zone01->Vertices()[found];
        
        found = zone01->FindFacesByVertices(f1, f2);
        O3D_ASSERT(found != -1);
        init = zone01->Faces()[found];
        
    // Mise a jour des voisins éventuels
    
        PO3DFace neighbor = cur->GetNeighbor(v2);
        PO3DFace prev = neighbor;
        
        while (neighbor && (neighbor->GetNextVertex(v2)->Get_i() != O3D_INT(child_heightmap_x - 1))) // on s'assure que c'est pas le voisin suivant sur le bord
        {
            prev = neighbor;
            neighbor = prev->GetNeighbor(v2);
            prev->SetVertex(prev->FindVertex(v2), f1);
        }
        
    // Connexion des deux faces
        cur->SetVertex(cur->FindVertex(v1), f2);
        cur->SetVertex(cur->FindVertex(v2), f1);
        cur->SetNeighbor(cur->FindVertex(f2), init);
        
        init->SetNeighbor(init->FindVertex(f1), cur);
        
    // Modification des types de vertex
        if (i < child_heightmap_y - 2)
        {
            f1->Set_type(O3D_VERTEX_NORMAL);
            f1->Set_edge(O3D_EDGE_NO);
        }
        else if (i == child_heightmap_y - 2)
        {
            f1->Set_type(O3D_VERTEX_EDGE);
            f1->Set_edge(O3D_EDGE_1);
        }
        
        if (i > 0)
        {
            f2->Set_type(O3D_VERTEX_NORMAL);
            f2->Set_edge(O3D_EDGE_NO);
        }
        else
        {
            f2->Set_type(O3D_VERTEX_EDGE);
            f2->Set_edge(O3D_EDGE_3);
        }
    }

    // On copie toutes les faces de 3 dans 2
    for (O3D_UINT i = 0 ; i < O3D_UINT(zone11->m_faces.size()) ; ++i)
    {
        zone01->m_faces.push_back(zone11->m_faces[i]);
        zone01->m_faces.back()->Set_index(O3D_UINT(zone01->m_faces.size()) - 1);
    }

	zone11->m_faces.clear();
    
    // On copie la plupart des vertex de 3 dans 2
    for (O3D_UINT i = 0 ; i < O3D_UINT(zone11->m_vertices.size()) ; ++i)
    {
		O3D_ASSERT(zone11->m_vertices[i]->Get_i() >= 0);

        if (O3D_UINT(zone11->m_vertices[i]->Get_i()) > child_heightmap_x - 1)
        {
            zone01->m_vertices.push_back(zone11->m_vertices[i]);
            zone01->m_vertices.back()->Set_index(O3D_UINT(zone01->m_vertices.size()) - 1);
        }
    }

	zone11->m_vertices.clear();
    
#ifdef _DEBUG

    for (O3D_UINT i = 0 ; i < O3D_UINT(zone01->m_faces.size()) ; ++i)
    {
        try
        {
            zone01->m_faces[i]->Check();
        }
        catch(...)
        {
			O3D_ASSERT(0);
        }
    }

#endif

/*==============================================================
Connection of the zone "this" and the zone 01
==============================================================*/

    for (O3D_UINT i = 0; i < 2 * child_heightmap_x-2; ++i)
    {
    // case of the zone 01
		found = zone01->FindVertexWithPosIJ(i+1, child_heightmap_y - 1);
        O3D_ASSERT(found != -1);
        v1 = zone01->Vertices()[found];
        
        found = zone01->FindVertexWithPosIJ(i, child_heightmap_y - 1);
        O3D_ASSERT(found != -1);
        v2 = zone01->Vertices()[found];

		found = zone01->FindFacesByVertices(v1, v2);
        O3D_ASSERT(found != -1);
        cur = zone01->Faces()[found];

    // A propos de 0
		found = this->FindVertexWithPosIJ(i+1, child_heightmap_y - 1);
        O3D_ASSERT(found != -1);
        f2 = this->m_vertices[found];

        found = this->FindVertexWithPosIJ(i, child_heightmap_y - 1);
        O3D_ASSERT(found != -1);
        f1 = this->m_vertices[found];
        
		found = this->FindFacesByVertices(f1, f2);
        O3D_ASSERT(found != -1);
        init = this->m_faces[found];
        
    // Mise a jour des voisins éventuels
    
        PO3DFace neighbor = cur->GetPrevNeighbor(v1);
        PO3DFace prev = neighbor;
        
        while (neighbor && (neighbor->GetPrevVertex(v1)->Get_j() != O3D_INT(child_heightmap_y - 1))) // on s'assure que c'est pas le voisin suivant sur le bord
        {
            prev = neighbor;
            neighbor = prev->GetPrevNeighbor(v1);
            prev->SetVertex(prev->FindVertex(v1), f2);
        }
        
    // Connexion des deux faces
        cur->SetVertex(cur->FindVertex(v1), f2);
        cur->SetVertex(cur->FindVertex(v2), f1);
        cur->SetNeighbor(cur->FindVertex(f2), init);
        
        init->SetNeighbor(init->FindVertex(f1), cur);
        
    // Modification des types de vertex
        if (i > 0)
        {
            f1->Set_type(O3D_VERTEX_NORMAL);
            f1->Set_edge(O3D_EDGE_NO);
        }
        else
        {
            f1->Set_type(O3D_VERTEX_EDGE);
            f1->Set_edge(O3D_EDGE_0);
        }

        if (i < 2*(child_heightmap_x - 1) - 1)
        {
            f2->Set_type(O3D_VERTEX_NORMAL);
            f2->Set_edge(O3D_EDGE_NO);
        }
        else if (i == 2*(child_heightmap_x - 1) - 1)
        {
            f2->Set_type(O3D_VERTEX_EDGE);
            f2->Set_edge(O3D_EDGE_2);
        }
    }

    // On copie toutes les faces de 2 dans 0
    for (O3D_UINT i = 0 ; i < O3D_UINT(zone01->m_faces.size()) ; ++i)
    {
        m_faces.push_back(zone01->m_faces[i]);
        m_faces.back()->Set_index(O3D_UINT(zone01->m_faces.size()) - 1);
    }

	zone01->m_faces.clear();
    
    // On copie la plupart des vertex de 2 dans 0
    for (O3D_UINT i = 0 ; i < O3D_UINT(zone01->m_vertices.size()) ; ++i)
    {
		O3D_ASSERT(zone01->m_vertices[i]->Get_j() >= 0);

        if (O3D_UINT(zone01->m_vertices[i]->Get_j()) > child_heightmap_y - 1)
        {
            m_vertices.push_back(zone01->m_vertices[i]);
            m_vertices.back()->Set_index(O3D_UINT(m_vertices.size()) - 1);
        }
     }

	zone01->m_vertices.clear();

#ifdef _DEBUG

    for (O3D_UINT i = 0 ; i < O3D_UINT(m_faces.size()) ; ++i)
    {
        try
        {
            m_faces[i]->Check();
        }
        catch(...)
        {
            O3D_ASSERT(0);
        }
    }

#endif

	// Pour finir, on réarrange les index
	for (O3D_UINT k = 0 ; k < O3D_UINT(m_faces.size()) ; ++k)
		m_faces[k]->Set_index(k);

	for (O3D_UINT k = 0 ; k < O3D_UINT(m_vertices.size()) ; ++k)
	{
		m_vertices[k]->Set_index(k);

/*		if (m_vertices[k]->Get_type() != O3D_VERTEX_NORMAL)
		{
			O3D_ASSERT((m_vertices[k]->Get_i() == 0) || (m_vertices[k]->Get_i() == 8)
				       || (m_vertices[k]->Get_j() == 0) || (m_vertices[k]->Get_j() == 4));
		}

		if ((m_vertices[k]->Get_i() == 0) || (m_vertices[k]->Get_i() == 8)
				       || (m_vertices[k]->Get_j() == 0) || (m_vertices[k]->Get_j() == 4))
		{
		}
		else
		{
			m_vertices[k]->Set_type(O3D_VERTEX_NORMAL);
			m_vertices[k]->Set_edge(O3D_EDGE_NO);
		}*/
	}

	m_initFaceNumber = O3D_UINT(m_faces.size());
	m_initVertexNumber = O3D_UINT(m_vertices.size());

	m_lastLodFaceNumber = m_initFaceNumber;
	m_lastLodVertexNumber = m_lastLodVertexNumber;

	m_init = O3D_TRUE;
}
