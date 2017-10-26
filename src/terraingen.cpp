/*---------------------------------------------------------------------------------------
  TerrainGen.cpp :created on 16/12/2003, 15h54
  -------------------------------------------------------------------------------------
  - Objective-3D is under copyright (c) 2006 Dream Overflow (see license.txt)
  - mailto:objective3d@free.fr
  - http://o3d.dreamoverflow.com
  -------------------------------------------------------------------------------------
  Brief: Two dimensional vector
---------------------------------------------------------------------------------------*/

#include "TerrainGen.h"
#include "Primitive.h"

#include "Func_ComputeEnergy.h"
#include "Func_Simplify.h"
#include "func_init.h"

#include <sstream>
#include <streambuf>

/*---------------------------------------------------------------------------------------
  Internal functions
---------------------------------------------------------------------------------------*/
O3D_BOOL IsPowerOf(O3D_UINT _value,	O3D_UINT _power)
{
	if ((_power == 1) || (_power == 0))
	{
		if (_value == 1) return O3D_TRUE;
		else return O3D_FALSE;
	}

	while ((_value != 1) && ((_value / _power)*_power == _value)) _value /= _power;

	return (_value == 1);
}

/*---------------------------------------------------------------------------------------
  Class O3DZoneManipResult
---------------------------------------------------------------------------------------*/
void O3DZoneManipResult::Write(std::ostream & _stream)
{
	// We write the header
	O3D_CHAR header[] = "ZONEMANIP RESULT";
	_stream.write(header, 16*sizeof(O3D_CHAR));

	// We write the heightmap
	O3D_UINT heightmapWidth = m_heightmapData.Width(), heightmapHeight = m_heightmapData.Width();

	_stream.write((const O3D_CHAR*)&heightmapWidth, sizeof(O3D_UINT));
	_stream.write((const O3D_CHAR*)&heightmapHeight, sizeof(O3D_UINT));

	_stream.write((const O3D_CHAR*)&m_dx, sizeof(O3D_FLOAT));
	_stream.write((const O3D_CHAR*)&m_dy, sizeof(O3D_FLOAT));
	_stream.write((const O3D_CHAR*)&m_dz, sizeof(O3D_FLOAT));

	if (m_heightmapData.Elt() > 0)
		_stream.write((const O3D_CHAR*)&m_heightmapData[0], m_heightmapData.Elt() * sizeof(O3D_INT));

	// We write the materials
	O3D_UINT materialWidth = m_materialData.Width(), materialHeight = m_materialData.Height();

	_stream.write((const O3D_CHAR*)&materialWidth, sizeof(O3D_UINT));
	_stream.write((const O3D_CHAR*)&materialHeight, sizeof(O3D_UINT));

	if (m_materialData.Elt() > 0)
		_stream.write((const O3D_CHAR*)&m_materialData[0], m_materialData.Elt() * sizeof(O3D_UINT));

	// We write the vertex index array
	O3D_UINT vertexIndexSize = O3D_UINT(m_vertexIndexArray.size());

	_stream.write((const O3D_CHAR*)&vertexIndexSize, sizeof(O3D_UINT));

	if (vertexIndexSize > 0)
		_stream.write((const O3D_CHAR*)&m_vertexIndexArray[0], vertexIndexSize * sizeof(O3D_UINT));

	// We write the vertex array
	O3D_UINT vertexArraySize = O3D_UINT(m_vertexArray.size());

	_stream.write((const O3D_CHAR*)&vertexArraySize, sizeof(O3D_UINT));

	if (vertexArraySize > 0)
		_stream.write((const O3D_CHAR*)&m_vertexArray[0], vertexArraySize * sizeof(std::pair<O3D_USHORT, O3D_USHORT>));

	// We write the face array
	O3D_UINT levelNumber = O3D_UINT(m_facesIndex.size());

	O3D_ASSERT(m_facesUseMat.size() == m_facesIndex.size());

	_stream.write((const O3D_CHAR*)&levelNumber, sizeof(O3D_UINT));

	for(O3D_UINT k = 0 ; k < levelNumber ; ++k)
	{
		O3D_UINT levelSize = O3D_UINT(m_facesIndex[k].size());
		O3D_INT useMaterial = O3D_INT(m_facesUseMat[k]);

		_stream.write((const O3D_CHAR*)&levelSize, sizeof(O3D_UINT));
		_stream.write((const O3D_CHAR*)&useMaterial, sizeof(O3D_INT));

		if (levelSize > 0)
			_stream.write((const O3D_CHAR*)&m_facesIndex[k][0], levelSize * sizeof(O3D_UINT));
	}

	// We write the array of error
	O3DZoneError errorDefault = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_lodError.resize(m_facesIndex.size(), errorDefault);

	_stream.write((const O3D_CHAR*)&m_lodError[0], m_lodError.size() * sizeof(O3DZoneError));

	// We write the other sub results
	O3D_UINT subResultNumber = O3D_UINT(m_children.size());

	_stream.write((const O3D_CHAR*)&subResultNumber, sizeof(O3D_UINT));

	for (O3D_UINT k = 0 ; k < subResultNumber ; ++k)
		m_children[k].Write(_stream);
}

