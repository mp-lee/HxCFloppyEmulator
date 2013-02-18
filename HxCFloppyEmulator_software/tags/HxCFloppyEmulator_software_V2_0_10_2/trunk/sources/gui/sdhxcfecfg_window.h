// generated by Fast Light User Interface Designer (fluid) version 1.0110

#ifndef sdhxcfecfg_window_h
#define sdhxcfecfg_window_h
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Slider.H>
extern void sdhxcfecfg_window_datachanged(Fl_Slider*, void*);
#include <FL/Fl_Value_Slider.H>
extern void sdhxcfecfg_window_datachanged(Fl_Value_Slider*, void*);
#include <FL/Fl_Check_Button.H>
extern void sdhxcfecfg_window_datachanged(Fl_Check_Button*, void*);
#include <FL/Fl_Button.H>
extern void load_ifcfg_window_bt(Fl_Button*, void*);
extern void save_ifcfg_window_bt(Fl_Button*, void*);
#include <FL/Fl_Choice.H>
extern void ifcfg_window_datachanged(Fl_Widget*, void*);
extern void sdhxcfecfg_window_bt_load(Fl_Button*, void*);
extern void sdhxcfecfg_window_bt_save(Fl_Button*, void*);

class sdhxcfecfg_window {
public:
  sdhxcfecfg_window();
  Fl_Double_Window *window;
  Fl_Slider *slider_uisound_level;
  Fl_Slider *slider_stepsound_level;
  Fl_Slider *slider_scrolltxt_speed;
  Fl_Value_Slider *valslider_device_standby_timeout;
  Fl_Value_Slider *valslider_device_backlight_timeout;
  Fl_Check_Button *chk_loadlastloaded;
  Fl_Check_Button *chk_disabediskdriveselector;
  Fl_Check_Button *chk_force_loading_startupa;
  Fl_Check_Button *chk_force_loading_startupb;
  Fl_Check_Button *chk_enable_autoboot_mode;
  Fl_Check_Button *chk_force_loading_autoboot;
  Fl_Check_Button *chk_preindex;
  Fl_Check_Button *chk_hfr_autoifmode;
  Fl_Check_Button *chk_hfe_doublestep;
  Fl_Choice *choice_hfeifmode;
private:
  void cb_OK_i(Fl_Button*, void*);
  static void cb_OK(Fl_Button*, void*);
};
#endif
