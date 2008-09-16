/*
 * GLUT Shapes Demo
 *
 * Written by Nigel Stewart November 2003
 *
 * This program is test harness for the sphere, cone
 * and torus shapes in GLUT.
 *
 * Spinning wireframe and smooth shaded shapes are
 * displayed until the ESC or q key is pressed.  The
 * number of geometry stacks and slices can be adjusted
 * using the + and - keys.
 */

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <GL/gl.h>
#include <stdlib.h>

#include <list>
#include <queue>
#include <stdlib.h>
#include <time.h>

#include "Settings.h"

#include "Particle.h"
#include "Wall.h"


using namespace std;
using namespace vmml;

enum CurrentItem
{
    PARTICLE,
    WALL,
    INVALID_ITEM_MARKER
};

CurrentItem m_currentItemToPlace = PARTICLE;

static list<Particle*> m_particles;
static list<Wall*> m_walls;

static Particle* m_mouseOverParticle = NULL;
static Wall* m_mouseOverWall = NULL;

static float m_currentWidth = 640.0f;
static float m_currentHeight = 640.0f;

static bool m_running = false;

static ParticleType m_currentType = NORMAL;
static Vector3f m_currentMousePos( 0,0,0 );
/* GLUT callback Handlers */


static void resize(int width, int height)
{
    m_currentWidth = width;
    m_currentHeight = height;
    const float ar = (float) width / (float) height;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho( -width/2, width/2, -height/2, height/2, -600, 600 );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    //glTranslatef( 0.0f, 0.0f, -100.0f );
}

void AddParticle( const Vector3f& position, ParticleType type = STATIC_REPULSIVE )
{
    m_particles.push_back( new Particle( position ) );
    m_particles.back()->m_partType = type;
    m_particles.back()->m_strength = 30.0f;
    m_particles.back()->m_color
        = Vector3f( (float)type/( INVALID_LAST_MARKER - 1),
                    1-(float)type/( INVALID_LAST_MARKER - 1),
                    0.2f );
}

void AddWall( const Vector3f& start, const Vector3f& end )
{
    m_walls.push_back( new Wall( start, end ) );
    m_walls.back()->m_color = Vector3f( 0.8f, 0.8f, 0.8f );
}

void GenerateAtLeader( ParticleType type = STATIC_REPULSIVE )
{
    AddParticle( m_particles.front()->m_position );
}

void DoPhysics( float timeStep )
{

    for( list<Particle*>::iterator it = m_particles.begin(); it != m_particles.end(); it++ )
    {
        // Load Object 1
        Particle* current = *it;
        // Interactions with other Particles
        for ( list<Particle*>::iterator it2 = it; it2 != m_particles.end(); it2++ )
        {
            if ((*it)==(*it2)) continue;
            if((*it)==m_mouseOverParticle || (*it2)==m_mouseOverParticle ) continue;
            // Load other object
            Particle* other = *it2;
            // Calculate distances
            float dx=other->m_position.x-current->m_position.x;
            float dy=other->m_position.y-current->m_position.y;
            float dz=other->m_position.z-current->m_position.z;
            float dx2 = dx*dx;
            float dy2 = dy*dy;
            float dz2 = dz*dz;
            float d2 = dx2+dy2+dz2;
            float d = sqrt(d2);

            // Calculate Force ( thresold to eliminate most extreme behaviour )
            if (d> min(PARTICLE_SIZE,3.0f))
            {
                // Normalize direction vector
                Vector3f dV( dx*timeStep/d, dy*timeStep/d, dz*timeStep/d );

                Vector3f replTerm = dV*REPULSION/d2;
                Vector3f attrTerm = dV*d/(ATTRACTION);

                Vector3f fV = attrTerm - replTerm;
                attrTerm.z = 0.0f;
                replTerm.z = 0.0f;
                fV.z = 0.0f;

                switch( current->m_partType )
                {
                    case NORMAL:
                    case STATIC_NORMAL:
                        other->m_velocity -= fV*current->m_strength;
                        break;
                    case REPULSIVE:
                    case STATIC_REPULSIVE:
                        other->m_velocity -= (fV - attrTerm)*current->m_strength;
                        break;
                    case ATTRACTIVE:
                    case STATIC_ATTRACTIVE:
                        other->m_velocity -= (fV + replTerm)*current->m_strength;
                        break;
                }

                switch( other->m_partType )
                {
                    case NORMAL:
                    case STATIC_NORMAL:
                        current->m_velocity += fV*other->m_strength;
                        break;
                    case REPULSIVE:
                    case STATIC_REPULSIVE:
                        current->m_velocity += (fV - attrTerm)*other->m_strength;
                        break;
                    case ATTRACTIVE:
                    case STATIC_ATTRACTIVE:
                        current->m_velocity += (fV + replTerm)*other->m_strength;
                        break;
                }
            }
        }

        for( list<Wall*>::iterator it2 = m_walls.begin(); it2 != m_walls.end(); it2++ )
        {
            Wall* wall = *it2;
            if( wall->Intersect( current->m_position, PARTICLE_SIZE ) )
            {
                Vector3f unitZ( 0,0,1 );
                Vector3f normal = unitZ.cross( wall->m_endPosition - wall->m_startPosition );
                normal.normalize();
                Vector3f refl = current->m_velocity - normal * 2.0f * ( normal.dot( current->m_velocity ) );
                current->m_velocity = refl;
            }
        }
    }
    // Movement
    int c = 0;
    for( list<Particle*>::iterator it = m_particles.begin(); it != m_particles.end(); it++ )
    {
        Particle* current = *it;
        if (!( ( current->m_partType == STATIC_ATTRACTIVE )
            || ( current->m_partType == STATIC_NORMAL  )
            || ( current->m_partType == STATIC_REPULSIVE  ) ) )
        {
            current->m_position += current->m_velocity;
            current->m_velocity *= FRICTION;

        }
        c++;
    }
}

