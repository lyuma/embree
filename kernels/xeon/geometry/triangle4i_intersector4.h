// ======================================================================== //
// Copyright 2009-2013 Intel Corporation                                    //
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

#ifndef __EMBREE_TRIANGLE4I_INTERSECTOR4_PLUECKER_H__
#define __EMBREE_TRIANGLE4I_INTERSECTOR4_PLUECKER_H__

#include "triangle4i.h"
#include "common/ray4.h"

namespace embree
{
  /*! Intersector4 for triangle4i */
  struct Triangle4iIntersector4Pluecker
  {
    typedef Triangle4i Primitive;

    static __forceinline void intersect(const sseb& valid_i, Ray4& ray, const Triangle4i& tri, const void* geom)
    {
      for (size_t i=0; i<tri.size(); i++)
      {
        STAT3(normal.trav_prims,1,popcnt(valid_i),4);

        /* load vertices */
        const Vec3fa* base = tri.v0[i];
        const Vec3fa& p0 = base[0];
        const Vec3fa& p1 = base[tri.v1[i]];
        const Vec3fa& p2 = base[tri.v2[i]];

        /* calculate vertices relative to ray origin */
        sseb valid = valid_i;
        const sse3f O = ray.org;
        const sse3f D = ray.dir;
        const sse3f v0 = sse3f(p0)-O;
        const sse3f v1 = sse3f(p1)-O;
        const sse3f v2 = sse3f(p2)-O;
        
        /* calculate triangle edges */
        const sse3f e0 = v2-v0;
        const sse3f e1 = v0-v1;
        const sse3f e2 = v1-v2;
        
        /* calculate geometry normal and denominator */
        const sse3f Ng = cross(e1,e0);
        const sse3f Ng2 = Ng+Ng;
        const ssef den = dot(sse3f(Ng2),D);
        const ssef absDen = abs(den);
        const ssef sgnDen = signmsk(den);
        
        /* perform edge tests */
        const ssef U = dot(sse3f(cross(v2+v0,e0)),D) ^ sgnDen;
        valid &= U >= 0.0f;
        if (likely(none(valid))) continue;
        const ssef V = dot(sse3f(cross(v0+v1,e1)),D) ^ sgnDen;
        valid &= V >= 0.0f;
        if (likely(none(valid))) continue;
        const ssef W = dot(sse3f(cross(v1+v2,e2)),D) ^ sgnDen;
        valid &= W >= 0.0f;
        if (likely(none(valid))) continue;
        
        /* perform depth test */
        const ssef T = dot(v0,sse3f(Ng2)) ^ sgnDen;
        valid &= (T >= absDen*ray.tnear) & (absDen*ray.tfar >= T);
        if (unlikely(none(valid))) continue;

        /* perform backface culling */
#if defined(__BACKFACE_CULLING__)
        valid &= den > ssef(zero);
        if (unlikely(none(valid))) continue;
#else
        valid &= den != ssef(zero);
        if (unlikely(none(valid))) continue;
#endif

        // FIXME: add ray mask test
        
        /* update hit information for all rays that hit the triangle */
        ray.u   = select(valid,U / absDen,ray.u );
        ray.v   = select(valid,V / absDen,ray.v );
        ray.tfar = select(valid,T / absDen,ray.tfar );
        ray.geomID = select(valid,tri.geomID[i],ray.geomID);
        ray.primID = select(valid,tri.primID[i],ray.primID);
        ray.Ng.x = select(valid,Ng2.x,ray.Ng.x);
        ray.Ng.y = select(valid,Ng2.y,ray.Ng.y);
        ray.Ng.z = select(valid,Ng2.z,ray.Ng.z);
      }
    }

    static __forceinline void intersect(const sseb& valid, Ray4& ray, const Triangle4i* tri, size_t num, const void* geom)
    {
      for (size_t i=0; i<num; i++)
        intersect(valid,ray,tri[i],geom);
    }
    
    static __forceinline sseb occluded(const sseb& valid_i, const Ray4& ray, const Triangle4i& tri, const void* geom)
    {
      sseb valid0 = valid_i;

      for (size_t i=0; i<tri.size(); i++)
      {
        STAT3(shadow.trav_prims,1,popcnt(valid_i),4);

        /* load vertices */
        const Vec3fa* base = tri.v0[i];
        const Vec3fa& p0 = base[0];
        const Vec3fa& p1 = base[tri.v1[i]];
        const Vec3fa& p2 = base[tri.v2[i]];

        /* calculate vertices relative to ray origin */
        sseb valid = valid0;
        const sse3f O = ray.org;
        const sse3f D = ray.dir;
        const sse3f v0 = sse3f(p0)-O;
        const sse3f v1 = sse3f(p1)-O;
        const sse3f v2 = sse3f(p2)-O;

        /* calculate triangle edges */
        const sse3f e0 = v2-v0;
        const sse3f e1 = v0-v1;
        const sse3f e2 = v1-v2;
        
        /* calculate geometry normal and denominator */
        const sse3f Ng = cross(e1,e0);
        const sse3f Ng2 = Ng+Ng;
        const ssef den = dot(sse3f(Ng2),D);
        const ssef absDen = abs(den);
        const ssef sgnDen = signmsk(den);
        
        /* perform edge tests */
        const ssef U = dot(sse3f(cross(v2+v0,e0)),D) ^ sgnDen;
        valid &= U >= 0.0f;
        if (likely(none(valid))) continue;
        const ssef V = dot(sse3f(cross(v0+v1,e1)),D) ^ sgnDen;
        valid &= V >= 0.0f;
        if (likely(none(valid))) continue;
        const ssef W = dot(sse3f(cross(v1+v2,e2)),D) ^ sgnDen;
        valid &= W >= 0.0f;
        if (likely(none(valid))) continue;
        
        /* perform depth test */
        const ssef T = dot(v0,sse3f(Ng2)) ^ sgnDen;
        valid &= (T >= absDen*ray.tnear) & (absDen*ray.tfar >= T);

        /* perform backface culling */
#if defined(__BACKFACE_CULLING__)
        valid &= den > ssef(zero);
        if (unlikely(none(valid))) continue;
#else
        valid &= den != ssef(zero);
        if (unlikely(none(valid))) continue;
#endif

        // FIXME: add ray mask test

        /* update occlusion */
        valid0 &= !valid;
        if (none(valid0)) break;
      }
      return !valid0;
    }

    static __forceinline sseb occluded(const sseb& valid, const Ray4& ray, const Triangle4i* tri, size_t num, const void* geom)
    {
      sseb valid0 = valid;
      for (size_t i=0; i<num; i++) {
        valid0 &= !occluded(valid0,ray,tri[i],geom);
        if (none(valid0)) break;
      }
      return !valid0;
    }
  };
}

#endif


