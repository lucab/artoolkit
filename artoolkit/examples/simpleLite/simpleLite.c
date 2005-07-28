/*
 *  simpleLite.c
 *
 *  Some code to demonstrate use of gsub_lite's argl*() functions.
 *  Shows the correct GLUT usage to read a video frame (in the idle callback)
 *  and to draw it (in the display callback).
 *
 *  Press '?' while running for help on available key commands.
 *
 *  Copyright (c) 2001-2004 Philip Lamb (PRL) phil@eden.net.nz. All rights reserved.
 *
 *	Rev		Date		Who		Changes
 *	1.0.0	20040302	PRL		Initial version, simple test animation using GLUT.
 *	1.0.1	20040721	PRL		Correctly sets window size; supports arVideoDispOption().
 *
 */
/*
 * 
 * This file is part of ARToolKit.
 * 
 * ARToolKit is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * ARToolKit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with ARToolKit; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 */


// ============================================================================
//	Includes
// ============================================================================

#include <stdio.h>
#include <stdlib.h>					// malloc(), free()
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif
#include <AR/config.h>
#include <AR/video.h>
#include <AR/param.h>			// arParamDisp()
#include <AR/ar.h>
#include <AR/gsub_lite.h>

// ============================================================================
//	Constants
// ============================================================================

#define VIEW_SCALEFACTOR		0.025		// 1.0 ARToolKit unit becomes 0.025 of my OpenGL units.
#define VIEW_DISTANCE_MIN		0.1			// Objects closer to the camera than this will not be displayed.
#define VIEW_DISTANCE_MAX		100.0		// Objects further away from the camera than this will not be displayed.

// ============================================================================
//	Global variables
// ============================================================================

// Preferences.
static int prefWindowed = TRUE;
static int prefWidth = 640;					// Fullscreen mode width.
static int prefHeight = 480;				// Fullscreen mode height.
static int prefDepth = 32;					// Fullscreen mode bit depth.
static int prefRefresh = 0;					// Fullscreen mode refresh rate. Set to 0 to use default rate.

// ARToolKit globals.
static long			gCallCountMarkerDetect = 0;
static int			gARTThreshhold = 100;
static int			gPatt_id;
static double		gPatt_width     = 80.0;
static double		gPatt_centre[2] = {0.0, 0.0};

static ARParam		gARTCparam;
static ARUint8		*gARTImage = NULL;
static double		gPatt_trans[3][4];
static int			gPatt_found;
static ARGL_CONTEXT_SETTINGS_REF gArglSettings = NULL;

// Other globals.
static int gDrawRotate = FALSE;
static float gDrawRotateAngle = 0;			// For use in drawing.

// ============================================================================
//	Functions
// ============================================================================

// Something to look at, draw a rotating colour cube.
static void DrawCube(void)
{
	// Colour cube data.
	static GLuint polyList = 0;
	float fSize = 0.5f;
	long f, i;	
	const GLfloat cube_vertices [8][3] = {
	{1.0, 1.0, 1.0}, {1.0, -1.0, 1.0}, {-1.0, -1.0, 1.0}, {-1.0, 1.0, 1.0},
	{1.0, 1.0, -1.0}, {1.0, -1.0, -1.0}, {-1.0, -1.0, -1.0}, {-1.0, 1.0, -1.0} };
	const GLfloat cube_vertex_colors [8][3] = {
	{1.0, 1.0, 1.0}, {1.0, 1.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 1.0, 1.0},
	{1.0, 0.0, 1.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0} };
	GLint cube_num_faces = 6;
	const short cube_faces [6][4] = {
	{3, 2, 1, 0}, {2, 3, 7, 6}, {0, 1, 5, 4}, {3, 0, 4, 7}, {1, 2, 6, 5}, {4, 5, 6, 7} };
	
	if (!polyList) {
		polyList = glGenLists (1);
		glNewList(polyList, GL_COMPILE);
		glBegin (GL_QUADS);
		for (f = 0; f < cube_num_faces; f++)
			for (i = 0; i < 4; i++) {
				glColor3f (cube_vertex_colors[cube_faces[f][i]][0], cube_vertex_colors[cube_faces[f][i]][1], cube_vertex_colors[cube_faces[f][i]][2]);
				glVertex3f(cube_vertices[cube_faces[f][i]][0] * fSize, cube_vertices[cube_faces[f][i]][1] * fSize, cube_vertices[cube_faces[f][i]][2] * fSize);
			}
				glEnd ();
		glColor3f (0.0, 0.0, 0.0);
		for (f = 0; f < cube_num_faces; f++) {
			glBegin (GL_LINE_LOOP);
			for (i = 0; i < 4; i++)
				glVertex3f(cube_vertices[cube_faces[f][i]][0] * fSize, cube_vertices[cube_faces[f][i]][1] * fSize, cube_vertices[cube_faces[f][i]][2] * fSize);
			glEnd ();
		}
		glEndList ();
	}
	
	glPushMatrix(); // Save world coordinate system.
	glTranslatef(0.0, 0.0, 0.5); // Place base of cube on marker surface.
	glRotatef(gDrawRotateAngle, 0.0, 0.0, 1.0); // Rotate about z axis.
	glDisable(GL_LIGHTING);	// Just use colours.
	glCallList(polyList);	// Draw the cube.
	glPopMatrix();	// Restore world coordinate system.
	
}

