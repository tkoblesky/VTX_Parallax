
int get_dead_config_l0(int ladder, int sensor)
{
  short l1=1;
  short l2=1;
  short l3=1;
  short l4=1;

  if(ladder == 0)//first west arm
    {
      if(sensor==2)
	{
	  l1 = 0;
	  l3 = 0;
	}
    }
  if(ladder==1)
    {
      if(sensor==0)
	{
	  l4 = 0;
	}
      if(sensor==2)
	{
	  l1 = 0;
	  l2 = 0;
	}
      if(sensor==3)
	{
	  l4 = 0;
	}
    }
  if(ladder==2)
    {
      if(sensor==0)
	{
	  l4 = 0;
	}
      if(sensor==1)
	{
	  l3 = 0;
	}
      if(sensor==2)
	{
	  l3 = 0;
	}
      if(sensor==3)
	{
	  l1 = 0;
	}
    }
  if(ladder==3)
    {
      if(sensor==1)
	{
	  l3 = 0;
	}
      if(sensor==3)
	{
	  l2 = 0;
	  l4 = 0;
	}
    }
  if(ladder==4)
    {
      if(sensor==1)
	{
	  l1 = 0;
	}
      if(sensor==2)
	{
	  l1 = 0;
	  l2 = 0;
	  l3 = 0;
	  l4 = 0;
	}
      if(sensor==3)
	{
	  l1 = 0;
	  l2 = 0;
	  l3 = 0;
	  l4 = 0;
	}
    }
  if(ladder==5)//now east arm
    {
      if(sensor==0)
	{
	  l1 = 0;
	  l2 = 0;
	  l3 = 0;
	  l4 = 0;
	}
      if(sensor==1)
	{
	  l1 = 0;
	  l2 = 0;
	  l3 = 0;
	  l4 = 0;
	}
      if(sensor==2)
	{
	  l3 = 0;
	}
      if(sensor==3)
	{
	  l3 = 0;
	  l4 = 0;
	}
    }
  if(ladder==6)
    {
      if(sensor==0)
	{
	  l1 = 0;
	  l2 = 0;
	  l3 = 0;
	  l4 = 0;
	}
      if(sensor==3)
	{
	  l1 = 0;
	}
    }
  if(ladder==7)
    {
      if(sensor==0)
	{
	  l1 = 0;
	}
    }
  if(ladder==8)
    {
      if(sensor==0)
	{
	  l1 = 0;
	  l3 = 0;
	}
      if(sensor==1)
	{
	  l3 = 0;
	  l4 = 0;
	}
      if(sensor==2)
	{
	  l2 = 0;
	}
      if(sensor==3)
	{
	  l1 = 0;
	  l2 = 0;
	}
    }
  if(ladder==9)
    {
      if(sensor==1)
	{
	  l2 = 0;
	  l4 = 0;
	}
      if(sensor==2)
	{
	  l2 = 0;
	}
      if(sensor==3)
	{
	  l3 = 0;
	}
    }

  int encoded = 8*l4+4*l3+2*l2+l1;
  return encoded;
}

