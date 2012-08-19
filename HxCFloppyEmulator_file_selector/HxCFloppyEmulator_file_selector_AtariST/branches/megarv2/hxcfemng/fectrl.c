/*
//
// Copyright (C) 2009, 2010, 2011, 2012 Jean-François DEL NERO
//
// This file is part of the HxCFloppyEmulator file selector.
//
// HxCFloppyEmulator file selector may be used and distributed without restriction
// provided that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// HxCFloppyEmulator file selector is free software; you can redistribute it
// and/or modify  it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// HxCFloppyEmulator file selector is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with HxCFloppyEmulator file selector; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __VBCC__
#include <tos.h>
#else
#include <mint/osbind.h>
#endif

#include <time.h>
/* #include <vt52.h>
 */

#include "gui_utils.h"
#include "cfg_file.h"
#include "hxcfeda.h"
#include "dir.h"
#include "filelist.h"
#include "gui_filelist.h"
#include "instajump.h"
#include "viewer.h"


#include "atari_hw.h"
/* #include "atari_regs.h" */

#include "fat_opts.h"
#include "fat_misc.h"
#include "fat_defs.h"
#include "fat_filelib.h"

#include "conf.h"


static unsigned short y_pos;
static unsigned long last_setlbabase;
static unsigned char sector[512];

unsigned char currentPath[4*256] = {"\\"};

static unsigned char sdfecfg_file[1024 + NUMBER_OF_SLOT * 128];
static char filter[17];


static unsigned char fRepaginate_files;
static unsigned char fRedraw_status;

static struct fs_dir_list_status file_list_status;

static UBYTE * _bigmem_adr;
static LONG    _bigmem_len;
static unsigned char _slotnumber;


// imported variables:
extern unsigned short SCREEN_XRESOL;
extern unsigned short SCREEN_YRESOL;
extern DirectoryEntry * gfl_dirEntLSB_ptr;

// exported variables:
unsigned char fExit=0;					// set to 1 to exit


void handle_exit()
{
	fl_shutdown();
	free(_bigmem_adr);
	restore_display();
	restore_atari_hw();
	exit(0);
}



void lockup()
{
	while(1) {
		get_char();
		if (fExit) {
			handle_exit();
		}
	}
}


int setlbabase(unsigned long lba)
{
	int ret;
	direct_access_cmd_sector * dacs;

	dacs=(direct_access_cmd_sector  *)sector;

	memset(&sector,0,512);

	sprintf(dacs->DAHEADERSIGNATURE,"HxCFEDA");
	dacs->cmd_code=1;
	dacs->parameter_0=(lba>>0)&0xFF;
	dacs->parameter_1=(lba>>8)&0xFF;
	dacs->parameter_2=(lba>>16)&0xFF;
	dacs->parameter_3=(lba>>24)&0xFF;
	dacs->parameter_4=0xA5;

	ret=writesector( 0,(unsigned char *)&sector);
	if(!ret)
	{
		hxc_printf_box(0,"FATAL ERROR: Write CTRL ERROR !");
		lockup();
	}

	return 0;
}


/**
 * Init the hardware
 * Display Firmware version
 * @return 0 on failure, 1 on success
 */
int hxc_media_init()
{
	unsigned char ret;
	unsigned char sector[512];
	direct_access_status_sector * dass;

	last_setlbabase=0xFFFFF000;
	ret=readsector(0,(unsigned char*)&sector,1);

	if(ret)
	{
		dass=(direct_access_status_sector *)sector;
		if(!strcmp(dass->DAHEADERSIGNATURE,"HxCFEDA"))
		{
			hxc_printf(0,0,SCREEN_YRESOL-30,"Firmware %s" ,dass->FIRMWAREVERSION);

			dass= (direct_access_status_sector *)sector;
			last_setlbabase=0;
			setlbabase(last_setlbabase);

			return 1;
		}

		hxc_printf_box(0,"FATAL ERROR: HxC Floppy Emulator not found!");

		return 0;
	}
	hxc_printf_box(0,"FATAL ERROR: Floppy Access error!  [%d]",ret);

	return 0;
}