void Reset()
{
    m_particles.clear();

    m_particles.push_back( new Particle( Vector3f(-250,0,0) ) );
    m_particles.back()->m_partType = NORMAL;
    m_particles.back()->m_color = Vector3f( 0,0.8f,0.5f );
    m_particles.back()->m_locked = true;
    m_particles.back()->m_strength = 30.0f;

    float patrolStrength = 10.0f;
    for( int fencepole = -BORDERDIST; fencepole<=BORDERDIST; fencepole += 20 )
    {
        m_particles.push_back( new Particle( Vector3f( fencepole, BORDERDIST, 0 ) ) );
        m_particles.back()->m_partType = STATIC_REPULSIVE;
        m_particles.back()->m_strength = patrolStrength;

        m_particles.push_back( new Particle( Vector3f( fencepole, -BORDERDIST, 0 ) ) );
        m_particles.back()->m_partType = STATIC_REPULSIVE;
        m_particles.back()->m_strength = patrolStrength;

        m_particles.push_back( new Particle( Vector3f( BORDERDIST, fencepole, 0 ) ) );
        m_particles.back()->m_partType = STATIC_REPULSIVE;
        m_particles.back()->m_strength = patrolStrength;

        m_particles.push_back( new Particle( Vector3f( -BORDERDIST, fencepole, 0 ) ) );
        m_particles.back()->m_partType = STATIC_REPULSIVE;
        m_particles.back()->m_strength = patrolStrength;
    }

    m_walls.clear();

    m_running = false;
}

void TryAgain()
{
    for( list<Particle*>::iterator it = m_particles.begin(); it != m_particles.end(); ++it )
    {
        Particle* cur = *it;
        cur->Reset();
    }
    m_running = false;
}

