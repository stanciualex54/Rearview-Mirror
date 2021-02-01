//aplicatia modeleaza o oglinda retrovizoare

#include "glos.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>

#pragma pack(1)

#define TEXTURE_IMG   0
#define TEXTURE_IMGFLIP   1
#define TEXTURE_BLACK   2
#define TEXTURE_COUNT   3
GLuint  textures[TEXTURE_COUNT];
const char* szTextureFiles[TEXTURE_COUNT] = { "4.tga", "oglinda.tga", "1.tga" };

typedef struct
{
    GLbyte	identsize;              // Size of ID field that follows header (0)
    GLbyte	colorMapType;           // 0 = None, 1 = paletted
    GLbyte	imageType;              // 0 = none, 1 = indexed, 2 = rgb, 3 = grey, +8=rle
    unsigned short	colorMapStart;          // First colour map entry
    unsigned short	colorMapLength;         // Number of colors
    unsigned char 	colorMapBits;   // bits per palette entry
    unsigned short	xstart;                 // image x origin
    unsigned short	ystart;                 // image y origin
    unsigned short	width;                  // width in pixels
    unsigned short	height;                 // height in pixels
    GLbyte	bits;                   // bits per pixel (8 16, 24, 32)
    GLbyte	descriptor;             // image descriptor
} TGAHEADER;
#pragma pack(8)


//////////////////////////////////////////////////////////////////
// Module globals to save source image data
static GLbyte* pImage = NULL;
static GLint iWidth, iHeight, iComponents;
static GLenum eFormat;
GLfloat rot1 = 0;

//GLint gltWriteTGA(const char* szFileName);
GLbyte* gltLoadTGA(const char* szFileName, GLint* iWidth, GLint* iHeight, GLint* iComponents, GLenum* eFormat);
void RenderScene(void);

GLbyte* gltLoadTGA(const char* szFileName, GLint* iWidth, GLint* iHeight, GLint* iComponents, GLenum* eFormat)
{
    FILE* pFile;			// File pointer
    TGAHEADER tgaHeader;		// TGA file header
    unsigned long lImageSize;		// Size in bytes of image
    short sDepth;			// Pixel depth;
    GLbyte* pBits = NULL;          // Pointer to bits

    // Default/Failed values
    *iWidth = 0;
    *iHeight = 0;
    *eFormat = GL_BGR_EXT;
    *iComponents = GL_RGB8;

    // Attempt to open the file
    pFile = fopen(szFileName, "rb");
    if (pFile == NULL)
        return NULL;

    // Read in header (binary)
    fread(&tgaHeader, sizeof(TGAHEADER), 1, pFile);

    // Get width, height, and depth of texture
    *iWidth = tgaHeader.width;
    *iHeight = tgaHeader.height;
    sDepth = tgaHeader.bits / 8;
    
    // Calculate size of image buffer
    lImageSize = tgaHeader.width * tgaHeader.height * sDepth;
    // Allocate memory and check for success
    pBits = (GLbyte*)malloc(lImageSize * sizeof(GLbyte));
    if (pBits == NULL)
        return NULL;

    // Read in the bits
    // Check for read error. This should catch RLE or other 
    // weird formats that I don't want to recognize
    if (fread(pBits, lImageSize, 1, pFile) != 1)
    {
        free(pBits);
        return NULL;
    }

    // Set OpenGL format expected
    switch (sDepth)
    {
    case 3:     // Most likely case
        *eFormat = GL_BGR_EXT;
        *iComponents = GL_RGB8;
        break;
    case 4:
        *eFormat = GL_BGRA_EXT;
        *iComponents = GL_RGBA8;
        break;
    case 1:
        *eFormat = GL_LUMINANCE;
        *iComponents = GL_LUMINANCE8;
        break;
    };


    // Done with File
    fclose(pFile);

    // Return pointer to image data
    return pBits;
}

