#include<iostream>
#include<vector>
#include<stdlib.h>
#include<time.h>
#include<math.h>
#include<stdlib.h>
#include<string>
#include<deque>
#include<time.h>

//ROOT includes
#include<TH2D.h>
#include<TGraph.h>
#include<TStyle.h>
#include<TCanvas.h>
#include<TFile.h>
#include<TRandom3.h>

//Project Includes
#include<sensor.h>
#include<track.h>
#include<event.h>
//#include<dead_config.cpp>
//#include<dead_config_349369.cpp>//run11 auau run
#include<dead_config_416892.cpp>//heau run
//BOOST includes
//#include <boost/thread.hpp>

using namespace std;

class sensor;
class track;
class event;

//--------------------------------------------------------------------------------
// This is a simulation of throwing straight line trajectory particles through the 
// PHENIX VTX detector. This simulation can mask areas of the VTX detector and
// create histograms of foreground, background and correlation functions.
// by Theo Koblesky 09-22-2012 theodore.koblesky@colorado.edu
//
// Now mixing in arbitrary zbins 10-03-2012
//--------------------------------------------------------------------------------

void build_detector(vector< vector<sensor> > & detector);

void build_cylinder(vector< vector<sensor> > & detector);

void apply_dead_map(vector< vector<sensor> > & detector);

int get_event_bin(double vtx_z, double z_lower, double z_higher, int nzbins);

