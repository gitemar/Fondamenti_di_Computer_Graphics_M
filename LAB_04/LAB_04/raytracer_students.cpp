#include "raytracer.h"
#include "material.h"
#include "vectors.h"
#include "argparser.h"
#include "raytree.h"
#include "utils.h"
#include "mesh.h"
#include "face.h"
#include "sphere.h"

// casts a single ray through the scene geometry and finds the closest hit
bool
RayTracer::CastRay (Ray & ray, Hit & h, bool use_sphere_patches) const
{
  bool answer = false;
  Hit nearest;
  nearest = Hit();

  // intersect each of the quads
  for (int i = 0; i < mesh->numQuadFaces (); i++)
  {
	Face *f = mesh->getFace (i);
	if (f->intersect (ray, h, args->intersect_backfacing))
	{
		if( h.getT() < nearest.getT() )
		{
			answer = true;
			nearest = h;
		}
	}
  }

  // intersect each of the spheres (either the patches, or the original spheres)
  if (use_sphere_patches)
  {
	for (int i = mesh->numQuadFaces (); i < mesh->numFaces (); i++)
	{
	  Face *f = mesh->getFace (i);
	  if (f->intersect (ray, h, args->intersect_backfacing))
	  {
		if( h.getT() < nearest.getT() )
		{
			answer = true;
			nearest = h;
		}
	  }
	}
  }
  else
  {
	const vector < Sphere * >&spheres = mesh->getSpheres ();
	for (unsigned int i = 0; i < spheres.size (); i++)
	{
	  if (spheres[i]->intersect (ray, h))
	  {
		if( h.getT() < nearest.getT() )
		{
			answer = true;
			nearest = h;
		}
	  }
	}
  }

  h = nearest;
  return answer;
}

Vec3f
RayTracer::TraceRay (Ray & ray, Hit & hit, int bounce_count) const
{

  hit = Hit ();
  bool intersect = CastRay (ray, hit, false);

  if( bounce_count == args->num_bounces )
  	RayTree::SetMainSegment (ray, 0, hit.getT () );
  else
	RayTree::AddReflectedSegment(ray, 0, hit.getT() );

  Vec3f answer = args->background_color;

  Material *m = hit.getMaterial ();
  if (intersect == true)
  {

	assert (m != NULL);
	Vec3f normal = hit.getNormal ();
	Vec3f point = ray.pointAtParameter (hit.getT ());

	// ----------------------------------------------
	// ambient light
	answer = args->ambient_light * m->getDiffuseColor ();


	// ----------------------------------------------
	// if the surface is shiny...
	Vec3f reflectiveColor = m->getReflectiveColor ();

	// ==========================================
	// ASSIGNMENT:  ADD REFLECTIVE LOGIC
	// ==========================================
	
	// se (il punto sulla superficie e' riflettente & bounce_count>0)
	//per vedere se è riflettente, la norma non sarà zero
	if (reflectiveColor.Length() != 0 && bounce_count > 0) {

		Vec3f rayVector = ray.getDirection();
		Vec3f reflection = rayVector - (2 * rayVector.Dot3(normal) * normal);
		reflection.Normalize();
		Ray* newRay = new Ray(point, reflection);

		//     aggiungere ad answer il contributo riflesso
		//	   invocare TraceRay(ReflectionRay, hit,bounce_count-1)
		answer += TraceRay(*newRay, hit, bounce_count - 1) * reflectiveColor;
	}

	// ----------------------------------------------
	// add each light
	int num_lights = mesh->getLights ().size ();
	Hit* newHit;
	bool colpito;
	Ray* nRay;
	Vec3f n_point, dista, pti[10], pointOnLight, dirToLight;

	for (int i = 0; i < num_lights; i++)
	{
	  // ==========================================
	  // ASSIGNMENT:  ADD SHADOW LOGIC
	  // ==========================================
	  Face *f = mesh->getLights ()[i];
	  extern bool softShadow;

	  if (args->softShadow) {
		  //calcolo soft shadows - parametro soft_shadows presente a linea di comando
		  int lightPointsMAX = 40;
		  for (int i = 0; i < lightPointsMAX; i++) {
			  newHit = new Hit();
			  pointOnLight = f->RandomPoint();
			  dirToLight = pointOnLight - point;
			  dirToLight.Normalize();

			  nRay = new Ray(point, dirToLight);
			  colpito = CastRay(*nRay, *newHit, 0);

			  if (colpito) {
				  n_point = nRay->pointAtParameter(newHit->getT());
				  //calcolo distanza (vettore) tra pto colpito dal raggio e pto sulla vista
				  dista.Sub(dista, n_point, pointOnLight);

				  // se e' la sorgente luminosa i-esima allora
				  if (dista.Length() < 0.01 && normal.Dot3(dirToLight) > 0) {
					  //	calcolare e aggiungere ad answer il contributo luminoso
					  Vec3f lColor = 0.2 * f->getMaterial()->getEmittedColor() * f->getArea();
					  answer += (m->Shade(ray, hit, dirToLight, lColor, args)) * (1.0/(float)lightPointsMAX);
				  }
				  // altrimenti
				  // la luce i non contribuisce alla luminosita' di point
			  }
		  }
	  }
	  else {
		  //calcolo hard shadows

		  //calcolo la direzione del raggio verso la luce
		  pointOnLight = f->computeCentroid();
		  dirToLight = pointOnLight - point;
		  dirToLight.Normalize();

		  // creare shadow ray verso il punto luce
		  nRay = new Ray(point, dirToLight);
		  newHit = new Hit();
		  colpito = CastRay(*nRay, *newHit, false);

		  // controllare il primo oggetto colpito da tale raggio
		  if (colpito) {
			  n_point = nRay->pointAtParameter(newHit->getT());
			  //calcolo distanza (vettore) tra pto colpito dal raggio e pto sulla vista
			  dista.Sub(dista, n_point, pointOnLight);

			  // se e' la sorgente luminosa i-esima allora
			  if (dista.Length() < 0.01 && normal.Dot3(dirToLight) > 0){
				  //	calcolare e aggiungere ad answer il contributo luminoso
				  Vec3f lColor = 0.2 * f->getMaterial()->getEmittedColor() * f->getArea();
				  answer += m->Shade(ray, hit, dirToLight, lColor, args);
			  }
			  // altrimenti
			  // la luce i non contribuisce alla luminosita' di point
		  }

	  }
     

	  if (normal.Dot3 (dirToLight) > 0)
	  {
		Vec3f lightColor = 0.2 * f->getMaterial ()->getEmittedColor () * f->getArea ();
		answer += m->Shade (ray, hit, dirToLight, lightColor, args);
	  }
	}
    
  }

  return answer;
}