void O3DZoneManipResult::Read(std::istream & _stream)
{
	// We read the header
	O3D_CHAR header[17];
	memset((void*)header, 0, 17);

	_stream.read(header, 16*sizeof(O3D_CHAR));
	O3D_ASSERT(strncmp((const char*)header, "ZONEMANIP RESULT", 16) == 0);

	// We read the heightmap
	O3D_UINT heightmapWidth = 0, heightmapHeight = 0;

	_stream.read((O3D_CHAR*)&heightmapWidth, sizeof(O3D_UINT));
	_stream.read((O3D_CHAR*)&heightmapHeight, sizeof(O3D_UINT));

	_stream.read((O3D_CHAR*)&m_dx, sizeof(O3D_FLOAT));
	_stream.read((O3D_CHAR*)&m_dy, sizeof(O3D_FLOAT));
	_stream.read((O3D_CHAR*)&m_dz, sizeof(O3D_FLOAT));

	m_heightmapData.SetSize(heightmapWidth, heightmapHeight);

	if (m_heightmapData.Elt() > 0)
		_stream.read((O3D_CHAR*)&m_heightmapData[0], m_heightmapData.Elt() * sizeof(O3D_INT));

	// We read the materials
	O3D_UINT materialWidth = 0, materialHeight = 0;

	_stream.read((O3D_CHAR*)&materialWidth, sizeof(O3D_UINT));
	_stream.read((O3D_CHAR*)&materialHeight, sizeof(O3D_UINT));

	m_materialData.SetSize(materialWidth, materialHeight);

	if (m_materialData.Elt() > 0)
		_stream.read((O3D_CHAR*)&m_materialData[0], m_materialData.Elt() * sizeof(O3D_UINT));

	// We read the vertex index array
	O3D_UINT vertexIndexSize = 0;

	_stream.read((O3D_CHAR*)&vertexIndexSize, sizeof(O3D_UINT));

	if (vertexIndexSize > 0)
	{
		m_vertexIndexArray.resize(vertexIndexSize);
		_stream.read((O3D_CHAR*)&m_vertexIndexArray[0], vertexIndexSize * sizeof(O3D_UINT));
	}

	// We read the vertex array
	O3D_UINT vertexArraySize = 0;

	_stream.read((O3D_CHAR*)&vertexArraySize, sizeof(O3D_UINT));

	if (vertexArraySize > 0)
	{
		m_vertexArray.resize(vertexArraySize);
		_stream.read((O3D_CHAR*)&m_vertexArray[0], vertexArraySize * sizeof(std::pair<O3D_USHORT, O3D_USHORT>));
	}

	// We read the face array
	O3D_UINT levelNumber = 0;

	_stream.read((O3D_CHAR*)&levelNumber, sizeof(O3D_UINT));

	if (levelNumber > 0)
	{
		m_facesIndex.resize(levelNumber);
		m_facesUseMat.resize(levelNumber);

		for(O3D_UINT k = 0 ; k < levelNumber ; ++k)
		{
			O3D_UINT levelSize = 0;
			O3D_INT useMaterial;

			_stream.read((O3D_CHAR*)&levelSize, sizeof(O3D_UINT));
			_stream.read((O3D_CHAR*)&useMaterial, sizeof(O3D_INT));

			m_facesUseMat[k] = O3D_BOOL(useMaterial);

			if (levelSize > 0)
			{
				m_facesIndex[k].resize(levelSize);
				_stream.read((O3D_CHAR*)&m_facesIndex[k][0], levelSize * sizeof(O3D_UINT));
			}
		}
	}

	// We write the array of error
	m_lodError.resize(levelNumber);

	_stream.read((O3D_CHAR*)&m_lodError[0], levelNumber * sizeof(O3DZoneError));

	// We read the other sub results
	O3D_UINT subResultNumber = 0;

	_stream.read((O3D_CHAR*)&subResultNumber, sizeof(O3D_UINT));

	if (subResultNumber > 0)
	{
		m_children.resize(subResultNumber);

		for (O3D_UINT k = 0 ; k < subResultNumber ; ++k)
			m_children[k].Read(_stream);
	}
}

O3DZoneManipResult::O3DZoneManipResult():
	m_heightmapData(),
	m_dx(1.0f),
	m_dy(1.0f),
	m_dz(1.0f),
	m_materialData(),
	m_vertexArray(),
	m_vertexIndexArray(),
	m_facesIndex(),
	m_facesUseMat(),
	m_lodError(),
	m_children()
{
}

O3DZoneManipResult::O3DZoneManipResult(const O3DZoneManipResult & _which):
	m_heightmapData(_which.m_heightmapData),
	m_dx(_which.m_dx),
	m_dy(_which.m_dy),
	m_dz(_which.m_dz),
	m_materialData(_which.m_materialData),
	m_vertexArray(_which.m_vertexArray),
	m_vertexIndexArray(_which.m_vertexIndexArray),
	m_facesIndex(_which.m_facesIndex),
	m_facesUseMat(_which.m_facesUseMat),
	m_lodError(_which.m_lodError),
	m_children(_which.m_children)
{
}

O3DZoneManipResult::~O3DZoneManipResult()
{
}

O3DZoneManipResult & O3DZoneManipResult::operator = (const O3DZoneManipResult & _which)
{
	m_heightmapData = _which.m_heightmapData;
	m_dx = _which.m_dx;
	m_dy = _which.m_dy;
	m_dz = _which.m_dz;
	m_materialData = _which.m_materialData;
	m_vertexArray = _which.m_vertexArray;
	m_vertexIndexArray = _which.m_vertexIndexArray;
	m_facesIndex = _which.m_facesIndex;
	m_facesUseMat = _which.m_facesUseMat;
	m_lodError = _which.m_lodError;
	m_children = _which.m_children;

	return *this;
}

/* Clear all allocated memory */
void O3DZoneManipResult::Destroy()
{
	m_heightmapData.Free();
	m_materialData.Free();
	m_vertexArray.clear();
	m_vertexIndexArray.clear();
	m_facesIndex.clear();
	m_facesUseMat.clear();
	m_lodError.clear();
	m_children.clear();
}

/* Return a binary buffer in which is stored all datas.
 * - _buffer must be NULL */
void O3DZoneManipResult::Write(O3D_CHAR * & _buffer, O3D_UINT & _size)
{
	O3D_ASSERT(_buffer == NULL);

	std::stringbuf strBuf;
	std::ostream stream(&strBuf);

	Write(stream);

	stream.flush();
	stream.seekp(0, std::ios_base::end);
	_size = O3D_UINT(stream.tellp());

	_buffer = new O3D_CHAR[_size];
	//strBuf._Sgetn_s(_buffer, _size, _size);
	strBuf.sgetn(_buffer,_size);
}

/* Initialize the result object with a buffer */
O3D_UINT O3DZoneManipResult::Read(const O3D_CHAR * _buffer, O3D_UINT size)
{
	Destroy();

	std::stringbuf strBuf;
	strBuf.sputn(_buffer, size);
	strBuf.pubseekpos(0, std::ios_base::in);
	std::istream stream(&strBuf);

	Read(stream);

	return O3D_UINT(stream.tellg());
}

O3D_UINT O3DZoneManipResult::GetLodNumber(O3D_UINT _level) const
{
	O3D_ASSERT(_level < GetLevelNumber());

	if (_level == 0)
		return GetLodNumber();
	else
		return m_children[0].GetLodNumber(_level - 1);
}

O3D_UINT O3DZoneManipResult::GetLevelNumber() const
{
	O3D_UINT counter = 1;

	const O3DZoneManipResult * _ptr = this;

	while (_ptr->HasChildren())
	{
		counter++;
		_ptr = &_ptr->GetChild(0);
	}

	return counter;
}

/*---------------------------------------------------------------------------------------
  Struct O3DTransformation
---------------------------------------------------------------------------------------*/
O3DTransformation::O3DTransformation():
	pFace(NULL),
	pVertex(NULL),
	ind(0xFFFFFFFF),
	energy(0.0f),
	ek(0.0f),
	en(0.0f),
	ee(0.0f)
{}

/* Functions */
PO3DVertex O3DTransformation::MovableVertex()
{
	switch(ind)
	{
	case 0: return pFace->GetNextVertex(pVertex);
	case 1: return pVertex;
	default: O3D_ASSERT(0);
	}

	return NULL;
}

