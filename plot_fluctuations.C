plot_fluctuations()
{


  char *filenames[7] = {"test1bin.root","test5bin.root","test20bin.root","test40bin.root","test100bin.root","test400bin.root","test1000bin.root"};

  TFile *file;
  TH2D *CF_tmp = NULL;
  double fluctuation = 0;
  float bins[8] = {0.1,2.0,8.0,32,48,152,648,1352};
  
  TH1D *histo = new TH1D("histo","histo",7,bins);

  for(int i = 0; i < 7 ; i++)
    {
      file = TFile::Open(Form("out/%s",filenames[i]));
      CF_tmp = (TH2D*)(file->Get("CF_total"));
      
      for(int xbin = 1; xbin < CF_tmp->GetNbinsX()+1; xbin++)
	{
	  for(int ybin = 1; ybin < CF_tmp->GetNbinsY()+1; ybin++)
	    {
	      fluctuation+=(CF_tmp->GetBinContent(xbin,ybin)-1);
	    }
	}
      fluctuation/=1.0*CF_tmp->GetNbinsX()*CF_tmp->GetNbinsY();
      histo->SetBinContent(i+1,fabs(fluctuation));
    }

  histo->SetMarkerStyle(22);
  histo->SetMarkerSize(2);
  histo->Draw("p");
}
