#ifndef PARTICLE_H_INCLUDED
#define PARTICLE_H_INCLUDED

//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
//=*=*=*=*=*=* Particles =*=*=*=*=*=
// This class is to simulate a form
// of particle, hopefully creating
// some interesting effects. The
// particle itself will behave in
// such a way that anything close to
// it will be repelled strongly and
// past a certain distance will be
// attracted weekly.
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=

#include <vmmlib/vmmlib.h>

using namespace vmml;

enum ParticleType
{
    NORMAL,
    REPULSIVE,
    ATTRACTIVE,
    STATIC_NORMAL,
    STATIC_REPULSIVE,
    STATIC_ATTRACTIVE,
    INVALID_LAST_MARKER
};

class Particle
{

public:
    Particle();
    Particle( const Vector3f& position );
    virtual ~Particle();
    void Reset();
    void Print( int number = 0);
    ParticleType m_partType;
    Vector3f m_position;
    Vector3f m_velocity;
    Vector3f m_color;
    float m_strength;
    Vector3f m_origPosition;
    bool m_locked;
};

#endif // PARTICLE_H_INCLUDED
