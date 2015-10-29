#ifndef DIALOGSKINNERH
#define DIALOGSKINNERH

#include "MediaLibraryInterface.h"


#define DCW_SUNKENBORDER 0x00010000
#define DCW_DIVIDER 0x00020000

enum
{
  WADLG_ITEMBG,
  WADLG_ITEMFG,
  WADLG_WNDBG,
  WADLG_BUTTONFG,
  WADLG_WNDFG,
  WADLG_HILITE,
  WADLG_SELCOLOR,
  WADLG_LISTHEADER_BGCOLOR,
  WADLG_LISTHEADER_FONTCOLOR,
  WADLG_LISTHEADER_FRAME_TOPCOLOR,
  WADLG_LISTHEADER_FRAME_MIDDLECOLOR,
  WADLG_LISTHEADER_FRAME_BOTTOMCOLOR,
  WADLG_LISTHEADER_EMPTY_BGCOLOR,
  WADLG_SCROLLBAR_FGCOLOR,
  WADLG_SCROLLBAR_BGCOLOR,
  WADLG_SCROLLBAR_INV_FGCOLOR,
  WADLG_SCROLLBAR_INV_BGCOLOR,
  WADLG_SCROLLBAR_DEADAREA_COLOR,
  WADLG_SELBAR_FGCOLOR,
  WADLG_SELBAR_BGCOLOR,
  WADLG_INACT_SELBAR_FGCOLOR,
  WADLG_INACT_SELBAR_BGCOLOR,
  WADLG_NUM_COLORS
};
COLORREF GetHTMLColor(int color);
class DialogSkinner
{

	typedef HBITMAP (*BitmapFunc)();
	typedef int (*ColorFunc)(int idx); // pass this an index, returns a RGB value (passing 0 or > 3 returns NULL)
	typedef int (*HandleFunc)(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	typedef void (*DrawFunc)(HWND hwndDlg, int *tab, int tabsize); // each entry in tab would be the id | DCW_*
public:
	DialogSkinner()
			: color(0), handle(0), draw(0), bitmap(0)
	{}
	int Color(int index)
	{
		if (!color)
			color = (ColorFunc)mediaLibrary.GetWADLGFunc(1);
		return color(index);
	}
	RGBQUAD GetRGB(int index)
	{
		COLORREF color = Color(index);
		
		RGBQUAD rgb;
		rgb.rgbReserved=0;
		rgb.rgbBlue=GetBValue(color);
		rgb.rgbGreen=GetGValue(color);
		rgb.rgbRed=GetRValue(color);
		return rgb;

		

	}
	int Handle(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (!handle)
			handle = (HandleFunc)mediaLibrary.GetWADLGFunc(2);
		return handle(dlg, msg, wParam, lParam);
	}

	void Draw(HWND dlg, int *tab, int tabSize)
	{
		if (!draw)
			draw = (DrawFunc)mediaLibrary.GetWADLGFunc(3);
		draw(dlg, tab, tabSize);
	}
	HFONT GetFont()
	{
		return (HFONT)mediaLibrary.GetWADLGFunc(66);
	}
	HBITMAP GetBitmap()
	{
		if (!bitmap)
			bitmap = (BitmapFunc)mediaLibrary.GetWADLGFunc(4);
		return bitmap();
	}
	ColorFunc color;
	HandleFunc handle;
	DrawFunc draw;
	BitmapFunc bitmap;
};
extern DialogSkinner dialogSkinner;





#endif