int hxc_media_read(unsigned long sector, unsigned char *buffer)
{
	direct_access_status_sector * dass;

	dass= (direct_access_status_sector *)buffer;

	more_busy();

	do
	{
		if((sector-last_setlbabase)>=8)
		{
			setlbabase(sector);
		}

		if(!readsector(0,buffer,0))
		{
			hxc_printf_box(0,"ERROR: Read ERROR ! fsector %d",(sector-last_setlbabase)+1);
			get_char_restore_box();
		}
		last_setlbabase=L_INDIAN(dass->lba_base);

		/* hxc_printf(0,0,0,"BA: %08X %08X" ,L_INDIAN(dass->lba_base),sector);*/
	}while((sector-L_INDIAN(dass->lba_base))>=8);

	if(!readsector((sector-last_setlbabase)+1,buffer,0))
	{
		hxc_printf_box(0,"FATAL ERROR: Read ERROR ! fsector %d",(sector-last_setlbabase)+1);
		lockup();
	}

	less_busy();

	return 1;
}

int hxc_media_write(unsigned long sector, unsigned char *buffer)
{
	more_busy();

	if((sector-last_setlbabase)>=8)
	{
		last_setlbabase=sector;
		setlbabase(sector);
	}

	if(!writesector((sector-last_setlbabase)+1,buffer))
	{
		hxc_printf_box(0,"FATAL ERROR: Write sector ERROR !");
		lockup();
	}

	less_busy();

	return 1;
}


int bios_media_read(unsigned long sector, unsigned char *buffer)
{
	Rwabs(0, buffer, 1, sector, 0);
	return 1;
}

int bios_media_write(unsigned long sector, unsigned char *buffer)
{
	Rwabs(1, buffer, 1, sector, 0);
	return 1;
}






char read_cfg_file()
{
	FL_FILE *file;

	// clear the buffer
	memset(sdfecfg_file, 0, 1024 + 128*NUMBER_OF_SLOT);

	file = fl_fopen("/HXCSDFE.CFG", "r");
	if (file)
	{
		// read the file
		fl_fread(sdfecfg_file, 1, 1024 + 128*NUMBER_OF_SLOT, file);

		// ignore that: use the compile-time one instead
		// number_of_slot=cfgfile_ptr->number_of_slot;

		fl_fclose(file);

		// apply color mode
		if(sdfecfg_file[256+128]!=0xFF)
		{
			set_color_scheme(sdfecfg_file[256+128]);
		}

		return 0;
	}

	hxc_printf_box(0,"ERROR: Access HXCSDFE.CFG file failed!");
	get_char_restore_box();
	return 1;
}

char save_cfg_file()
{
	unsigned char ret;
	FL_FILE *file;

	ret=0;

	// open the file for reading. Actual write will use fl_fswrite to write sectors, but we need the file handle
	file = fl_fopen("/HXCSDFE.CFG", "r");
	if (file)
	{
		/* Save the sectors */
		if (fl_fswrite((unsigned char*)sdfecfg_file, 2 + NUMBER_OF_SLOT/4, 0, file) != 2 + NUMBER_OF_SLOT/4)
		{
			hxc_printf_box(0,"ERROR: Write file failed!");
			get_char_restore_box();
			ret=1;
		}

		/* Close file */
		fl_fclose(file);
	}
	else
	{
		// this should never happens, since the HXCSDFE.CFG file is mandatory
		hxc_printf_box(0,"ERROR: Create file failed!");
		get_char_restore_box();
		ret=1;
	}

	return ret;
}








void _printslotstatus(disk_in_drive * disks_a,  disk_in_drive * disks_b)
{
	char tmp_str[17];

	hxc_printf(0,0,SLOT_Y_POS,"Slot %02d:", _slotnumber);

	/* clear_line(SLOT_Y_POS+8,0); */
	hxc_printf(0,0,SLOT_Y_POS+8,"Drive A:                 ");
	if( disks_a->DirEnt.name[0] )
	{
		memcpy(tmp_str,disks_a->DirEnt.longName,16);
		tmp_str[16]=0;
		hxc_printf(0,0,SLOT_Y_POS+8,"Drive A: %s", tmp_str);
	}

	/* clear_line(SLOT_Y_POS+16,0); */
	hxc_printf(0,0,SLOT_Y_POS+16,"Drive B:                 ");

	if( disks_b->DirEnt.name[0] )
	{
		memcpy(tmp_str,disks_b->DirEnt.longName,16);
		tmp_str[16]=0;
		hxc_printf(0,0,SLOT_Y_POS+16,"Drive B: %s", tmp_str);
	}
}