//GLint gltWriteTGA(const char* szFileName)
//{
//    FILE* pFile;                // File pointer
//    TGAHEADER tgaHeader;		// TGA file header
//    unsigned long lImageSize;   // Size in bytes of image
//    GLbyte* pBits = NULL;      // Pointer to bits
//    GLint iViewport[4];         // Viewport in pixels
//    GLenum lastBuffer;          // Storage for the current read buffer setting
//
//    // Get the viewport dimensions
//    glGetIntegerv(GL_VIEWPORT, iViewport);
//
//    // How big is the image going to be (targas are tightly packed)
//    lImageSize = iViewport[2] * 3 * iViewport[3];
//
//    // Allocate block. If this doesn't work, go home
//    pBits = (GLbyte*)malloc(lImageSize);
//    if (pBits == NULL)
//        return 0;
//
//    // Read bits from color buffer
//    glPixelStorei(GL_PACK_ALIGNMENT, 1);
//    glPixelStorei(GL_PACK_ROW_LENGTH, 0);
//    glPixelStorei(GL_PACK_SKIP_ROWS, 0);
//    glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
//
//    // Get the current read buffer setting and save it. Switch to
//    // the front buffer and do the read operation. Finally, restore
//    // the read buffer state
//    glGetIntegerv(GL_READ_BUFFER, (GLint*)&lastBuffer);
//    glReadBuffer(GL_FRONT);
//    glReadPixels(0, 0, iViewport[2], iViewport[3], GL_BGR_EXT, GL_UNSIGNED_BYTE, pBits);
//    glReadBuffer(lastBuffer);
//
//    // Initialize the Targa header
//    tgaHeader.identsize = 0;
//    tgaHeader.colorMapType = 0;
//    tgaHeader.imageType = 2;
//    tgaHeader.colorMapStart = 0;
//    tgaHeader.colorMapLength = 0;
//    tgaHeader.colorMapBits = 0;
//    tgaHeader.xstart = 0;
//    tgaHeader.ystart = 0;
//    tgaHeader.width = iViewport[2];
//    tgaHeader.height = iViewport[3];
//    tgaHeader.bits = 24;
//    tgaHeader.descriptor = 0;
//
//
//    // Attempt to open the file
//    pFile = fopen(szFileName, "wb");
//    if (pFile == NULL)
//    {
//        free(pBits);    // Free buffer and return error
//        return 0;
//    }
//
//    // Write the header
//    fwrite(&tgaHeader, sizeof(TGAHEADER), 1, pFile);
//
//    // Write the image data
//    fwrite(pBits, lImageSize, 1, pFile);
//
//    // Free temporary buffer and close the file
//    free(pBits);
//    fclose(pFile);
//
//    // Success!
//    return 1;
//}


//////////////////////////////////////////////////////////////////
// This function does any needed initialization on the rendering
// context. 
void SetupRC()
{
    GLbyte* pBytes;
    GLint iWidth, iHeight, iComponents;
    GLenum eFormat;
    GLint iLoop;

    // Black background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Textures applied as decals, no lighting or coloring effects
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    // Load textures
    glGenTextures(TEXTURE_COUNT, textures);
    for (iLoop = 0; iLoop < TEXTURE_COUNT; iLoop++)
    {
        // Bind to next texture object
        glBindTexture(GL_TEXTURE_2D, textures[iLoop]);

        // Load texture, set filter and wrap modes
        pBytes = gltLoadTGA(szTextureFiles[iLoop], &iWidth, &iHeight,
            &iComponents, &eFormat);

        // Load texture, set filter and wrap modes
        gluBuild2DMipmaps(GL_TEXTURE_2D, iComponents, iWidth, iHeight, eFormat, GL_UNSIGNED_BYTE, pBytes);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        // Don't need original texture data any more
        free(pBytes);
    }
    //glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
}

void fundal() {

    glPushMatrix();

    // img
    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_IMG]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-20.0f, -10.0f, -10.0f);

    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(20.0f, -10.0f, -10.0f);

    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(20.0f, 10.0f, -10.0f);

    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-20.0f, 10.0f, -10.0f);
    glEnd();

    glPopMatrix();
}
void rama()
{

    glPushMatrix();
    //glColor3f(0.2, 0.2, 0.2);
    glColor3f(0, 0, 0);
    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_BLACK]);

    glBegin(GL_POLYGON);

    glVertex3f(-19.0f, -9.0f, -14.0f);
    glVertex3f(-20.0f, -5.0f, -14.0f);
    glVertex3f(-21.0f, 0.0f,  -14.0f);
    glVertex3f(-20.0f, 5.0f,  -14.0f);
    glVertex3f(-19.0f, 9.0f,  -14.0f);
    glVertex3f(-6.0f, 10.0f,  -14.0f);
    glVertex3f(6.0f, 10.0f,   -14.0f);
    glVertex3f(19.0f, 9.0f,   -14.0f);
    glVertex3f(20.0f, 5.0f,   -14.0f);
    glVertex3f(21.0f, 0.0f,   -14.0f);
    glVertex3f(20.0f, -5.0f,  -14.0f);
    glVertex3f(19.0f, -9.0f,  -14.0f);
    glVertex3f(6.0f, -10.0f,  -14.0f);
    glVertex3f(-6.0f, -10.0f, -14.0f);

    glEnd();

    glPopMatrix();

}