void run_sim(double z_lower,double z_higher,double nzbins,bool apply_dead, string filename = "test.root")
{
  time_t start, end;
  time(&start);
  int seed_file = open("/dev/random", O_RDONLY);
  int seed1,seed2,seed3;//grab 3 seeds
  read(seed_file, &seed1, sizeof seed1);//first seed is for random z vertex using TRandom3
  read(seed_file, &seed2, sizeof seed2);//second seed is for random_shuffle using c++ rand
  read(seed_file, &seed3, sizeof seed3);//third seed is for random event generation
  close(seed_file);

  TRandom3 *zrandom = new TRandom3();
  zrandom->SetSeed(seed1);
  srand(seed2);
  TRandom3 *event_random = new TRandom3();
  event_random->SetSeed(seed3);
  cout << "random seed1 = " << seed1 << endl;
  cout << "random seed2 = " << seed2 << endl;
  cout << "random seed3 = " << seed3 << endl;
   //detector storage
  vector<vector<sensor> > detector;//detector[layer][sensor]
  build_detector(detector);//create sensor objects active area of sensors of VTX and fill detector
  //build_cylinder(detector);
  if(apply_dead )
    apply_dead_map(detector);//puts holes in the detector on a RO chip level
  
  TFile *file = new TFile(filename.c_str(),"RECREATE");

  //user defined variables
  int nevents = 1000000;
  int ntracks =10;
  unsigned int pool_depth = 10;
  bool make_fg = true;
  bool make_bg = true;
  int req_layers = 3;
  Int_t nbins_mixed = 100;//no. of bins for the FG BG & CF 
  bool make_plots = true;//make hitmap and track plots

  gStyle->SetOptStat(0);
  
  //some temporary holder variables
  vector<deque<event*> > event_buffer;
  deque<event*> dummy;
  vector<TH2D*> FGs;
  vector<TH2D*>BGs;
  bool fg_event;
  int iassociated_track_start;
  vector<int> FG_nevents;
  double actual_fg_pairs = 0;
  vector<int> BG_nevents;
  double actual_bg_pairs = 0;
  TH2D * tracks_hist;
  vector<TH2D*> layer_histos;
  double pi = acos(-1.0);
  int red = 632+2;
  int blue = 600+2;
  int green = 416+2;


  //intialize histograms and event buffer
  for(int ibin = 0; ibin<nzbins; ibin++)
    {
      FGs.push_back(new TH2D(Form("FG_%i",ibin),Form("FG_%i",ibin),nbins_mixed,-2,2,nbins_mixed,-pi/2.,3.*pi/2.));
      BGs.push_back(new TH2D(Form("BG_%i",ibin),Form("BG_%i",ibin),nbins_mixed,-2,2,nbins_mixed,-pi/2.,3.*pi/2.));
      FG_nevents.push_back(0);
      BG_nevents.push_back(0);
      event_buffer.push_back(dummy);
    }

  if(make_plots)
    {
      layer_histos.push_back(new TH2D("layer1","layer1",200,-20,20,200,-pi/2.,3.*pi/2.));
      layer_histos.push_back(new TH2D("layer2","layer2",200,-20,20,200,-pi/2.,3.*pi/2.));
      layer_histos.push_back(new TH2D("layer3","layer3",200,-20,20,200,-pi/2.,3.*pi/2.));
      layer_histos.push_back(new TH2D("layer4","layer4",200,-20,20,200,-pi/2.,3.*pi/2.));
      tracks_hist = new TH2D("tracks","tracks",200,-2,2,200,-pi/2.,3.*pi/2.);
      for(int i = 0; i < 4; i++)
	{
	  layer_histos[i]->SetXTitle("z (cm)");
	  layer_histos[i]->SetYTitle("#phi");
	  layer_histos[i]->SetTitle(Form("Layer %d",i));
	}
      tracks_hist->SetXTitle("#eta");
      tracks_hist->SetYTitle("#phi");
    }

  //Event Loop
  for(int ievent = 0; ievent<nevents; ievent++)
    {
      //create event object
      event *ev = new event();
      
      //
      ev->set_random(event_random);

      //determine the z vertex for the event
      double vtx_z;
      if(z_lower!= z_higher)
	vtx_z = z_lower + zrandom->Rndm()*(z_higher-z_lower);
      else
	vtx_z = z_lower;
      ev->set_vtx_z(vtx_z);

      //generate random trakcs and drop ones that don't go through detector enough times
      ev->generate_filter_tracks(ntracks,req_layers,detector);

      //find the z vertex bin the event belongs to
      int z_vertex_bin = get_event_bin(vtx_z,z_lower,z_higher,nzbins);
      if(z_vertex_bin==-1)
	{
	  cout<<"error finding bin for z vertex: "<<vtx_z<<endl;
	  continue;
	}

      //add the event to the correct event buffer bin
      event_buffer[z_vertex_bin].push_back(ev);

      //once there is enough events in a event buffer bin, create fg & bg events
      if(event_buffer[z_vertex_bin].size()==pool_depth+1)
	{
	  //get vector of tracks from first event in buffer (trigger event)
	  vector<track> trig_tracks;
	  event_buffer[z_vertex_bin].front()->get_tracks(trig_tracks);
	  
	  //loop through all the events in buffer
	  for(unsigned int ievnt = 0; ievnt<event_buffer[z_vertex_bin].size();ievnt++)
	    {
	      
	      if(ievnt==0)//make fg event
		fg_event=true;
	      else//make bg events
		fg_event=false;
	      
	      //get vector of tracks from event in buffer (associated events)
	      vector<track> assoc_tracks;
	      event_buffer[z_vertex_bin][ievnt]->get_tracks(assoc_tracks);

	      for(unsigned int itrigger_track = 0; itrigger_track<trig_tracks.size();itrigger_track++)
		{
		  //track holder from trigger event
		  track trigger_track = trig_tracks[itrigger_track];
		  
		  if(fg_event)//do a trangular traverse if fg event
		    iassociated_track_start=itrigger_track+1;
		  else//else just do a square traverse
		    iassociated_track_start=0;
		  
		  for(unsigned int iassociated_track = iassociated_track_start;iassociated_track<assoc_tracks.size();iassociated_track++)
		    {
		      //track holder from the associated event
		      track associated_track = assoc_tracks[iassociated_track];

		      double theta1 = trigger_track.get_theta();
		      double theta2 = associated_track.get_theta();
		      
		      double eta1 = -log( tan( theta1/2.0) );
		      double eta2 = -log( tan( theta2/2.0) );
		      double d_eta = eta1 - eta2;
		     
		      double d_phi = trigger_track.get_phi()-associated_track.get_phi();

		      if(zrandom->Rndm()>0.5)
			{
			  d_eta*=-1.0;
			  d_phi*=-1.0;
			}
		      
		      //perform phi folding
		      if(d_phi < -pi/2.)
			{
			  d_phi+=2.*pi;
			}
		      else if(d_phi > 3.*pi/2.)
			{
			  d_phi-=2.*pi;
			}
		      
		      //fill histograms and record number of events
		      if(fg_event)
			{
			  actual_fg_pairs+=1.0;
			  FGs[z_vertex_bin]->Fill(d_eta,d_phi);
			}
		      else
			{  
			  actual_bg_pairs+=1.0;
			  BGs[z_vertex_bin]->Fill(d_eta,d_phi);
			}
		      
		    }//track2
		}//track1
	    }//event1
	  
	  // after making all possible mixed events
	  // pop the last event
	  delete event_buffer[z_vertex_bin].front();
	  event_buffer[z_vertex_bin].pop_front();
	
	  //increment the bg & fg event counters
	  FG_nevents[z_vertex_bin]++;
	  BG_nevents[z_vertex_bin]+=pool_depth;
	  
	}//ifpool

      if(make_plots)
	{
	  vector<track> tracks;
	  ev->get_tracks(tracks);
	  double vtx_Z = ev->get_vtx_z();
	  for(unsigned int itrack = 0; itrack<tracks.size();itrack++)
	    {
	      vector<double> hit_rad = tracks[itrack].get_hit_rad();

	      double phi = tracks[itrack].get_phi();
	      double theta = tracks[itrack].get_theta();
	      double eta = -log( tan( theta/2.0) );
	      tracks_hist->Fill(eta,phi);

	      for(unsigned int ilayer = 0; ilayer<hit_rad.size(); ilayer++)
		{
		  if(hit_rad[ilayer]==-1.0)
		    continue;

		  double z = hit_rad[ilayer]/tan(theta)+vtx_Z;

		  layer_histos[ilayer]->Fill(z,phi);
		}
	    }
	}

      if((ievent+1)%100==0)
	{
	  cout<<(ievent+1)<<" processed "<<(float)(ievent+1)/nevents*100.<<" % completed"<<endl;
	}
	  
      //at the end of generated events
      if(ievent==nevents-1)
	{
	  cout<<"emptying pools"<<endl;
	  for(int ibin = 0; ibin<nzbins;ibin++)
	    {
	      while(event_buffer[ibin].size()>0)
		{
		  //get vector of tracks from first event in buffer (trigger event)
		  vector<track> trig_tracks;
		  event_buffer[ibin].front()->get_tracks(trig_tracks);
	  
		  //loop through all the events in buffer
		  for(unsigned int ievnt = 0; ievnt<event_buffer[ibin].size();ievnt++)
		    {
	      
		      if(ievnt==0)//make fg event
			fg_event=true;
		      else//make bg events
			fg_event=false;
	      
		      //get vector of tracks from event in buffer (associated events)
		      vector<track> assoc_tracks;
		      event_buffer[ibin][ievnt]->get_tracks(assoc_tracks);

		      for(unsigned int itrigger_track = 0; itrigger_track<trig_tracks.size();itrigger_track++)
			{
			  //track holder from trigger event
			  track trigger_track = trig_tracks[itrigger_track];
		  
			  if(fg_event)//do a trangular traverse if fg event
			    iassociated_track_start=itrigger_track+1;
			  else        //else just do a square traverse
			    iassociated_track_start=0;
		  
			  for(unsigned int iassociated_track = iassociated_track_start;iassociated_track<assoc_tracks.size();iassociated_track++)
			    {
			      //track holder from the associated event
			      track associated_track = assoc_tracks[iassociated_track];

			      double theta1 = trigger_track.get_theta();
			      double theta2 = associated_track.get_theta();
		      
			      double eta1 = -log( tan( theta1/2.0) );
			      double eta2 = -log( tan( theta2/2.0) );
			      double d_eta = eta1 - eta2;

			      double d_phi = trigger_track.get_phi()-associated_track.get_phi();
			        
			      if(zrandom->Rndm()>0.5)
				{
				  d_eta*=-1.0;
				  d_phi*=-1.0;
				}
			      
			      //perform phi folding
			      if(d_phi < -pi/2.)
				{
				  d_phi+=2.*pi;
				}
			      else if(d_phi > 3.*pi/2.)
				{
				  d_phi-=2.*pi;
				}
			      
			      //fill histograms and record number of events
			      if(fg_event)
				{
				  actual_fg_pairs+=1.0;
				  FGs[ibin]->Fill(d_eta,d_phi);
				}
			      else
				{  
				  actual_bg_pairs+=1.0;
				  BGs[ibin]->Fill(d_eta,d_phi);
				}
		      
			    }//track2
			}//track1
		    }//event1
	  
		  // after making all possible mixed events
		  // pop the last event
		  delete event_buffer[ibin].front();
		  event_buffer[ibin].pop_front();
	
		  //increment the bg & fg event counters
		  FG_nevents[ibin]++;
		  BG_nevents[ibin]+=pool_depth;
		}
	    }
	}
    }

  //draw plots
  if(make_plots)
    {
      vector<TCanvas*> canvases;
      canvases.push_back(new TCanvas("c0","c0",600,600));
      canvases.push_back(new TCanvas("c1","c1",600,600));
      canvases.push_back(new TCanvas("c2","c2",600,600));
      canvases.push_back(new TCanvas("c3","c3",600,600));
      canvases[0]->cd();
      for(int ilyr = 0; ilyr<4; ilyr++)
	{
	  canvases[ilyr]->cd();
	  layer_histos[ilyr]->Draw("same");
	  //draw sensors
	  for(unsigned int i = 0; i<detector[ilyr].size(); i++)
	    {
	      //if(ilyr==1) continue;
	      TGraph *graph = detector[ilyr][i].get_graph();
	      graph->SetLineWidth(3-0.4*ilyr);
	      if(ilyr==0)
		graph->SetLineColor(red);
	      if(ilyr==2)
		graph->SetLineColor(blue);
	      if(ilyr==3)
		graph->SetLineColor(green);
	      if(ilyr==0 && i==0)
		graph->Draw("same");
	      else
		graph->Draw("same");
	      for(int chip = 0; chip<4; chip++)
		{
		  TGraph *graph2 = detector[ilyr][i].get_dead_graph(chip);
		  if(graph2)
		    graph2->Draw("same");
		}
	    }
	  
	}
      
      TCanvas *c6 = new TCanvas("c6","c6",600,600);
      c6->cd();
      
      tracks_hist->Draw("colz");
      tracks_hist->Write();
    }

  file->cd();
  //make the summed foreground
  TH2D *FG=NULL;
  TH2D *BG=NULL;
  if(make_fg)
    {
      int total_fg_events = 0;
      double total_fg_pairs = 0;
      float expected_fg_pairs = 0;
      //loop over the zbins and directly add the different fgs together
      for(int ibin = 0; ibin<nzbins;ibin++)
	{
	  if(ibin==0)
	    FG = (TH2D*)FGs[ibin]->Clone();	    
	  else
	    FG->Add(FGs[ibin]);
	  
	  total_fg_pairs += FGs[ibin]->Integral();
	  total_fg_events+=FG_nevents[ibin];
	  expected_fg_pairs+=ntracks*(ntracks-1)/2.*FG_nevents[ibin];
	}

      //FG->Sumw2();
      FG->Scale(1.0/total_fg_events);
      cout<<"integrated FG pairs: "<<total_fg_pairs<<" iterated FG pairs: "<<actual_fg_pairs<<endl;
      cout<<"expected FG pairs: "<<expected_fg_pairs<<endl;
      cout<<"total FG events: "<<total_fg_events<<endl;
      FG->SetName("FG_total");
      FG->SetTitle(FG->GetName());
      //TCanvas * c5 = new TCanvas("c5","c5",800,800);
      //c5->cd();
      //FG->Draw("colz");
      FG->Write();
    }
  if(make_bg)
    {
      int total_bg_events = 0;
      double total_bg_pairs = 0;
      double expected_bg_pairs =0;

      for(int ibin = 0; ibin<nzbins;ibin++)
	{
	  if(ibin == 0)
	    BG = (TH2D*)BGs[ibin]->Clone();
	  else
	    BG->Add(BGs[ibin]);
	  total_bg_pairs += BGs[ibin]->Integral();
	  expected_bg_pairs+=(double)(ntracks*ntracks)*BG_nevents[ibin];
	  total_bg_events+=BG_nevents[ibin];
	}
      //correction due to pool depth
      double expected_bg_pairs_corrected = expected_bg_pairs -nzbins * ntracks*ntracks * (pool_depth)*(pool_depth+1)/2.;
      
      //BG->Sumw2();
      BG->Scale(1.0/total_bg_events);
      cout<<"integrated BG pairs: "<<total_bg_pairs<<" iterated BG pairs: "<<actual_bg_pairs<<endl;
      cout<<"expected BG pairs: "<<expected_bg_pairs<<" corrected: "<<expected_bg_pairs_corrected<<endl;
      cout<<"total BG events: "<<total_bg_events<<endl;      
      BG->SetName("BG_total"); 
      BG->SetTitle(BG->GetName());
      BG->Write();
    }

  //make CF
  if(make_bg && make_fg)
    {
      TH2D *CF=NULL;
      double total_fg_pairs = 0;
      
      //make the CF bin by bin
      /*for(int ibin = 0; ibin<nzbins;ibin++)
	{
	  if(ibin==0)
	    {
	      CF = (TH2D*)FGs[ibin]->Clone();
	      CF->Divide(BGs[ibin]);
	      CF->Scale(BGs[ibin]->Integral());
	    }
	  else
	    {
	      TH2D *temp_cf = (TH2D*)FGs[ibin]->Clone();
	      temp_cf->Divide(BGs[ibin]);
	      temp_cf->Scale(BGs[ibin]->Integral());
	      CF->Add(temp_cf);
	      delete temp_cf;
	    }
	 
	  total_fg_pairs += FGs[ibin]->Integral();
	  
	}*/

      //CF->Scale(1.0/total_fg_pairs);

      CF = (TH2D*)FG->Clone();
      CF->Divide(BG);
      
      CF->Scale(BG->Integral()*1.0/FG->Integral());
      CF->SetName("CF_total");
      CF->SetTitle(CF->GetName());
      CF->Write();
    }
  file->Write();
  file->Close();
  time(&end);
  double diff = difftime(end,start);
  cout<<"time taken is: "<<diff<<" seconds"<<endl;
}