void display_slot()
{
	_printslotstatus((void *) sdfecfg_file + 1024 + _slotnumber*128, (void *) sdfecfg_file + 1024 + _slotnumber*128 + 64) ;
}


void next_slot(unsigned char slotnumber, signed char increment)
{
	slotnumber += increment;
	if(slotnumber>(NUMBER_OF_SLOT-1))
	{	// slot 0 is reserved for autoboot
		slotnumber=1;
	} else if (0 == slotnumber)
	{
		slotnumber = NUMBER_OF_SLOT-1;
	}
	_slotnumber = slotnumber;
	display_slot();
}

void clear_slot()
{
	memset((void*) sdfecfg_file + 1024 + _slotnumber*128,     0, 2*64);
}

void insert_in_slot(unsigned char drive)
{
	void *diskslot_ptr;
	diskslot_ptr = sdfecfg_file + 1024 + _slotnumber*128 + drive*64;

	memset(diskslot_ptr, 0, sizeof(disk_in_drive));
	memcpy(diskslot_ptr, gfl_dirEntLSB_ptr, sizeof(struct ShortDirectoryEntry));
	display_slot();
}




/**
 * Display the current folder
 */
void displayFolder()
{
	int i;
	UWORD curdir_len;

	curdir_len = (SCREEN_XRESOL - CURDIR_X_POS)/8;
	hxc_printf(0, CURDIR_X_POS, CURDIR_Y_POS, "Current directory:");

	for(i=CURDIR_X_POS; i<SCREEN_XRESOL; i=i+8) {
		hxc_printf(0, i, CURDIR_Y_POS+8, " ");
	}

	if(strlen((const char *)currentPath) < curdir_len)
		hxc_printf(0, CURDIR_X_POS, CURDIR_Y_POS+8, "%s", currentPath);
	else
		hxc_printf(0, CURDIR_X_POS, CURDIR_Y_POS+8, "...%s", &currentPath[strlen((const char *)currentPath)-curdir_len]+3);
}



void enter_sub_dir()
{
	int currentPathLength;
	unsigned char folder[LFN_MAX_SIZE];
	unsigned char c;
	int i;
	int old_index;

	old_index=strlen((const char *)currentPath);

	if ( (gfl_dirEntLSB_ptr->longName[0] == (unsigned char)'.') && (gfl_dirEntLSB_ptr->longName[1] == (unsigned char)'.') )
	{
		currentPathLength = strlen((const char *)currentPath) - 1;
		do
		{
			currentPath[ currentPathLength ] = 0;
			currentPathLength--;
		}
		while ( currentPath[ currentPathLength ] != (unsigned char)'/' );

		/*if ( currentPath[ currentPathLength-1 ] != (unsigned char)':' )
		{
			currentPath[ currentPathLength ] = 0;
		}*/
	}
	else
	{
		if((gfl_dirEntLSB_ptr->longName[0] == (unsigned char)'.'))
		{
		}
		else
		{
			for (i=0; i < LFN_MAX_SIZE; i++ )
			{
				c = gfl_dirEntLSB_ptr->longName[i];
				if ( ( c >= (32+0) ) && (c <= 127) )
				{
					folder[i] = c;
				}
				else
				{
					folder[i] = 0;
					i = LFN_MAX_SIZE;
				}
			}
			folder[LFN_MAX_SIZE-1] = 0;

			currentPathLength = strlen((const char *)currentPath);
			/*if ( currentPath[ currentPathLength-1-1 ] != (unsigned char)':' )
			{
				strcat( currentPath, "/" );
			}*/

			if( currentPath[ currentPathLength-1] != '/')
			strcat((char *)currentPath, "/");

			strcat((char *)currentPath, (char *)folder);
		}

		/* strcat( currentPath, "/" ); */
	}

	displayFolder();

	more_busy();
	if(!fl_list_opendir((const char *)currentPath, &file_list_status))
	{
		currentPath[old_index]=0;
		displayFolder();
	} else {
		dir_scan();
		fRepaginate_files = 1;
	}
	less_busy();
}