PO3DVertex O3DTransformation::StaticVertex()
{
	switch(ind)
	{
	case 0: return pVertex;
	case 1: return pFace->GetNextVertex(pVertex);
	default: O3D_ASSERT(0);
	}

	return NULL;
}

O3D_BOOL O3DTransformation::operator == (const O3DTransformation & _which)
{
	return (((this->pVertex == _which.pVertex) && (this->pFace == _which.pFace)) ||
			((this->pFace->GetNeighbor(this->pVertex)) && (this->pFace->GetNeighbor(this->pVertex) == _which.pFace) &&
			(_which.pFace->GetNeighbor(_which.pVertex)) && (_which.pFace->GetNeighbor(_which.pVertex) == this->pFace) &&
			(this->pFace->GetNextVertex(this->pVertex) == _which.pVertex) && (_which.pFace->GetNextVertex(_which.pVertex) == this->pVertex)));
}

/*---------------------------------------------------------------------------------------
  Class O3DZoneManip
---------------------------------------------------------------------------------------*/
O3DZoneManip::O3DZoneManip(O3DZoneManip * _pMessageHandler):
	m_instruction(),
	m_init(O3D_FALSE),
	m_heightmap(),
	m_vertices(),
	m_faces(),
	m_deleted_faces(),
	m_vertex_list(),
	m_vertex_index(),
	m_transformations(),
	m_deleted_transformations(),
	m_currentRule(-1),
	m_currentLevel(-1),
	m_lastLodFaceNumber(0),
	m_lastLodVertexNumber(0),
	m_initFaceNumber(0),
	m_initVertexNumber(0),
	m_zoneManipArray(),
	m_mutexFunction(),
	m_pMessageHandler(_pMessageHandler),
	m_progression(0.0f),
	m_pauseRequested(O3D_FALSE),
	m_abortRequested(O3D_FALSE),
	m_pauseMutex(),
	m_resumeMutex(),
	m_workingMutex()
{
}

O3DZoneManip::O3DZoneManip():
	m_instruction(),
	m_init(O3D_FALSE),
	m_heightmap(),
	m_vertices(),
	m_faces(),
	m_deleted_faces(),
	m_vertex_list(),
	m_vertex_index(),
	m_transformations(),
	m_deleted_transformations(),
	m_currentRule(-1),
	m_currentLevel(-1),
	m_lastLodFaceNumber(0),
	m_lastLodVertexNumber(0),
	m_initFaceNumber(0),
	m_initVertexNumber(0),
	m_zoneManipArray(),
	m_mutexFunction(),
	m_pMessageHandler(NULL),
	m_progression(0.0f),
	m_pauseRequested(O3D_FALSE),
	m_abortRequested(O3D_FALSE),
	m_pauseMutex(),
	m_resumeMutex(),
	m_workingMutex()
{
	m_pMessageHandler = this;
}

O3DZoneManip::~O3DZoneManip()
{
	for (O3D_IT_FaceArray it = m_faces.begin() ; it != m_faces.end() ; it++)
		(*it)->Destroy();

	for (O3D_IT_VertexArray it = m_vertices.begin() ; it != m_vertices.end() ; it++)
		(*it)->Destroy();

	for (O3D_IT_FaceArray it = m_deleted_faces.begin() ; it != m_deleted_faces.end() ; it++)
		(*it)->Destroy();

	for (O3D_IT_VertexArray it = m_vertex_list.begin() ; it != m_vertex_list.end() ; it++)
		(*it)->Destroy();


	m_faces.clear();
	m_vertices.clear();
	m_deleted_faces.clear();
	m_vertex_list.clear();
	m_vertex_index.clear();

	for(O3D_UINT k = 0 ; k < m_zoneManipArray.Elt() ; ++k)
		O3D_DELETE(m_zoneManipArray[k]);
}

/* Initialize the object with the provided instructions */
void O3DZoneManip::Init(const O3DZoneManipInfo & _instruction)
{
	// First, we check all datas
	O3D_ASSERT(IsPowerOf(_instruction.layerWidth-1, 2));
	O3D_ASSERT(IsPowerOf(_instruction.layerHeight-1, 2));

	O3D_ASSERT(_instruction.pSource != NULL);

	O3D_ASSERT(_instruction.rules.size() >= 1);

	for (O3D_UINT k = 0 ; k < O3D_UINT(_instruction.rules.size()) ; ++k)
	{
		O3D_ASSERT(_instruction.rules[k].size() >= 1);

		for (O3D_UINT m = 0 ; m < O3D_UINT(_instruction.rules[k].size()) ; ++m)
		{
			O3D_ASSERT(!_instruction.rules[k][m].useMaterial || (_instruction.pMaterial != NULL));
		}
	}

	m_instruction = _instruction;

	m_progression = 0.0f;

	// In fact, this object always use the first rules from its instruction object, since the other levels
	// are erased before being passed to this object.
	m_currentLevel = 0;
	m_currentRule = 0;

	// Next, if there are more than one level, we initialize all subobject
	if (m_instruction.rules.size() > 1)
	{
		// We must split the heightmap, material and energetic in four sub array
		O3DArray2Di		aHeightmap;
		O3DArray2Dui	aMaterial;
		O3DArray2Df		aEnergetic;

		aHeightmap.SetSize(m_instruction.layerWidth, m_instruction.layerHeight);
		memcpy((void*)&aHeightmap[0], (const void*)m_instruction.pSource, aHeightmap.Elt() * sizeof(O3D_INT));

		if (m_instruction.pMaterial != NULL)
		{
			aMaterial.SetSize(m_instruction.layerWidth, m_instruction.layerHeight);
			memcpy((void*)&aMaterial[0], (const void*)m_instruction.pMaterial, aMaterial.Elt() * sizeof(O3D_INT));
		}

		if (m_instruction.pEnergetic != NULL)
		{
			aEnergetic.SetSize(m_instruction.layerWidth, m_instruction.layerHeight);
			memcpy((void*)&aEnergetic[0], (const void*)m_instruction.pEnergetic, aEnergetic.Elt() * sizeof(O3D_INT));
		}

		// We allocate memory for the four manip object
		m_zoneManipArray.SetSize(2,2);

		for (O3D_UINT j = 0 ; j < 2 ; ++j)
			for (O3D_UINT i = 0 ; i < 2 ; ++i)
			{
				m_zoneManipArray(i,j) = new O3DZoneManip((m_pMessageHandler!=NULL)?m_pMessageHandler:this);

				// Sub part of the heightmap
				O3DArray2Di subHeightmap;
				aHeightmap.GetSubArray(2, 2, subHeightmap, i, j);
				O3D_INT * pSource = new O3D_INT[subHeightmap.Elt()];
				memcpy((void*)pSource, (const void*)&subHeightmap[0], subHeightmap.Elt() * sizeof(O3D_INT));

				// Sub part of the material
				O3D_UINT * pMaterial = NULL;
				if (m_instruction.pMaterial != NULL)
				{
					O3DArray2Dui subMaterial;
					aMaterial.GetSubArray(2, 2, subMaterial, i, j);
					pMaterial = new O3D_UINT[aMaterial.Elt()];
					memcpy((void*)pMaterial, (const void*)&subMaterial[0], subMaterial.Elt() * sizeof(O3D_UINT));
				}

				// Sub part of the index
				O3D_FLOAT * pEnergetic = NULL;
				if (m_instruction.pEnergetic != NULL)
				{
					O3DArray2Df subEnergetic;
					aEnergetic.GetSubArray(2, 2, subEnergetic, i, j);
					pEnergetic = new O3D_FLOAT[aEnergetic.Elt()];
					memcpy((void*)pEnergetic, (const void*)&subEnergetic[0], subEnergetic.Elt() * sizeof(O3D_UINT));
				}

				// Construction of the new rules array
				O3D_T_LevelRulesArray zoneRules(m_instruction.rules);
				zoneRules.erase(zoneRules.begin());

				// We must build the instruction object
				O3DZoneManipInfo info;
				info.pSource = pSource;
				info.pMaterial = pMaterial;
				info.pEnergetic = pEnergetic;
				info.layerWidth = subHeightmap.Width();
				info.layerHeight = subHeightmap.Height();
				info.dx = m_instruction.dx;
				info.dy = m_instruction.dy;
				info.dz = m_instruction.dz;
				info.rules = zoneRules;
				info.ce = m_instruction.ce;
				info.cn = m_instruction.cn;
				info.ck = m_instruction.ck;

				m_zoneManipArray(i,j)->Init(info);
				m_zoneManipArray(i,j)->SetCurrentLevel(GetCurrentLevel() + 1);
			}
	}
	else
	{
		// We extract the buffer provided by the user
		m_heightmap.SetSize(m_instruction.layerWidth, m_instruction.layerHeight);
		memcpy(&m_heightmap[0], m_instruction.pSource, m_heightmap.Elt() * sizeof(O3D_INT));

		// First stage, we initialise vertex and face array
		InitVertexArray();
		InitFaceArray();

		CheckFaces();

		m_init = O3D_TRUE;

		m_lastLodFaceNumber = 2 * (m_heightmap.Width() - 1) * (m_heightmap.Height() - 1);
		m_lastLodVertexNumber = m_heightmap.Elt();

		m_initFaceNumber = m_lastLodFaceNumber;
		m_initVertexNumber = m_lastLodVertexNumber;

		// For the other zone, Vertex arrary and face array initialisation will be done
		// by merging the result from the 4 sub manip objects.
	}
}