void apply_dead_map(vector< vector<sensor> > & detector)
{
  vector< vector<int> > configuration_1;
  vector<int> l0;
  for(int iladder = 0; iladder<5;iladder++)//layer 1
    {
      int count = 0;
      for(int isensor = 0; isensor<8;isensor++)
	{
	  int ladder = iladder;
	  int sensor = isensor;
	  if(isensor%2==1)//switch off ladders to match detector storage scheme
	    {
	      ladder = iladder + 5;
	      sensor = count;
	      count++;
	    }
	  else
	    {
	      sensor/=2;
	    }
	  int encoded = get_dead_config_l0(ladder,sensor);
	  l0.push_back(encoded);
	}
    }
  configuration_1.push_back(l0);

  vector<int> l1;
  for(int iladder = 0; iladder<10;iladder++)//layer 2
    {
      int count = 0;
      for(int isensor = 0; isensor<8;isensor++)
	{
	  int ladder = iladder;
	  int sensor = isensor;
	  if(isensor%2==1)//switch off ladders to match detector storage scheme
	    {
	      ladder = iladder + 10;
	      sensor = count;
	      count++;
	    }
	  else
	    {
	      sensor/=2;
	    }
	  int encoded = get_dead_config_l1(ladder,sensor);
	  l1.push_back(encoded);
	}
    }
  configuration_1.push_back(l1);

  vector<int> l2;
  for(int iladder = 0; iladder<8;iladder++)//layer 3
    {
      int count = 0;
      for(int isensor = 0; isensor<10;isensor++)
	{
	  int ladder = iladder;
	  int sensor = isensor;
	  if(isensor%2==1)//switch off ladders to match detector storage scheme
	    {
	      ladder = iladder + 8;
	      sensor = count;
	      count++;
	    }
	  else
	    {
	      sensor/=2;
	    }
	  int encoded = get_dead_config_l2(ladder,sensor);
	  l2.push_back(encoded);
	}
    }
  configuration_1.push_back(l2);
  
  vector<int> l3;
  for(int iladder = 0; iladder<12;iladder++)//layer 4
    {
      int count = 0;
      for(int isensor = 0; isensor<12;isensor++)
	{
	  int ladder = iladder;
	  int sensor = isensor;
	  if(isensor%2==1)//switch off ladders to match detector storage scheme
	    {
	      ladder = iladder + 12;
	      sensor = count;
	      count++;
	    }	  
	  else
	    {
	      sensor/=2;
	    }
	  int encoded = get_dead_config_l3(ladder,sensor);
	  l3.push_back(encoded);
	}
    }
  configuration_1.push_back(l3);
  
  for(int ilayer = 0; ilayer<4;ilayer++)
    {
      for(unsigned int isensor = 0; isensor<detector[ilayer].size();isensor++)
	{
	  short ll1 = (configuration_1[ilayer][isensor] & 1) == 1;
	  short ll2 = (configuration_1[ilayer][isensor] & 2) == 2;
	  short ll3 = (configuration_1[ilayer][isensor] & 4) == 4;
	  short ll4 = (configuration_1[ilayer][isensor] & 8) == 8;
	  detector[ilayer][isensor].set_dead_chips(ll1,ll2,ll3,ll4);
	}
    }

}