void handle_show_all_slots(void)
{
	#define ALLSLOTS_Y_POS 16
	char tmp_str[17];
	int i;
	void *diskslot_ptr;

	diskslot_ptr = (void *) sdfecfg_file + 1024 + 1*128;	// start at slot 1

	clear_list(5);

	for ( i = 1; i < NUMBER_OF_SLOT; i++ )
	{
		memcpy(tmp_str, ((disk_in_drive *) diskslot_ptr)->DirEnt.longName, 16);
		tmp_str[16]=0;
		hxc_printf(0,0,ALLSLOTS_Y_POS + (i*8),"Slot %02d - A : %s", i, tmp_str);
		diskslot_ptr += 64;

		memcpy(tmp_str, ((disk_in_drive *) diskslot_ptr)->DirEnt.longName, 16);
		tmp_str[16]=0;
		hxc_printf(0,40*8,ALLSLOTS_Y_POS + (i*8),"B : %s", tmp_str);
		diskslot_ptr += 64;
	}

	hxc_printf(1,0,ALLSLOTS_Y_POS + NUMBER_OF_SLOT*8 + 1,"---Press any key---");
	wait_function_key();
}





void handle_help()
{
	int i;
	char *help1[] = {
		"Function Keys (1/2):",
		"",
		"Up/Down         : Browse the SDCard files (Shift:page, Ctrl:first/last page)",
		"Left/Right      : Browse the slots (Ctrl:first/last)",
		"Insert          : Insert the selected file in the current slot to A:",
		"                  Enter a subfolder",
		"Clr Home        : Insert the selected file in the current slot to B:",
		"Pipe            : Insert the selected file in the current slot to A: and",
		"                  select the next slot",
		"F7              : Insert the selected file in the slot to 1 and restart the",
		"                  computer with this disk.",
		"Undo            : Revert changes",
//		"Backspace       : Clear the current slot",
		"Delete          : Clear the current slot",
		"TAB             : Show all slots selections",
		"",
		"---Any key to continue---"
	};
	char *help2[] = {
		"Function Keys (2/2):",
		"F1              : Filter files in the current folder",
		"                  Type the word to search then enter",
		"                  Type the word to search then enter",
		"                  Enter blank to abort the filter",
		"F2              : Change color",
		"F3              : View the current file",
		"F4              : Settings menu",
		"F8              : Reboot",
		"F9              : Save",
		"F10             : Save and Reboot"
	};

	clear_list(5);
	for (i=0; i<16; i++) {
		hxc_printf(0,0,HELP_Y_POS+(i*8), help1[i]);
	}

	wait_function_key();

	clear_list(5);
	for (i=0; i<11; i++) {
		hxc_printf(0,0,HELP_Y_POS+(i*8), help2[i]);
	}
	display_credits(++i);

	wait_function_key();
}



