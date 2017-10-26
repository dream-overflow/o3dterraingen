/*---------------------------------------------------------------------------------------
  Func_Simplify.cpp :created on 16/12/2003, 15h54
  -------------------------------------------------------------------------------------
  - Objective-3D is under copyright (c) 2006 Dream Overflow (see license.txt)
  - mailto:objective3d@free.fr
  - http://o3d.dreamoverflow.com
  -------------------------------------------------------------------------------------
  Brief:
---------------------------------------------------------------------------------------*/

/* Apply the first simplification */
void O3DZoneManip::Simplify()
{
	O3D_ASSERT(m_transformations.size() > 0);

// Avant de simplifier en appliquant la transformation du haut de la pile
// on détermine l'ensemble des transformations à recalculer
// On s'apercoit qu'une transformation doit etre racalculer des que un de ses deux vertices appartient au voisinage de
// la transformation que l'on va effectuer.

	O3D_IT_TransformationMultimap first = m_transformations.begin();
	O3DTransformation t_transf = first->second;
	m_transformations.erase(first);

	PO3DFace f1 = t_transf.pFace;
	PO3DFace f2 = t_transf.pFace->GetNeighbor(t_transf.pVertex);

	PO3DVertex v1 = t_transf.pVertex;
	PO3DVertex v2 = t_transf.pFace->GetNextVertex(v1);

	PO3DFace f1r = f1->GetNeighbor(v2); // voisin droit de la face 1
	PO3DFace f1l = f1->GetNeighbor(f1->GetNextVertex(v2));

	PO3DFace f2r = (f2) ? f2->GetNeighbor(v1) : PO3DFace(); // voisin droit de la face 2
	PO3DFace f2l = (f2) ? f2->GetPrevNeighbor(f1) : PO3DFace();

	PO3DVertex target = t_transf.StaticVertex();
	PO3DVertex vmove = t_transf.MovableVertex();

	// Vertex en bout des diamand

	PO3DVertex e1 = f1->GetPrevVertex(v1); // e1 pour extreme 1, v1 est en fait new_vertex
	PO3DVertex e2 = (f2) ? f2->GetPrevVertex(v2) : PO3DVertex();

	O3D_ASSERT(IsLegalTransformation(f1, vmove, target));

//	target->Set_current_lvl(m_current_level+1); 23082006

//	vmove->Set_active(O3D_FALSE); 23082006
	vmove->Set_geom(target);
	// Si le vertex que l'on dépace a deja bougé, alors il n'appartient pas au prochain niveau


	// Avant de pouvoir modifier la pile des transformations, on construit la liste des vertices du voisinage.
	// Si une transformation utilise une de ces vertices, il faut recalculer

	O3D_T_FaceArray face_altered;
	O3D_T_VertexArray vertex_list;

	if (vmove == v1)
	{
		BuildNeighborList(f1, v1, f1, f2, face_altered);

		if (f2)
			face_altered.push_back(f2); // J'ajoute manuellement la face qui est en fait "_to_face" de la fonction
										// BuildNeighborList car d'autres fonctions utilisent précisément le fait
										// que la face de destination n'appartient pas au vector "face_altered".

		for (O3D_IT_FaceArray _it = face_altered.begin(); _it != face_altered.end(); _it++)
			vertex_list.push_back((*_it)->GetPrevVertex(v1));
	}
	else
	{
		BuildNeighborList(f2, v2, f2, f1, face_altered);

		face_altered.push_back(f1);

		for (O3D_IT_FaceArray _it = face_altered.begin(); _it != face_altered.end(); _it++)
			vertex_list.push_back((*_it)->GetNextVertex(v2));
	}

	O3DTransformation f1l_f1;
	// On s'assure que le modèle est correct
	if ((f1l) && (f1l->Get_id() < f1->Get_id()))
	{
		f1l_f1.pFace = f1l;
		f1l_f1.pVertex = v1;
	}
	else
	{
		f1l_f1.pFace = f1;
		f1l_f1.pVertex = e1;
	}

	O3DTransformation f1r_f1;
	if ((f1r) && (f1r->Get_id() < f1->Get_id()))
	{
		f1r_f1.pFace = f1r;
		f1r_f1.pVertex = e1;
	}
	else
	{
		f1r_f1.pFace = f1;
		f1r_f1.pVertex = v2;
	}

	O3DTransformation f2l_f2;
	if ((f2l) && (f2l->Get_id() < f2->Get_id()))
	{
		f2l_f2.pFace = f2l;
		f2l_f2.pVertex = v2;
	}
	else
	{
		f2l_f2.pFace = f2;
		f2l_f2.pVertex = e2;
	}

	O3DTransformation f2r_f2;
	if ((f2r) && (f2r->Get_id() < f2->Get_id()))
	{
		f2r_f2.pFace = f2r;
		f2r_f2.pVertex = e2;
	}
	else
	{
		f2r_f2.pFace = f2;
		f2r_f2.pVertex = v1;
	}


	// On a créé deux modèles qui servent a tester, maintenant, on créer la solution des modèles
	O3DTransformation _sol_f1lr; O3D_BOOL _sol_f1lr_used = false;
	if ((f1l) && (f1r))
	{
		// la face de plus petit id prend la transformation
		if (f1l->Get_id() < f1r->Get_id())
		{
			_sol_f1lr.pFace = f1l;
			_sol_f1lr.pVertex = v1;
		}
		else
		{
			_sol_f1lr.pFace = f1r;
			_sol_f1lr.pVertex = e1;
		}
	}
	else if (!f1l)
	{
		O3D_ASSERT(f1r); // Les deux ne le sont pas en meme temps

		// pas le choix, f1r prend la transformation
		_sol_f1lr.pFace = f1r;
		_sol_f1lr.pVertex = e1;
	}
	else if (!f1r)
	{
		O3D_ASSERT(f1l); // Les deux ne le sont pas en meme temps

		// pas le choix, f1r prend la transformation
		_sol_f1lr.pFace = f1l;
		_sol_f1lr.pVertex = v1;
	}

	// Solution pour la face f2
	O3DTransformation _sol_f2lr; O3D_BOOL _sol_f2lr_used = false;
	if (f2)
	{
		if ((f2l) && (f2r))
		{
			// la face de plus petit id prend la transformation
			if (f2l->Get_id() < f2r->Get_id())
			{
				_sol_f2lr.pFace = f2l;
				_sol_f2lr.pVertex = v2;
			}
			else
			{
				_sol_f2lr.pFace = f2r;
				_sol_f2lr.pVertex = e2;
			}
		}
		else if (!f2l)
		{
			O3D_ASSERT(f2r); // Les deux ne le sont pas en meme temps

			// pas le choix, f2r prend la transformation
			_sol_f2lr.pFace = f2r;
			_sol_f2lr.pVertex = e2;
		}
		else if (!f2r)
		{
			O3D_ASSERT(f2l); // Les deux ne le sont pas en meme temps

			// pas le choix, f2r prend la transformation
			_sol_f2lr.pFace = f2l;
			_sol_f2lr.pVertex = v2;
		}
	}

// Pour le premier passage, on s'occupe de garder et de supprimer ou de corriger les transformations
// On verra apres pour celles qui doivent etre recalculer


	O3D_IT_TransformationMultimap it_map = m_transformations.begin(), eraseIt;

	while (it_map != m_transformations.end())
	{
		if ((it_map->second == f1l_f1) || (it_map->second == f1r_f1))
		{
			if (!_sol_f1lr_used) // si on a pas encore eu cette transformation
			{
				it_map->second = _sol_f1lr;
				_sol_f1lr_used = true;

				it_map++;
				continue;
			}
			else // alors on a une transformation qui a déja un équivalent
			{
				eraseIt = it_map;
				++it_map;

				m_transformations.erase(eraseIt);
				continue;
			}
		}

		if ((f2) && ((it_map->second == f2l_f2) || (it_map->second == f2r_f2)))
		{
			if (!_sol_f2lr_used) // si on a pas encore eu cette transformation
			{
				it_map->second = _sol_f2lr;
				_sol_f2lr_used = true;

				it_map++;
				continue;
			}
			else // alors on a une transformation qui a déja un équivalent
			{
				eraseIt = it_map;
				++it_map;

				m_transformations.erase(eraseIt);
				continue;
			}
		}

		it_map++;
	}



	O3D_IT_TransformationList it_list = m_deleted_transformations.begin() ;

	while (it_list != m_deleted_transformations.end())
	{
		if ((*it_list == f1l_f1) || (*it_list == f1r_f1))
		{
			if (!_sol_f1lr_used) // si on a pas encore eu cette transformation
			{
				*it_list = _sol_f1lr;
				_sol_f1lr_used = true;

				it_list++;
				continue;
			}
			else // alors on a une transformation qui a déja un équivalent
			{
				it_list = m_deleted_transformations.erase(it_list);
				continue;
			}
		}

		if ((f2) && ((*it_list == f2l_f2) || (*it_list == f2r_f2)))
		{
			if (!_sol_f2lr_used) // si on a pas encore eu cette transformation
			{
				*it_list = _sol_f2lr;
				_sol_f2lr_used = true;

				it_list++;
				continue;
			}
			else // alors on a une transformation qui a déja un équivalent
			{
				it_list = m_deleted_transformations.erase(it_list);
				continue;
			}
		}

		it_list++;
	}



	std::vector<O3DTransformation> need_refresh;

	it_map = m_transformations.begin();
	O3DTransformation * current = NULL;
	bool find = false;

	while (it_map != m_transformations.end())
	{
		current = &it_map->second;

		O3D_ASSERT(current->pFace != f1);
		O3D_ASSERT(current->pFace != f2);

		find = false;

		for(O3D_UINT m=0; m<vertex_list.size() ; m++)
		{
			if ((current->pVertex == vertex_list[m]) || (current->pFace->GetNextVertex(current->pVertex) == vertex_list[m]))
			{
				need_refresh.push_back(*current);

				eraseIt = it_map;
				++it_map;

				m_transformations.erase(eraseIt);

				find = true;
				break;
			}
		}

		if (!find)
		{
			if (current->pVertex == v2) current->pVertex = target;
			if (current->pVertex == v1) current->pVertex = target;

			it_map++;
		}
	}


	find = false;

	it_list = m_deleted_transformations.begin() ;
	while (it_list != m_deleted_transformations.end())
	{
		find = false;

		for(O3D_UINT m=0; m<vertex_list.size() ; m++)
		{
			if ((it_list->pVertex == vertex_list[m]) || (it_list->pFace->GetNextVertex(it_list->pVertex) == vertex_list[m]))
			{
				need_refresh.push_back(*it_list);
				it_list = m_deleted_transformations.erase(it_list);
				find = true;
				break;
			}
		}

		// Initialement ya que la premiere ligne, je sais plus pk.
		if (!find)
		{
			if (it_list->pVertex == v2) it_list->pVertex = target;
			if (it_list->pVertex == v1) it_list->pVertex = target;

			it_list++;
		}
	}

	// on modifie au prélable les faces voisines pour les vertex et les voisins soient correct

	// Comme la face f1 et f2 vont etre supprimer, il faut faire en sorte que leurs voisins coincident apres la transformation
	if (f1l)
	{
		O3D_ASSERT(f1l->FindVertex(v1) != -1);
		f1l->SetNeighbor(f1l->FindVertex(v1), f1r);
	}

	if (f1r)
	{
		O3D_ASSERT(f1r->FindVertex(f1->GetNextVertex(v2)) != -1);
		f1r->SetNeighbor(f1r->FindVertex(f1->GetNextVertex(v2)), f1l);
	}

	// on fait la meme chose pour la face 2
	if (f2 && f2l)
		f2l->SetNeighbor(f2l->FindVertex(v2), f2r);

	if (f2 && f2r)
		f2r->SetNeighbor(f2r->FindVertex(f2->GetNextVertex(v1)), f2l);

	O3D_INT r = 0;

	for (O3D_UINT i=0 ; i<face_altered.size() ; i++)
		if (((r = face_altered[i]->FindVertex(vmove)) != -1) && (face_altered[i] != f1) && (face_altered[i] != f2))
			face_altered[i]->SetVertex(r, target);

	int valid_ind = -1;
	float Esav = 0.0f;

// Tout ce qui a été recalculé, doit etre réintégré dans la pile de transformation et trier
  for (O3D_UINT m=0; m<need_refresh.size(); m++)
  {
    if (need_refresh[m].pVertex == v2) need_refresh[m].pVertex = target;
    if (need_refresh[m].pVertex == v1) need_refresh[m].pVertex = target;

    if (need_refresh[m].pFace->GetNeighbor(need_refresh[m].pVertex))
    {
      O3D_ASSERT(need_refresh[m].pFace->Get_id() < need_refresh[m].pFace->GetNeighbor(need_refresh[m].pVertex)->Get_id());
    }

    valid_ind = -1;
    need_refresh[m].ind = 0;

    if (this->IsLegalTransformation(need_refresh[m].pFace, need_refresh[m].pVertex, need_refresh[m].StaticVertex())
        && this->ComputeEnergy(need_refresh[m]))
        {
            valid_ind = 0;
			Esav = need_refresh[m].energy;
        }

    need_refresh[m].ind = 1;

    if (this->IsLegalTransformation(need_refresh[m].pFace, need_refresh[m].pVertex, need_refresh[m].StaticVertex())
        && this->ComputeEnergy(need_refresh[m]))
        {
            if (valid_ind == 0)
            {
                if (Esav < need_refresh[m].energy)
                {
                    need_refresh[m].energy = Esav;
                    need_refresh[m].ind = 0;
                    PushTransformation(need_refresh[m]);
                }
                else PushTransformation(need_refresh[m]);
            }
            else PushTransformation(need_refresh[m]);
        }
        else if (valid_ind == 0)
        {
            need_refresh[m].energy = Esav;
            need_refresh[m].ind = 0;
            PushTransformation(need_refresh[m]);
        }
        else
        {
            m_deleted_transformations.push_back(need_refresh[m]);
        }

	}


	/* Check */
/*	try
	{
		for (O3D_UINT k = 0 ; k < m_faces.size() ; ++k)
		{
			//if ((m_faces[k] != f1))/ &&(m_faces[k] != f2))
				m_faces[k]->Check();
		}

		std::cout << "CHECK TERMINE" << std::endl;
	}
	catch(const E_not_valid & _err)
	{
		std::cout << "ERREUR : " << _err.Msg().GetData() << std::endl;
		system("PAUSE");
	}*/

	// il reste a supprimer la face f2 et f1 du vector<CFace*> et le vertex v2 du vector<CVertex*>

	O3D_ASSERT(m_faces[f1->Get_index()] == f1);
	if (f2) { O3D_ASSERT(m_faces[f2->Get_index()] == f2); }

	m_faces[f1->Get_index()] = m_faces.back();
	m_faces[f1->Get_index()]->Set_index(f1->Get_index());
	m_faces.pop_back();

	m_deleted_faces.push_back(f1);

	if (f2)
	{
		m_faces[f2->Get_index()] = m_faces.back();
		m_faces[f2->Get_index()]->Set_index(f2->Get_index());
		m_faces.pop_back();

		m_deleted_faces.push_back(f2);
	}

	O3D_ASSERT(m_vertices[v1->Get_index()] == v1);
	O3D_ASSERT(m_vertices[v2->Get_index()] == v2);

	// On enleve le vertex move du vector m_vertices et on l'ajoute à la la suite du vector resultat.
	m_vertices[vmove->Get_index()] = m_vertices.back();
	m_vertices[vmove->Get_index()]->Set_index(vmove->Get_index());
	m_vertices.pop_back();

	m_vertex_list.push_back(vmove);
	m_vertex_list.back()->Set_index(O3D_UINT(m_vertex_list.size() - 1));	// With this line, we are sure that index in the final vector are correct
																			// and can be immediately save in a buffer.

/*	O3DEvent event(TERRAIN_GEN_ITERATION_DONE);
	ThrowEvent(event);*/

	if (m_deleted_transformations.size() > MAX_DELETED_TRANSFORMATION_SIZE)
		BuildLegalTransformations();
}
