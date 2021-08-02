/*==================================================================================
* COSC 363  Computer Graphics (2020)
* Department of Computer Science and Software Engineering, University of Canterbury.
*
* A basic ray tracer
* See Lab07.pdf, Lab08.pdf for details.
*===================================================================================
*/
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "SceneObject.h"
#include "Ray.h"
#include <GL/freeglut.h>
#include "Plane.h"

using namespace std;

const float WIDTH = 20.0;
const float HEIGHT = 20.0;
const float EDIST = 40.0;
const int NUMDIV = 500;
const int MAX_STEPS = 5;
const float XMIN = -WIDTH * 0.5;
const float XMAX =  WIDTH * 0.5;
const float YMIN = -HEIGHT * 0.5;
const float YMAX =  HEIGHT * 0.5;

vector<SceneObject*> sceneObjects;


//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{
	glm::vec3 backgroundCol(0);						//Background colour = (0,0,0)
	glm::vec3 lightPos(10, 40, -3);					//Light's position
    glm::vec3 light2(30, 40, 0);
    glm::vec3 ambientCol(0.2);
    glm::vec3 color(0);
	SceneObject* obj;



    ray.closestPt(sceneObjects);					//Compare the ray with all objects in the scene
    if(ray.index == -1) return backgroundCol;		//no intersection
	obj = sceneObjects[ray.index];					//object on which the closest point of intersection is found

    color = obj->lighting(lightPos, -ray.dir, ray.hit);			//Object's colour
    glm::vec3 normalVector = obj->normal(ray.hit);

    glm::vec3 lightVec = lightPos - ray.hit;
    glm::vec3 lightVec2 = light2 - ray.hit;
    glm::vec3 lightUnit = glm::normalize(lightVec);
    glm::vec3 lightUnit2 = glm::normalize(lightVec2);

    glm::vec3 reflVector = glm::reflect(-lightUnit, normalVector);
    glm::vec3 reflVector2 = glm::reflect(-lightUnit2, normalVector);

    float lDotn = glm::dot(lightUnit, normalVector);
    float lDotn2 = glm::dot(lightUnit2, normalVector);

    float refV = glm::dot(reflVector, -ray.dir);
    float refV2 = glm::dot(reflVector2, -ray.dir);

    //procedural pattern used on sphere
    if (ray.index == 2 && step < MAX_STEPS) {
        glm::vec3 color1 = glm::vec3(0.9,0,0);
        glm::vec3 color2 = glm::vec3(0.0,0.4,0.0);
        glm::vec3 color3 = glm::vec3(0.8,0.8,0.2);
        float function = fmod((ray.hit.y + ray.hit.x), 3);
        if (function <= 1.0) {
            color = color1;
        } else if (function <= 2.0 && function > 1.0){
            color = color2;
        } else {
            color = color3;
        }
    }

    if (ray.index == 3 && step < MAX_STEPS) {

       glm::vec3 normalVector = obj->normal(ray.hit);
       float eta = 1/1.5;
       glm::vec3 refractedDir = glm::refract(ray.dir, normalVector, eta);
       Ray refractedRay1(ray.hit, refractedDir);
       refractedRay1.closestPt(sceneObjects);
       glm::vec3 m = obj->normal(refractedRay1.hit);
       glm::vec3 refractedDir2 = glm::refract(refractedDir, -m, 1.0f/eta);
       Ray refractedRay2(refractedRay1.hit, refractedDir2);
       refractedRay2.closestPt(sceneObjects);
       glm::vec3 refractedCol = trace(refractedRay2, step+1);

       color = (color * 0.2f) + (refractedCol * 0.8f);
    }

    //reflective sphere
    if (obj->isReflective() && step < MAX_STEPS) {
        float rho = obj->getReflectionCoeff();
        glm::vec3 normalVec = obj->normal(ray.hit);
        glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
        Ray reflectedRay(ray.hit, reflectedDir);
        glm::vec3 reflectedColor = trace(reflectedRay, step + 1);
        color = color + (rho * reflectedColor);
    }

    //procedural pattern used on floor
    if(ray.index == 4 && step < MAX_STEPS){
         int crossx = (int)((ray.hit.x + 50) /8) % 2;
         int crossz = (int)((ray.hit.z + 200) /8) % 2;

        if((crossx && crossz) || (!crossx && !crossz)){
            color = glm::vec3(0.2,0.2,0.2);}
        else{
            color = glm::vec3(1,1,1);}
     }

    //procedural pattern used on back wall
    if(ray.index == 5 && step < MAX_STEPS)
    {
        if (((-int(ray.hit.x) - int(ray.hit.y)) % 2 == 1) or ((int(ray.hit.x) + int(ray.hit.y)) % 2 == 1)){
            color = glm::vec3(0.6,0.2,0.9);
        }
        else color = glm::vec3(0.4, 0.5, 0.2);
    }

    Ray shadowRay(ray.hit, lightVec);
    shadowRay.closestPt(sceneObjects);

    float spec = 0.0;
    if (refV > 0) {
        spec = pow(refV, 10);
    }
    if (ray.index == 6) {
        spec = 0;
    }
    ambientCol += spec;

    if (lDotn <= 0 ||
            (shadowRay.index > -1 && shadowRay.dist < glm::length(lightVec)) || ray.index == 5) {
        color = ambientCol*color;
    } else {
        color = ambientCol*color + (lDotn*color + spec);
    }

    Ray shadow2(ray.hit, lightUnit2);
    shadow2.closestPt(sceneObjects);

    float spec2 = 0.0;
    if (refV2 > 0) {
        spec2 = pow(refV2, 10);
    }
    if (ray.index == 5) {
        spec2 = 0;
    }
    ambientCol += spec2;

    if (lDotn2 <= 0 ||
            (shadow2.index > -1 && shadow2.dist < glm::length(lightVec2))) {
        color = ambientCol*color;
    } else {
        color = ambientCol*color + (lDotn2*color + spec2);
    }

    if (ray.index == 6 && step < MAX_STEPS) {
        Ray transRay(ray.hit, ray.dir);
        glm::vec3 transColor = trace(transRay, step+1);
        color += 0.3f*color + 0.7f*transColor;
    }

	return color;
}