int get_dead_config_l1(int ladder, int sensor)
{
  short l1=1;
  short l2=1;
  short l3=1;
  short l4=1;
  
  if(ladder == 0)//first west arm
    {
      if(sensor==0)
	{
	  l1=0;
	  l2=0;
	  l3=0;
	  l4=0;
	}
      if(sensor==1)
	{
	  l1=0;
	  l2=0;
	  l3=0;
	  l4=0;
	}
    }
  if(ladder==1)
    {
      if(sensor==2)
	{
	  l1 = 0;
	}
    }
  if(ladder==2)
    {
      if(sensor==0)
	{
	  l1 = 0;
	}
      if(sensor==2)
	{
	  l1 = 0;
	  l2 = 0;
	}
    }
  if(ladder==3)
    {
      if(sensor==0)
	{
	  l1 = 0;
	  l2 = 0;
	}
      if(sensor==1)
	{
	  l2 = 0;
	  l4 = 0;
	}
      if(sensor==2)
	{
	  l1 = 0;
	}
      if(sensor==3)
	{
	  l4 = 0;
	}
    }
  if(ladder==4)
    {
      if(sensor==3)
	{
	  l1 = 0;
	  l3 = 0;
	  l4 = 0;
	}
    }
  if(ladder==5)
    {
      if(sensor==0)
	{
	  l1 = 0;
	  l2 = 0;
	  l3 = 0;
	  l4 = 0;
	}
      if(sensor==1)
	{
	  l1 = 0;
	  l2 = 0;
	  l3 = 0;
	  l4 = 0;
	}
      if(sensor==3)
	{
	  l3 = 0;
	  l4 = 0;
	}
    }
  if(ladder==6)
    {
      if(sensor == 2)
	{
	  l3 = 0;
	  l4 = 0;
	}
      if(sensor == 3)
	{
	  l1 = 0;
	  l2 = 0;
	  l3 = 0;
	  l4 = 0;
	}
    }
  if(ladder==7)
    {
      if(sensor == 2)
	{
	  l3 = 0;
	  l4 = 0;
	}
      if(sensor == 3)
	{
	  l1 = 0;
	  l2 = 0;
	  l3 = 0;
	  l4 = 0;
	}
    }

  if(ladder==8)
    {
      if(sensor==2)
	{
	  l4 = 0;
	}
    }
  if(ladder==9)
    {
      if(sensor==0)
	{
	  l2 = 0;
	  l3 = 0;
	}
    }
  if(ladder==10)//east arm
    {
      if(sensor==0)
	{
	  l1 = 0;
	  l4 = 0;
	}
      if(sensor==1)
	{
	  l1 = 0;
	  l4 = 0;
	}
      if(sensor==2)
	{
	  l4 = 0;
	}
      if(sensor==3)
	{
	  l1 = 0;
	  l2 = 0;
	  l3 = 0;
	  l4 = 0;
	}
    }
  if(ladder==11)
    {
      if(sensor==0)
	{
	  l1 = 0;
	  l2 = 0;
	}
      if(sensor==1)
	{
	  l1 = 0;
	  l2 = 0;
	  l4 = 0;
	}
      if(sensor==2)
	{
	  l1 = 0;
	  l2 = 0;
	  l4 = 0;
	}
      if(sensor==3)
	{
	  l1 = 0;
	  l2 = 0;
	  l3 = 0;
	  l4 = 0;
	}
    }
  if(ladder==12)
    {
      if(sensor==1)
	{
	  l4 = 0;
	}
    }
  if(ladder==13)
    {
      if(sensor==0)
	{
	  l2 = 0;
	  l4 = 0;
	}
      if(sensor==1)
	{
	  l2 = 0;
	  l4 = 0;
	}
      if(sensor==2)
	{
	  l3 = 0;
	}
    }
  if(ladder==14)
    {
      if(sensor==0)
	{
	  l1 = 0;
	  l2 = 0;
	  l3 = 0;
	  l4 = 0;
	}
      if(sensor==1)
	{
	  l1 = 0;
	  l2 = 0;
	  l3 = 0;
	  l4 = 0;
	}
    }
  if(ladder==15)
    {
    }
  if(ladder==16)
    {
      if(sensor==0)
	{
	  l1 = 0;
	  l2 = 0;
	  l3 = 0;
	  l4 = 0;
	}
      if(sensor==1)
	{
	  l1 = 0;
	  l2 = 0;
	  l3 = 0;
	  l4 = 0;
	}
      if(sensor==2)
	{
	  l2 = 0;
	}
      if(sensor==3)
	{
	  l1 = 0;
	  l3 = 0;
	}
    }
  if(ladder==17)
    {
      if(sensor==0)
	{
	  l2 = 0;
	  l4 = 0;
	}
      if(sensor==1)
	{
	  l2 = 0;
	  l4 = 0;
	} 
      if(sensor==2)
	{
	  l1 = 0;
	  l3 = 0;
	  l4 = 0;
	}
      if(sensor==3)
	{
	  l4 = 0;
	}
    }
  if(ladder==18)
    {
      if(sensor==0)
	{
	  l3 = 0;
	  l4 = 0;
	}
      if(sensor==1)
	{
	  l1 = 0;
	  l2 = 0;
	  l3 = 0;
	  l4 = 0;
	}
      if(sensor==2)
	{
	  l1 = 0;
	}
      if(sensor==3)
	{
	  l2 = 0;
	  l3 = 0;
	  l4 = 0;
	}
    }
  if(ladder==19)
    {
      if(sensor==0)
	{
	  l2 = 0;
	  l3 = 0;
	  l4 = 0;
	}
      if(sensor==1)
	{
	  l1 = 0;
	  l2 = 0;
	  l3 = 0;
	  l4 = 0;
	}
      if(sensor==3)
	{
	  l1 = 0;
	}
    }

  int encoded = 8*l4+4*l3+2*l2+l1;
  return encoded;
}