void display(void)
{
    glClearColor( 0,0,0,1 );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float i = 0;
    list<Particle*>::iterator it = m_particles.begin();
    while( it != m_particles.end() )
    {
        Particle* cur = *it;
        glColor3f( cur->m_color.x, cur->m_color.y, cur->m_color.z );
        if( cur == m_mouseOverParticle )
        {
            glColor3f( cur->m_color.x + 0.4f, cur->m_color.y + 0.4f, cur->m_color.z + 0.4f );
        }
        glPushMatrix();
        glTranslatef( cur->m_position.x, cur->m_position.y, cur->m_position.z );
        glutSolidSphere( PARTICLE_SIZE/2 ,20,20);
        glPopMatrix();
        it++;
        i += 1.0f/(float)m_particles.size();
    }

    glLineWidth( PARTICLE_SIZE );
    glBegin( GL_LINES );

    for( list<Wall*>::iterator itWall = m_walls.begin(); itWall != m_walls.end(); ++itWall )
    {
        Wall* cur = (*itWall);

        glColor3f( cur->m_color.x, cur->m_color.y, cur->m_color.z );
        glVertex3f( cur->m_startPosition.x, cur->m_startPosition.y, cur->m_startPosition.z );
        glVertex3f( cur->m_endPosition.x, cur->m_endPosition.y, cur->m_endPosition.z );
    }
    glEnd();


    if( m_currentItemToPlace == PARTICLE && !m_mouseOverParticle && !m_running )
    {
        glColor3f( (float)m_currentType/( INVALID_LAST_MARKER - 1),
                   1-(float)m_currentType/( INVALID_LAST_MARKER - 1),
                   0.2f );

        glPushMatrix();
        glTranslatef( m_currentMousePos.x, m_currentMousePos.y, m_currentMousePos.z );
        glutSolidSphere( PARTICLE_SIZE/2 ,20,20);
        glPopMatrix();

    }

    if( m_currentItemToPlace == WALL && !m_mouseOverWall && !m_running )
    {
        glLineWidth( PARTICLE_SIZE );
        glBegin( GL_LINES );

        for( list<Wall*>::iterator itWall = m_walls.begin(); itWall != m_walls.end(); ++itWall )
        {
            Wall* cur = (*itWall);
            Vector3f delta( 50, 0,0 );
            glColor3f( cur->m_color.x, cur->m_color.y, cur->m_color.z );
            glVertex3f( m_currentMousePos.x - 50, m_currentMousePos.y, m_currentMousePos.z );
            glVertex3f( m_currentMousePos.x + 50, m_currentMousePos.y, m_currentMousePos.z );
        }
        glEnd();
    }
    //glEnd();

    glutSwapBuffers();
    if( m_running )
        DoPhysics( 0.001f );
}

static void key(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 27 :
        case 'q':
            exit(0);
            break;
        case 32:
            m_running = true;
            break;
        case 'r':
            TryAgain();
            break;
        case 't':
            Reset();
            break;
        case 's':
            m_currentItemToPlace = (CurrentItem) ( ((int)m_currentItemToPlace +  1) % ((int)INVALID_ITEM_MARKER) );
            printf("Current item: %i\n", (int)m_currentItemToPlace );
            break;
    }
}

void specialKey( int key, int x, int y )
{
    float movementDist = 1.0f;
    switch( key )
    {
        case GLUT_KEY_LEFT:
            m_particles.front()->m_position.x -= movementDist;
            break;
        case GLUT_KEY_RIGHT:
            m_particles.front()->m_position.x += movementDist;
            break;
        case GLUT_KEY_DOWN:
            if( m_currentType == 0 )
            {
                m_currentType = (ParticleType)(INVALID_LAST_MARKER - 1);
                printf("Current Type: %i\n", m_currentType );
            }
            else
            {
                m_currentType = (ParticleType)(m_currentType - 1 );
                printf("Current Type: %i\n", m_currentType );
            }
            //m_particles.front()->m_position.y -= movementDist;
            break;
        case GLUT_KEY_UP:
            if( m_currentType == INVALID_LAST_MARKER-1 )
            {
                m_currentType = (ParticleType)0;
                printf("Current Type: %i\n", m_currentType );
            }
            else
            {
                m_currentType = (ParticleType)(m_currentType + 1 );
                printf("Current Type: %i\n", m_currentType );
            }
            //m_particles.front()->m_position.y += movementDist;
            break;
    }

    glutPostRedisplay();
}

static bool mousePressed = false;
static bool rightMousePressed = false;