glm::vec3 antiAliasing(glm::vec3 eye, float pixel_size, float xp, float yp){

    float pix_quart = pixel_size * 0.25;
    float pix_3qaurt = pixel_size * 0.75;
    glm::vec3 color(0);
    glm::vec3 avg(0.25);

    Ray ray = Ray(eye, glm::vec3(xp + pix_quart, yp + pix_quart, -EDIST));
    color += trace(ray, 1);

    ray = Ray(eye, glm::vec3(xp + pix_quart, yp + pix_3qaurt, -EDIST));
    color += trace(ray, 1);

    ray = Ray(eye, glm::vec3(xp + pix_3qaurt, yp + pix_quart, -EDIST));
    color += trace(ray, 1);

    ray = Ray(eye, glm::vec3(xp + pix_3qaurt, yp + pix_3qaurt, -EDIST));
    color += trace(ray, 1);

    color *= avg;

    return color;


}

void drawCube(float length, float width, float height, float x, float y, float z) {
    glm::vec3 a = glm::vec3(x, y, z);
    glm::vec3 b = glm::vec3(x + length, y, z);
    glm::vec3 c = glm::vec3(x + length, y + height, z);
    glm::vec3 d = glm::vec3(x, y + height, z);
    glm::vec3 e = glm::vec3(x + length, y, z - width);
    glm::vec3 f = glm::vec3(x + length, y + height, z - width);
    glm::vec3 g = glm::vec3(x, y + height, z - width);
    glm::vec3 h = glm::vec3(x, y, z - width);

    Plane *front = new Plane(a, b, c, d);
    front->setColor(glm::vec3(0, 0, 1));
    Plane *right = new Plane(b, e, f, c);
    right->setColor(glm::vec3(0, 0, 1));
    Plane *back = new Plane(e, h, g, f);
    back->setColor(glm::vec3(0, 0, 1));
    Plane *sideFour = new Plane(d, g, h, a);
    sideFour->setColor(glm::vec3(0, 0, 1));
    Plane *bottom = new Plane(d, c, f, g);
    bottom->setColor(glm::vec3(0, 0, 1));
    Plane *top = new Plane(h, e, b, a);
    top->setColor(glm::vec3(0, 0, 1));



    sceneObjects.push_back(front);
    sceneObjects.push_back(right);
    sceneObjects.push_back(back);
    sceneObjects.push_back(sideFour);
    sceneObjects.push_back(bottom);
    sceneObjects.push_back(top);
}

