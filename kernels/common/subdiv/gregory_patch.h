// ======================================================================== //
// Copyright 2009-2015 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once

#include "catmullclark_patch.h"
#include "bezier_patch.h"

namespace embree
{  
  template<typename Vertex, typename Vertex_t = Vertex>
  class __aligned(64) GregoryPatchT
  {
    typedef CatmullClarkPatchT<Vertex,Vertex_t> CatmullClarkPatch;
    typedef GeneralCatmullClarkPatchT<Vertex,Vertex_t> GeneralCatmullClarkPatch;

  public:
    Vertex v[4][4];
    Vertex f[2][2];
    
    __forceinline Vertex& p0() { return v[0][0]; }
    __forceinline Vertex& p1() { return v[0][3]; }
    __forceinline Vertex& p2() { return v[3][3]; }
    __forceinline Vertex& p3() { return v[3][0]; }
    
    __forceinline Vertex& e0_p() { return v[0][1]; }
    __forceinline Vertex& e0_m() { return v[1][0]; }
    __forceinline Vertex& e1_p() { return v[1][3]; }
    __forceinline Vertex& e1_m() { return v[0][2]; }
    __forceinline Vertex& e2_p() { return v[3][2]; }
    __forceinline Vertex& e2_m() { return v[2][3]; }
    __forceinline Vertex& e3_p() { return v[2][0]; }
    __forceinline Vertex& e3_m() { return v[3][1]; }
    
    __forceinline Vertex& f0_p() { return v[1][1]; }
    __forceinline Vertex& f1_p() { return v[1][2]; }
    __forceinline Vertex& f2_p() { return v[2][2]; }
    __forceinline Vertex& f3_p() { return v[2][1]; }
    __forceinline Vertex& f0_m() { return f[0][0]; }
    __forceinline Vertex& f1_m() { return f[0][1]; }
    __forceinline Vertex& f2_m() { return f[1][1]; }
    __forceinline Vertex& f3_m() { return f[1][0]; }
    
    __forceinline const Vertex& p0() const { return v[0][0]; }
    __forceinline const Vertex& p1() const { return v[0][3]; }
    __forceinline const Vertex& p2() const { return v[3][3]; }
    __forceinline const Vertex& p3() const { return v[3][0]; }
    
    __forceinline const Vertex& e0_p() const { return v[0][1]; }
    __forceinline const Vertex& e0_m() const { return v[1][0]; }
    __forceinline const Vertex& e1_p() const { return v[1][3]; }
    __forceinline const Vertex& e1_m() const { return v[0][2]; }
    __forceinline const Vertex& e2_p() const { return v[3][2]; }
    __forceinline const Vertex& e2_m() const { return v[2][3]; }
    __forceinline const Vertex& e3_p() const { return v[2][0]; }
    __forceinline const Vertex& e3_m() const { return v[3][1]; }
    
    __forceinline const Vertex& f0_p() const { return v[1][1]; }
    __forceinline const Vertex& f1_p() const { return v[1][2]; }
    __forceinline const Vertex& f2_p() const { return v[2][2]; }
    __forceinline const Vertex& f3_p() const { return v[2][1]; }
    __forceinline const Vertex& f0_m() const { return f[0][0]; }
    __forceinline const Vertex& f1_m() const { return f[0][1]; }
    __forceinline const Vertex& f2_m() const { return f[1][1]; }
    __forceinline const Vertex& f3_m() const { return f[1][0]; }
    
    __forceinline Vertex initCornerVertex(const CatmullClarkPatch& irreg_patch, const size_t index) {
      return irreg_patch.ring[index].getLimitVertex();
    }
    
    __forceinline Vertex initPositiveEdgeVertex(const CatmullClarkPatch& irreg_patch, const size_t index, const Vertex& p_vtx) {
      return 1.0f/3.0f * irreg_patch.ring[index].getLimitTangent() + p_vtx;
    }
    
    __forceinline Vertex initNegativeEdgeVertex(const CatmullClarkPatch& irreg_patch, const size_t index, const Vertex& p_vtx) {
      return 1.0f/3.0f * irreg_patch.ring[index].getSecondLimitTangent() + p_vtx;
    }
    
