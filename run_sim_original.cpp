#include<iostream>
#include<vector>
#include<stdlib.h>
#include<time.h>
#include<math.h>
#include<stdlib.h>
#include<string>

//ROOT includes
#include<TH2D.h>
#include<TGraph.h>
#include<TStyle.h>
#include<TCanvas.h>
#include<TFile.h>

//Project Includes
#include<sensor.h>
#include<track.h>
#include<event.h>
//#include<dead_config.cpp>
#include<dead_config_349369.cpp>

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

void apply_dead_map(vector< vector<sensor> > & detector);

int get_event_bin(double vtx_z, double lower_z, int nmixing_bins);

void run_sim(double z_range,double nzbins,bool apply_dead, string filename = "test.root")
{
  int seed_file = open("/dev/urandom", O_RDONLY);
  int seed;
  read(seed_file, &seed, sizeof seed);
  close(seed_file);

  srand(seed);
  cout << "random seed = " << seed << endl;

  gStyle->SetOptStat(0);//get rid of statistics box in histograms
  vector<vector<sensor> > detector;//detector[layer][sensor]
  build_detector(detector);//create sensor objects active area of sensors of VTX and fill detector
  if(apply_dead)
    apply_dead_map(detector);//puts holes in the detector on a RO chip level
  
  //user defined variables
  TFile *file = new TFile(filename.c_str(),"RECREATE");
  int nevents = 8000;
  int ntracks = 500;
  double vtx_z_lower= -z_range;
  double vtx_z_upper= z_range;
  int pool_depth = 4;
  bool make_fg = true;
  bool make_bg = true;
  int req_layers = 3;
  Int_t nbins_layers_z = 350;//no. of bins for the cluster histograms
  Int_t nbins_layers_phi = 500;
  Int_t nbins_mixed = 100;//no. of bins for the FG BG & CF
  int nmixing_bins = nzbins; 
  double pi = acos(-1.0);
  vector<event> events;

  //Generate events
  for(int ievent = 0; ievent<nevents; ievent++)
    {
      event ev;
      
      double vtx_z;
      if(vtx_z_lower!= vtx_z_upper)
	vtx_z = vtx_z_lower + (float)rand()/((float)RAND_MAX/(vtx_z_upper-vtx_z_lower));
      else
	vtx_z = vtx_z_lower;

      ev.set_vtx_z(vtx_z);

      //generate random trakcs and drop ones that don't go through detector enough times
      ev.generate_filter_tracks(ntracks,req_layers,detector);   
  
      events.push_back(ev);

      if((ievent+1)%100==0)
	cout<<(ievent+1)<<" generated "<<(float)(ievent+1)/nevents*100.<<" % completed"<<endl;
    }

  vector<TH2D*> layer_histos;
  layer_histos.push_back(new TH2D("layer1","layer1",nbins_layers_z,-20,20,nbins_layers_phi,-pi/2.,3.*pi/2.));
  layer_histos.push_back(new TH2D("layer2","layer2",nbins_layers_z,-20,20,nbins_layers_phi,-pi/2.,3.*pi/2.));
  layer_histos.push_back(new TH2D("layer3","layer3",nbins_layers_z,-20,20,nbins_layers_phi,-pi/2.,3.*pi/2.));
  layer_histos.push_back(new TH2D("layer4","layer4",nbins_layers_z,-20,20,nbins_layers_phi,-pi/2.,3.*pi/2.));
  TH2D * tracks_hist = new TH2D("tracks","tracks",nbins_mixed,-2,2,nbins_mixed,-pi/2.,3.*pi/2.);

  //fill hit distributions of layers and tracks
  for(int ievent = 0; ievent<nevents; ievent++)
    {
      vector<track> tracks;
      events[ievent].get_tracks(tracks);
      double vtx_Z = events[ievent].get_vtx_z();
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
  
  //filter the events into z vertex bins
  vector< vector< event> > event_bins;
  if(make_fg || make_bg)
    {
      vector<event> dummy;
      for(int i = 0; i<nmixing_bins; i++)
	{
	  event_bins.push_back(dummy);
	}
	 
      for(int ievent = 0; ievent<nevents; ievent++)
	{
	  double z_vtx = events[ievent].get_vtx_z();
	  int event_bin = get_event_bin(z_vtx,z_range,nmixing_bins);//assumes centered around zero
	  if(event_bin==-1)
	    {
	      cout<<"error finding bin for z vertex: "<<z_vtx<<endl;
	      continue;
	    }
	  event_bins[event_bin].push_back(events[ievent]);
	}
    }

  //Calculate dphi detas and fill FG
  vector<TH2D*> FGs;
  vector<int> FG_nevents;
  double actual_fg_pairs = 0;
  if(make_fg)
    {
     
      int counter = 0;
      cout<<"Creating FG"<<endl;
      for(int ibin = 0; ibin<nmixing_bins;ibin++)
	{
	  int nevents_fg = 0;
	  TH2D *temp_fg = new TH2D(Form("FG_%i",ibin),Form("FG_%i",ibin),nbins_mixed,-2,2,nbins_mixed,-pi/2.,3.*pi/2.);
	  
	  for(unsigned int ievent = 0; ievent<event_bins[ibin].size();ievent++)
	    {
	      counter++;
	      nevents_fg++;

	      vector<track> tracks;
	      event_bins[ibin][ievent].get_tracks(tracks);
	      for(unsigned int itrack1 = 0; itrack1<tracks.size();itrack1++)
		{
		  track tr1 = tracks[itrack1];

		  for(unsigned int itrack2 = itrack1+1;itrack2<tracks.size();itrack2++)
		    {
		      track tr2 = tracks[itrack2];

		      double d_phi = tr1.get_phi()-tr2.get_phi();
		      if(d_phi < -pi/2.)
			d_phi+=2.*pi;
		      else if(d_phi > 3.*pi/2.)
			d_phi-=2.*pi;

		      double eta1 = -log( tan( tr1.get_theta()/2.0) );
		      double eta2 = -log( tan( tr2.get_theta()/2.0) );
		      double d_eta = eta1 - eta2;
		      actual_fg_pairs+=1.0;
		      temp_fg->Fill(d_eta,d_phi);
		    }
		}
	      if(counter%100==0)
		cout<<counter<<" processed "<<(float)counter/nevents*100.<<" % completed"<<endl;	      
	    }
	  //temp_fg->Scale(1.0/nevents_fg);
	  FGs.push_back(temp_fg);
	  FG_nevents.push_back(nevents_fg);
	}
    }

  vector<TH2D*> BGs;
  vector<int> BG_nevents;
  double actual_bg_pairs = 0;
  if(make_bg)
    {
      int counter = 0;
      cout<<"Creating BG"<<endl;
      for(int ibin = 0; ibin<nmixing_bins;ibin++)
	{
	  int nevent_bg = 0;
	  TH2D *temp_bg = new TH2D(Form("BG_%i",ibin),Form("BG_%i",ibin),nbins_mixed,-2,2,nbins_mixed,-pi/2.,3.*pi/2.);
	  
	  for(unsigned int ievent1 = 0; ievent1<event_bins[ibin].size();ievent1++)
	    {
	      counter++;

	      vector<track> tracks1;
	      event_bins[ibin][ievent1].get_tracks(tracks1);

	      for(unsigned int ievent2 = ievent1+1; ievent2<ievent1+1+pool_depth;ievent2++) 
		{
		  if(ievent2==event_bins[ibin].size())//just a check to make sure we don't go out of bounds
		    break;
		  nevent_bg++;//keep track of how many mixed events we make

		  vector<track> tracks2;
		  event_bins[ibin][ievent2].get_tracks(tracks2);

		  for(unsigned int itrack1 = 0; itrack1<tracks1.size();itrack1++)
		    {
		      track tr1 = tracks1[itrack1];
		      
		      for(unsigned int itrack2 = 0;itrack2<tracks2.size();itrack2++)
			{
			  track tr2 = tracks2[itrack2];

			  double d_phi = tr1.get_phi()-tr2.get_phi();
			  if(d_phi < -pi/2.)
			    d_phi+=2.*pi;
			  else if(d_phi > 3.*pi/2.)
			    d_phi-=2.*pi;

			  double eta1 = -log( tan( tr1.get_theta()/2.0) );
			  double eta2 = -log( tan( tr2.get_theta()/2.0) );
			  double d_eta = eta1 - eta2;
			  actual_bg_pairs+=1.0;
			  temp_bg->Fill(d_eta,d_phi);
			}
		    }
		}
	      if(counter%100==0)
		cout<<counter<<" processed "<<(float)counter/nevents*100.<<" % completed"<<endl;
	      
	    }
	  //temp_bg->Scale(1.0/nevent_bg);
	  BGs.push_back(temp_bg);
	  BG_nevents.push_back(nevent_bg);
	}
    }

  vector<TCanvas*> canvases;
  canvases.push_back(new TCanvas("c0","c0",600,600));
  canvases.push_back(new TCanvas("c1","c1",600,600));
  canvases.push_back(new TCanvas("c2","c2",600,600));
  canvases.push_back(new TCanvas("c3","c3",600,600));

  for(int ilyr = 0; ilyr<4; ilyr++)
    {
      canvases[ilyr]->cd();
      layer_histos[ilyr]->Draw("colz");
      
      //draw sensors
      for(unsigned int i = 0; i<detector[ilyr].size(); i++)
	{
	  detector[ilyr][i].get_graph()->Draw("same");
	  for(int chip = 0; chip<4; chip++)
	    {
	      TGraph *graph = detector[ilyr][i].get_dead_graph(chip);
	      if(graph)
		graph->Draw("same");
	    }
	}
      //layer_histos[ilyr]->Write();
    }
  
  TCanvas *c6 = new TCanvas("c6","c6",600,600);
  c6->cd();
  
  tracks_hist->Draw("colz");
  tracks_hist->Write();
  
  if(make_fg)
    {
      TCanvas *c4 = new TCanvas("c4","c4",600,600);
      c4->cd();
      TH2D *FG;
      int total_fg_events = 0;
      double total_fg_pairs = 0;
      float expected_fg_pairs = 0;
      for(int ibin = 0; ibin<nmixing_bins;ibin++)
	{
	  if(ibin==0)
	    FG = (TH2D*)FGs[ibin]->Clone();	    
	  else
	    FG->Add(FGs[ibin]);
	  total_fg_pairs += FGs[ibin]->Integral();
	  total_fg_events+=FG_nevents[ibin];
	  expected_fg_pairs+=ntracks*(ntracks-1)/2.*event_bins[ibin].size();
	}
      FG->Sumw2();
      FG->Scale(1.0/total_fg_events);
      cout<<"integrated FG pairs: "<<total_fg_pairs<<" iterated FG pairs: "<<actual_fg_pairs<<endl;
      cout<<"expected FG pairs: "<<expected_fg_pairs<<endl;
      cout<<"total FG events: "<<total_fg_events<<endl;
      FG->SetName("FG_total");
      FG->SetTitle(FG->GetName());
      //   FG->Draw("colz");
      FG->Write();
    }
  if(make_bg)
    {
      TCanvas *c5 = new TCanvas("c5","c5",600,600);
      c5->cd();
      int total_bg_events = 0;
      double total_bg_pairs = 0;
      double expected_bg_pairs =0;

      TH2D *BG;
      for(int ibin = 0; ibin<nmixing_bins;ibin++)
	{
	  if(ibin == 0)
	    BG = (TH2D*)BGs[ibin]->Clone();
	  else
	    BG->Add(BGs[ibin]);
	  total_bg_pairs += BGs[ibin]->Integral();
	  expected_bg_pairs+=ntracks*ntracks *event_bins[ibin].size() * (pool_depth);
	  total_bg_events+=BG_nevents[ibin];
	}
      double expected_bg_pairs_corrected = expected_bg_pairs -nmixing_bins * ntracks*ntracks * (pool_depth)*(pool_depth+1)/2.;
	  
      BG->Sumw2();
      BG->Scale(1.0/total_bg_events);
      cout<<"integrated BG pairs: "<<total_bg_pairs<<" iterated BG pairs: "<<actual_bg_pairs<<endl;
      cout<<"expected BG pairs: "<<expected_bg_pairs<<" corrected: "<<expected_bg_pairs_corrected<<endl;
      cout<<"total BG events: "<<total_bg_events<<endl;      
      BG->SetName("BG_total");
      BG->SetTitle(BG->GetName());
      //   BG->Draw("colz");
      BG->Write();
    }  

  
  if(make_bg && make_fg)
    {
      double total_fg_pairs;
      TCanvas *c7 = new TCanvas("c7","c7",600,600);
      c7->cd();
      TH2D *CF;
      for(int ibin = 0; ibin<nmixing_bins;ibin++)
	{
	  if(ibin==0)
	    {
	      CF = (TH2D*)FGs[ibin]->Clone();
	      CF->Divide(BGs[ibin]);
	      CF->Scale(BGs[ibin]->Integral());
	      total_fg_pairs = FGs[ibin]->Integral();
	    }
	  else
	    {
	      TH2D *temp_cf = (TH2D*)FGs[ibin]->Clone();
	      temp_cf->Divide(BGs[ibin]);
	      temp_cf->Scale(BGs[ibin]->Integral());
	      CF->Add(temp_cf);
	      total_fg_pairs+=FGs[ibin]->Integral();
	      delete temp_cf;
	    }
	      
	}
      CF->Scale(1.0/total_fg_pairs);
      CF->SetName("CF_total");
      CF->SetTitle(CF->GetName());

      //    CF->Draw("colz");
      CF->Write();
    }
  file->Close();
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

int get_event_bin(double vtx_z, double lower_z, int nmixing_bins)
{
  if(lower_z ==0.0)
    return 0;
  double z_bin_width = (2*lower_z)/nmixing_bins;
  for(int ibin = 0; ibin< nmixing_bins; ibin++)
    {
      if( vtx_z < z_bin_width * (ibin+1) - lower_z && vtx_z > z_bin_width * ibin - lower_z)
	return ibin;
    }
  return -1;
}