static void DrawCubeUpdate(float timeDelta)
{
	if (gDrawRotate) {
		gDrawRotateAngle += timeDelta * 45.0f; // Rotate cube at 45 degrees per second.
		if (gDrawRotateAngle > 360.0f) gDrawRotateAngle -= 360.0f;
	}
}

// Sets up gARTCparam.
static int demoARSetupCamera(const unsigned char *cparam_name, char *vconf)
{	
    ARParam  wparam;
	int xsize, ysize;

    // Open the video path.
    if (arVideoOpen(vconf) < 0) {
    	fprintf(stderr, "demoARSetupCamera(): Unable to open connection to camera.\n");
    	return (FALSE);
	}
	
    // Find the size of the window.
    if (arVideoInqSize(&xsize, &ysize) < 0) return (FALSE);
    fprintf(stdout, "Camera image size (x,y) = (%d,%d)\n", xsize, ysize);
	
	// Load the camera parameters, resize for the window and init.
    if (arParamLoad(cparam_name, 1, &wparam) < 0) {
		fprintf(stderr, "demoARSetupCamera(): Error loading parameter file %s for camera.\n", cparam_name);
        return (FALSE);
    }
    arParamChangeSize(&wparam, xsize, ysize, &gARTCparam);
    arInitCparam(&gARTCparam);
    fprintf(stdout, "*** Camera Parameter ***\n");
    arParamDisp(&gARTCparam);
	
    if (arVideoCapStart() != 0) {
    	fprintf(stderr, "demoARSetupCamera(): Unable to begin camera data capture.\n");
		return (FALSE);		
	}
	
	return (TRUE);
}

static int demoARSetupMarker(const unsigned char *patt_name, int *patt_id)
{
	
    if((*patt_id = arLoadPatt(patt_name)) < 0) {
        fprintf(stderr, "demoARSetupMarker(): pattern load error !!\n");
        return (FALSE);
    }
	
	return (TRUE);
}

// Report state of ARToolKit global variables arFittingMode,
// arImageProcMode, arglDrawMode, arTemplateMatchingMode, arMatchingPCAMode.
static void demoARDebugReportMode(void)
{
	if(arFittingMode == AR_FITTING_TO_INPUT ) {
		fprintf(stderr, "FittingMode (Z): INPUT IMAGE\n");
	} else {
		fprintf(stderr, "FittingMode (Z): COMPENSATED IMAGE\n");
	}
	
	if( arImageProcMode == AR_IMAGE_PROC_IN_FULL ) {
		fprintf(stderr, "ProcMode (X)   : FULL IMAGE\n");
	} else {
		fprintf(stderr, "ProcMode (X)   : HALF IMAGE\n");
	}
	
	if( arglDrawMode == AR_DRAW_BY_GL_DRAW_PIXELS ) {
		fprintf(stderr, "DrawMode (C)   : GL_DRAW_PIXELS\n");
	} else if( arglTexmapMode == AR_DRAW_TEXTURE_FULL_IMAGE ) {
		fprintf(stderr, "DrawMode (C)   : TEXTURE MAPPING (FULL RESOLUTION)\n");
	} else {
		fprintf(stderr, "DrawMode (C)   : TEXTURE MAPPING (HALF RESOLUTION)\n");
	}
	
	if( arTemplateMatchingMode == AR_TEMPLATE_MATCHING_COLOR ) {
		fprintf(stderr, "TemplateMatchingMode (M)   : Color Template\n");
	} else {
		fprintf(stderr, "TemplateMatchingMode (M)   : BW Template\n");
	}
	
	if( arMatchingPCAMode == AR_MATCHING_WITHOUT_PCA ) {
		fprintf(stderr, "MatchingPCAMode (P)   : Without PCA\n");
	} else {
		fprintf(stderr, "MatchingPCAMode (P)   : With PCA\n");
	}
}