/* Start the simplification */
O3DZoneManipExitCode O3DZoneManip::Start(O3DZoneManipResult & _result)
{
	O3D_ASSERT(!IsWorking());
	O3DFastMutexLocker locker(m_workingMutex);

/*	O3DEvent eventIterationDone(TERRAIN_GEN_ITERATION_DONE);
	O3DEvent eventLevelDone(TERRAIN_GEN_LEVEL_DONE);
	O3DEvent eventZoneDone(TERRAIN_GEN_ZONE_DONE);*/
	O3D_ASSERT(m_pMessageHandler != NULL);

	_result.Destroy();

	// If there are subzones, we start them first
	if (m_zoneManipArray.Elt() > 0)
	{
		O3D_ASSERT(m_instruction.rules.size() > 0);
		O3D_ASSERT(m_zoneManipArray.Elt() == 4);

		for (O3D_UINT k = 0 ; k < 4 ; ++k)
		{
			O3DZoneManipResult result;

			O3DZoneManipExitCode exitCode;
			if ((exitCode = m_zoneManipArray[k]->Start(result)) != O3D_ZONE_MANIP_SUCCESS)
				return exitCode;

			_result.m_children.push_back(result);
		}

		// Now we initialize this object
		Init(m_zoneManipArray);

		CheckFaces();
	}
	else
	{
		O3D_ASSERT(!Achieved());
	}

	O3D_ASSERT(m_init);

	// Declaration of the array of list of faces
	std::vector<O3D_T_FaceArray> levelFaceArray;
	std::vector<O3DZoneManipResult::O3DZoneError> generatedError;

#ifdef _MSC_VER
#pragma warning(disable:4127)
	while (O3D_TRUE)
#pragma warning(default:4127)
#else
	while (O3D_TRUE)
#endif
	{
		// Rebuilding the list of simplification is not necessary
		O3D_ASSERT(BuildLegalTransformations());

		while (!LevelAchieved() && (m_transformations.size() > 0))
		{
			Simplify();

			// It means, that abord is requested. Pause just block the execution in this function
			if (!MessageHandling())
			{
				return O3D_ZONE_MANIP_ABORTED;
			}

//			m_pMessageHandler->ThrowEvent(eventIterationDone);
			EVT_ITERATION_DONE();
		}

		// The first level is not a real one, we only reach the first rule
		// We clear all saved vertex
		if (m_currentRule == 0)
			m_vertex_list.clear();
		else
			//m_pMessageHandler->ThrowEvent(eventLevelDone);
			EVT_LEVEL_DONE();

		m_vertex_index.push_back(O3D_UINT(m_vertex_list.size()));

		// Refresh of somes counters

		m_lastLodFaceNumber = O3D_UINT(m_faces.size());
		m_lastLodVertexNumber = O3D_UINT(m_vertices.size());

		// We compute the error betwenn the initial heightmap and the new one
		O3DZoneManipResult::O3DZoneError errorValue;
		ComputeCurrentError(errorValue);

		generatedError.push_back(errorValue);

		// Since we must wait that all transformation is done before saving the face indices,
		// we must save the list of faces for each levels.
		// But we must not just copy the pointer, the face must kept intact and not be modified
		// by the future simplification

		// In this array, we first list in the first computed. So the first list of triangle is the best level
		// Later this order will change.
		levelFaceArray.push_back(O3D_T_FaceArray());

		// But there is a little problem here, the neighbors are copied too, but they point to wrong faces
		for (O3D_CIT_FaceArray it = m_faces.begin() ; it != m_faces.end() ; it++)
		{
			levelFaceArray.back().push_back(PO3DFace(new O3DFace(*(*it))));

			PO3DFace & last = levelFaceArray.back().back();

			// to avoid futur errors, we destroy the neighbors defined in this face
			last->SetNeighbor(0, PO3DFace((O3DFace*)NULL));
			last->SetNeighbor(1, PO3DFace((O3DFace*)NULL));
			last->SetNeighbor(2, PO3DFace((O3DFace*)NULL));
		}

		if (m_currentRule < O3D_INT(m_instruction.rules[0].size()-1))
			m_currentRule++;
		else
		{
			// Otherwise, if it's the last level, we must add all remaining vertex in the final list
			m_vertex_list.insert(m_vertex_list.end(), m_vertices.begin(), m_vertices.end());

			for (O3D_UINT k = 0 ; k < O3D_UINT(m_vertex_list.size()) ; ++k)
				m_vertex_list[k]->Set_index(k);

			m_vertex_index.push_back(O3D_UINT(m_vertex_list.size()));
			break;
		}
	}

	// We save result
	// First we save all vertex depending on their position in the heightmap
	O3D_ASSERT(_result.m_vertexArray.size() == 0);
	_result.m_vertexArray.reserve(2 * m_vertex_list.size());

	for (O3D_IT_VertexArray it = m_vertex_list.begin() ; it != m_vertex_list.end() ; it++)
		_result.m_vertexArray.push_back(std::pair<O3D_USHORT, O3D_USHORT>(O3D_USHORT((*it)->Get_i()),O3D_USHORT((*it)->Get_j())));

	_result.m_vertexIndexArray = m_vertex_index;
	_result.m_heightmapData = m_heightmap;
	_result.m_dx = m_instruction.dx;
	_result.m_dy = m_instruction.dy;
	_result.m_dz = m_instruction.dz;

	// Now we can save the list of faces for each levels, since "index" of all final vertex are defined.
	_result.m_facesIndex.resize(levelFaceArray.size());
	_result.m_facesUseMat.clear();
	_result.m_lodError.clear();

	// We read the list in the reverse order, we must do the same to read the rules
	std::vector<std::vector<O3D_UINT> >::reverse_iterator itTarget = _result.m_facesIndex.rbegin();
	std::vector<O3DZoneManipResult::O3DZoneError>::reverse_iterator itError = generatedError.rbegin();
	O3D_UINT ruleIndex = O3D_UINT(m_instruction.rules.front().size()) - 1;

	for(std::vector<O3D_T_FaceArray>::iterator it = levelFaceArray.begin() ; it != levelFaceArray.end() ; it++, itTarget++, ruleIndex--, itError++)
	{
		itTarget->reserve(3 * it->size());	///< A face is 3 index

		_result.m_facesUseMat.push_back((m_instruction.rules.front()[ruleIndex].useMaterial) && (m_instruction.pMaterial != NULL));
		_result.m_lodError.push_back(*itError);

		for(O3D_T_FaceArray::iterator itFace = it->begin() ; itFace != it->end() ; itFace++)
		{
			itTarget->push_back((*itFace)->GetVertex(0)->Get_index());
			itTarget->push_back((*itFace)->GetVertex(1)->Get_index());
			itTarget->push_back((*itFace)->GetVertex(2)->Get_index());
		}
	}

	if (m_instruction.pMaterial != NULL)
	{
		_result.m_materialData.SetSize(m_instruction.layerWidth, m_instruction.layerHeight);
		memcpy((void*)&_result.m_materialData[0], (const void*)m_instruction.pMaterial, _result.m_materialData.Elt() * sizeof(O3D_UINT));
	}

//	m_pMessageHandler->ThrowEvent(eventZoneDone);
	EVT_ZONE_DONE();

	return O3D_ZONE_MANIP_SUCCESS;
}

