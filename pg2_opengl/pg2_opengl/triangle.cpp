#include "pch.h"
#include "triangle.h"


Vector3 Tangent(const Vector3& t, const Vector3& b, const Vector3& n)
{
	return (t - t.DotProduct(n) * n).Normalize();
}

Triangle::Triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2, Surface* surface)
{
	vertices_[0] = v0;
	vertices_[1] = v1;
	vertices_[2] = v2;

	// ukazatel na surface schováme (!pokud se tam vleze!) do paddingu prvního vertexu
	//*reinterpret_cast<Surface **>(vertices_[0].pad) = surface;	

	const Vector3 e1 = v1.position - v0.position;
	const Vector3 e2 = v2.position - v0.position;

	const Vector3 du1 = v1.texture_coords->u - v0.texture_coords->u;
	const Vector3 du2 = v2.texture_coords->u - v0.texture_coords->u;
	const Vector3 dv1 = v1.texture_coords->v - v0.texture_coords->v;
	const Vector3 dv2 = v2.texture_coords->v - v0.texture_coords->v;

	Vector3 d = du1 * dv2 - du2 * dv1;
	d.x = 1 / d.x;
	d.y = 1 / d.y;
	d.z = 1 / d.z;
	const Vector3 t = d * (dv2 * e1 - dv1 * e2);
	const Vector3 b = d * (-du2 * e1 + du1 * e2);
	
	vertices_[0].tangent = Tangent(t, b, v0.normal);
	vertices_[1].tangent = Tangent(t, b, v1.normal);
	vertices_[2].tangent = Tangent(t, b, v2.normal);
}


Vertex Triangle::vertex( const int i )
{
	return vertices_[i];
}

Surface * Triangle::surface()
{	
	return *reinterpret_cast<Surface **>( vertices_[0].pad ); // FIX: chybí verze pro 64bit
}

void Triangle::setMaterialIndex(int matIdx) {
	vertices_[0].materialIdx = matIdx;
	vertices_[1].materialIdx = matIdx;
	vertices_[2].materialIdx = matIdx;
}