void kbd(int key, int x, int y) {

    if (key == GLUT_KEY_DOWN) {
        rot1 += 0.1;
    }

    if (rot1 > 5)
    {
        rot1 = 0;
    }
    glutPostRedisplay();
}
void oglinda()
{

    glPushMatrix();
    
    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_IMGFLIP]);

    glBegin(GL_QUADS);

    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-19.0f, -9.0f, 15.0f);

    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(19.0f, -9.0f, 15.0f);

    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(19.0f, 9.0f, 15.0f);

    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-19.0f, 9.0f, 15.0f);

    /*glTexCoord2f(0.05f, 0.0f);
    glVertex3f(-19.0f, -9.0f, 15.0f);
    glTexCoord2f(0.0f, 0.28f);
    glVertex3f(-20.0f, -5.0f, 15.0f);
    glTexCoord2f(0.0f, 0.53f);
    glVertex3f(-21.0f, 0.0f,  15.0f);
    glTexCoord2f(0.0f, 0.78f);
    glVertex3f(-20.0f, 5.0f,  15.0f);
    glTexCoord2f(0.0f, 1.0f); 
    glVertex3f(-19.0f, 9.0f,  15.0f);
    glTexCoord2f(0.25f, 1.0f);
    glVertex3f(-6.0f, 10.0f,  15.0f);
    glTexCoord2f(0.75f, 1.0f);
    glVertex3f(6.0f, 10.0f,   15.0f);
    glTexCoord2f(1.0f, 1.0f); 
    glVertex3f(19.0f, 9.0f,   15.0f);
    glTexCoord2f(1.0f, 0.75f);
    glVertex3f(20.0f, 5.0f,   15.0f);
    glTexCoord2f(1.0f, 0.5f); 
    glVertex3f(21.0f, 0.0f,   15.0f);
    glTexCoord2f(1.0f, 0.25f);
    glVertex3f(20.0f, -5.0f,  15.0f);
    glTexCoord2f(1.0f, 0.0f); 
    glVertex3f(19.0f, -9.0f,  15.0f);*/

    glEnd();

    glPopMatrix();

}
void oglindaBlend()
{
    glPushMatrix();
    
    glBegin(GL_POLYGON);

    /*glVertex3f(-19.0f, -9.0f, -15.0f);
    glVertex3f(19.0f, -9.0f, -15.0f);
    glVertex3f(19.0f, 9.0f, -15.0f);
    glVertex3f(-19.0f, 9.0f, -15.0f);*/

    glVertex3f(-19.0f, -9.0f, -15.0f);
    glVertex3f(-20.0f, -5.0f, -15.0f);
    glVertex3f(-21.0f, 0.0f,  -15.0f);
    glVertex3f(-20.0f, 5.0f,  -15.0f);
    glVertex3f(-19.0f, 9.0f,  -15.0f);
    glVertex3f(-6.0f, 10.0f,  -15.0f);
    glVertex3f(6.0f, 10.0f,   -15.0f);
    glVertex3f(19.0f, 9.0f,   -15.0f);
    glVertex3f(20.0f, 5.0f,   -15.0f);
    glVertex3f(21.0f, 0.0f,   -15.0f);
    glVertex3f(20.0f, -5.0f,  -15.0f);
    glVertex3f(19.0f, -9.0f,  -15.0f);
    
    glEnd();

    glPopMatrix();

}

///////////////////////////////////////////////////////////////////////        
// Called to draw scene
void RenderScene(void)
{
    glPushMatrix();

    glPushMatrix();
    glTranslatef(0, 0, rot1);
    fundal();
    glPopMatrix();

    rama();
    
    glEnable(GL_STENCIL_TEST); // activam testul sablon
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    //glDepthMask(GL_FALSE);
    
    glStencilFunc(GL_ALWAYS, 1, 0xFF);// doar punem patratul in bufferul stencil
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    oglindaBlend();
       
    //glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    //glDepthMask(GL_TRUE);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glPushMatrix();
    glTranslatef(0, 0, 7);
    glScalef(1, 1, -1);
    glStencilFunc(GL_LEQUAL, 1, 0xFF);
    glTranslatef(0, 0, rot1);
    oglinda();
    glPopMatrix(); 

    glDisable(GL_STENCIL_TEST); 

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    //glDepthMask si  glDepthFunc de sus si jos ma ajuta sa fac din nou oglinda vizibila
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_LIGHTING);

    glColor4f(1, 1, 1, 0.1);
    oglindaBlend();

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

    glPopMatrix();

    glFlush();
    // Do the buffer Swap
    glutSwapBuffers();
}

void ChangeSize(int w, int h)
{
    GLfloat fAspect;

    // Prevent a divide by zero
    if (h == 0)
        h = 1;

    // Set Viewport to window dimensions
    glViewport(0, 0, w, h);

    fAspect = (GLfloat)w / (GLfloat)h;
   
    // Reset coordinate system
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Produce the perspective projection
    gluPerspective(90.0f, fAspect, 1, 1200);

    /*glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();*/
}

/////////////////////////////////////////////////////////////
// Main program entrypoint
int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GL_DOUBLE | GLUT_STENCIL);
    glutInitWindowSize(1280, 720);
    glutCreateWindow("Oglinda retrovizoare");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutSpecialFunc(kbd);
    SetupRC();          // Do setup
    glutMainLoop();     // Main program loop
    
    return 0;
}
