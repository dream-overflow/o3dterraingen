/*---------------------------------------------------------------------------------------
  TerrainGen.h :created on 16/12/2003, 15h54
  -------------------------------------------------------------------------------------
  - Objective-3D is under copyright (c) 2006 Dream Overflow (see license.txt)
  - mailto:objective3d@free.fr
  - http://o3d.dreamoverflow.com
  -------------------------------------------------------------------------------------
  Brief: Two dimensional vector
---------------------------------------------------------------------------------------*/

#ifdef _MSC_VER
#pragma once
#endif

#ifndef __O3DTERRAINGEN_H
#define __O3DTERRAINGEN_H

#ifdef _MSC_VER
	#pragma warning(disable:4245)
	#pragma warning(disable:4244)
	#pragma warning(disable:4127)
#endif

#include <o3d/Base/Base.h>
#include <o3d/Base/TemplateArray2d.h>
#include <o3d/Math/Vector2.h>
#include <o3d/Base/EvtHandler.h>
#include <o3d/Base/Thread.h>

#ifdef _MSC_VER
	#pragma warning(default:4245)
	#pragma warning(default:4244)
	#pragma warning(default:4127)
#endif

#include <vector>
#include <map>
#include <list>

// Must be removed
#include "Primitive.h"
#include "Infos.h"

#define MAX_DELETED_TRANSFORMATION_SIZE 1000

struct O3DZoneManipInfo;
struct O3DRule;
struct O3DTransformation;

class O3DVertex;
class O3DFace;
class O3DZoneManip;

typedef std::vector<O3DRule>				O3D_T_LevelRules;
typedef O3D_T_LevelRules::iterator			O3D_IT_LevelRules;
typedef O3D_T_LevelRules::const_iterator	O3D_CIT_LevelRules;

typedef std::vector<O3D_T_LevelRules>			O3D_T_LevelRulesArray;
typedef O3D_T_LevelRulesArray::iterator			O3D_IT_LevelRulesArray;
typedef O3D_T_LevelRulesArray::const_iterator	O3D_CIT_LevelRulesArray;

typedef std::vector<PO3DVertex>				O3D_T_VertexArray;
typedef O3D_T_VertexArray::iterator			O3D_IT_VertexArray;
typedef O3D_T_VertexArray::const_iterator	O3D_CIT_VertexArray;

typedef std::vector<PO3DFace>			O3D_T_FaceArray;
typedef O3D_T_FaceArray::iterator		O3D_IT_FaceArray;
typedef O3D_T_FaceArray::const_iterator	O3D_CIT_FaceArray;

typedef O3DTemplateArray2D<O3DZoneManip*>		O3D_T_ZoneManipArray;

typedef std::list<O3DTransformation>				O3D_T_TransformationList;
typedef O3D_T_TransformationList::iterator			O3D_IT_TransformationList;
typedef O3D_T_TransformationList::const_iterator	O3D_CIT_TransformationList;

typedef std::multimap<O3D_FLOAT, O3DTransformation, std::less<O3D_FLOAT> > O3D_T_TransformationMultimap;
typedef O3D_T_TransformationMultimap::iterator O3D_IT_TransformationMultimap;
typedef O3D_T_TransformationMultimap::const_iterator O3D_CIT_TransformationMultimap;

typedef std::vector<std::pair<O3D_USHORT, O3D_USHORT> > O3D_T_ResultVertexArray;
typedef std::vector<std::vector<O3D_UINT> > O3D_T_ResultFaceArray;