void handle_emucfg(void)
{
	cfgfile * cfgfile_ptr;
	int i;
	unsigned char c;
	signed char direct;

	clear_list(5);
	cfgfile_ptr=(cfgfile * )sdfecfg_file;

	UWORD ypos = HELP_Y_POS;

	i=0;
	hxc_printf(0,0,ypos, "SD HxC Floppy Emulator settings:");

	ypos += 16;
	hxc_printf(0,0,ypos, "Track step sound :");
	hxc_printf(0,SCREEN_XRESOL/2,ypos, "%s ",cfgfile_ptr->step_sound?"on":"off");

	ypos += 8;
	hxc_printf(0,0,ypos, "User interface sound:");
	hxc_printf(0,SCREEN_XRESOL/2,ypos, "%d  ",cfgfile_ptr->buzzer_duty_cycle);

	ypos += 8;
	hxc_printf(0,0,ypos, "LCD Backlight standby:");
	hxc_printf(0,SCREEN_XRESOL/2,ypos, "%d s",cfgfile_ptr->back_light_tmr);

	ypos += 8;
	hxc_printf(0,0,ypos, "SDCard Standby:");
	hxc_printf(0,SCREEN_XRESOL/2,ypos, "%d s",cfgfile_ptr->standby_tmr);

	ypos += 16;
	hxc_printf(1,0,ypos, "---Press Esc to exit---");

	i=2;
	do
	{
		invert_line(i);
		c=wait_function_key()>>16;
		invert_line(i);
		ypos = HELP_Y_POS+(i<<3);

		if (0x48 ==c && i>2) { /* Up */
			i--;
		} else if (0x50 ==c && i<5) { /* Down */
			i++;
		} else if ((0x4b == c) || (0x4d == c)) { /* Left, Right */
			direct = -1;
			if (0x4d == c) {
				direct = 1;
			}
			switch(i)
			{
			case 2:
				cfgfile_ptr->step_sound =~ cfgfile_ptr->step_sound;
				hxc_printf(0, SCREEN_XRESOL/2, ypos, "%s ", cfgfile_ptr->step_sound?"on":"off");
			break;
			case 3:
				cfgfile_ptr->buzzer_duty_cycle += direct;
				if (cfgfile_ptr->buzzer_duty_cycle >= 0x80) {
					cfgfile_ptr->buzzer_duty_cycle = 0x7f;
				}
				hxc_printf(0, SCREEN_XRESOL/2, ypos, "%d  ", cfgfile_ptr->buzzer_duty_cycle);
				if(!cfgfile_ptr->buzzer_duty_cycle) { cfgfile_ptr->ihm_sound=0x00; }
				else {cfgfile_ptr->ihm_sound=0xff;}
				break;
			case 4:
				cfgfile_ptr->back_light_tmr += direct;
				hxc_printf(0, SCREEN_XRESOL/2, ypos, "%d s  ", cfgfile_ptr->back_light_tmr);
			break;
			case 5:
				cfgfile_ptr->standby_tmr += direct;
				hxc_printf(0, SCREEN_XRESOL/2, ypos, "%d s  ", cfgfile_ptr->standby_tmr);
			break;
			}
		}
	}while(c!=0x01 && !fExit); /* Esc */

}