/* Abort the simplification */
void O3DZoneManip::Abort(O3D_BOOL _wait)
{
	if (!IsWorking())
		return;

	O3D_ASSERT(!m_abortRequested);

	if (IsPaused())
		Resume();

	m_abortRequested = O3D_TRUE;

	if (_wait)
	{
		m_workingMutex.Lock();
		m_workingMutex.Unlock();
	}
}

/* Pause the simplification */
void O3DZoneManip::Pause(O3D_BOOL _wait)
{
	if (!IsWorking())
		return;

	if (m_resumeMutex.IsLocked())	///< Means that the task is already paused
		return;

	O3D_ASSERT(!m_pauseRequested);

	m_pauseRequested = O3D_TRUE;

	if (_wait)
	{
		m_resumeMutex.Lock();
		m_pauseMutex.Lock();	///< Wait for the thread to unlock this mutex
		m_pauseMutex.Unlock();
	}
}

/* Resume the simplification */
void O3DZoneManip::Resume()
{
	if (!IsPaused())
		return;

	if (!IsWorking())
		return;

	O3D_ASSERT(m_resumeMutex.IsLocked());

	m_resumeMutex.Unlock();
}

void O3DZoneManip::InitVertexArray()
{
	PO3DVertex pVertex = NULL;

	for (O3D_UINT j = 0 ; j < Height() ; ++j)
	{
		O3D_FLOAT cy(j * Dy());

		for (O3D_UINT i = 0 ; i < Width() ; ++i)
		{
			O3D_FLOAT cx(i * Dx());

			pVertex = new O3DVertex();

			pVertex->Set_point(O3DVector3(cx, cy, PosZ(i,j)));
			pVertex->Set_type(O3D_VERTEX_NORMAL);
			pVertex->Set_edge(O3D_EDGE_NO);

			if (i == 0)		 { pVertex->Set_type(O3D_VERTEX_EDGE); pVertex->Set_edge(O3D_EDGE_0); }
			else if (j == 0) { pVertex->Set_type(O3D_VERTEX_EDGE); pVertex->Set_edge(O3D_EDGE_3); }
			else if (j == Height() - 1) { pVertex->Set_type(O3D_VERTEX_EDGE); pVertex->Set_edge(O3D_EDGE_1); }
			else if (i == Width() - 1)	{ pVertex->Set_type(O3D_VERTEX_EDGE); pVertex->Set_edge(O3D_EDGE_2); }

			if ((i == 0) && (j == 0))						{ pVertex->Set_type(O3D_VERTEX_CORNER);  pVertex->Set_edge(O3D_EDGE_0); }
			if ((i == 0) && (j == Height()-1))	{ pVertex->Set_type(O3D_VERTEX_CORNER); pVertex->Set_edge(O3D_EDGE_1); }
			if ((i == Width()-1) && (j == Height()-1))	{ pVertex->Set_type(O3D_VERTEX_CORNER); pVertex->Set_edge(O3D_EDGE_2); }
			if ((i == Width()-1) && (j == 0))						{ pVertex->Set_type(O3D_VERTEX_CORNER); pVertex->Set_edge(O3D_EDGE_3); }

			pVertex->Get_i() = i;
			pVertex->Get_j() = j;
			pVertex->Set_id(O3DVertex::GetNewId());
			pVertex->Set_index((O3D_UINT)m_vertices.size());

			m_vertices.push_back(pVertex);
		}
	}
}