    void initFaceVertex(const CatmullClarkPatch& irreg_patch, 
			const size_t index, 
			const Vertex& p_vtx, 
                        const Vertex& e0_p_vtx, 
			const Vertex& e1_m_vtx, 
			const unsigned int face_valence_p1,
 			const Vertex& e0_m_vtx,	
			const Vertex& e3_p_vtx,	
			const unsigned int face_valence_p3,
			Vertex& f_p_vtx, 
			Vertex& f_m_vtx)
    {
      const unsigned int face_valence = irreg_patch.ring[index].face_valence;
      const unsigned int edge_valence = irreg_patch.ring[index].edge_valence;
      const unsigned int border_index = irreg_patch.ring[index].border_index;
      
      const Vertex& vtx     = irreg_patch.ring[index].vtx;
      const Vertex e_i      = irreg_patch.ring[index].getEdgeCenter(0);
      const Vertex c_i_m_1  = irreg_patch.ring[index].getQuadCenter(0);
      const Vertex e_i_m_1  = irreg_patch.ring[index].getEdgeCenter(1);
      
      Vertex c_i, e_i_p_1;
      const bool hasHardEdge =
        std::isinf(irreg_patch.ring[index].vertex_crease_weight) &&
        std::isinf(irreg_patch.ring[index].crease_weight[0]);
                
      if (unlikely(border_index == edge_valence-2) || hasHardEdge)
      {
        /* mirror quad center and edge mid-point */
        c_i     = c_i_m_1 + 2 * (e_i - c_i_m_1);
        e_i_p_1 = e_i_m_1 + 2 * (vtx - e_i_m_1);
      }
      else
      {
        c_i     = irreg_patch.ring[index].getQuadCenter( face_valence-1 );
        e_i_p_1 = irreg_patch.ring[index].getEdgeCenter( face_valence-1 );
      }
      
      Vertex c_i_m_2, e_i_m_2;
      if (unlikely(border_index == 2 || face_valence == 2 || hasHardEdge))
      {
        /* mirror quad center and edge mid-point */
        c_i_m_2  = c_i_m_1 + 2 * (e_i_m_1 - c_i_m_1);
        e_i_m_2  = e_i + 2 * (vtx - e_i);	  
      }
      else
      {
        c_i_m_2  = irreg_patch.ring[index].getQuadCenter( 1 );
        e_i_m_2  = irreg_patch.ring[index].getEdgeCenter( 2 );
      }      
      
      const float d = 3.0f;
      const float c     = cosf(2.0*M_PI/(float)face_valence);
      const float c_e_p = cosf(2.0*M_PI/(float)face_valence_p1);
      const float c_e_m = cosf(2.0*M_PI/(float)face_valence_p3);
      
      const Vertex r_e_p = 1.0f/3.0f * (e_i_m_1 - e_i_p_1) + 2.0f/3.0f * (c_i_m_1 - c_i);
      const Vertex r_e_m = 1.0f/3.0f * (e_i     - e_i_m_2) + 2.0f/3.0f * (c_i_m_1 - c_i_m_2);

      f_p_vtx = 1.0f / d * (c_e_p * p_vtx + (d - 2.0f*c - c_e_p) * e0_p_vtx + 2.0f*c* e1_m_vtx + r_e_p);      
      f_m_vtx = 1.0f / d * (c_e_m * p_vtx + (d - 2.0f*c - c_e_m) * e0_m_vtx + 2.0f*c* e3_p_vtx + r_e_m);     
    }

    __noinline void init(const CatmullClarkPatch& patch)
    {
      assert( patch.ring[0].hasValidPositions() );
      assert( patch.ring[1].hasValidPositions() );
      assert( patch.ring[2].hasValidPositions() );
      assert( patch.ring[3].hasValidPositions() );
      
      p0() = initCornerVertex(patch,0);
      p1() = initCornerVertex(patch,1);
      p2() = initCornerVertex(patch,2);
      p3() = initCornerVertex(patch,3);

      e0_p() = initPositiveEdgeVertex(patch,0, p0());
      e1_p() = initPositiveEdgeVertex(patch,1, p1());
      e2_p() = initPositiveEdgeVertex(patch,2, p2());
      e3_p() = initPositiveEdgeVertex(patch,3, p3());

      e0_m() = initNegativeEdgeVertex(patch,0, p0());
      e1_m() = initNegativeEdgeVertex(patch,1, p1());
      e2_m() = initNegativeEdgeVertex(patch,2, p2());
      e3_m() = initNegativeEdgeVertex(patch,3, p3());

      const unsigned int face_valence_p0 = patch.ring[0].face_valence;
      const unsigned int face_valence_p1 = patch.ring[1].face_valence;
      const unsigned int face_valence_p2 = patch.ring[2].face_valence;
      const unsigned int face_valence_p3 = patch.ring[3].face_valence;
      
      initFaceVertex(patch,0,p0(),e0_p(),e1_m(),face_valence_p1,e0_m(),e3_p(),face_valence_p3,f0_p(),f0_m() );
      initFaceVertex(patch,1,p1(),e1_p(),e2_m(),face_valence_p2,e1_m(),e0_p(),face_valence_p0,f1_p(),f1_m() );
      initFaceVertex(patch,2,p2(),e2_p(),e3_m(),face_valence_p3,e2_m(),e1_p(),face_valence_p1,f2_p(),f2_m() );
      initFaceVertex(patch,3,p3(),e3_p(),e0_m(),face_valence_p0,e3_m(),e2_p(),face_valence_p3,f3_p(),f3_m() );

    }
    
