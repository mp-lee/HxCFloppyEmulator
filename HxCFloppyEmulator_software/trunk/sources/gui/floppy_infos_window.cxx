// generated by Fast Light User Interface Designer (fluid) version 1.0110

#include "floppy_infos_window.h"

extern "C"
{
	#include "libhxcfe.h"
	#include "usb_hxcfloppyemulator.h"
}

#include "cb_floppy_infos_window.h"
#include "fl_mouse_box.h"
floppy_infos_window::floppy_infos_window() {
  { window = new Fl_Double_Window(1003, 670, "Visual Floppy disk");
    window->user_data((void*)(this));
    { Fl_Group* o = new Fl_Group(0, 502, 155, 166, "Track");
      o->box(FL_ENGRAVED_FRAME);
      o->labeltype(FL_EMBOSSED_LABEL);
      o->labelsize(12);
      o->align(FL_ALIGN_TOP_LEFT|FL_ALIGN_INSIDE);
      { track_number_slide = new Fl_Value_Slider(6, 541, 139, 19, "Track number");
        track_number_slide->type(1);
        track_number_slide->labelsize(12);
        track_number_slide->textsize(12);
        track_number_slide->callback((Fl_Callback*)disk_infos_window_callback, (void*)(this));
      } // Fl_Value_Slider* track_number_slide
      { side_number_slide = new Fl_Value_Slider(5, 601, 140, 19, "Side number");
        side_number_slide->type(1);
        side_number_slide->labelsize(12);
        side_number_slide->textsize(12);
        side_number_slide->callback((Fl_Callback*)disk_infos_window_callback, (void*)(this));
      } // Fl_Value_Slider* side_number_slide
      o->end();
    } // Fl_Group* o
    { Fl_Group* o = new Fl_Group(455, 502, 509, 167, "Status");
      o->box(FL_ENGRAVED_FRAME);
      o->labeltype(FL_ENGRAVED_LABEL);
      o->labelsize(12);
      o->align(FL_ALIGN_TOP_LEFT|FL_ALIGN_INSIDE);
      { x_pos = new Fl_Output(460, 519, 250, 15);
        x_pos->labelsize(10);
        x_pos->textsize(10);
      } // Fl_Output* x_pos
      { y_pos = new Fl_Output(710, 519, 250, 15);
        y_pos->labelsize(10);
        y_pos->textsize(10);
      } // Fl_Output* y_pos
      { global_status = new Fl_Output(460, 534, 500, 13);
        global_status->labelsize(10);
        global_status->textsize(10);
      } // Fl_Output* global_status
      { object_txt = new Fl_Text_Display(460, 547, 500, 119);
        object_txt->user_data((void*)(this));
      } // Fl_Text_Display* object_txt
      o->end();
    } // Fl_Group* o
    { Fl_Button* o = new Fl_Button(969, 504, 30, 163, "OK");
      o->callback((Fl_Callback*)floppy_infos_ok, (void*)(this));
    } // Fl_Button* o
    { floppy_map_disp = new Fl_Group(0, 1, 1000, 500, "Floppy Map");
      floppy_map_disp->box(FL_ENGRAVED_FRAME);
      floppy_map_disp->labeltype(FL_EMBOSSED_LABEL);
      floppy_map_disp->labelsize(12);
      floppy_map_disp->align(FL_ALIGN_TOP_LEFT|FL_ALIGN_INSIDE);
      floppy_map_disp->end();
    } // Fl_Group* floppy_map_disp
    { Fl_Group* o = new Fl_Group(155, 502, 300, 167, "View");
      o->box(FL_ENGRAVED_FRAME);
      o->labeltype(FL_EMBOSSED_LABEL);
      o->labelsize(12);
      o->align(FL_ALIGN_TOP_LEFT|FL_ALIGN_INSIDE);
      { x_time = new Fl_Value_Slider(160, 522, 288, 20, "full x time scale (ms)");
        x_time->type(5);
        x_time->labelsize(12);
        x_time->textsize(12);
        x_time->callback((Fl_Callback*)disk_infos_window_callback, (void*)(this));
      } // Fl_Value_Slider* x_time
      { y_time = new Fl_Value_Slider(160, 557, 288, 20, "full y time scale (us)");
        y_time->type(5);
        y_time->labelsize(12);
        y_time->textsize(12);
        y_time->callback((Fl_Callback*)disk_infos_window_callback, (void*)(this));
      } // Fl_Value_Slider* y_time
      { x_offset = new Fl_Value_Slider(160, 595, 288, 20, "x offset (% of the track len)");
        x_offset->type(5);
        x_offset->labelsize(12);
        x_offset->textsize(12);
        x_offset->callback((Fl_Callback*)disk_infos_window_callback, (void*)(this));
      } // Fl_Value_Slider* x_offset
      { track_view_bt = new Fl_Round_Button(188, 640, 64, 4, "Track view");
        track_view_bt->type(102);
        track_view_bt->down_box(FL_ROUND_DOWN_BOX);
        track_view_bt->callback((Fl_Callback*)disk_infos_window_callback, (void*)(this));
      } // Fl_Round_Button* track_view_bt
      { disc_view_bt = new Fl_Round_Button(340, 640, 64, 4, "Disk view");
        disc_view_bt->type(102);
        disc_view_bt->down_box(FL_ROUND_DOWN_BOX);
        disc_view_bt->callback((Fl_Callback*)disk_infos_window_callback, (void*)(this));
      } // Fl_Round_Button* disc_view_bt
      o->end();
    } // Fl_Group* o

	Fl_Mouse_Box *m = new Fl_Mouse_Box(floppy_map_disp->x(), floppy_map_disp->y(),floppy_map_disp->w(), floppy_map_disp->h(), 0);
	m->callback(mouse_di_cb);

    window->end();
  } // Fl_Double_Window* window
}
