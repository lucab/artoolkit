/*
 *  gsub_lite_demo.c
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

#define MY_SCALEFACTOR			0.025		// 1.0 ARToolKit unit becomes 0.025 of my OpenGL units.
#define MY_CPARAM				"Data/camera_para.dat"
#define MY_PATTERN				"Data/patt.hiro"

#define VIEW_DISTANCE_MIN		0.1			// Objects closer to the camera than this will not be displayed.
#define VIEW_DISTANCE_MAX		100.0		// Objects further away from the camera than this will not be displayed.

// ============================================================================
//	Global variables
// ============================================================================

// Preferences.
static BOOL prefWindowed = TRUE;

// ARToolKit globals.
static long			gMainCountCallsMarkerDetect = 0;
static int			gARTThreshhold = 100;
static ARParam		gARTCparam;
static int			gPatt_id;
static double		gPatt_width     = 80.0;
static double		gPatt_centre[2] = {0.0, 0.0};
static double		gPatt_trans[3][4];
static BOOL			gPatt_found;
static ARUint8		*gARTImage = NULL;
static ARGL_CONTEXT_SETTINGS_REF gContextSettings = NULL;

// Other globals.
static float gDrawRotateAngle;			// For use in drawing.

// ============================================================================
//	Functions
// ============================================================================

BOOL demoARSetupCamera(const unsigned char *cparam_name, ARParam *cparam)
{
	// Press the '?' key while running to see what options are accepted.
#if defined(_WIN32)
	char *vconf = "flipV,showDlg";
#elif defined(__APPLE__)
	char *vconf = NULL;
#else
	char *vconf = NULL;
#endif
	
    ARParam  wparam;
	int xsize, ysize;
	
    // Open the video path.
    if(arVideoOpen(vconf) < 0) return (FALSE);
	
    // Find the size of the window.
    if(arVideoInqSize(&xsize, &ysize) < 0) return (FALSE);
    fprintf(stderr, "demoARSetupCamera(): Image size (x,y) = (%d,%d)\n", xsize, ysize);
	
    // Set the initial camera parameters.
    if(arParamLoad(cparam_name, 1, &wparam) < 0) {
        fprintf(stderr, "demoARSetupCamera(): Camera parameter load error !!\n");
        return (FALSE);
    }
    arParamChangeSize(&wparam, xsize, ysize, cparam);
    arInitCparam(cparam);
    fprintf(stderr, "*** Camera Parameter ***\n");
    arParamDisp(cparam);
	
    arVideoCapStart();
	
	return (TRUE);
}

BOOL demoARSetupMarker(const unsigned char *patt_name, int *patt_id)
{
	
    if((*patt_id = arLoadPatt(patt_name)) < 0) {
        fprintf(stderr, "demoARSetupMarker(): pattern load error !!\n");
        return (FALSE);
    }
	
	return (TRUE);
}

// Report state of ARToolKit global variables arFittingMode,
// arImageProcMode, arglDrawMode, arTemplateMatchingMode, arMatchingPCAMode.
void demoARDebugReportMode(void)
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

void Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
		case 0x1B:						// Quit.
		case 'Q':
		case 'q':
			arglCleanup(gContextSettings);
			arVideoCapStop();
			arVideoClose();
			exit(0);
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
			fprintf(stderr, "*** %f (frame/sec)\n", (double)gMainCountCallsMarkerDetect/arUtilTimer());
			gMainCountCallsMarkerDetect = 0;
			demoARDebugReportMode();
			break;
			break;
		case 'R':
		case 'r':
			#ifdef AR_OPENGL_TEXTURE_RECTANGLE
			arglTexRectangle = !arglTexRectangle;
			fprintf(stderr, "Toggled arglTexRectangle to %d.\n", arglTexRectangle);
			#endif // AR_OPENGL_TEXTURE_RECTANGLE
			break;
		case '?':
		case '/':
			printf("Keys:\n");
			printf(" q or [esc]    Quit demo.\n");
			printf(" c             Change arglDrawMode and arglTexmapMode.\n");
			printf(" r             Toggle arglTexRectangle.\n");
			printf(" ? or /        Show this help.\n");
			printf(" ? or /        Show this help.\n");
			printf("\nAdditionally, the ARVideo library supplied the following help text:\n");
			arVideoDispOption();
			break;
		default:
			break;
	}
}

void Idle(void)
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
	if (s_elapsed < 0.01) return; // Don't update more often than 100 Hz.
	ms_prev = ms;
	
	// Update drawing.
	gDrawRotateAngle += s_elapsed * 5.0; // Rotate cube at 5 degrees per second.
	if (gDrawRotateAngle > 360.0) gDrawRotateAngle -= 360.0;
	
	// Grab a video frame.
	if((image = arVideoGetImage()) != NULL) {
		gARTImage = image;
		
		gMainCountCallsMarkerDetect++; // Increment ARToolKit FPS counter.
		
		// Detect the markers in the video frame.
		if(arDetectMarker(gARTImage, gARTThreshhold, &marker_info, &marker_num) < 0) {
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
		
		if(k != -1) {
			// Get the transformation between the marker and the real camera into gPatt_trans.
			arGetTransMat(&(marker_info[k]), gPatt_centre, gPatt_width, gPatt_trans);
			gPatt_found = TRUE;
		} else {
			gPatt_found = FALSE;
		}
		
		// Tell GLUT the display has changed.
		glutPostRedisplay();
	}
		
}

//
//	The function is called on events when the visibility of the
//	GLUT window changes (including when it first becomes visible).
//
void Visibility(int visible)
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
void Reshape(int w, int h)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Call through to anyone else who needs to know about window sizing here.
}

// Something to look at, draw a rotating colour cube.
void Draw(void)
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
	glRotatef(gDrawRotateAngle, 0.0, 0.0, 1.0); // Rotate about z axis.
	glDisable(GL_LIGHTING);	// Just use colours.
	glCallList(polyList);	// Draw the cube.
	glPopMatrix();	// Restore world coordinate system.

}

//
// The function is called when the window needs redrawing.
//
void Display(void)
{

    static GLdouble *p = NULL;
	GLdouble  m[16];
	
	if (gARTImage) {

		// Context setup.
		glDrawBuffer(GL_BACK);
		arglDispImage(gARTImage, &gARTCparam, 1.0, gContextSettings);	// zoom = 1.0.
	
		arVideoCapNext();
				
		if (gPatt_found) {
			glClear(GL_DEPTH_BUFFER_BIT);	// Clear the buffers for new frame.
			
			if (p == NULL) {
				p = (GLdouble *)malloc(16 * (sizeof(GLdouble)));
				arglCameraFrustum(&gARTCparam, VIEW_DISTANCE_MIN, VIEW_DISTANCE_MAX, p);
			}
			glMatrixMode(GL_PROJECTION);
			glLoadMatrixd(p);
			glMatrixMode(GL_MODELVIEW);
			
			// Load the camera transformation matrix.
			// ARToolKit supplied distance in millimetres, but I want OpenGL to work in metres.
			arglCameraView(gPatt_trans, m, MY_SCALEFACTOR);
			glLoadMatrixd(m);
			
			glTranslatef(0.0, 0.0, 0.5); // Place base of cube on marker surface.
			Draw();
			
			gPatt_found = FALSE;
		}
		
		glutSwapBuffers();
	}

}


int main(int argc, char** argv)
{
	const unsigned char *cparam_name    = MY_CPARAM;
	const unsigned char *patt_name      = MY_PATTERN;
	char glutGamemode[32];
	
	if (!demoARSetupCamera(cparam_name, &gARTCparam)) {
		fprintf(stderr, "main(): Unable to set up AR camera.\n");
		exit(-1);
	}
	demoARDebugReportMode();
	if (!demoARSetupMarker(patt_name, &gPatt_id)) {
		fprintf(stderr, "main(): Unable to set up AR marker.\n");
		exit(-1);
	}
	glutInit(&argc, argv);
	
	// Set up the OpenGL context.
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	if (!prefWindowed) {								// GLUT mono fullscreen.
		//sprintf(glutGamemode, "%ix%i:%i@%i", prefWidth, prefHeight, prefDepth, prefRefresh);
		sprintf(glutGamemode, "%ix%i", gARTCparam.xsize, gARTCparam.ysize);
		glutGameModeString(glutGamemode);
		glutEnterGameMode();
	} else {											// GLUT mono windowed.
		glutInitWindowSize(gARTCparam.xsize, gARTCparam.ysize);
		//glutInitWindowPosition(100, 100);
		glutCreateWindow(argv[0]);
	}

	// Setup argl library for current context.
	if ((gContextSettings = arglSetupForCurrentContext()) == NULL) {
		fprintf(stderr, "main(): arglSetupForCurrentContext() returned error.\n");
		exit(-1);
	}
		
	// Register GLUT event-handling callbacks.
	// NB: Idle() is registered by Visibility.
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutVisibilityFunc(Visibility);
	glutKeyboardFunc(Keyboard);
	
	glutMainLoop();

	return (0);
}