    void computeGregoryPatchFacePoints(const unsigned int face_valence,
				       const Vertex& r_e_p, 
				       const Vertex& r_e_m, 					 
				       const Vertex& p_vtx, 
				       const Vertex& e0_p_vtx, 
				       const Vertex& e1_m_vtx, 
				       const unsigned int face_valence_p1,
				       const Vertex& e0_m_vtx,	
				       const Vertex& e3_p_vtx,	
				       const unsigned int face_valence_p3,
				       Vertex& f_p_vtx, 
				       Vertex& f_m_vtx,
                                       const float d = 3.0f)
    {
      const float c     = cosf(2.0*M_PI/(float)face_valence);
      const float c_e_p = cosf(2.0*M_PI/(float)face_valence_p1);
      const float c_e_m = cosf(2.0*M_PI/(float)face_valence_p3);
      
      f_p_vtx = 1.0f / d * (c_e_p * p_vtx + (d - 2.0f*c - c_e_p) * e0_p_vtx + 2.0f*c* e1_m_vtx + r_e_p);      
      f_m_vtx = 1.0f / d * (c_e_m * p_vtx + (d - 2.0f*c - c_e_m) * e0_m_vtx + 2.0f*c* e3_p_vtx + r_e_m);      
      f_p_vtx = 1.0f / d * (c_e_p * p_vtx + (d - 2.0f*c - c_e_p) * e0_p_vtx + 2.0f*c* e1_m_vtx + r_e_p);      
      f_m_vtx = 1.0f / d * (c_e_m * p_vtx + (d - 2.0f*c - c_e_m) * e0_m_vtx + 2.0f*c* e3_p_vtx + r_e_m);
    }

    __noinline void init(const GeneralCatmullClarkPatch& patch)
    {
      assert(patch.size() == 4);
#if 0
      CatmullClarkPatch qpatch; patch.init(qpatch);
      init(qpatch);
#else
      const float face_valence_p0 = patch.ring[0].face_valence;
      const float face_valence_p1 = patch.ring[1].face_valence;
      const float face_valence_p2 = patch.ring[2].face_valence;
      const float face_valence_p3 = patch.ring[3].face_valence;

      Vertex p0_r_p, p0_r_m;
      patch.ring[0].computeGregoryPatchEdgePoints( p0(), e0_p(), e0_m(), p0_r_p, p0_r_m );

      Vertex p1_r_p, p1_r_m;
      patch.ring[1].computeGregoryPatchEdgePoints( p1(), e1_p(), e1_m(), p1_r_p, p1_r_m );
      
      Vertex p2_r_p, p2_r_m;
      patch.ring[2].computeGregoryPatchEdgePoints( p2(), e2_p(), e2_m(), p2_r_p, p2_r_m );

      Vertex p3_r_p, p3_r_m;
      patch.ring[3].computeGregoryPatchEdgePoints( p3(), e3_p(), e3_m(), p3_r_p, p3_r_m );

      computeGregoryPatchFacePoints(face_valence_p0, p0_r_p, p0_r_m, p0(), e0_p(), e1_m(), face_valence_p1, e0_m(), e3_p(), face_valence_p3, f0_p(), f0_m() );
      computeGregoryPatchFacePoints(face_valence_p1, p1_r_p, p1_r_m, p1(), e1_p(), e2_m(), face_valence_p2, e1_m(), e0_p(), face_valence_p0, f1_p(), f1_m() );
      computeGregoryPatchFacePoints(face_valence_p2, p2_r_p, p2_r_m, p2(), e2_p(), e3_m(), face_valence_p3, e2_m(), e1_p(), face_valence_p1, f2_p(), f2_m() );
      computeGregoryPatchFacePoints(face_valence_p3, p3_r_p, p3_r_m, p3(), e3_p(), e0_m(), face_valence_p0, e3_m(), e2_p(), face_valence_p3, f3_p(), f3_m() );

#endif
    }
   
    
    __noinline void init_bezier(const CatmullClarkPatch& patch) // FIXME: this should go to bezier class, initialization is not correct
    {
      assert( patch.isRegular() );
      init( patch );
      f0_p() = (f0_p() + f0_m()) * 0.5f;
      f1_p() = (f1_p() + f1_m()) * 0.5f;
      f2_p() = (f2_p() + f2_m()) * 0.5f;
      f3_p() = (f3_p() + f3_m()) * 0.5f;
      f0_m() = Vertex( zero );
      f1_m() = Vertex( zero );
      f2_m() = Vertex( zero );
      f3_m() = Vertex( zero );      
    }
    
