run_sim_program()
{

  gSystem->SetFlagsOpt(" -O2 ");
  gSystem->CompileMacro("run_sim.cpp","kO");

  run_sim(-10,10, 1, true, Form("out/test1bin.root"));

  run_sim(-10,10, 5, true, Form("out/test5bin.root"));

  run_sim(-10,10, 20, true, Form("out/test20bin.root"));

  run_sim(-10,10, 40, true, Form("out/test40bin.root"));

  run_sim(-10,10, 100, true, Form("out/test100bin.root"));

  run_sim(-10,10, 400, true, Form("out/test400bin.root"));

  run_sim(-10,10, 1000, true, Form("out/test1000bin.root"));
    
    
  for(int i = 0;i<40;i++)
    run_sim(-10+i*20.0/40.0,-10+(i+1)*20.0/40.0, 1, true, Form("out/test_zbin%d.root",i));

}