//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
void display()
{
	float xp, yp;  //grid point
	float cellX = (XMAX-XMIN)/NUMDIV;  //cell width
	float cellY = (YMAX-YMIN)/NUMDIV;  //cell height
	glm::vec3 eye(0., 0., 0.);

	glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glBegin(GL_QUADS);  //Each cell is a tiny quad.

	for(int i = 0; i < NUMDIV; i++)	//Scan every cell of the image plane
	{
		xp = XMIN + i*cellX;
		for(int j = 0; j < NUMDIV; j++)
		{
			yp = YMIN + j*cellY;

		    glm::vec3 dir(xp+0.5*cellX, yp+0.5*cellY, -EDIST);	//direction of the primary ray

		    Ray ray = Ray(eye, dir);

            glm::vec3 col = antiAliasing(eye,cellX,xp,yp); //Anti-aliasing


            //glm::vec3 col = trace (ray, 1); //Trace the primary ray and get the colour value
			glColor3f(col.r, col.g, col.b);
            glVertex2f(xp, yp);
			glVertex2f(xp+cellX, yp);
			glVertex2f(xp+cellX, yp+cellY);
			glVertex2f(xp, yp+cellY);
        }
    }

    glEnd();
    glFlush();
}



//---This function initializes the scene ------------------------------------------- 
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL orthographc projection matrix for drawing the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize()
{
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(XMIN, XMAX, YMIN, YMAX);

    glClearColor(0, 0, 0, 1);

    //index 0
    Sphere *sphere1 = new Sphere(glm::vec3(-5.0, 0.0, -90.0), 15.0);
    sphere1->setColor(glm::vec3(0, 0, 1));
    sceneObjects.push_back(sphere1);
    sphere1->setShininess(25);
    sphere1->setReflectivity(true, 0.8);

    //index 1
    Sphere *sphere2 = new Sphere(glm::vec3(5.0, -10.0, -60.0), 5.0);
    sphere2->setColor(glm::vec3(0, 1, 0));
    sceneObjects.push_back(sphere2);
    sphere2->setShininess(25);
    sphere2->setReflectivity(true, 0.3);


    //index 2
    Sphere *sphere3 = new Sphere(glm::vec3(5.0, 5.0, -70.0), 4.0);
    sphere3->setColor(glm::vec3(1, 0, 0));
    sceneObjects.push_back(sphere3);
    sphere3->setShininess(25);
    sphere3->setReflectivity(true, 0.3);

    //index 3
    Sphere *sphere4 = new Sphere(glm::vec3(10.0, 10.0, -60.0), 3.0);
    sphere4->setColor(glm::vec3(1, 1, 1));
    sphere4->setShininess(25);
    sceneObjects.push_back(sphere4);

    //index 4
    Plane *plane = new Plane (glm::vec3(-50., -20, -40),
                              glm::vec3(50., -20, -40),
                              glm::vec3(50., -20, -200),
                              glm::vec3(-50., -20, -200));
    plane->setColor(glm::vec3(0.8));
    sceneObjects.push_back(plane);
    plane->setSpecularity(true);
    plane->setShininess(true);

    //index 5
    Plane *wall = new Plane(glm::vec3(-50., -20, -200),
                                glm::vec3(50., -20, -200),
                                glm::vec3(50., 50, -200),
                                glm::vec3(-50., 50, -200));
    wall->setColor(glm::vec3(0.2,0.2,0.2));
    sceneObjects.push_back(wall);

    //index 6
    Sphere *sphere5 = new Sphere(glm::vec3(-10.0, 10.0, -70.0), 2.0);
    sphere5->setColor(glm::vec3(0.2, 0.4, 0.9));
    sceneObjects.push_back(sphere5);

    drawCube(5, 5, 5, 10, -5, -70);

}


int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("Raytracing");

    glutDisplayFunc(display);
    initialize();

    glutMainLoop();
    return 0;
}
