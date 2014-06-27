#pragma once

#include <math.h>
#include <iostream>
#include <fstream>
#include <gl\freeglut.h>		// OpenGL header files
#include "KinectBasic.h"
#include "QueryTimeCheck.h"
#include <list>
#define TIME_CHECK_
#pragma warning(disable:4996)
#define BLOCK 4
#define READ_SIZE 110404

using namespace std;


struct Vertex{
	float X;
	float Y;
	float Z;
};

// variables for GUI
const float TRACKBALLSIZE = 0.8f;
const int RENORMCOUNT = 97;
const int width = KinectBasic::nColorWidth;
const int height = KinectBasic::nColorHeight;

GLint drag_state = 0;
GLint button_state = 0;

GLint rot_x = 0;
GLint rot_y = 0;
GLint trans_x = 0;
GLint trans_y = 0;
GLint trans_z = 0;

float quat[4] = {0};
float t[3] = {0};

Vertex *vertex;

float vert[1080][1920][3];

bool recheck;
bool oM;

// variables for display OpenGL based point viewer
int dispWindowIndex = 0;
GLuint dispBindIndex = 0;
const float dispPointSize = 2.0f;

// variables for display text
string dispString = "";
const string dispStringInit = "Depth Threshold: D\nInfrared Threshold: I\nNonlocal Means Filter: N\nPick BodyIndex: P\nAccumulate Mode: A\nSelect Mode: C,B(select)\nSave: S\nReset View: R\nQuit: ESC";
string frameRate;

HANDLE hMutex;
KinectBasic kinect;

// functions for GUIs
void InitializeTextureInfo();
void InitializeWindow();
void InitializeWindow(int argc, char* argv[]);

// high-level functions for GUI
void draw_center();
void idle();
void display();
void close();
void special(int, int, int) {}
void keyboard(unsigned char, int, int);
void mouse(int, int, int, int);
void reshape(int, int);
void motion(int, int);


// basic functions for computation/GUI
// trackball codes were imported from those of Gavin Bell
// which appeared in SIGGRAPH '88
void vzero(float*);
void vset(float*, float, float, float);
void vsub(const float*, const float*, float*);
void vcopy(const float*, float*);
void vcross(const float *v1, const float *v2, float *cross);
float vlength(const float *v);
void vscale(float *v, float div);
void vnormal(float *v);
float vdot(const float *v1, const float *v2);
void vadd(const float *src1, const float *src2, float *dst);

void trackball(float q[4], float, float, float, float);
void add_quats(float*, float*, float*);
void axis_to_quat(float a[3], float phi, float q[4]);
void normalize_quat(float q[4]);
float tb_project_to_sphere(float, float, float);
void build_rotmatrix(float m[4][4], float q[4]);
void Reader();
void DrawObj();

/*
 * (c) Copyright 1993, 1994, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 * Permission to use, copy, modify, and distribute this software for
 * any purpose and without fee is hereby granted, provided that the above
 * copyright notice appear in all copies and that both the copyright notice
 * and this permission notice appear in supporting documentation, and that
 * the name of Silicon Graphics, Inc. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.
 *
 * THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS"
 * AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR
 * FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL SILICON
 * GRAPHICS, INC.  BE LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT,
 * SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY
 * KIND, OR ANY DAMAGES WHATSOEVER, INCLUDING WITHOUT LIMITATION,
 * LOSS OF PROFIT, LOSS OF USE, SAVINGS OR REVENUE, OR THE CLAIMS OF
 * THIRD PARTIES, WHETHER OR NOT SILICON GRAPHICS, INC.  HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
 * POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * US Government Users Restricted Rights
 * Use, duplication, or disclosure by the Government is subject to
 * restrictions set forth in FAR 52.227.19(c)(2) or subparagraph
 * (c)(1)(ii) of the Rights in Technical Data and Computer Software
 * clause at DFARS 252.227-7013 and/or in similar or successor
 * clauses in the FAR or the DOD or NASA FAR Supplement.
 * Unpublished-- rights reserved under the copyright laws of the
 * United States.  Contractor/manufacturer is Silicon Graphics,
 * Inc., 2011 N.  Shoreline Blvd., Mountain View, CA 94039-7311.
 *
 * OpenGL(TM) is a trademark of Silicon Graphics, Inc.
 */


/*
 * Trackball code:
 *
 * Implementation of a virtual trackball.
 * Implemented by Gavin Bell, lots of ideas from Thant Tessman and
 *   the August '88 issue of Siggraph's "Computer Graphics," pp. 121-129.
 *
 * Vector manip code:
 *
 * Original code from:
 * David M. Ciemiewicz, Mark Grossman, Henry Moreton, and Paul Haeberli
 *
 * Much mucking with by:
 * Gavin Bell
 */