void build_detector(vector< vector<sensor> > & detector)
{
  vector<double> radii;
  radii.push_back(2.63);
  radii.push_back(5.13);
  radii.push_back(12.59);
  radii.push_back(11.42);
  radii.push_back(10.15);
  radii.push_back(17.56);
  radii.push_back(16.34);
  radii.push_back(15.13);
  
  vector<short> nladder;
  nladder.push_back(5);
  nladder.push_back(10);
  nladder.push_back(8);
  nladder.push_back(12);
  
  vector<short> nsensor;
  nsensor.push_back(4);
  nsensor.push_back(4);
  nsensor.push_back(5);
  nsensor.push_back(6);

  vector<double> phi_gap;
  phi_gap.push_back(-0.002);
  phi_gap.push_back(-0.005);
  phi_gap.push_back(0.04);
  phi_gap.push_back(0.027);
  
  vector<double> z_gap;
  z_gap.push_back(.167);
  z_gap.push_back(.167);
  z_gap.push_back(.37);
  z_gap.push_back(.37);

  vector<double> west_phi_corner;
  west_phi_corner.push_back(1.95);
  west_phi_corner.push_back(1.95);
  west_phi_corner.push_back(1.96);
  west_phi_corner.push_back(1.86);
  
  vector<double> east_phi_corner;
  east_phi_corner.push_back(-1.16);
  east_phi_corner.push_back(-1.2);
  east_phi_corner.push_back(-1.208);
  east_phi_corner.push_back(-1.28);

  vector<double> z_corner;
  z_corner.push_back(-11.5);
  z_corner.push_back(-11.5);
  z_corner.push_back(-16.1);
  z_corner.push_back(-19.3);
  
  vector<double> dphi;
  dphi.push_back(1.28);
  dphi.push_back(1.28);
  dphi.push_back(3.076);
  dphi.push_back(3.072);
  
  vector<double> dz;
  dz.push_back(5.56);
  dz.push_back(5.56);
  dz.push_back(6.004);
  dz.push_back(6.004);
  
  for(int ilayer = 0; ilayer<4; ilayer++)
    {
      vector<sensor> layer;
      double east_phi = east_phi_corner[ilayer];
      double west_phi = west_phi_corner[ilayer];

      for(int iladder = 0; iladder<nladder[ilayer];iladder++)
	{
	  double radius_east;
	  double radius_west;
	  if(ilayer<2)
	    {
	      radius_west = radius_east = radii[ilayer];
	    }
	  else if(ilayer==2)
	    {
	      radius_east = radii[ilayer+(iladder+1)%3];
	      radius_west = radii[ilayer+(nladder[ilayer]-iladder)%3];
	    }
	  else if(ilayer==3)
	    {
	      radius_east = radii[ilayer+2+(nladder[ilayer]-iladder)%3];
	      radius_west = radii[ilayer+2+(iladder)%3];
	    }
		  
	  for(int isensor = 0; isensor<nsensor[ilayer]; isensor++)
	    {
	      sensor east;
	      sensor west;
	      double z = z_corner[ilayer]+z_gap[ilayer]+isensor*(dz[ilayer]+z_gap[ilayer]);
	      east.set_sensor(z,east_phi,z+dz[ilayer],east_phi+dphi[ilayer]/radius_east,radius_east);
	      west.set_sensor(z,west_phi,z+dz[ilayer],west_phi+dphi[ilayer]/radius_west,radius_west);
	      if((isensor+iladder+ilayer)%1==0 )
		{
		  layer.push_back(east);
		  layer.push_back(west);
		}
	    }
	  east_phi += (dphi[ilayer]/radius_east+phi_gap[ilayer]);
	  west_phi += (dphi[ilayer]/radius_west+phi_gap[ilayer]);
	}
      detector.push_back(layer);
    }
}

