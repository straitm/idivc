idivc_input_event get_event(const uint64_t current_event);
uint64_t root_init(const uint64_t maxevent, const bool clobber,
                   const char * const outfile,
                   const char * const * const infiles,
                   const int nfiles);
void write_event(const idivc_output_event & out);
void root_finish();
