#ifndef WALL_H_INCLUDED
#define WALL_H_INCLUDED

#include <vmmlib/vmmlib.h>

using namespace vmml;

class Wall
{

public:
    Wall();
    Wall( const Vector3f& start, const Vector3f& end );
    virtual ~Wall();
    bool Intersect( const Vector3f& position, float radius );
    Vector3f m_startPosition;
    Vector3f m_endPosition;
    Vector3f m_color;
    bool m_locked;
};

#endif // WALL_H_INCLUDED