static void Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
		case 0x1B:						// Quit.
		case 'Q':
		case 'q':
			arglCleanup(gArglSettings);
			arVideoCapStop();
			arVideoClose();
			exit(0);
			break;
		case ' ':
			gDrawRotate = !gDrawRotate;
			break;
		case 'C':
		case 'c':
			if( arglDrawMode == AR_DRAW_BY_GL_DRAW_PIXELS ) {
				arglDrawMode  = AR_DRAW_BY_TEXTURE_MAPPING;
				arglTexmapMode = AR_DRAW_TEXTURE_FULL_IMAGE;
			} else if( arglTexmapMode == AR_DRAW_TEXTURE_FULL_IMAGE ) {
				arglTexmapMode = AR_DRAW_TEXTURE_HALF_IMAGE;
			} else {
				arglDrawMode  = AR_DRAW_BY_GL_DRAW_PIXELS;
			}
			fprintf(stderr, "*** Camera - %f (frame/sec)\n", (double)gCallCountMarkerDetect/arUtilTimer());
			gCallCountMarkerDetect = 0;
			arUtilTimerReset();
			demoARDebugReportMode();
			break;
			break;
#ifdef AR_OPENGL_TEXTURE_RECTANGLE
		case 'R':
		case 'r':
			arglTexRectangle = !arglTexRectangle;
			fprintf(stderr, "Toggled arglTexRectangle to %d.\n", arglTexRectangle);
			break;
#endif // AR_OPENGL_TEXTURE_RECTANGLE
		case '?':
		case '/':
			printf("Keys:\n");
			printf(" q or [esc]    Quit demo.\n");
			printf(" c             Change arglDrawMode and arglTexmapMode.\n");
#ifdef AR_OPENGL_TEXTURE_RECTANGLE
			printf(" r             Toggle arglTexRectangle.\n");
#endif // AR_OPENGL_TEXTURE_RECTANGLE
			printf(" ? or /        Show this help.\n");
			printf("\nAdditionally, the ARVideo library supplied the following help text:\n");
			arVideoDispOption();
			break;
		default:
			break;
	}
}

static void Idle(void)
{
	static int ms_prev;
	int ms;
	float s_elapsed;
	ARUint8 *image;
	
	ARMarkerInfo    *marker_info;					// Pointer to array holding the details of detected markers.
    int             marker_num;						// Count of number of markers detected.
    int             j, k;
	
	// Find out how long since Idle() last ran.
	ms = glutGet(GLUT_ELAPSED_TIME);
	s_elapsed = (float)(ms - ms_prev) * 0.001;
	if (s_elapsed < 0.01f) return; // Don't update more often than 100 Hz.
	ms_prev = ms;
	
	// Update drawing.
	DrawCubeUpdate(s_elapsed);
	
	// Grab a video frame.
	if ((image = arVideoGetImage()) != NULL) {
		gARTImage = image;
		
		gCallCountMarkerDetect++; // Increment ARToolKit FPS counter.
		
		// Detect the markers in the video frame.
		if (arDetectMarker(gARTImage, gARTThreshhold, &marker_info, &marker_num) < 0) {
			exit(-1);
		}
		
		// Check through the marker_info array for highest confidence
		// visible marker matching our preferred pattern.
		k = -1;
		for (j = 0; j < marker_num; j++) {
			if (marker_info[j].id == gPatt_id) {
				if (k == -1) k = j; // First marker detected.
				else if(marker_info[j].cf > marker_info[k].cf) k = j; // Higher confidence marker detected.
			}
		}
		
		if (k != -1) {
			// Get the transformation between the marker and the real camera into gPatt_trans.
			arGetTransMat(&(marker_info[k]), gPatt_centre, gPatt_width, gPatt_trans);
			gPatt_found = TRUE;
		}
		
		// Tell GLUT the display has changed.
		glutPostRedisplay();
	}	
}