//! Rule type definition
enum O3DRuleType {
	O3D_RULE_FACE_NUMBER_LIMIT = 0,
	O3D_RULE_VERTEX_NUMBER_LIMIT,
	O3D_RULE_FACE_NUMBER_LOD_RELATIVE_PERCENTAGE,
	O3D_RULE_VERTEX_NUMBER_LOD_RELATIVE_PERCENTAGE,
	O3D_RULE_FACE_NUMBER_LEVEL_RELATIVE_PERCENTAGE,
	O3D_RULE_VERTEX_NUMBER_LEVEL_RELATIVE_PERCENTAGE,
	O3D_RULE_FACE_NUMBER_ABSOLUTE_PERCENTAGE,
	O3D_RULE_VERTEX_NUMBER_ABSOLUTE_PERCENTAGE,
	O3D_RULE_ERROR_RELATIVE,	//!< Not supported yet
	O3D_RULE_ERROR_ABSOLUTE,	//!< Not supported yet
	O3D_RULE_COST_LIMIT,		//!< Not supported yet
	O3D_RULE_NUMBER
};

enum O3DZoneManipExitCode {
	O3D_ZONE_MANIP_UNDEFINED,
	O3D_ZONE_MANIP_SUCCESS,
	O3D_ZONE_MANIP_ABORTED
};

// Event list
/*#define TERRAIN_GEN_EVENT_OFFSET O3D_EVENT_LAST + 10000

const O3DEventId TERRAIN_GEN_ITERATION_DONE = O3DEventId(TERRAIN_GEN_EVENT_OFFSET + 1);
const O3DEventId TERRAIN_GEN_LEVEL_DONE = O3DEventId(TERRAIN_GEN_EVENT_OFFSET + 2);
const O3DEventId TERRAIN_GEN_ZONE_DONE = O3DEventId(TERRAIN_GEN_EVENT_OFFSET + 3);*/

//! Class in which is store the result of a simplification
class O3DCLM_API O3DZoneManipResult
{
	friend class O3DZoneManip;

public:

	/* Types */
	struct O3DZoneError
	{
		O3D_FLOAT maxError;
		O3D_FLOAT minError;
		O3D_FLOAT medError;
		O3D_FLOAT medDelta;	///< Ecart type
	};

private:

	/* Members */
	O3DArray2Di m_heightmapData;
	O3D_FLOAT m_dx, m_dy, m_dz;

	O3DArray2Dui m_materialData;

	O3D_T_ResultVertexArray m_vertexArray;
	std::vector<O3D_UINT> m_vertexIndexArray;

	O3D_T_ResultFaceArray m_facesIndex;		///< Face indices store as GL_TRIANGLES
	std::vector<O3D_BOOL> m_facesUseMat;	///< true if the associated lod use materials

	std::vector<O3DZoneError> m_lodError;		///< Stored the error generated by the simplification

	std::vector<O3DZoneManipResult> m_children;			///< Used for multi level simplification

	/* Restricted */
	void Write(std::ostream & _stream);
	void Read(std::istream & _stream);

public:

	/* Constructors */
	O3DZoneManipResult();
	O3DZoneManipResult(const O3DZoneManipResult &);
	~O3DZoneManipResult();

	O3DZoneManipResult & operator = (const O3DZoneManipResult &);

	/* Clear all allocated memory */
	void Destroy();

	/* Return true if the object contains valid results */
	O3D_BOOL IsOk() const { return (m_heightmapData.Elt() > 0); }

	/* Return a binary buffer in which is stored all datas.
	 * - _buffer must be NULL */
	void Write(O3D_CHAR * & _buffer, O3D_UINT & _size);

	/* Initialize the result object with a buffer */
	O3D_UINT Read(const O3D_CHAR * _buffer, O3D_UINT size);

	O3D_BOOL HasChildren() const { return (m_children.size() == 4); }
	const O3DZoneManipResult & GetChild(O3D_UINT _index) const { O3D_ASSERT(_index < O3D_UINT(m_children.size())); return m_children[_index]; }

	O3D_UINT GetLodNumber() const { return O3D_UINT(m_facesIndex.size()); }
	O3D_UINT GetLodNumber(O3D_UINT _level) const;
	O3D_UINT GetLevelNumber() const;

