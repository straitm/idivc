struct idivc_input_event {
  double tstart[520];
  short pmt[520];
};

struct idivc_output_event {
  float timeid;
  float timeiv;
  int firstidpmt;
  int firstivpmt;
};