int main(int argc, char* argv[])
{
	unsigned short i;
	unsigned char bootdev;
	unsigned char c;
	long key;

	init_display();

	// malloc all the available memory
	_bigmem_len = (long)    malloc(-1L);
	_bigmem_adr = (UBYTE *) malloc(_bigmem_len);
	fli_init(_bigmem_adr, _bigmem_len);

	bootdev=0;/* argv[1][0]-'0'; */

	/* Initialise File IO Library */
	fl_init();

	init_atari_hw();

	fn_diskio_read media_read_callback;
	fn_diskio_write media_write_callback;

	if (emulatordetect())
	{
		media_read_callback = bios_media_read;
		media_write_callback = bios_media_write;
	}
	else
	{
		init_atari_fdc(bootdev);

		if(!hxc_media_init()) {
			lockup();
		}

		media_read_callback = hxc_media_read;
		media_write_callback = hxc_media_write;
	}


	/* Attach media access functions to library*/
	if (fl_attach_media(media_read_callback, media_write_callback) != FAT_INIT_OK)
	{
		hxc_printf_box(0,"FATAL ERROR: Media attach failed !");
		lockup();
	}

	hxc_printf_box(0,"Reading HXCSDFE.CFG ...");
	read_cfg_file();
	restore_box();

	strcpy((char *)currentPath, "/" );

	_slotnumber=1;

//	selectorpos=0;
//	page_number=0;

	// get all the files in the dir
	dir_scan();
	dir_setFilter(filter);

	fRepaginate_files = 1;
	fRedraw_status = 1;


	do
	{
		y_pos=FILELIST_Y_POS;

		gfl_showFilesForPage(fRepaginate_files, fRedraw_status);

		if (fRedraw_status)
		{
			redraw_statusl();
			display_slot();
			displayFolder();
			fRedraw_status=0;
		}
		fRepaginate_files=0;

		key = gfl_mainloop();

		UBYTE isDir = (gfl_dirEntLSB_ptr->attributes&0x10);
		UWORD keylow = key>>16;
		char clear_instajump = 1;

		if (keylow == 0) {
		} else if (isDir && (keylow==0x1c || keylow==0x52 || keylow==0x47 || keylow==0x2b) ) {
			enter_sub_dir();
		} else if (keylow == 0x4d) { /* Right: Next slot */
			next_slot(_slotnumber, +1);
		} else if (keylow == 0x474) { /* Ctrl Right: Last slot */
			next_slot(0,0); // slot 0 is invalid, will be replacedd by last slot
		} else if (keylow == 0x4b) { /* Left: Previous slot */
			next_slot(_slotnumber, -1);
		} else if (keylow == 0x473) { /* Ctrl Left: First slot */
			next_slot(1,0); // slot 1 is always the first usable slot (slot 0 reserved for autoboot)
		} else if (keylow == 0x61) { /* Undo : revert all changes */
			hxc_printf_box(0,"Reloading HXCSDFE.CFG ...");
			read_cfg_file();
			restore_box();
			display_slot();
		} else if (keylow==0x52) {  /* Insert: Insert Drive A */
			insert_in_slot(0);
		} else if (keylow==0x47) {  /* ClrHome: Insert Drive B */
			insert_in_slot(1);
		} else if (keylow==0x2b) {  /* Pipe: Insert, Next slot */
			insert_in_slot(0);
			next_slot(_slotnumber, +1);
		} else if (keylow==0x41) {  /* F7: Insert, Select, Reboot */
			_slotnumber = 1;
			insert_in_slot(0);
			hxc_printf_box(0,"Saving selection and restart...");
			save_cfg_file();
			restore_box();
			hxc_printf_box(0,">>>>>Rebooting...<<<<<");
			/* sleep(1); */
			jumptotrack0();
			reboot();
		} else if (keylow==0x62) { /* Help */
			handle_help();
			fRedraw_status = 1;
		} else if (keylow==0x0f) { /* Tab: Show Slots */
			handle_show_all_slots();
			fRedraw_status = 1;
//		} else if (keylow==0x0e) { /* Backspace: Clear slot */
//			clear_slot();
//			display_slot();
		} else if (keylow==0x53) { /* Delete: Clear SLot*/
			clear_slot();
			display_slot();
		} else if (keylow==0x3b) { /* F1: Filter */
			for(i=FILTER_X_POS+13*8; i<SCREEN_XRESOL; i=i+8) {
				hxc_printf(0, i, FILTER_Y_POS, " ");
			}
//			flush_char();
			i=0;
			do
			{
				filter[i]=0;
				c=get_char();
				if(c!='\n' && c>=' ')
				{
					filter[i]=c;
					hxc_printf(0, FILTER_X_POS+13*8+(8*i), FILTER_Y_POS, "%c", c);
					i++;
				}
			}while(c!='\n' && i<16);
			filter[i]=0;

			/* get_str(&filter); */
			mystrlwr(filter);
			fRepaginate_files=1;
		} else if (keylow==0x3c) { /* F2: Change palette */
			sdfecfg_file[256+128] = set_color_scheme(0xff);
		} else if (keylow==0x3d) { /* F3: View */
			viewer(0);
			fRedraw_status = 1;
		} else if (keylow==0x3e) { /* F4: Emuconfig */
			handle_emucfg();
			fRedraw_status = 1;
		} else if (keylow==0x42) { /* F8: Reboot */
			hxc_printf_box(0,">>>>>Rebooting...<<<<<");
			/* sleep(1); */
			jumptotrack0();
			reboot();
		} else if (keylow==0x43) { /* F9: Save */
			hxc_printf_box(0,"Saving selection...");
			save_cfg_file();
			restore_box();
		} else if (keylow==0x44) { /* F10: Save, Reboot */
			hxc_printf_box(0,"Saving selection and restart...");
			save_cfg_file();
			restore_box();
			hxc_printf_box(0,">>>>>Rebooting...<<<<<");
			/* sleep(1); */
			jumptotrack0();
			reboot();
//		} else if (keylow==0x1f) { /* S: Sort */
//			fli_sort();
//			fRepaginate_files=1;
		} else {
			// hxc_printf(0,0,0,"key:%08lx!",key);
			if ((char) key >= ' ')
			{
				clear_instajump = 0;
				ij_keyEvent((char) key);
				i = ij_performSearch();
				if (i != 0xffff) {
					gfl_jumpToFile(i);
				} else {
					// not found: retry
					ij_keyEvent('\0');
					i = ij_performSearch();
					if (i != 0xffff) {
						gfl_jumpToFile(i);
					}
				}
			}
		}
		if (clear_instajump) {
			ij_clear();
		}
	} while (!fExit);

	handle_exit();
	return 0;
}