    __forceinline void exportControlPoints( Vertex matrix[4][4], Vertex f_m[2][2] ) const
    {
      for (size_t y=0; y<4; y++)
	for (size_t x=0; x<4; x++)
	  matrix[y][x] = (Vertex_t)v[y][x];
      
      for (size_t y=0; y<2; y++)
	for (size_t x=0; x<2; x++)
	  f_m[y][x] = (Vertex_t)f[y][x];
    }
    
    __forceinline void exportDenseConrolPoints( Vertex matrix[4][4] ) const 
    {
      for (size_t y=0; y<4; y++)
	for (size_t x=0; x<4; x++)
	  matrix[y][x] = (Vertex_t)v[y][x];
      
      matrix[0][0].w = f[0][0].x;
      matrix[0][1].w = f[0][0].y;
      matrix[0][2].w = f[0][0].z;
      matrix[0][3].w = 0.0f;
      
      matrix[1][0].w = f[0][1].x;
      matrix[1][1].w = f[0][1].y;
      matrix[1][2].w = f[0][1].z;
      matrix[1][3].w = 0.0f;
      
      matrix[2][0].w = f[1][1].x;
      matrix[2][1].w = f[1][1].y;
      matrix[2][2].w = f[1][1].z;
      matrix[2][3].w = 0.0f;
      
      matrix[3][0].w = f[1][0].x;
      matrix[3][1].w = f[1][0].y;
      matrix[3][2].w = f[1][0].z;
      matrix[3][3].w = 0.0f;
    }
   
    
    static __forceinline void computeInnerVertices(const Vertex matrix[4][4], const Vertex f_m[2][2], const float uu, const float vv,
						   Vertex_t& matrix_11, Vertex_t& matrix_12, Vertex_t& matrix_22, Vertex_t& matrix_21)
    {
      if (unlikely(uu == 0.0f || uu == 1.0f || vv == 0.0f || vv == 1.0f)) 
      {
	matrix_11 = matrix[1][1];
	matrix_12 = matrix[1][2];
	matrix_22 = matrix[2][2];
	matrix_21 = matrix[2][1];	 
      }
      else
      {
	const Vertex_t f0_p = matrix[1][1];
	const Vertex_t f1_p = matrix[1][2];
	const Vertex_t f2_p = matrix[2][2];
	const Vertex_t f3_p = matrix[2][1];
        
	const Vertex_t f0_m = f_m[0][0];
	const Vertex_t f1_m = f_m[0][1];
	const Vertex_t f2_m = f_m[1][1];
	const Vertex_t f3_m = f_m[1][0];
        
	matrix_11 = (      uu  * f0_p +       vv  * f0_m)*rcp(uu+vv);
	matrix_12 = ((1.0f-uu) * f1_m +       vv  * f1_p)*rcp(1.0f-uu+vv);
	matrix_22 = ((1.0f-uu) * f2_p + (1.0f-vv) * f2_m)*rcp(2.0f-uu-vv);
	matrix_21 = (      uu  * f3_m + (1.0f-vv) * f3_p)*rcp(1.0f+uu-vv);
      }
    } 

    static __forceinline Vertex eval(const Vertex matrix[4][4], const Vertex f[2][2], const float& uu, const float& vv) 
    {
      Vertex_t v_11, v_12, v_22, v_21;
      computeInnerVertices(matrix,f,uu,vv,v_11, v_12, v_22, v_21);
      
      const float one_minus_uu = 1.0f - uu;
      const float one_minus_vv = 1.0f - vv;      
      
      const float B0_u = one_minus_uu * one_minus_uu * one_minus_uu;
      const float B0_v = one_minus_vv * one_minus_vv * one_minus_vv;
      const float B1_u = 3.0f * (one_minus_uu * uu * one_minus_uu);
      const float B1_v = 3.0f * (one_minus_vv * vv * one_minus_vv);
      const float B2_u = 3.0f * (uu * one_minus_uu * uu);
      const float B2_v = 3.0f * (vv * one_minus_vv * vv);
      const float B3_u = uu * uu * uu;
      const float B3_v = vv * vv * vv;
      
      const Vertex_t res = 
	(B0_u * matrix[0][0] + B1_u * matrix[0][1] + B2_u * matrix[0][2] + B3_u * matrix[0][3]) * B0_v + 
	(B0_u * matrix[1][0] + B1_u * v_11    + B2_u * v_12    + B3_u * matrix[1][3]) * B1_v + 
	(B0_u * matrix[2][0] + B1_u * v_21    + B2_u * v_22    + B3_u * matrix[2][3]) * B2_v + 
	(B0_u * matrix[3][0] + B1_u * matrix[3][1] + B2_u * matrix[3][2] + B3_u * matrix[3][3]) * B3_v; 
      
      return res;
    }

