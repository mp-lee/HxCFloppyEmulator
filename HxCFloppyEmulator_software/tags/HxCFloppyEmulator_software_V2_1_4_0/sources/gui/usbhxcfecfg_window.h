// generated by Fast Light User Interface Designer (fluid) version 1.0300

#ifndef usbhxcfecfg_window_h
#define usbhxcfecfg_window_h
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Value_Output.H>
#include <FL/Fl_Button.H>
extern void resetusbstat_bt(Fl_Button*, void*);
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Check_Button.H>
extern void usbifcfg_window_datachanged(Fl_Widget*, void*);
#include <FL/Fl_Choice.H>
extern void ifcfg2_window_datachanged(Fl_Widget*, void*);

class usbhxcfecfg_window {
public:
  usbhxcfecfg_window();
  Fl_Double_Window *window;
  Fl_Output *strout_usbhfestatus;
  Fl_Output *strout_maxsettletime;
  Fl_Output *strout_minsettletime;
  Fl_Value_Output *valout_synclost;
  Fl_Output *strout_packetsent;
  Fl_Output *strout_datasent;
  Fl_Output *strout_datathroughput;
  Fl_Value_Slider *slider_usbpacket_size;
  Fl_Round_Button *rbt_ds2;
  Fl_Round_Button *rbt_ds3;
  Fl_Check_Button *chk_twistedcable;
  Fl_Check_Button *chk_disabledrive;
  Fl_Round_Button *rbt_ds0;
  Fl_Round_Button *rbt_ds1;
  Fl_Check_Button *chk_autoifmode;
  Fl_Check_Button *chk_doublestep;
  Fl_Choice *choice_ifmode;
private:
  inline void cb_OK_i(Fl_Button*, void*);
  static void cb_OK(Fl_Button*, void*);
};
#endif
