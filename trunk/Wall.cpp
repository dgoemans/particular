#include "Settings.h"
#include "Wall.h"

using namespace vmml;

Wall::Wall()
:
m_startPosition( 0,0,0 ),
m_endPosition( 0,0,0 ),
m_color( 0,0,0 ),
m_locked( false )
{
}

Wall::Wall( const Vector3f& start, const Vector3f& end )
:
m_startPosition( start ),
m_endPosition( end ),
m_color( 0,0,0 ),
m_locked( false )
{
}

Wall::~Wall()
{
}

bool Wall::Intersect( const Vector3f& position, float radius )
{
    float a, b, c, mu, i ;
    Vector3f delta( PARTICLE_SIZE/2, PARTICLE_SIZE/2, PARTICLE_SIZE/2 );

    // NEED RANGE CHECKING!!!
    float d_start = position.distanceSquared( m_startPosition );
    float d_end = position.distanceSquared( m_endPosition );

    if( d_start < d_end )
    {
        Vector3f startToEnd = m_endPosition - m_startPosition;
        startToEnd.normalise();

        Vector3f startToPos = position - m_startPosition;
        startToPos.normalise();

        if( startToEnd.dot( startToPos ) < 0 )// && startToPos.lengthSquared() > (PARTICLE_SIZE/2)*(PARTICLE_SIZE/2) )
            return false;
    }
    else if( d_end < d_start )
    {
        Vector3f startToEnd = m_startPosition - m_endPosition;
        startToEnd.normalise();

        Vector3f startToPos = position - m_endPosition;
        startToPos.normalise();

        if( startToEnd.dot( startToPos ) < 0 ) //&& startToPos.lengthSquared() > (PARTICLE_SIZE/2)*(PARTICLE_SIZE/2) )
            return false;
    }


    Vector3f dist = m_endPosition - m_startPosition;
    a = dist.dot( dist );
    Vector3f startToPos = m_startPosition - position;
    b = 2 * dist.dot( startToPos );
    c =  position.lengthSquared()
        + m_startPosition.lengthSquared() -
        2* ( position.dot( m_startPosition ) ) - radius*radius ;
    i =   b * b - 4 * a * c ;

    if ( i < 0.0 )
    {
        return false;
    }
    else
    {
        return true;
    }
}