    __forceinline Vertex eval(const float uu, const float vv) const {
      return eval(v,f,uu,vv);
    }

    static __forceinline Vertex tangentU(const Vertex matrix[4][4], const Vertex f_m[2][2], const float uu, const float vv)  // FIXME: why not using basis functions
    {
      /* interpolate inner vertices */
      Vertex_t matrix_11, matrix_12, matrix_22, matrix_21;
      computeInnerVertices(matrix,f_m,uu,vv,matrix_11, matrix_12, matrix_22, matrix_21);
      
      /* tangentU */
      const Vertex_t col0 = deCasteljau(vv, (Vertex_t)matrix[0][0], (Vertex_t)matrix[1][0], (Vertex_t)matrix[2][0], (Vertex_t)matrix[3][0]);
      const Vertex_t col1 = deCasteljau(vv, (Vertex_t)matrix[0][1], (Vertex_t)matrix_11   , (Vertex_t)matrix_21   , (Vertex_t)matrix[3][1]);
      const Vertex_t col2 = deCasteljau(vv, (Vertex_t)matrix[0][2], (Vertex_t)matrix_12   , (Vertex_t)matrix_22   , (Vertex_t)matrix[3][2]);
      const Vertex_t col3 = deCasteljau(vv, (Vertex_t)matrix[0][3], (Vertex_t)matrix[1][3], (Vertex_t)matrix[2][3], (Vertex_t)matrix[3][3]);
      
      return deCasteljau_tangent(uu, col0, col1, col2, col3);
    }

    static __forceinline Vertex tangentV(const Vertex matrix[4][4], const Vertex f_m[2][2], const float uu, const float vv) // FIXME: why not using basis functions
    {
      /* interpolate inner vertices */
      Vertex_t matrix_11, matrix_12, matrix_22, matrix_21;
      computeInnerVertices(matrix,f_m,uu,vv,matrix_11, matrix_12, matrix_22, matrix_21);
      
      /* tangentV */
      const Vertex_t row0 = deCasteljau(uu, (Vertex_t)matrix[0][0], (Vertex_t)matrix[0][1], (Vertex_t)matrix[0][2], (Vertex_t)matrix[0][3]);
      const Vertex_t row1 = deCasteljau(uu, (Vertex_t)matrix[1][0], (Vertex_t)matrix_11   , (Vertex_t)matrix_12   , (Vertex_t)matrix[1][3]);
      const Vertex_t row2 = deCasteljau(uu, (Vertex_t)matrix[2][0], (Vertex_t)matrix_21   , (Vertex_t)matrix_22   , (Vertex_t)matrix[2][3]);
      const Vertex_t row3 = deCasteljau(uu, (Vertex_t)matrix[3][0], (Vertex_t)matrix[3][1], (Vertex_t)matrix[3][2], (Vertex_t)matrix[3][3]);
      
      return deCasteljau_tangent(vv, row0, row1, row2, row3);
    }

    static __forceinline Vertex normal(const Vertex matrix[4][4], const Vertex f_m[2][2], const float uu, const float vv)  // FIXME: why not using basis functions
    {
      /* interpolate inner vertices */
      Vertex_t matrix_11, matrix_12, matrix_22, matrix_21;
      computeInnerVertices(matrix,f_m,uu,vv,matrix_11, matrix_12, matrix_22, matrix_21);
      
      /* tangentU */
      const Vertex_t col0 = deCasteljau(vv, (Vertex_t)matrix[0][0], (Vertex_t)matrix[1][0], (Vertex_t)matrix[2][0], (Vertex_t)matrix[3][0]);
      const Vertex_t col1 = deCasteljau(vv, (Vertex_t)matrix[0][1], (Vertex_t)matrix_11   , (Vertex_t)matrix_21   , (Vertex_t)matrix[3][1]);
      const Vertex_t col2 = deCasteljau(vv, (Vertex_t)matrix[0][2], (Vertex_t)matrix_12   , (Vertex_t)matrix_22   , (Vertex_t)matrix[3][2]);
      const Vertex_t col3 = deCasteljau(vv, (Vertex_t)matrix[0][3], (Vertex_t)matrix[1][3], (Vertex_t)matrix[2][3], (Vertex_t)matrix[3][3]);
      
      const Vertex_t tangentU = deCasteljau_tangent(uu, col0, col1, col2, col3);
      
      /* tangentV */
      const Vertex_t row0 = deCasteljau(uu, (Vertex_t)matrix[0][0], (Vertex_t)matrix[0][1], (Vertex_t)matrix[0][2], (Vertex_t)matrix[0][3]);
      const Vertex_t row1 = deCasteljau(uu, (Vertex_t)matrix[1][0], (Vertex_t)matrix_11   , (Vertex_t)matrix_12   , (Vertex_t)matrix[1][3]);
      const Vertex_t row2 = deCasteljau(uu, (Vertex_t)matrix[2][0], (Vertex_t)matrix_21   , (Vertex_t)matrix_22   , (Vertex_t)matrix[2][3]);
      const Vertex_t row3 = deCasteljau(uu, (Vertex_t)matrix[3][0], (Vertex_t)matrix[3][1], (Vertex_t)matrix[3][2], (Vertex_t)matrix[3][3]);
      
      const Vertex_t tangentV = deCasteljau_tangent(vv, row0, row1, row2, row3);
      
      /* normal = tangentU x tangentV */
      const Vertex_t n = cross(tangentV,tangentU);
      
      return n;     
    }
   
