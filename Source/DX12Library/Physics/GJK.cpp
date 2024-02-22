#include "GJK.h"
#include "Support.h"

static void addToSimplex(GJKSimplex* simplex, XMVECTOR point)
{
	switch (simplex->num)
	{
	case 1:
		simplex->b = simplex->a;
		simplex->a = point;
		break;
	case 2:
		simplex->c = simplex->b;
		simplex->b = simplex->a;
		simplex->a = point;
		break;
	case 3:
		simplex->d = simplex->c;
		simplex->c = simplex->b;
		simplex->b = simplex->a;
		simplex->a = point;
		break;
	default:
		assert(false);
		break;
	}

	++simplex->num;
}

static bool doSimplex2(GJKSimplex* simplex, XMVECTOR* direction)
{
	XMVECTOR a = simplex->a;	// the last point added
	XMVECTOR b = simplex->b;

	XMVECTOR ao = -a;
	XMVECTOR ab = b - a;

	if (0.0f <= XMVectorGetX(XMVector3Dot(ab, ao)))
	{
		simplex->a = a;
		simplex->b = b;
		simplex->num = 2;
		*direction = XMVector3Cross(XMVector3Cross(ab, ao), ab);
	}
	else
	{
		simplex->a = a;
		simplex->num = 1;
		*direction = ao;
	}

	return false;
}

static bool doSimplex3(GJKSimplex* simplex, XMVECTOR* direction)
{
	XMVECTOR a = simplex->a;	// the last point added
	XMVECTOR b = simplex->b;
	XMVECTOR c = simplex->c;

	XMVECTOR ao = -a;
	XMVECTOR ab = b - a;
	XMVECTOR ac = c - a;
	XMVECTOR abc = XMVector3Cross(ab, ac);

	if (0.0f <= XMVectorGetX(XMVector3Dot(XMVector3Cross(abc, ac), ao)))
	{
		if (0.0f <= XMVectorGetX(XMVector3Dot(ac, ao)))
		{
			// AC region
			simplex->a = a;
			simplex->b = c;
			simplex->num = 2;
			*direction = XMVector3Cross(XMVector3Cross(ac, ao), ac);
		}
		else if (0.0f <= XMVectorGetX(XMVector3Dot(ab, ao)))
		{
			// AB region
			simplex->a = a;
			simplex->b = b;
			simplex->num = 2;
			*direction = XMVector3Cross(XMVector3Cross(ab, ao), ab);
		}
		else
		{
			// A region
			simplex->a = a;
			*direction = ao;
		}
	}
	else if	(0.0f <= XMVectorGetX(XMVector3Dot(XMVector3Cross(ab, abc), ao)))
	{
		if (0.0f <= XMVectorGetX(XMVector3Dot(ab, ao)))
		{
			// AB region
			simplex->a = a;
			simplex->b = b;
			simplex->num = 2;
			*direction = XMVector3Cross(XMVector3Cross(ab, ao), ab);
		}
		else
		{
			// A region
			simplex->a = a;
			*direction = ao;
		}
	}
	else
	{
		if (0.0f <= XMVectorGetX(XMVector3Dot(abc, ao)))
		{
			// ABC region ("up")
			simplex->a = a;
			simplex->b = b;
			simplex->c = c;
			simplex->num = 3;
			*direction = abc;
		}
		else
		{
			// ABC region ("down")
			simplex->a = a;
			simplex->b = c;
			simplex->c = b;
			simplex->num = 3;
			*direction = -abc;
		}
	}
	
	return false;
}