void O3DZoneManip::InitFaceArray()
{
	PO3DFace pFace;
	O3D_INT ind = 0;

	m_faces.reserve(2*m_vertices.size());

	for (O3D_UINT j = 0 ; j < Height() - 1 ; ++j)
		for (O3D_UINT i = 0 ; i < Width() ; ++i)
		{
			if (i != Width() - 1)
			{
				pFace = PO3DFace(new O3DFace());
				pFace->SetVertex(0, m_vertices[ind]);
				pFace->SetVertex(1, m_vertices[ind + Width()]);
				pFace->SetVertex(2, m_vertices[ind + 1]);
				pFace->Set_id(O3DFace::GetNewId());
				pFace->Set_index((O3D_UINT)m_faces.size());

				m_faces.push_back(pFace);

				pFace = new O3DFace();
				pFace->SetVertex(0, m_vertices[ind+1]);
				pFace->SetVertex(1, m_vertices[ind + Width()]);
				pFace->SetVertex(2, m_vertices[ind+ Width() + 1]);
				pFace->Set_id(O3DFace::GetNewId());
				pFace->Set_index((O3D_UINT)m_faces.size());

				m_faces.push_back(pFace);
			}

			ind++;
		}

	ind = 0;

	for (O3D_UINT j = 0 ; j < Height() - 1 ; ++j)
		for (O3D_UINT i = 0 ; i < Width() - 1 ; ++i)
		{
			// paramètrage des voisins
			if (i == 0) m_faces[ind]->SetNeighbor(0, NULL);
			else m_faces[ind]->SetNeighbor(0, m_faces[ind - 1]);

			m_faces[ind]->SetNeighbor(1, m_faces[ind + 1]);

			if (j == 0) m_faces[ind]->SetNeighbor(2, NULL);
			else m_faces[ind]->SetNeighbor(2, m_faces[ind - 2*(Width() - 1) + 1]);

			ind++;

			// paramètrage des voisins
			m_faces[ind]->SetNeighbor(0, m_faces[ind-1]);

			if (j == Height() - 2) m_faces[ind]->SetNeighbor(1, NULL);
			else m_faces[ind]->SetNeighbor(1, m_faces[ind + 2*(Width()-1) - 1]);

			if (i == Width() - 2) m_faces[ind]->SetNeighbor(2, NULL);
			else m_faces[ind]->SetNeighbor(2, m_faces[ind + 1]);

			ind++;
		}
}

/* Check the link between faces */
void O3DZoneManip::CheckFaces()
{
	for (O3D_IT_FaceArray it = m_faces.begin() ; it != m_faces.end() ; it++)
		(*it)->Check();
}

/* Construct the list of all neighbors of a specific vertex and face.
 * The parameter _from_face indicate the starting face and _to_face the final face.
 * So that, _to_face won't be inserted in the list. */
void O3DZoneManip::BuildNeighborList(PO3DFace _base_face, PO3DVertex _vertex, PO3DFace _from_face, PO3DFace _to_face, O3D_T_FaceArray & _vect)
{
	O3D_ASSERT(_vertex);
/* Modif */ //	O3D_ASSERT(_base_face != NULL);

	PO3DFace t_face;

	if (_from_face)
	{
		t_face = _from_face;

		while ((t_face) && (t_face != _to_face))
		{
			_vect.push_back(t_face);
			t_face = t_face->GetPrevNeighbor(_vertex);
		}

		// si la boucle s'est arreté car t_face = NULL, faut repartir de l'autre sens
		if ((_to_face) && (!t_face))
		{
			if (_to_face->GetNeighbor(_vertex))
			{
				t_face = _to_face->GetNeighbor(_vertex);

				while (t_face)
				{
					_vect.push_back(t_face);
					t_face = t_face->GetNeighbor(_vertex);
				}
			}
		}
	}
	else
	{
		if (!_to_face) return;

		if (_to_face->GetNeighbor(_vertex))
		{
			t_face = _to_face->GetNeighbor(_vertex);

			while (t_face)
			{
				_vect.push_back(t_face);
				t_face = t_face->GetNeighbor(_vertex);
			}
		}
	}
}

/* Return O3D_TRUE, if the simplification is legal */
O3D_BOOL O3DZoneManip::IsLegalTransformation(PO3DFace _face, PO3DVertex _vertex, PO3DVertex _target) const
{
	O3D_ASSERT(_face && _vertex);

	PO3DVertex v1 = _vertex;
	PO3DVertex v2 = _face->GetNextVertex(_vertex);

	PO3DVertex target; // vertex cible pour CLM_SIMPL_TO_VERTEX
	PO3DVertex vmove; // vertex qui se déplace (qui va donc etre détruit)

	O3D_ASSERT(_target);

	target = _target;

	if (target != v1) vmove = v1;
	else vmove = v2;

	// si le vertex que l'on déplace est un coin
	if (vmove->Get_type() == O3D_VERTEX_CORNER) return O3D_FALSE;

	// Si le vertex qu'on bouge passe d'un état particulier à un état normal
	if ((vmove->Get_type() != O3D_VERTEX_NORMAL) && (target->Get_type() == O3D_VERTEX_NORMAL)) return O3D_FALSE;

	// Si la simplification des bords est interdite
	if ((vmove->Get_type() != O3D_VERTEX_NORMAL) && (target->Get_type() != O3D_VERTEX_NORMAL)) return O3D_FALSE;

	if ((vmove->Get_type() == O3D_VERTEX_EDGE) && (target->Get_type() == O3D_VERTEX_EDGE) && (vmove->Get_edge() != target->Get_edge())) return O3D_FALSE;

	if (GetRule(0,m_currentRule).useMaterial)
		if (!MaterialSimplificationAllowed(vmove)) return O3D_FALSE;

  // dans tous les autres cas
  return O3D_TRUE;
}

/* Return O3D_TRUE, if the vertex can belong to a simplification */
O3D_BOOL O3DZoneManip::MaterialSimplificationAllowed(PO3DVertex vertex) const
{
	O3D_ASSERT(GetRule(0,m_currentRule).useMaterial);

	O3D_ASSERT((vertex->Get_i() >= 0) && (O3D_UINT(vertex->Get_i()) < Width()));
	O3D_ASSERT((vertex->Get_j() >= 0) && (O3D_UINT(vertex->Get_j()) < Height()));

	O3D_UINT posi = O3D_UINT(vertex->Get_i());
	O3D_UINT posj = O3D_UINT(vertex->Get_j());

	if (posi > 0)
		if (Material(posi - 1, posj) != Material(posi, posj))
			return O3D_FALSE;

	if (posi < Width() - 1)
		if (Material(posi + 1, posj) != Material(posi, posj))
			return O3D_FALSE;

	if (posj > 0)
		if (Material(posi, posj - 1) != Material(posi,posj))
			return O3D_FALSE;

	if (posj < Height() - 1)
		if (Material(posi, posj+1) != Material(posi, posj))
			return O3D_FALSE;

	return O3D_TRUE;
}