    __forceinline Vertex normal( const float uu, const float vv) const {
      return normal(v,f,uu,vv);
    }

    __forceinline Vertex tangentU( const float uu, const float vv) const {
      return tangentU(v,f,uu,vv);
    }

    __forceinline Vertex tangentV( const float uu, const float vv) const {
      return tangentV(v,f,uu,vv);
    }

    template<class M, class T>
      static __forceinline Vec3<T> eval_t(const Vertex matrix[4][4], const Vec3<T> f[2][2], const T& uu, const T& vv) 
    {
      const M m_border = (uu == 0.0f) | (uu == 1.0f) | (vv == 0.0f) | (vv == 1.0f);
      
      const Vec3<T> f0_p = Vec3<T>(matrix[1][1].x,matrix[1][1].y,matrix[1][1].z);
      const Vec3<T> f1_p = Vec3<T>(matrix[1][2].x,matrix[1][2].y,matrix[1][2].z);
      const Vec3<T> f2_p = Vec3<T>(matrix[2][2].x,matrix[2][2].y,matrix[2][2].z);
      const Vec3<T> f3_p = Vec3<T>(matrix[2][1].x,matrix[2][1].y,matrix[2][1].z);
      
      const Vec3<T> f0_m = f[0][0];
      const Vec3<T> f1_m = f[0][1];
      const Vec3<T> f2_m = f[1][1];
      const Vec3<T> f3_m = f[1][0];
      
      const T one_minus_uu = T(1.0f) - uu;
      const T one_minus_vv = T(1.0f) - vv;      
      
      const Vec3<T> f0_i = (          uu * f0_p +           vv * f0_m) * rcp(uu+vv);
      const Vec3<T> f1_i = (one_minus_uu * f1_m +           vv * f1_p) * rcp(one_minus_uu+vv);
      const Vec3<T> f2_i = (one_minus_uu * f2_p + one_minus_vv * f2_m) * rcp(one_minus_uu+one_minus_vv);
      const Vec3<T> f3_i = (          uu * f3_m + one_minus_vv * f3_p) * rcp(uu+one_minus_vv);
      
      const Vec3<T> F0( select(m_border,f0_p.x,f0_i.x), select(m_border,f0_p.y,f0_i.y), select(m_border,f0_p.z,f0_i.z) );
      const Vec3<T> F1( select(m_border,f1_p.x,f1_i.x), select(m_border,f1_p.y,f1_i.y), select(m_border,f1_p.z,f1_i.z) );
      const Vec3<T> F2( select(m_border,f2_p.x,f2_i.x), select(m_border,f2_p.y,f2_i.y), select(m_border,f2_p.z,f2_i.z) );
      const Vec3<T> F3( select(m_border,f3_p.x,f3_i.x), select(m_border,f3_p.y,f3_i.y), select(m_border,f3_p.z,f3_i.z) );
      
      const T B0_u = one_minus_uu * one_minus_uu * one_minus_uu;
      const T B0_v = one_minus_vv * one_minus_vv * one_minus_vv;
      const T B1_u = 3.0f * (one_minus_uu * uu * one_minus_uu);
      const T B1_v = 3.0f * (one_minus_vv * vv * one_minus_vv);
      const T B2_u = 3.0f * (uu * one_minus_uu * uu);
      const T B2_v = 3.0f * (vv * one_minus_vv * vv);
      const T B3_u = uu * uu * uu;
      const T B3_v = vv * vv * vv;
      
      const T x = 
	(B0_u * matrix[0][0].x + B1_u * matrix[0][1].x + B2_u * matrix[0][2].x + B3_u * matrix[0][3].x) * B0_v + 
	(B0_u * matrix[1][0].x + B1_u * F0.x           + B2_u * F1.x           + B3_u * matrix[1][3].x) * B1_v + 
	(B0_u * matrix[2][0].x + B1_u * F3.x           + B2_u * F2.x           + B3_u * matrix[2][3].x) * B2_v + 
	(B0_u * matrix[3][0].x + B1_u * matrix[3][1].x + B2_u * matrix[3][2].x + B3_u * matrix[3][3].x) * B3_v; 
      
      const T y = 
	(B0_u * matrix[0][0].y + B1_u * matrix[0][1].y + B2_u * matrix[0][2].y + B3_u * matrix[0][3].y) * B0_v + 
	(B0_u * matrix[1][0].y + B1_u * F0.y           + B2_u * F1.y           + B3_u * matrix[1][3].y) * B1_v + 
	(B0_u * matrix[2][0].y + B1_u * F3.y           + B2_u * F2.y           + B3_u * matrix[2][3].y) * B2_v + 
	(B0_u * matrix[3][0].y + B1_u * matrix[3][1].y + B2_u * matrix[3][2].y + B3_u * matrix[3][3].y) * B3_v; 
      
      const T z = 
	(B0_u * matrix[0][0].z + B1_u * matrix[0][1].z + B2_u * matrix[0][2].z + B3_u * matrix[0][3].z) * B0_v + 
	(B0_u * matrix[1][0].z + B1_u * F0.z           + B2_u * F1.z           + B3_u * matrix[1][3].z) * B1_v + 
	(B0_u * matrix[2][0].z + B1_u * F3.z           + B2_u * F2.z           + B3_u * matrix[2][3].z) * B2_v + 
	(B0_u * matrix[3][0].z + B1_u * matrix[3][1].z + B2_u * matrix[3][2].z + B3_u * matrix[3][3].z) * B3_v; 
      
      
      return Vec3<T>(x,y,z);
    }
    