int get_dead_config_l2(int ladder, int sensor)
{
  short l1=1;
  short l2=1;
  short l3=1;
  short l4=1;
  
  if(ladder==0)//west arm first
    {
      l1 = 0;
      l2 = 0;
      l3 = 0;
      l4 = 0;
    }
  if(ladder==1)
    {
    }
  if(ladder==2)
    {
    }
  if(ladder==3)
    {
    }
  if(ladder==4)
    {
      if(sensor==3)
	{
	  l3 = 0;
	  l4 = 0;
	}
    }
  if(ladder==5)
    {
      l1 = 0;
      l2 = 0;
      l3 = 0;
      l4 = 0;
    }
  if(ladder==6)
    {
    }
  if(ladder==7)
    {
      l1 = 0;
      l2 = 0;
      l3 = 0;
      l4 = 0;
    }
  if(ladder==8)///now east arm
    {
      if(sensor==1)
	{
	  l1 = 0;
	  l2 = 0;
	  l3 = 0;
	  l4 = 0;
	}
    }
  if(ladder==9)
    {
      l1 = 0;
      l2 = 0;
      l3 = 0;
      l4 = 0;
    }
  if(ladder==10)
    {
    }
  if(ladder==11)
    {
    }
  if(ladder==12)
    {
    }
  if(ladder==13)
    {
      l1 = 0;
      l2 = 0;
      l3 = 0;
      l4 = 0;
    }
  if(ladder==14)
    {
      l1 = 0;
      l2 = 0;
      l3 = 0;
      l4 = 0;
    }
  if(ladder==15)
    {
      l1 = 0;
      l2 = 0;
      l3 = 0;
      l4 = 0;
    }
  int encoded = 8*l4+4*l3+2*l2+l1;
  return encoded;
}

int get_dead_config_l3(int ladder, int sensor)
{
  short l1=1;
  short l2=1;
  short l3=1;
  short l4=1;
  
  if(ladder==0)//west arm first
    {
    }
  if(ladder==1)
    {
    }
  if(ladder==2)
    {
      if(sensor==2)
	{
	  l1 = 0;
	  l2 = 0;
	  l3 = 0;
	  l4 = 0;
	}
    }
  if(ladder==3)
    {
      if(sensor==0)
	{
	  l1 = 0;
	  l2 = 0;
	  l3 = 0;
	  l4 = 0;
	}
    }
  if(ladder==4)
    {
      l1 = 0;
      l2 = 0;
      l3 = 0;
      l4 = 0; 
    }
  if(ladder==5)
    {
    }
  if(ladder==6)
    {
    }
  if(ladder==7)
    {
    }
  if(ladder==8)
    {
    }
  if(ladder==9)
    {
      l1 = 0;
      l2 = 0;
      l3 = 0;
      l4 = 0;
    }
  if(ladder==10)
    {
      if(sensor!=1)
	{
	  l1 = 0;
	  l2 = 0;
	  l3 = 0;
	  l4 = 0;
	}
    }
  if(ladder==11)
    {
    }
  if(ladder==12)//now east arm
    {
    }
  if(ladder==13)
    {
      l1 = 0;
      l2 = 0;
      l3 = 0;
      l4 = 0;
    }
  if(ladder==14)
    {
    }
  if(ladder==15)
    {
    }
  if(ladder==16)
    {
      l1 = 0;
      l2 = 0;
      l3 = 0;
      l4 = 0;
    }
  if(ladder==17)
    {
    }
  if(ladder==18)
    {
    }
  if(ladder==19)
    {
    }
  if(ladder==20)
    {
    }
  if(ladder==21)
    {
    }
  if(ladder==22)
    {
    }
  if(ladder==23)
    {
      l1 = 0;
      l2 = 0;
      l3 = 0;
      l4 = 0;
    }
      
  int encoded = 8*l4+4*l3+2*l2+l1;
  return encoded;
}

  