static bool doSimplex4(GJKSimplex* simplex, XMVECTOR* direction)
{
	XMVECTOR a = simplex->a;	// the last point added
	XMVECTOR b = simplex->b;
	XMVECTOR c = simplex->c;
	XMVECTOR d = simplex->d;

	XMVECTOR ao = -a;
	XMVECTOR ab = b - a;
	XMVECTOR ac = c - a;
	XMVECTOR ad = d - a;
	XMVECTOR abc = XMVector3Cross(ab, ac);
	XMVECTOR acd = XMVector3Cross(ac, ad);
	XMVECTOR adb = XMVector3Cross(ad, ab);

	unsigned char planeInformation = 0x0;

	if (0.0f <= XMVectorGetX(XMVector3Dot(abc, ao)))
	{
		planeInformation |= 0x1;
	}
	if (0.0f <= XMVectorGetX(XMVector3Dot(acd, ao)))
	{
		planeInformation |= 0x2;
	}
	if (0.0f <= XMVectorGetX(XMVector3Dot(adb, ao)))
	{
		planeInformation |= 0x4;
	}

	switch (planeInformation)
	{
	case 0x0:
		// Intersection
		return true;
		break;
	case 0x1:
		// Triangle ABC
		if (0.0f <= XMVectorGetX(XMVector3Dot(XMVector3Cross(abc, ac), ao)))
		{
			if (0.0f <= XMVectorGetX(XMVector3Dot(ac, ao)))
			{
				// AC region
				simplex->a = a;
				simplex->b = c;
				simplex->num = 2;
				*direction = XMVector3Cross(XMVector3Cross(ac, ao), ac);
			}
			else if (0.0f <= XMVectorGetX(XMVector3Dot(ab, ao)))
			{
				// AB region
				simplex->a = a;
				simplex->b = b;
				simplex->num = 2;
				*direction = XMVector3Cross(XMVector3Cross(ab, ao), ab);
			}
			else
			{
				// A region
				simplex->a = a;
				*direction = ao;
			}
		}
		else if (0.0f <= XMVectorGetX(XMVector3Dot(XMVector3Cross(ab, abc), ao)))
		{
			if (0.0f <= XMVectorGetX(XMVector3Dot(ab, ao)))
			{
				// AB region
				simplex->a = a;
				simplex->b = b;
				simplex->num = 2;
				*direction = XMVector3Cross(XMVector3Cross(ab, ao), ab);
			}
			else
			{
				// A region
				simplex->a = a;
				*direction = ao;
			}
		}
		else
		{
			// ABC region
			simplex->a = a;
			simplex->b = b;
			simplex->c = c;
			simplex->num = 3;
			*direction = abc;
		}
		break;
	case 0x2:
		// Triangle ACD
		if (0.0f <= XMVectorGetX(XMVector3Dot(XMVector3Cross(acd, ad), ao)))
		{
			if (0.0f <= XMVectorGetX(XMVector3Dot(ad, ao)))
			{
				// AD region
				simplex->a = a;
				simplex->b = d;
				simplex->num = 2;
				*direction = XMVector3Cross(XMVector3Cross(ad, ao), ad);
			}
			else if (0.0f <= XMVectorGetX(XMVector3Dot(ac, ao)))
			{
				// AC region
				simplex->a = a;
				simplex->b = c;
				simplex->num = 2;
				*direction = XMVector3Cross(XMVector3Cross(ab, ao), ab);
			}
			else
			{
				// A region
				simplex->a = a;
				*direction = ao;
			}
		}
		else if (0.0f <= XMVectorGetX(XMVector3Dot(XMVector3Cross(ac, acd), ao)))
		{
			if (0.0f <= XMVectorGetX(XMVector3Dot(ac, ao)))
			{
				// AC region
				simplex->a = a;
				simplex->b = c;
				simplex->num = 2;
				*direction = XMVector3Cross(XMVector3Cross(ac, ao), ac);
			}
			else
			{
				// A region
				simplex->a = a;
				*direction = ao;
			}
		}
		else
		{
			// ACD region
			simplex->a = a;
			simplex->b = c;
			simplex->c = d;
			simplex->num = 3;
			*direction = acd;
		}
		break;
	case 0x3:
		// Line AC
		if (0.0f <= XMVectorGetX(XMVector3Dot(ac, ao)))
		{
			simplex->a = a;
			simplex->b = c;
			simplex->num = 2;
			*direction = XMVector3Cross(XMVector3Cross(ac, ao), ac);
		}
		else
		{
			simplex->a = a;
			simplex->num = 1;
			*direction = ao;
		}
		break;
	case 0x4:
		// Triangle ADB
		if (0.0f <= XMVectorGetX(XMVector3Dot(XMVector3Cross(adb, ab), ao)))
		{
			if (0.0f <= XMVectorGetX(XMVector3Dot(ab, ao)))
			{
				// AB region
				simplex->a = a;
				simplex->b = b;
				simplex->num = 2;
				*direction = XMVector3Cross(XMVector3Cross(ab, ao), ab);
			}
			else if (0.0f <= XMVectorGetX(XMVector3Dot(ad, ao)))
			{
				// AD region
				simplex->a = a;
				simplex->b = d;
				simplex->num = 2;
				*direction = XMVector3Cross(XMVector3Cross(ad, ao), ad);
			}
			else
			{
				// A region
				simplex->a = a;
				*direction = ao;
			}
		}
		else if (0.0f <= XMVectorGetX(XMVector3Dot(XMVector3Cross(ad, adb), ao)))
		{
			if (0.0f <= XMVectorGetX(XMVector3Dot(ad, ao)))
			{
				// AD region
				simplex->a = a;
				simplex->b = d;
				simplex->num = 2;
				*direction = XMVector3Cross(XMVector3Cross(ab, ao), ab);
			}
			else
			{
				// A region
				simplex->a = a;
				*direction = ao;
			}
		}
		else
		{
			// ADB region
			simplex->a = a;
			simplex->b = d;
			simplex->c = b;
			simplex->num = 3;
			*direction = adb;
		}
		break;
	case 0x5:
		// Line AB
		if (0.0f <= XMVectorGetX(XMVector3Dot(ab, ao)))
		{
			simplex->a = a;
			simplex->b = b;
			simplex->num = 2;
			*direction = XMVector3Cross(XMVector3Cross(ab, ao), ab);
		}
		else
		{
			simplex->a = a;
			simplex->num = 1;
			*direction = ao;
		}
		break;
	case 0x6:
		// Line AD
		if (0.0f <= XMVectorGetX(XMVector3Dot(ad, ao)))
		{
			simplex->a = a;
			simplex->b = d;
			simplex->num = 2;
			*direction = XMVector3Cross(XMVector3Cross(ad, ao), ad);
		}
		else
		{
			simplex->a = a;
			simplex->num = 1;
			*direction = ao;
		}
		break;
	case 0x7:
		// Point A
		simplex->a = a;
		simplex->num = 1;
		*direction = ao;
		break;
	}

	return false;
}

static bool doSimplex(GJKSimplex* simplex, XMVECTOR* direction)
{
	switch (simplex->num)
	{
	case 2:
		return doSimplex2(simplex, direction);
	case 3:
		return doSimplex3(simplex, direction);
	case 4:
		return doSimplex4(simplex, direction);
	}

	assert(false);
	return false;
}

bool GJKCollides(Collider* collider1, Collider* collider2, GJKSimplex* _simplex)
{
	GJKSimplex simplex;

	simplex.a = supportPointOfMinkowskiDifference(collider1, collider2, XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
	simplex.num = 1;

	XMVECTOR direction = -simplex.a;

	for (size_t i = 0; i < 100; ++i)
	{
		XMVECTOR nextPoint = supportPointOfMinkowskiDifference(collider1, collider2, direction);

		if (XMVectorGetX(XMVector3Dot(nextPoint, direction)) < 0.0f)
		{
			// No intersection
			return false;
		}

		addToSimplex(&simplex, nextPoint);

		if (true == doSimplex(&simplex, &direction))
		{
			// Intersection
			if (nullptr != _simplex)
			{
				*_simplex = simplex;
			}

			return true;
		}
	}

	OutputDebugString(L"GJK didn't converge.\n");
	return false;
}