    template<class M, class T>
      static __forceinline Vec3<T> normal_t(const Vertex matrix[4][4], const Vec3<T> f[2][2], const T& uu, const T& vv) 
    {
      const M m_border = (uu == 0.0f) | (uu == 1.0f) | (vv == 0.0f) | (vv == 1.0f);
      
      const Vec3<T> f0_p = Vec3<T>(matrix[1][1].x,matrix[1][1].y,matrix[1][1].z);
      const Vec3<T> f1_p = Vec3<T>(matrix[1][2].x,matrix[1][2].y,matrix[1][2].z);
      const Vec3<T> f2_p = Vec3<T>(matrix[2][2].x,matrix[2][2].y,matrix[2][2].z);
      const Vec3<T> f3_p = Vec3<T>(matrix[2][1].x,matrix[2][1].y,matrix[2][1].z);

      const Vec3<T> f0_m = f[0][0];
      const Vec3<T> f1_m = f[0][1];
      const Vec3<T> f2_m = f[1][1];
      const Vec3<T> f3_m = f[1][0];
      
      const T one_minus_uu = T(1.0f) - uu;
      const T one_minus_vv = T(1.0f) - vv;      
      
      const Vec3<T> f0_i = (          uu * f0_p +           vv * f0_m) * rcp(uu+vv);
      const Vec3<T> f1_i = (one_minus_uu * f1_m +           vv * f1_p) * rcp(one_minus_uu+vv);
      const Vec3<T> f2_i = (one_minus_uu * f2_p + one_minus_vv * f2_m) * rcp(one_minus_uu+one_minus_vv);
      const Vec3<T> f3_i = (          uu * f3_m + one_minus_vv * f3_p) * rcp(uu+one_minus_vv);

#if 1
      const M m_corner0 = (uu == 0.0f) & (vv == 0.0f);
      const M m_corner1 = (uu == 1.0f) & (vv == 0.0f);
      const M m_corner2 = (uu == 1.0f) & (vv == 1.0f);
      const M m_corner3 = (uu == 0.0f) & (vv == 1.0f);
      
      const Vec3<T> matrix_11( select(m_corner0,f0_p.x,f0_i.x), select(m_corner0,f0_p.y,f0_i.y), select(m_corner0,f0_p.z,f0_i.z) );
      const Vec3<T> matrix_12( select(m_corner1,f1_p.x,f1_i.x), select(m_corner1,f1_p.y,f1_i.y), select(m_corner1,f1_p.z,f1_i.z) );
      const Vec3<T> matrix_22( select(m_corner2,f2_p.x,f2_i.x), select(m_corner2,f2_p.y,f2_i.y), select(m_corner2,f2_p.z,f2_i.z) );
      const Vec3<T> matrix_21( select(m_corner3,f3_p.x,f3_i.x), select(m_corner3,f3_p.y,f3_i.y), select(m_corner3,f3_p.z,f3_i.z) );
#else
      const Vec3<T> matrix_11( select(m_border,f0_p.x,f0_i.x), select(m_border,f0_p.y,f0_i.y), select(m_border,f0_p.z,f0_i.z) );
      const Vec3<T> matrix_12( select(m_border,f1_p.x,f1_i.x), select(m_border,f1_p.y,f1_i.y), select(m_border,f1_p.z,f1_i.z) );
      const Vec3<T> matrix_22( select(m_border,f2_p.x,f2_i.x), select(m_border,f2_p.y,f2_i.y), select(m_border,f2_p.z,f2_i.z) );
      const Vec3<T> matrix_21( select(m_border,f3_p.x,f3_i.x), select(m_border,f3_p.y,f3_i.y), select(m_border,f3_p.z,f3_i.z) );
#endif
      
      const Vec3<T> matrix_00 = Vec3<T>(matrix[0][0].x,matrix[0][0].y,matrix[0][0].z);
      const Vec3<T> matrix_10 = Vec3<T>(matrix[1][0].x,matrix[1][0].y,matrix[1][0].z);
      const Vec3<T> matrix_20 = Vec3<T>(matrix[2][0].x,matrix[2][0].y,matrix[2][0].z);
      const Vec3<T> matrix_30 = Vec3<T>(matrix[3][0].x,matrix[3][0].y,matrix[3][0].z);
      
      const Vec3<T> matrix_01 = Vec3<T>(matrix[0][1].x,matrix[0][1].y,matrix[0][1].z);
      const Vec3<T> matrix_02 = Vec3<T>(matrix[0][2].x,matrix[0][2].y,matrix[0][2].z);
      const Vec3<T> matrix_03 = Vec3<T>(matrix[0][3].x,matrix[0][3].y,matrix[0][3].z);
      
      const Vec3<T> matrix_31 = Vec3<T>(matrix[3][1].x,matrix[3][1].y,matrix[3][1].z);
      const Vec3<T> matrix_32 = Vec3<T>(matrix[3][2].x,matrix[3][2].y,matrix[3][2].z);
      const Vec3<T> matrix_33 = Vec3<T>(matrix[3][3].x,matrix[3][3].y,matrix[3][3].z);
      
      const Vec3<T> matrix_13 = Vec3<T>(matrix[1][3].x,matrix[1][3].y,matrix[1][3].z);
      const Vec3<T> matrix_23 = Vec3<T>(matrix[2][3].x,matrix[2][3].y,matrix[2][3].z);
      
      /* tangentU */
      const Vec3<T> col0 = deCasteljau(vv, matrix_00, matrix_10, matrix_20, matrix_30);
      const Vec3<T> col1 = deCasteljau(vv, matrix_01, matrix_11, matrix_21, matrix_31);
      const Vec3<T> col2 = deCasteljau(vv, matrix_02, matrix_12, matrix_22, matrix_32);
      const Vec3<T> col3 = deCasteljau(vv, matrix_03, matrix_13, matrix_23, matrix_33);
      
      const Vec3<T> tangentU = deCasteljau_tangent(uu, col0, col1, col2, col3);
      
      /* tangentV */
      const Vec3<T> row0 = deCasteljau(uu, matrix_00, matrix_01, matrix_02, matrix_03);
      const Vec3<T> row1 = deCasteljau(uu, matrix_10, matrix_11, matrix_12, matrix_13);
      const Vec3<T> row2 = deCasteljau(uu, matrix_20, matrix_21, matrix_22, matrix_23);
      const Vec3<T> row3 = deCasteljau(uu, matrix_30, matrix_31, matrix_32, matrix_33);
      
      const Vec3<T> tangentV = deCasteljau_tangent(vv, row0, row1, row2, row3);
      
      /* normal = tangentU x tangentV */
      const Vec3<T> n = cross(tangentV,tangentU);
      return n;
    }
    
    __forceinline BBox<Vertex> bounds() const
    {
      const Vertex *const cv = &v[0][0];
      BBox<Vertex> bounds (cv[0]);
      for (size_t i=1; i<16; i++) 
        bounds.extend( cv[i] );
      bounds.extend(f[0][0]);
      bounds.extend(f[1][0]);
      bounds.extend(f[1][1]);
      bounds.extend(f[1][1]);
      return bounds;
    }
    
    friend std::ostream& operator<<(std::ostream& o, const GregoryPatchT& p)
    {
      for (size_t y=0; y<4; y++)
	for (size_t x=0; x<4; x++)
	  o << "v[" << y << "][" << x << "] " << p.v[y][x] << std::endl;
      
      for (size_t y=0; y<2; y++)
	for (size_t x=0; x<2; x++)
	  o << "f[" << y << "][" << x << "] " << p.f[y][x] << std::endl;
      return o;
    } 
  };

  typedef GregoryPatchT<Vec3fa,Vec3fa_t> GregoryPatch3fa;
}
