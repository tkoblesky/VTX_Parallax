#ifndef _EVENT_H_
#define _EVENT_H_

#include<algorithm>

class event
{
public:
  void set_vtx_z(double z){vtx_z = z;}
  double get_vtx_z(){return vtx_z;}
  void generate_filter_tracks(int ntracks, unsigned int req_layers, vector< vector<sensor> > & detector);
  void get_tracks(vector<track> & a){for(unsigned int i = 0; i<tracks.size();i++) a.push_back(tracks[i]);}
  void set_random(TRandom3* rand_ptr);
private:
  double vtx_z;
  vector<track> tracks;
  TRandom3 *random;
};
			


// random generator function:
ptrdiff_t myrandom (ptrdiff_t i) { return rand()%i;}

// pointer object to it:
ptrdiff_t (*p_myrandom)(ptrdiff_t) = myrandom;
				   
  
void event::set_random(TRandom3* rand_ptr)
{
  random=rand_ptr;
}

void event::generate_filter_tracks(int ntracks, unsigned int req_layers, vector< vector<sensor> > & detector)
{
  double pi = acos(-1.0);
  vector<double> radii;
  radii.push_back(2.63);
  radii.push_back(5.13);
  radii.push_back(11.39);
  radii.push_back(16.34);

  vector<double> z_range;
  z_range.push_back(11.5);
  z_range.push_back(11.5);
  z_range.push_back(15.5);
  z_range.push_back(19.5);

  for(int itrack = 0; itrack<ntracks;itrack++)
    {
      //generate random straight track
      double phi = -pi/2. + random->Rndm()*(3.*pi/2.+pi/2.);
      double eta = -2.5 + random->Rndm()*(2*2.5);//throw eta tracks from -2.5 to 2.5
      double theta = 2.0*atan(exp(-eta));//calculate theta 

      //filter tracks that don't go through active area of detector
      vector<double> hit_rad;
      unsigned int hit_count = 0;

      //find where the track hit each layer
      for(int ilayer = 0; ilayer<4;ilayer++)
	{
	  bool is_hit = false;
	  
	  //loop through all the sensors in the layer until we find one that is hit
	  for(unsigned int isensor = 0; isensor<detector[ilayer].size(); isensor++)
	    {
	      //calculate using the sensor radii
	      double radius = detector[ilayer][isensor].get_radius();
	      double z = radius / tan(theta) + vtx_z;
	      
	      if(detector[ilayer][isensor].is_sensor_hit(z,phi))
		{
		  is_hit = true;
		  hit_rad.push_back(radius);
		  hit_count++;
		  break;//once we find a hit in the layer stop searching
		}
	    }
	  if(!is_hit)
	    hit_rad.push_back(-1.0);//mark the layer as not being hit
	}
      if(hit_count < req_layers)//if it doesn't go through enough layers, drop the track
	{
	  --itrack;
	  continue;
	}
      track tr;
      tr.set_phi(phi);
      tr.set_theta(theta);
      tr.set_hit_rad(hit_rad);
      tracks.push_back(tr);
    }

  
  // random_shuffle (tracks.begin(),tracks.end(),p_myrandom );//randomly shuffle the tracks
}


#endif //_EVENT_H_
