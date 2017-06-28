#ifndef _TRACK_H_
#define _TRACK_H_
class track
{
public:
  void set_phi(double ph){phi=ph;}
  double get_phi(){return phi;}
  void set_theta(double the){theta = the;}
  double get_theta(){return theta;}
  void set_vtx_z(double z){vtx_z = z;}
  double get_vtx_z(){return vtx_z;}
  void set_hit_rad(vector<double> a){hit_rad = a;}
  vector<double> get_hit_rad(){return hit_rad;}
private:
  double phi;
  double theta;
  double vtx_z;
  vector<double> hit_rad;
};
#endif //_TRACK_H_