void mouse(int button, int state, int x, int y)
{
    if( m_running )
        return;

    if( m_currentItemToPlace == PARTICLE )
    {
        if( !m_mouseOverParticle )
        {
            Vector3f clickPos( x - m_currentWidth/2, ( -y + m_currentHeight/2 ), 0 );
            if (button == GLUT_LEFT_BUTTON)
            {
                if ( state == GLUT_DOWN && !mousePressed )
                {
                    AddParticle( clickPos, m_currentType );
                    mousePressed = true;
                }
                else if( state == GLUT_UP )
                {
                    mousePressed = false;
                }
            }
        }
        else if( !m_mouseOverParticle->m_locked && !rightMousePressed )
        {
            if( button == GLUT_RIGHT_BUTTON )
            {
                if( state == GLUT_DOWN )
                {
                    rightMousePressed = true;
                    m_particles.remove( m_mouseOverParticle );
                    delete m_mouseOverParticle;
                }
            }
        }
    }
    else if( m_currentItemToPlace == WALL )
    {
        if( !m_mouseOverWall )
        {
            Vector3f clickPos( x - m_currentWidth/2, ( -y + m_currentHeight/2 ), 0 );
            if (button == GLUT_LEFT_BUTTON)
            {
                if ( state == GLUT_DOWN && !mousePressed )
                {
                    Vector3f delta( 50, 0, 0 );
                    AddWall( clickPos + delta, clickPos - delta );
                    mousePressed = true;
                }
                else if( state == GLUT_UP )
                {
                    mousePressed = false;
                }
            }
        }
        else if( !m_mouseOverWall->m_locked && !rightMousePressed )
        {
            if( button == GLUT_RIGHT_BUTTON )
            {
                if( state == GLUT_DOWN )
                {
                    rightMousePressed = true;
                    m_walls.remove( m_mouseOverWall );
                    delete m_mouseOverWall;
                }
            }
        }
    }

    if( button == GLUT_RIGHT_BUTTON && state == GLUT_UP )
    {
        rightMousePressed = false;
    }
}

void drag( int x, int y )
{
    if( m_running || rightMousePressed )
        return;

    Vector3f mousePos( x - m_currentWidth/2, ( -y + m_currentHeight/2 ), 0 );

    if( m_mouseOverParticle )
    {
        m_mouseOverParticle->m_position = mousePos;
        m_mouseOverParticle->m_origPosition = mousePos;
    }

    if( m_mouseOverWall )
    {
        Vector3f mouseToWallStart = mousePos - m_mouseOverWall->m_startPosition;
        Vector3f mouseToWallEnd = mousePos - m_mouseOverWall->m_endPosition;

        if( mouseToWallEnd.lengthSquared() < mouseToWallStart.lengthSquared() )
        {
            m_mouseOverWall->m_endPosition = mousePos;
        }
        else
        {
            m_mouseOverWall->m_startPosition = mousePos;
        }
    }
}

void currentMousePos( int x, int y )
{
    if( m_running )
    {
        m_currentMousePos = Vector3f( 0,0,0 );
        m_mouseOverParticle = NULL;
        return;
    }

    Vector3f mousePos( x - m_currentWidth/2, ( -y + m_currentHeight/2 ), 0 );

    for( list<Particle*>::iterator it = m_particles.begin(); it != m_particles.end(); ++it )
    {
        Particle* cur = *it;
        if( cur->m_position.distanceSquared( mousePos ) < PARTICLE_SIZE*PARTICLE_SIZE )
        {
            m_mouseOverParticle = cur;
            return;
        }
    }

    m_mouseOverParticle = NULL;

    for( list<Wall*>::iterator it = m_walls.begin(); it != m_walls.end(); ++it )
    {
        Wall* cur = *it;
        if( cur->Intersect( mousePos, PARTICLE_SIZE/2 ) )
        {
            m_mouseOverWall = cur;
            return;
        }
    }

    m_mouseOverWall = NULL;
    m_currentMousePos = mousePos;
}

static void idle(void)
{
    glutPostRedisplay();
}
/* Program entry point */

int main(int argc, char *argv[])
{
    srand ( time(NULL) );

    Reset();

    glutInit(&argc, argv);
    glutInitWindowSize(BORDERDIST*2 + PARTICLE_SIZE,BORDERDIST*2 + PARTICLE_SIZE);
    glutInitWindowPosition(10,10);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

    glutCreateWindow("Particular");

    glutReshapeFunc(resize);
    glutDisplayFunc(display);
    glutKeyboardFunc(key);
    glutSpecialFunc(specialKey);
    glutMouseFunc( mouse );
    glutMotionFunc( drag );
    glutPassiveMotionFunc( currentMousePos );
    glutIdleFunc(idle);

    glClearColor(1,1,1,1);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);

    glViewport(0, 0, 640, 480);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho( 0, 640, 0, 480, -600, 600 );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity() ;


    glutMainLoop();

    return EXIT_SUCCESS;
}