	O3D_BOOL LodUseMaterial(O3D_UINT _level) const { O3D_ASSERT(m_facesUseMat.size() == m_facesIndex.size()); O3D_ASSERT(_level < m_facesUseMat.size()); return m_facesUseMat[_level]; }

	void GetUnits(O3D_FLOAT & _dx, O3D_FLOAT & _dy, O3D_FLOAT & _dz) const { _dx = m_dx; _dy = m_dy; _dz = m_dz; }
	const O3DArray2Di & GetHeightmap() const { return m_heightmapData; }
	const O3DArray2Dui & GetMaterial() const { return m_materialData; }

	const O3D_T_ResultVertexArray & GetVertexArray() const { return m_vertexArray; }
	const std::vector<O3D_UINT> & GetVertexIndexArray() const { return m_vertexIndexArray; }

	const O3D_T_ResultFaceArray & GetFaceIndex() const { return m_facesIndex; }

	const O3DZoneError & GetLodError(O3D_UCHAR _lod) const { O3D_ASSERT(_lod < O3D_UCHAR(m_lodError.size())); return m_lodError[_lod]; }
};

/* Struct to provide in order to initialize the zoneManip object */
struct O3DZoneManipInfo
{
	O3D_UINT layerWidth;
	O3D_UINT layerHeight;

	O3D_INT *	pSource;			///< Elevation datas
	O3D_UINT *	pMaterial;			///< Simplifications do not modify this repartition
	O3D_FLOAT * pEnergetic;			///< Energetic layer (not used yet)

	O3D_FLOAT dx, dy, dz;			///< Units of the map

	O3D_T_LevelRulesArray rules;

	O3D_FLOAT ck, ce, cn;			///< Energetic coefficients
};

struct O3DRule
{
	O3DRuleType type;

	O3D_UINT value;

	O3D_BOOL useMaterial;
	O3D_BOOL useEnergetic;
};

/*---------------------------------------------------------------------------------------
  Struct O3DTransformation
---------------------------------------------------------------------------------------*/
struct O3DTransformation
{
	/* Members */
	PO3DFace	pFace;
	PO3DVertex	pVertex;

	O3D_INT ind;			///< ind = 0 means that vertex will move to himself (in fact, he doesnt move)
							///< ind = 1 means that vertex will move to the next vertex in the face

	O3D_FLOAT energy;
	O3D_FLOAT ek, en, ee;

	O3D_FLOAT error;		///< Error generated by the transformation between the future mesh and the initial heightmap

	/* Constructors */
	O3DTransformation();

	/* Functions */
	PO3DVertex MovableVertex();
	PO3DVertex StaticVertex();

	O3D_BOOL operator == (const O3DTransformation & _which);
};

class O3DCLM_API O3DZoneManip : public O3DEvtHandler
{
private:

	/* Members */
	O3DZoneManipInfo m_instruction;		///< Struct provided by the user to define the zone simplification

	O3D_BOOL m_init;		///< true if the object is ready

	O3DArray2Di			m_heightmap;		///< Heightmap extracted from the instruction

	O3D_T_VertexArray	m_vertices;			///< Array of active vertices
	O3D_T_FaceArray		m_faces;			///< Array of active faces
	O3D_T_FaceArray		m_deleted_faces;	///< Array of inactive faces (avoid that the user become responsible of allocated faces)

	O3D_T_VertexArray		m_vertex_list;			///< Final list of vertex. The first of the list is the first inserted.
	std::vector<O3D_UINT>	m_vertex_index;	///< Index of different levels of vertex

	O3D_T_TransformationMultimap	m_transformations;				///< Map of all legal transformations sorted by energy
	O3D_T_TransformationList		m_deleted_transformations;	///< List of temporary illegal transformations
														///< Some of them can become legal due to other transformations
	O3D_INT m_currentRule;		///< Index of the current used rule
	O3D_INT m_currentLevel;		///< Level of subdivision

