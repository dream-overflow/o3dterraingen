/*---------------------------------------------------------------------------------------
  Func_ComputeEnergy.cpp :created on 16/12/2003, 15h54
  -------------------------------------------------------------------------------------
  - Objective-3D is under copyright (c) 2006 Dream Overflow (see license.txt)
  - mailto:objective3d@free.fr
  - http://o3d.dreamoverflow.com
  -------------------------------------------------------------------------------------
  Brief:
---------------------------------------------------------------------------------------*/


/* Compute the cost of a transformation */
O3D_BOOL O3DZoneManip::ComputeEnergy(O3DTransformation & _transformation)
{
	PO3DVertex v1 = _transformation.pVertex;
	PO3DVertex v2 = _transformation.pFace->GetNextVertex(_transformation.pVertex);

	PO3DVertex target = _transformation.StaticVertex();
	O3D_ASSERT((target == v1) || (target == v2));

	PO3DFace f1 = _transformation.pFace;
	PO3DFace f2 = _transformation.pFace->GetNeighbor(_transformation.pVertex);

	// Le calcul se fait en 3 étapes
	float Ek = 0.0f; // carré des longueurs
	float En = 0.0f; // nombre de voisin
//	float Ee = 0.0f; // erreur


//============================================================================//
// 1) La différence des carrés des longueurs
//============================================================================//

// Détermination des faces affectées par la transformation et en contact avec le vertex1
	O3D_T_FaceArray face_av1;
	BuildNeighborList(f1, v1, f1, f2, face_av1);
	O3D_ASSERT(face_av1.size() > 0);

	O3D_T_FaceArray face_av2;
	BuildNeighborList(f2, v2, f2, f1, face_av2);

// Note: on construit la liste des vertex modifiés pour les deux vertex, alors que c'est
// inutile en mode TO_VERTEX. Seul un des vertex est modifié. Mais bon, pour le moment...

// Toutes les faces ayant en commun v1 ou v2 sont connues.
// Chaque vertex contenant v1 ou v2 est donc répertorié deux fois.
// On va donc calculer deux fois la variation

// on détermine le point qui remplacera les deux vertex
	O3DVector3 point = _transformation.StaticVertex()->Get_point();

// Calcul de la somme des carrés des longueurs liées à v1 ou v2 avant transformation
	float longueur = 0.0f;
	O3DVector3 vect1, vect2; // vecteur temporaire
	float t; // valeur temporaire

	for (O3D_UINT i = 0 ; i< face_av1.size() ; ++i)
	{
		// Pour chaque face on isole les segments de chaque coté de v1
		// Comme on sait pas vraiment si le segment c'est v1 et le suivant, ou v1 et le précédent, on somme les deux
		// Chaque segment sera donc compté deux fois au final, faudrait donc diviser la valeur par 2.

		vect1 = v1->Get_point() - face_av1[i]->GetPrevVertex(v1)->Get_point();
		vect2 = v1->Get_point() - face_av1[i]->GetNextVertex(v1)->Get_point();


		t = vect1.Length();
		longueur -= t*t;

		t= vect2.Length();
		longueur -= t*t;
	}

	for (O3D_UINT i = 0 ; i < face_av2.size() ; ++i)
	{
		// Pour chaque face on isole les segments de chaque coté de v1
		vect1 = v2->Get_point() - face_av2[i]->GetPrevVertex(v2)->Get_point();
		vect2 = v2->Get_point() - face_av2[i]->GetNextVertex(v2)->Get_point();

		t = vect1.Length();
		longueur -= t*t;

		t = vect2.Length();
		longueur -= t*t;
	}

// On soustrait ensuite le carré des longueurs apres transformation
// Les triangles concernés sont les memes que précédemment, sauf qu'il faut enlever v1 et v2, car ils disparaissent dans la simplification

	for (O3D_UINT i = 0 ; i < face_av1.size() ; ++i)
	{
		if (face_av1[i] != f1)
		{
			vect1 = point - face_av1[i]->GetPrevVertex(v1)->Get_point();
			vect2 = point - face_av1[i]->GetNextVertex(v1)->Get_point();

			t = vect1.Length();
			longueur += t*t;

			t = vect2.Length();
			longueur += t*t;
		}
	}

	for (O3D_UINT i=0 ; i<face_av2.size(); i++)
	{
		if (face_av2[i] != f2)
		{
			vect1 = point - face_av2[i]->GetPrevVertex(v2)->Get_point();
			vect2 = point - face_av2[i]->GetNextVertex(v2)->Get_point();

			t = vect1.Length();
			longueur += t*t;

			t = vect2.Length();
			longueur += t*t;
		}
	}

//	O3D_ASSERT(longueur > -epsilon);

	// Il faut encore diviser par deux le résultat
	longueur *= 0.5f;

	// Et enfin
	Ek = Ck() * longueur;



//============================================================================//
// 2) L'influence du nombre de voisins
//============================================================================//

	// Ce paramètre sert a éviter qu'un vertex soit commun a trop de triangle

	// f1 est toujours valide, et donc face_av1 est tjs non vide.
	int nbr = (O3D_INT)face_av1.size() - 1;

	// si le nombre de voisin concerné par v2 est non nul
	if (face_av2.size() > 0)
		nbr += (O3D_INT)face_av2.size() - 1;

	// ET donc
	En = Cn() * nbr;





//============================================================================//
// 3) L'erreur relative engendrée
//============================================================================//
	float Er = 0.0f;

// Pour Chaque triangle final,
// il faut déterminer d'abord l'ensemble des points de la map initiale concernés.
// On détermine un rectangle dans lequel est inclu la face

  int xmax, ymax; // coordonnées du rectangle sur la map2D
  int xmin, ymin;

  O3DVector3 p0, p1, p2; // points temporaire du triangle
  O3DVector3 p; // pareil
  O3DVector3 n0, n1, n2; // vecteur X normal au coté X
  O3DVector3 plan; // vecteur normal au plan
  float plan_cte = 0.0f;
  float error_generated = 0.0f;

  for (O3D_UINT i=0; i < face_av1.size(); i++)
  {
    if (face_av1[i] != f1)
    {
      // on définie les 3 points du triangle
      if (face_av1[i]->GetVertex(0) != v1) p0 = face_av1[i]->GetVertex(0)->Get_point();
        else p0 = point;
      if (face_av1[i]->GetVertex(1) != v1) p1 = face_av1[i]->GetVertex(1)->Get_point();
        else p1 = point;
      if (face_av1[i]->GetVertex(2) != v1) p2 = face_av1[i]->GetVertex(2)->Get_point();
        else p2 = point;

      xmax = (int)ceil(1.0f/Dx() * o3d::Max(p0[X], o3d::Max(p1[X], p2[X])));
      ymax = (int)ceil(1.0f/Dy() * o3d::Max(p0[Y], o3d::Max(p1[Y], p2[Y])));

      xmin = (int)floor(1.0f/Dx() * o3d::Min(p0[X], o3d::Min(p1[X], p2[X])));
      ymin = (int)floor(1.0f/Dy() * o3d::Min(p0[Y], o3d::Min(p1[Y], p2[Y])));

      // donc l'ensemble des points concernés est dans le rectangle (xmin, ymin, xmax, ymax)
      // on calcul les vecteurs normaux aux cotés.
      n0 = O3DVector3(0.0f, 0.0f, 1.0f) ^ (p0 - p1);
      n1 = O3DVector3(0.0f, 0.0f, 1.0f) ^ (p1 - p2);
      n2 = O3DVector3(0.0f, 0.0f, 1.0f) ^ (p2 - p0);

      // si le triangle suite a la transformation actuelle devient plat, alors on interrompt la transformation
      // Cette détection est lourde donc je ne l'a fais pas dans Is_legal_transformation
//      if ((n0 % n1).Norm2() < float_delta)


	  if ((n0 ^ n1)[Z] > -o3d::Limits<O3D_FLOAT>::Epsilon())
        return false;

      // On détermine l'équation du plan pour le calcul de la hauteur du point sur le triangle

      plan = (p0 - p1) ^ (p2 - p1);

      // plan définie a une constante près.
      plan_cte = plan[X] * p0[X] + plan[Y] * p0[Y] + plan[Z] * p0[Z];

      // Donc un point est dans le triangle si son produit scalaire est positif avec chacun des vecteurs.
      for (int x=xmin; x < xmax; x++)
      for (int y=ymin; y < ymax; y++)
      {
        p = O3DVector3((float)x * Dx(), (float)y * Dy(), 0.0f);
        if ((((p - p0) * n0) >= -o3d::Limits<O3D_FLOAT>::Epsilon())
          &&
           (((p - p1) * n1) >= -o3d::Limits<O3D_FLOAT>::Epsilon())
          &&
           (((p - p2) * n2) >= -o3d::Limits<O3D_FLOAT>::Epsilon()))
          {
            // Alors le point est dedans.

            // d'ou l'altitude du point projeté sur le triangle
            p[Z] = plan_cte - plan[X] * p[X] - plan[Y] * p[Y];

            O3D_ASSERT(plan[Z] > -o3d::Limits<O3D_FLOAT>::Epsilon());
            p[Z] /= plan[Z];

			// Calcul de l'erreur absolue (tabs)

			float tabs = fabs(p[Z] - PosZ(x,y));
			if (tabs > error_generated) error_generated = tabs;

            Er += Ce() * tabs;
          }

      }
    }
  }

// on refait la meme chose pour les voisins du vertex2

  for (O3D_UINT i=0; i < face_av2.size(); i++)
  {
    if (face_av2[i] != f2)
    {

      // on définie les 3 points du triangle
      if (face_av2[i]->GetVertex(0) != v2) p0 = face_av2[i]->GetVertex(0)->Get_point();
        else p0 = point;
      if (face_av2[i]->GetVertex(1) != v2) p1 = face_av2[i]->GetVertex(1)->Get_point();
        else p1 = point;
      if (face_av2[i]->GetVertex(2) != v2) p2 = face_av2[i]->GetVertex(2)->Get_point();
        else p2 = point;

      xmax = (int)ceil(1.0f/Dx() * o3d::Max(p0[X], o3d::Max(p1[X], p2[X])));
      ymax = (int)ceil(1.0f/Dy() * o3d::Max(p0[Y], o3d::Max(p1[Y], p2[Y])));

      xmin = (int)floor(1.0f/Dx() * o3d::Min(p0[X], o3d::Min(p1[X], p2[X])));
      ymin = (int)floor(1.0f/Dy() * o3d::Min(p0[Y], o3d::Min(p1[Y], p2[Y])));

      // donc l'ensemble des points concernés est dans le rectangle (xmin, ymin, xmax, ymax)
      // on calcul les vecteurs normaux aux cotés.
      n0 = O3DVector3(0.0f, 0.0f, 1.0f) ^ (p0 - p1);
      n1 = O3DVector3(0.0f, 0.0f, 1.0f) ^ (p1 - p2);
      n2 = O3DVector3(0.0f, 0.0f, 1.0f) ^ (p2 - p0);

      // si le triangle suite a la transformation actuelle devient plat, alors on interrompt la transformation
      // Cette détection est lourde donc je ne l'a fais pas dans Is_legal_transformation
//      if ((n0 % n1).Norm2() < float_delta)
      if ((n0 ^ n1)[Z] > -o3d::Limits<O3D_FLOAT>::Epsilon())
        return false;

      // On détermine l'équation du plan pour le calcul de la hauteur du point sur le triangle

      plan = (p0 - p1) ^ (p2 - p1);

      // plan définie a une constante près.
      plan_cte = plan[X] * p0[X] + plan[Y] * p0[Y] + plan[Z] * p0[Z];

      // Donc un point est dans le triangle si son produit scalaire est positif avec chacun des vecteurs.
      for (int x=xmin; x < xmax; x++)
      for (int y=ymin; y < ymax; y++)
      {
        p = O3DVector3((float)x * Dx(), (float)y * Dy(), 0.0f);
        if ((((p - p0) * n0) >= -o3d::Limits<O3D_FLOAT>::Epsilon())
          &&
           (((p - p1) * n1) >= -o3d::Limits<O3D_FLOAT>::Epsilon())
          &&
           (((p - p2) * n2) >= -o3d::Limits<O3D_FLOAT>::Epsilon()))
          {
            // Alors le point est dedans.

            // d'ou l'altitude du point projeté sur le triangle
            p[Z] = plan_cte - plan[X] * p[X] - plan[Y] * p[Y];
            O3D_ASSERT(plan[Z] > -o3d::Limits<O3D_FLOAT>::Epsilon());

            p[Z] /= plan[Z];

			// Calcul de l'erreur absolue (tabs)

            float tabs = fabs(p[Z] - PosZ(x,y));
			if (tabs > error_generated) error_generated = tabs;

            Er += Ce() * tabs;
          }

      }
    }
  }

//============================================================================//
// 4) Surcout des vertices du bord
//============================================================================//

  float Eedge = 0.0f;

// on enregistre dans l'objet passé en paramètre
  _transformation.ek = Ek;
  _transformation.en = En;
  _transformation.ee = Er;

  _transformation.energy = Ek + En + Er + Eedge;
  _transformation.pFace = f1;
  _transformation.pVertex = v1;

  _transformation.error = error_generated;

  // It may looks strange to test an other condition about the legality of a transformation,
  // but some datas used by the criteria were computed in this function.

  return O3D_TRUE;
}