/* Find the vertex which has _i and _j as coordinate on the heightmap */
O3D_INT O3DZoneManip::FindVertexWithPosIJ(O3D_UINT _i, O3D_UINT _j) const
{
    for (O3D_INT i = 0 ; i < O3D_INT(m_vertices.size()) ; ++i)
        if ((m_vertices[i]->Get_i() == O3D_INT(_i)) && (m_vertices[i]->Get_j() == O3D_INT(_j))) return i;

    return -1;
}

/* Find the face who owns vertex v1 and v2 in the counter clock wise. */
O3D_INT O3DZoneManip::FindFacesByVertices(PO3DVertex v1, PO3DVertex v2) const
{
    for (O3D_INT i = 0 ; i < O3D_INT(m_faces.size()) ; ++i)
        if (m_faces[i]->FindVertex(v1) != -1)
            if (m_faces[i]->GetNextVertex(v1) == v2) return i;

    return -1;
}

/* Add a transformation to the ordered list */
void O3DZoneManip::PushTransformation(const O3DTransformation & _which)
{
	m_transformations.insert(O3D_T_TransformationMultimap::value_type(_which.energy, _which));
}

/* Build the list of legal transformation */
O3D_BOOL O3DZoneManip::BuildLegalTransformations()
{
	m_deleted_transformations.clear();
	m_transformations.clear();

	O3DTransformation t_transf; // transformation temporaire
	O3DTransformation t_transf2; // transformation temporaire
	int valid_ind = -1;

	// on passe donc en revue chaque face
	for (O3D_UINT k = 0 ; k < m_faces.size() ; k++)
	{
		// on balaye tous les voisins
		for (int i=0; i<3; i++)
		{

		// pour éviter de faire deux fois le meme calcul et de mettre deux fois la meme transformation dans la liste
		if (!m_faces[k]->GetNeighbor(i) || ((m_faces[k]->GetNeighbor(i)) && (m_faces[k]->Get_id() < m_faces[k]->GetNeighbor(i)->Get_id())))
		{
			valid_ind = -1;
			t_transf.ind = -1;
			t_transf2.ind = -1;
			// on vérifie que la transformation est légale
			if (IsLegalTransformation(m_faces[k], m_faces[k]->GetVertex(i), m_faces[k]->GetVertex(i)))
			{
				t_transf.ind = 0;
				t_transf.pVertex = m_faces[k]->GetVertex(i);
				t_transf.pFace = m_faces[k];

				if (ComputeEnergy(t_transf))
				valid_ind = 0;
			}

			if (IsLegalTransformation(m_faces[k], m_faces[k]->GetVertex(i), m_faces[k]->GetNextVertex(i)))
			{
				t_transf2.ind = 1;
				t_transf2.pVertex = m_faces[k]->GetVertex(i);
				t_transf2.pFace = m_faces[k];

				if (ComputeEnergy(t_transf2))
				{
					if (valid_ind == 0)
					{
						if (t_transf.energy < t_transf2.energy)	PushTransformation(t_transf);
						else PushTransformation(t_transf2);
					}
					else PushTransformation(t_transf2);
				}
				else if (valid_ind == 0) PushTransformation(t_transf);
			}
			else if (valid_ind == 0) PushTransformation(t_transf);
			}
		}
	}

	return O3D_TRUE;
}

/* Return O3D_FALSE, if the next level is not reached */
O3D_BOOL O3DZoneManip::LevelAchieved() const
{
	const O3DRule & currentRule = GetRule(0, m_currentRule);

	switch (currentRule.type)
	{
	case O3D_RULE_FACE_NUMBER_LIMIT:
		return (O3D_UINT(m_faces.size()) < currentRule.value);
	case O3D_RULE_VERTEX_NUMBER_LIMIT:
		return (O3D_UINT(m_vertices.size()) < currentRule.value);
	case O3D_RULE_FACE_NUMBER_LOD_RELATIVE_PERCENTAGE:
		return (O3D_UINT(m_faces.size()) * 100 < m_lastLodFaceNumber * currentRule.value);
	case O3D_RULE_VERTEX_NUMBER_LOD_RELATIVE_PERCENTAGE:
		return (O3D_UINT(m_vertices.size()) * 100 < m_lastLodVertexNumber * currentRule.value);

	case O3D_RULE_FACE_NUMBER_LEVEL_RELATIVE_PERCENTAGE:
		return (O3D_UINT(m_faces.size()) * 100 < m_initFaceNumber * currentRule.value);
	case O3D_RULE_VERTEX_NUMBER_LEVEL_RELATIVE_PERCENTAGE:
		return (O3D_UINT(m_vertices.size()) * 100 < m_initVertexNumber * currentRule.value);

	case O3D_RULE_FACE_NUMBER_ABSOLUTE_PERCENTAGE:
		{
			if (m_currentLevel == 0)
				return (O3D_UINT(m_faces.size()) * 100 < 2 * (m_heightmap.Width()-1)*(m_heightmap.Height()-1) * currentRule.value);
			else
			{
				O3D_UINT absoluteFace = 2*(m_heightmap.Width()-1)*(m_heightmap.Height()-1)*(4 << (m_currentLevel-1));
				return (O3D_UINT(4*m_faces.size()) * 100 < absoluteFace * currentRule.value);
			}
		}
	case O3D_RULE_VERTEX_NUMBER_ABSOLUTE_PERCENTAGE:
		{
			if (m_currentLevel == 0)
				return (O3D_UINT(m_faces.size()) * 10 < m_heightmap.Elt() * currentRule.value);
			else
			{
				O3D_UINT absoluteVertex = ((m_heightmap.Width() - 1) * (2 << (m_currentLevel-1)) + 1)*((m_heightmap.Height() - 1) * (2 << (m_currentLevel-1)) + 1);
				return (O3D_UINT(4*m_vertices.size()) * 10 < absoluteVertex * currentRule.value);
			}
		}
	default:
		O3D_ASSERT(0);
		return O3D_TRUE;
	}
}

/* Return O3D_TRUE, if the task is currently in function Start() */
O3D_BOOL O3DZoneManip::IsWorking() const
{
	return m_workingMutex.IsLocked();
}

/* Return O3D_TRUE, if the task is currently paused */
O3D_BOOL O3DZoneManip::IsPaused() const
{
	return m_resumeMutex.IsLocked();
}

/* Function that returns O3D_FALSE, if abort is requested, else block the execution if pause is requested */
O3D_BOOL O3DZoneManip::MessageHandling()
{
	if (m_pMessageHandler != this)
		return m_pMessageHandler->MessageHandling();

	// Else it means that this object is the event manager
	if (m_abortRequested)
	{
		m_abortRequested = O3D_FALSE;

		return O3D_FALSE;
	}

	if (m_pauseRequested)
	{
		m_pauseRequested = O3D_FALSE;

		m_pauseMutex.Unlock();		///< Allow the Pause function to return (if wait is true)
		m_resumeMutex.Lock();		///< Wait for the resume mutex to be unlock to continue
		m_resumeMutex.Unlock();
		m_pauseMutex.Lock();
	}

	return O3D_TRUE;
}