	O3D_UINT m_lastLodFaceNumber;		///< Number of face at the end of the last level (used for rules)
	O3D_UINT m_lastLodVertexNumber;		///< Number of vertex at the end of the last level

	O3D_UINT m_initFaceNumber;			///< Number of face when the object was initialized
	O3D_UINT m_initVertexNumber;

	O3D_T_ZoneManipArray	m_zoneManipArray;	///< Array of manip object used if there are more than 1 level

	O3DFastMutex	m_mutexFunction;	///< Use for various operations

	/* Message handling */
	O3DZoneManip * m_pMessageHandler;	///< Which object does manage message

	O3D_FLOAT m_progression;

	O3D_BOOL m_pauseRequested;
	O3D_BOOL m_abortRequested;

	O3DFastMutex m_pauseMutex;      ///< Always lock. It allows the Pause function to return only after the pause of this task
	O3DFastMutex m_resumeMutex;     ///< This task will resume if this mutex is released
	O3DFastMutex m_workingMutex;    ///< Always lock. Allows the Abort function to return only after this task stopped

	/* Restricted */
	// Internal constructor used to create sub manip object
	O3DZoneManip(O3DZoneManip * _pMessageHandler);
	O3DZoneManip(const O3DZoneManip &);
	O3DZoneManip & operator = (const O3DZoneManip &);

	O3D_BOOL HasMaterial() const { return (m_instruction.pMaterial != NULL); }
	O3D_BOOL HasEnergetic() const { return (m_instruction.pEnergetic != NULL); }

	O3D_UINT Width() const { return m_instruction.layerWidth; }
	O3D_UINT Height() const { return m_instruction.layerHeight; }

	O3D_FLOAT Dx() const { return m_instruction.dx; }
	O3D_FLOAT Dy() const { return m_instruction.dy; }
	O3D_FLOAT Dz() const { return m_instruction.dz; }

	O3D_FLOAT Ck() const { return m_instruction.ck; }
	O3D_FLOAT Ce() const { return m_instruction.ce; }
	O3D_FLOAT Cn() const { return m_instruction.cn; }

	O3D_FLOAT PosZ(O3D_UINT i, O3D_UINT j) const { return m_instruction.dz * O3D_FLOAT(m_heightmap(i,j)); }

	O3DArray2Di & Heightmap() { return m_heightmap; }
	const O3DArray2Di & Heightmap() const { return m_heightmap; }

	O3D_T_VertexArray & Vertices() { return m_vertices; }
	const O3D_T_VertexArray & Vertices() const { return m_vertices; }

	O3D_T_FaceArray & Faces() { return m_faces; }
	const O3D_T_FaceArray & Faces() const { return m_faces; }

	O3D_UINT Material(O3D_UINT i, O3D_UINT j) const { O3D_ASSERT(m_instruction.pMaterial != NULL); return m_instruction.pMaterial[j * Width() + i]; }

	const O3DRule & GetRule(O3D_INT _level, O3D_INT _rule) const { O3D_ASSERT((_level >= 0) && (_level < O3D_INT(m_instruction.rules.size()))); O3D_ASSERT((_rule >= 0) && (_rule < O3D_INT(m_instruction.rules[_level].size()))); return m_instruction.rules[_level][_rule]; }

	/* Define the current level */
	void SetCurrentLevel(O3D_UINT _level) { m_currentLevel = _level; }
	O3D_UINT GetCurrentLevel() const { return m_currentLevel; }

	/* Initialize the vertex and face array */
	void InitVertexArray();
	void InitFaceArray();

	/* Check the link between faces */
	void CheckFaces();

	/* Construct the list of all neighbors of a specific vertex and face.
	 * The parameter _from_face indicate the starting face and _to_face the final face.
	 * So that, _to_face won't be inserted in the list. */
	void BuildNeighborList(PO3DFace _base_face, PO3DVertex _vertex, PO3DFace _from_face, PO3DFace _to_face, O3D_T_FaceArray & _vect);