//
//	The function is called on events when the visibility of the
//	GLUT window changes (including when it first becomes visible).
//
static void Visibility(int visible)
{
	if (visible == GLUT_VISIBLE) {
		glutIdleFunc(Idle);
	} else {
		glutIdleFunc(NULL);
	}
}

//
//	The function is called when the
//	GLUT window is resized.
//
static void Reshape(int w, int h)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Call through to anyone else who needs to know about window sizing here.
}

//
// The function is called when the window needs redrawing.
//
static void Display(void)
{
    static GLdouble *p = NULL;
	GLdouble  m[16];
	
	// Context setup.
	glDrawBuffer(GL_BACK);
	arglDispImage(gARTImage, &gARTCparam, 1.0, gArglSettings);	// zoom = 1.0.
	
	arVideoCapNext();
				
	if (gPatt_found) {
		glClear(GL_DEPTH_BUFFER_BIT);	// Clear the buffers for new frame.
		
		// Projection transformation.
		if (p == NULL) {
			p = (GLdouble *)malloc(16 * (sizeof(GLdouble)));
			arglCameraFrustum(&gARTCparam, VIEW_DISTANCE_MIN, VIEW_DISTANCE_MAX, p);
		}
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixd(p);
		glMatrixMode(GL_MODELVIEW);
		
		// Viewing transformation.
		glLoadIdentity();
		// Lighting and geometry that moves with the camera should go here.
		// (I.e. must be specified before viewing transformations.)
		//none
		
		// ARToolKit supplied distance in millimetres, but I want OpenGL to work in metres.
		arglCameraView(gPatt_trans, m, VIEW_SCALEFACTOR);
		glLoadMatrixd(m);
		
		// All other lighting and geometry goes here.
		DrawCube();
		
		gPatt_found = FALSE;
	} // gPatt_found
	
	// Any 2D overlays go here.
	//none
	
	glutSwapBuffers();
}

int main(int argc, char** argv)
{
	char glutGamemode[32];
	const unsigned char *cparam_name = 
		"Data/camera_para.dat";
	char *vconf = // Camera configuration.
#if defined(_WIN32)
		"Data\\WDM_camera_flipV.xml";
#elif defined(__APPLE__)
		"";
#else
		"-dev=/dev/video0 -channel=0 -palette=YUV420P -width=320 -height=240";
#endif
	const unsigned char *patt_name  = "Data/patt.hiro";
	
	// ----------------------------------------------------------------------------
	// Library inits.
	//

	glutInit(&argc, argv);

	// ----------------------------------------------------------------------------
	// Hardware setup.
	//

	if (!demoARSetupCamera(cparam_name, vconf)) {
		fprintf(stderr, "main(): Unable to set up AR camera.\n");
		exit(-1);
	}
	demoARDebugReportMode();
	if (!demoARSetupMarker(patt_name, &gPatt_id)) {
		fprintf(stderr, "main(): Unable to set up AR marker.\n");
		exit(-1);
	}
	
	// ----------------------------------------------------------------------------
	// Library setup.
	//

	// Set up GL context(s) for OpenGL to draw into.
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	if (!prefWindowed) {
		if (prefRefresh) sprintf(glutGamemode, "%ix%i:%i@%i", prefWidth, prefHeight, prefDepth, prefRefresh);
		else sprintf(glutGamemode, "%ix%i:%i", prefWidth, prefHeight, prefDepth);
		glutGameModeString(glutGamemode);
		glutEnterGameMode();
	} else {
		glutInitWindowSize(gARTCparam.xsize, gARTCparam.ysize);
		glutCreateWindow(argv[0]);
	}

	// Setup argl library for current context.
	if ((gArglSettings = arglSetupForCurrentContext()) == NULL) {
		fprintf(stderr, "main(): arglSetupForCurrentContext() returned error.\n");
		exit(-1);
	}
	arUtilTimerReset();
		
	// Register GLUT event-handling callbacks.
	// NB: Idle() is registered by Visibility.
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutVisibilityFunc(Visibility);
	glutKeyboardFunc(Keyboard);
	
	glutMainLoop();

	return (0);
}
