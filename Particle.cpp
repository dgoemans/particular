#include "Particle.h"

Particle::Particle()
:
m_position( 0,0,0 ),
m_origPosition( 0,0,0 ),
m_velocity( 0,0,0 ),
m_partType( NORMAL ),
m_strength( 1 ),
m_color( 0,0,1 ),
m_locked( false )
{
}

Particle::Particle( const Vector3f& position )
:
m_position( position ),
m_origPosition( position ),
m_velocity( 0,0,0 ),
m_partType( NORMAL ),
m_strength( 1 ),
m_color( 0,0,1 ),
m_locked( false )
{
}

Particle::~Particle()
{
}

void Particle::Reset()
{
    m_velocity = Vector3f( 0,0,0 );
    m_position = m_origPosition;
}

void Particle::Print( int number )
{
    printf("Particle %i: pos->x:%f, pos->y:%f. pos->z:%f\n", number, m_position.x, m_position.y, m_position.z );
    printf("Particle %i: vel->x:%f, vel->y:%f. vel->z:%f\n", number, m_velocity.x, m_velocity.y, m_velocity.z );
}