	/* Compute the cost of a transformation and store it in the provided object
	 * Return false, if the transformation is not legal */
	O3D_BOOL ComputeEnergy(O3DTransformation & _transformation);

	/* Return O3D_TRUE, if the simplification is legal */
	O3D_BOOL IsLegalTransformation(PO3DFace _face, PO3DVertex _vertex, PO3DVertex _target) const;

	/* Return O3D_TRUE, if the vertex can belong to a simplification */
	O3D_BOOL MaterialSimplificationAllowed(PO3DVertex vertex) const;

	/* Find the vertex which has _i and _j as coordinate on the heightmap */
	O3D_INT FindVertexWithPosIJ(O3D_UINT _i, O3D_UINT _j) const;

	/* Find the face who owns vertex v1 and v2 in the counter clock wise. */
	O3D_INT FindFacesByVertices(PO3DVertex v1, PO3DVertex v2) const;

	/* Apply the first simplification */
	void Simplify();

	/* Add a transformation to the ordered list */
	void PushTransformation(const O3DTransformation & _which);

	/* Build the list of legal transformation */
	O3D_BOOL BuildLegalTransformations();

	/* Return O3D_FALSE, if the next level is not reached */
	O3D_BOOL LevelAchieved() const;

	/* Return O3D_TRUE, if all the simplification was done */
	O3D_BOOL Achieved() const { return ((m_currentRule == O3D_INT(m_instruction.rules[0].size()-1)) && (LevelAchieved() || (m_transformations.size() == 0))); }

	/* Return O3D_TRUE, if the task is currently in function Start() */
	O3D_BOOL IsWorking() const;

	/* Return O3D_TRUE, if the task is currently paused */
	O3D_BOOL IsPaused() const;

	/* Initialize the object with 4 other zoneManip by merging them.
	 * Function used if the number of levels in instruction.rules is greater than one */
	void Init(const O3D_T_ZoneManipArray & _array);

	/* Function that returns O3D_FALSE, if abort is requested, else block the execution if pause is requested */
	O3D_BOOL MessageHandling();

	/* Functions that return a heightmap built with a list of triangles */
	void BuildHeightmap(const O3D_T_VertexArray & _vertexArray, const O3D_T_FaceArray & _faceArray, const O3DVector2ui & _size, O3DArray2Df & _heightmap);

	/* Return the max, the min, the average, and "ecart type" of the difference */
	void HeightmapDifference(const O3DArray2Df & _h1, const O3DArray2Df & _h2, O3D_FLOAT & _max, O3D_FLOAT & _min, O3D_FLOAT & _med, O3D_FLOAT & _ec);

	/* Return different values associated with the error generated by the simplification */
	void ComputeCurrentError(O3DZoneManipResult::O3DZoneError & _error);

	/* Return the global progression */
	O3D_FLOAT GetProgression(O3D_UINT) const;

public:

	/* Constructors */
	O3DZoneManip();
	~O3DZoneManip();

	/* Initialize the object with the provided instructions */
	void Init(const O3DZoneManipInfo & _instruction);

	/* Start the simplification */
	O3DZoneManipExitCode Start(O3DZoneManipResult & _result);

	/* Abort the simplification */
	void Abort(O3D_BOOL _wait = O3D_TRUE);

	/* Pause the simplification */
	void Pause(O3D_BOOL _wait = O3D_TRUE);

	/* Resume the simplification */
	void Resume();

	/* Return the progression of this object */
	O3D_FLOAT GetProgression() const;

public signals:

	SIGNAL_0(EVT_ITERATION_DONE);
	SIGNAL_0(EVT_LEVEL_DONE);
	SIGNAL_0(EVT_ZONE_DONE);
};

#endif // __O3DTERRAINGEN_H