/* Functions that return a heightmap built with a list of triangles */
void O3DZoneManip::BuildHeightmap(const O3D_T_VertexArray & _vertexArray, const O3D_T_FaceArray & _faceArray, const O3DVector2ui & _size, O3DArray2Df & _heightmap)
{
	_heightmap.SetSize(_size[X], _size[Y]);

	for (O3D_CIT_FaceArray it = _faceArray.begin() ; it != _faceArray.end() ; it++)
	{
		const PO3DVertex & v0 = (*it)->GetVertex(0);
		const PO3DVertex & v1 = (*it)->GetVertex(1);
		const PO3DVertex & v2 = (*it)->GetVertex(2);

		// We determine the box which contains the triangle
		O3D_INT xMin = o3d::Min<O3D_INT>(o3d::Min<O3D_INT>(v0->Get_i(), v1->Get_i()), v2->Get_i());
		O3D_INT yMin = o3d::Min<O3D_INT>(o3d::Min<O3D_INT>(v0->Get_j(), v1->Get_j()), v2->Get_j());

		O3D_INT xMax = o3d::Max<O3D_INT>(o3d::Max<O3D_INT>(v0->Get_i(), v1->Get_i()), v2->Get_i());
		O3D_INT yMax = o3d::Max<O3D_INT>(o3d::Max<O3D_INT>(v0->Get_j(), v1->Get_j()), v2->Get_j());

		O3DVector2i n0(v1->Get_j() - v0->Get_j(), - v1->Get_i() + v0->Get_i());
		O3DVector2i n1(v2->Get_j() - v1->Get_j(), - v2->Get_i() + v1->Get_i());
		O3DVector2i n2(v0->Get_j() - v2->Get_j(), - v0->Get_i() + v2->Get_i());

		for (O3D_INT j = yMin ; j <= yMax ; ++j)
			for (O3D_INT i = xMin ; i <= xMax ; ++i)
			{
				O3DVector2i p0(i - v0->Get_i(), j - v0->Get_j());
				O3DVector2i p1(i - v1->Get_i(), j - v1->Get_j());

				if (((p0 * n0) >= 0) && ((p0 * n2) >= 0) && ((p1 * n1) >= 0))
				{
					// We must compute the altitude of the point (x,y) projected on the face

					O3DVector3 c0(O3D_FLOAT(v1->Get_i() - v0->Get_i()), O3D_FLOAT(v1->Get_j() - v0->Get_j()), O3D_FLOAT(v1->Get_point()[Z] - v0->Get_point()[Z]));
					O3DVector3 c1(O3D_FLOAT(v2->Get_i() - v1->Get_i()), O3D_FLOAT(v2->Get_j() - v1->Get_j()), O3D_FLOAT(v2->Get_point()[Z] - v1->Get_point()[Z]));

					O3DVector3 normal = c0 ^ (-c1);
					_heightmap(i,j) = - (normal[X] * O3D_FLOAT(p0[X]) + normal[Y] * O3D_FLOAT(p0[Y]))/normal[Z] + v0->Get_point()[Z];
				}
			}
	}
}

/* Return the max, the min, the average, and "ecart type" of the difference */
void O3DZoneManip::HeightmapDifference(const O3DArray2Df & _h1, const O3DArray2Df & _h2, O3D_FLOAT & _max, O3D_FLOAT & _min, O3D_FLOAT & _med, O3D_FLOAT & _ec)
{
	O3D_ASSERT(_h1.Width() == _h2.Width());
	O3D_ASSERT(_h1.Height() == _h2.Height());

	if (_h1.Elt() == 0)
	{
		_max = 0.0f;
		_min = 0.0f;
		_med = 0.0f;
		_ec = 0.0f;
		return;
	}

	O3D_FLOAT ratio = 1.0f / O3D_FLOAT(_h1.Elt());

	_max = (_h1[0]-_h2[0]);
	_min = (_h1[0]-_h2[0]);
	_med = (_h1[0]-_h2[0]) * ratio;

	O3D_FLOAT delta;

	for (O3D_UINT k = 0 ; k < _h1.Elt() ; ++k)
	{
		delta = o3d::Abs<O3D_FLOAT>(_h1[k] - _h2[k]);

		_max = o3d::Max<O3D_FLOAT>(delta, _max);
		_min = o3d::Min<O3D_FLOAT>(delta, _min);

		_med += ratio * delta;
	}

	_ec = 0.0f;

	// Second passage pour le calcul de l'écart type
	for (O3D_UINT k = 0 ; k < _h1.Elt() ; ++k)
	{
		delta = _h1[k] - _h2[k];

		_ec += ratio * o3d::Sqr<O3D_FLOAT>(delta - _med);
	}

	_ec = sqrtf(_ec);
}

/* Return different values associated with the error generated by the simplification */
void O3DZoneManip::ComputeCurrentError(O3DZoneManipResult::O3DZoneError & _error)
{
	O3DArray2Df tHeightmap;

	BuildHeightmap(m_vertices, m_faces, O3DVector2ui(m_heightmap.Width(), m_heightmap.Height()), tHeightmap);

	O3DArray2Df initHeightmap;
	initHeightmap.SetSize(m_heightmap.Width(), m_heightmap.Height());

	for (O3D_UINT k = 0 ; k < initHeightmap.Elt() ; ++k)
		initHeightmap[k] = m_heightmap[k] * m_instruction.dz;

	HeightmapDifference(tHeightmap, initHeightmap, _error.maxError, _error.minError, _error.medError, _error.medDelta);
}

/* Return the global progression */
O3D_FLOAT O3DZoneManip::GetProgression(O3D_UINT _levelNumber) const
{
	O3D_FLOAT progress = 0.0f;
	O3D_FLOAT factor = 1.0f / O3D_FLOAT(_levelNumber);

	// Si il y a des sous objets
	for (O3D_UINT k = 0 ; k < m_zoneManipArray.Elt() ; ++k)
	{
		progress += m_zoneManipArray[k]->GetProgression(_levelNumber);
	}

	if ((m_instruction.rules.size() > 0) && (m_instruction.rules[0].size() > 0))
	{
		factor /= O3D_FLOAT(1 << (2* GetCurrentLevel()));

		if (m_instruction.rules[0].size() == 1)
			progress += factor;
		else
			progress += factor * O3D_FLOAT(m_currentRule) / O3D_FLOAT(m_instruction.rules[0].size() - 1);
	}

	return progress;
}

/* Return the progression of this object */
O3D_FLOAT O3DZoneManip::GetProgression() const
{
	return GetProgression(O3D_UINT(m_instruction.rules.size()));
}
