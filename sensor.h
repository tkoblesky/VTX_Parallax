#ifndef _SENSOR_H_
#define _SENSOR_H_

class sensor
{
public:
  void set_sensor(double x1, double y1, double x2, double y2, double rad);
  bool is_sensor_hit(double x, double y);
  double get_radius(){return radius;}
  TGraph * get_graph();
  TGraph * get_dead_graph(int chip);
  void set_dead_chips(short,short,short,short);
private:
  double _x1, _y1, _x2, _y2, radius;
  vector<short> RO_chips_out;
};

void sensor::set_sensor(double x1, double y1, double x2, double y2, double rad)
{
  _x1 = x1;
  _x2 = x2;
  _y1 = y1;
  _y2 = y2;
  radius = rad;
  set_dead_chips(1,1,1,1);
}

bool sensor::is_sensor_hit(double x, double y)
{
  double z_length = _x2 - _x1;
  if(x < _x2 && x > _x1 && y < _y2 && y > _y1 
     && ((z_length/4. + _x1 > x && RO_chips_out[0] == 1)  
	 || (z_length/2. + _x1 > x && z_length/4. + _x1 <= x && RO_chips_out[1] == 1)
	 || (3.*z_length/4. + _x1 > x && z_length/2. + _x1 <= x && RO_chips_out[2] == 1)
	 || (3.*z_length/4. + _x1 <= x && RO_chips_out[3] == 1)))
    return true;
  else 
    return false;
}

TGraph * sensor::get_graph()
{
  float slopex = (_x2-_x1)/99;
  float slopey = (_y2-_y1)/99;
  double x[400];
  double y[400];
  for(int i = 0; i<400;i++)
    {
      if(i<100)
	{
	  x[i] = _x1+i*slopex;
	  y[i] = _y2;
	}
      else if(i<200)
	{
	  x[i] = _x2;
	  y[i] = _y2-(i-100)*slopey;
	}
      else if(i<300)
	{
	  x[i] = _x2-(i-200)*slopex;
	  y[i] = _y1;
	}
      else 
	{
	  x[i] = _x1;
	  y[i] = _y1+(i-300)*slopey;
	}
    }
  TGraph *graph = new TGraph(400,x,y);
  return graph;
}

TGraph * sensor::get_dead_graph(int chip)
{
  TGraph *graph;
  if(RO_chips_out[chip]==1)
    {
      graph = NULL;
    }
  else
    {
      double xlength = ( _x2 - _x1)/4.;
      double x[200], y[200];
      double slopex = xlength/49;
      double slopey = (_y2 - _y1)/49;
      for(int i = 0; i <200; i++)
	{
	  if(i<50)
	    {
	      x[i] = _x1 + xlength * chip + i * slopex;
	      y[i] = _y2 - i * slopey;
	    }
	  else if(i<100)
	    {
	      x[i] = _x1 + xlength * (chip+1);
	      y[i] = _y1 + (i-50) * slopey;
	    }
	  else if(i<150)
	    {
	      x[i] = _x1 + xlength * (chip+1) - (i-100) * slopex;
	      y[i] = _y2 - (i-100) * slopey;
	    }
	  else 
	    {
	      x[i] = _x1 + xlength * chip;
	      y[i] = _y1 + (i-150) * slopey;
	    }
	}
      graph = new TGraph(200,x,y);
    }
  return graph;
}

void sensor::set_dead_chips(short l1, short l2, short l3, short l4)
{
  RO_chips_out.clear();
  RO_chips_out.push_back(l1);
  RO_chips_out.push_back(l2);
  RO_chips_out.push_back(l3);
  RO_chips_out.push_back(l4);
}

#endif //_SENSOR_H_