int get_event_bin(double vtx_z, double z_lower, double z_higher, int nzbins)
{
  if(z_lower==z_higher)
    return 0;
  double z_bin_width = (z_higher-z_lower)/nzbins;
  for(int ibin = 0; ibin< nzbins; ibin++)
    {
      if( vtx_z < z_bin_width * (ibin+1) + z_lower && vtx_z > z_bin_width * ibin + z_lower)
	return ibin;
    }
  return -1;
}

void build_cylinder(vector< vector<sensor> > & detector)
{
  double pi = acos(-1.0);
  vector<sensor> dummy;

  sensor l1;
  l1.set_sensor(-11.5,-pi/2.,-0.25,3.*pi/2.,2.63);
  dummy.push_back(l1);
  l1.set_sensor(0.25,-pi/2.,11.5,3.*pi/2.,2.63);
  dummy.push_back(l1);
  detector.push_back(dummy);
  dummy.clear();

  sensor l2;
  l2.set_sensor(-11.5,-pi/2.,-0.25,3.*pi/2.,5.13);
  dummy.push_back(l2);
  l2.set_sensor(0.25,-pi/2.,11.5,3.*pi/2.,5.13);
  dummy.push_back(l2);
  detector.push_back(dummy);
  dummy.clear();

  sensor l3;
  l3.set_sensor(-16.1,-pi/2.,-0.25,3.*pi/2.,11.39);
  dummy.push_back(l3);
  l3.set_sensor(0.25,-pi/2.,16.1,3.*pi/2.,11.39);
  dummy.push_back(l3);
  detector.push_back(dummy);
  dummy.clear();

  sensor l4;
  l4.set_sensor(-19.3,-pi/2.,-0.25,3*pi/2.,16.34);
  dummy.push_back(l4);
  l4.set_sensor(0.25,-pi/2.,19.3,3*pi/2.,16.34);
  dummy.push_back(l4);
  detector.push_back(dummy);
  dummy.clear();

}
