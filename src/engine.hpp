/*
* Copyright (c) 2007, ai-chan
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the ASSDraw3 Team nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY AI-CHAN ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL AI-CHAN BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
///////////////////////////////////////////////////////////////////////////////
// Name:        engine.hpp
// Purpose:     header file for ASSDraw drawing engine
// Author:      ai-chan
// Created:     08/26/06
// Copyright:   (c) ai-chan
// Licence:     3-clause BSD
///////////////////////////////////////////////////////////////////////////////

#pragma once

// we use these 2 standard libraries
#include <math.h>
#include <list>
#include <set>
#include <vector>

#include "wx.hpp"

// agg support
#include "wxAGG/AGGWindow.h"

#include <agg_color_rgba.h>
#include <agg_rendering_buffer.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_scanline_p.h>
#include <agg_path_storage.h>
#include <agg_renderer_base.h>
#include <agg_renderer_primitives.h>
#include <agg_renderer_scanline.h>
#include <agg_trans_affine.h>
#include <agg_conv_transform.h>
#include <agg_conv_curve.h>
#include <agg_conv_dash.h>
#include <agg_conv_stroke.h>
#include <agg_conv_contour.h>

// Command type
enum CMDTYPE
{
	M = 0,
	N = 1,
	L = 2,
	B = 3,
	S = 4,
	P = 5,
	C = 6
};

// Point type
enum POINTTYPE
{
	MP, // main point
	CP  // control point
};

// A PointSystem is a centralized entity holding the parameters:
// scale, originx and originy, all of which are needed by Point
class PointSystem
{
public:
	PointSystem(double sc = 1.0, double origx = 0.0, double origy = 0.0) { Set(sc, origx, origy); }

	// set scale, originx and originy;
	void Set(double sc, double origx, double origy)
	{
		scale = sc;
		originx = origx;
		originy = origy;
	}

	wxRealPoint ToWxRealPoint(double x, double y) { return wxRealPoint(originx + x * scale, originy + y * scale); }

	// given drawing command coordinates returns the wxPoint on the GUI
	wxPoint ToWxPoint(double x, double y) { return wxPoint((int) (originx + x * scale), (int) (originy + y * scale)); }

	// given wxPoint on the GUI returns the nearest drawing command coords
	void FromWxPoint(int wxpx, int wxpy, int &x, int &y)
	{
		x = int(floor(((double) wxpx - originx) / scale + 0.5));
		y = int(floor(((double) wxpy - originy) / scale + 0.5));
	}

	// given wxPoint on the GUI returns the nearest drawing command coords
	void FromWxPoint(wxPoint wxp, int &x, int &y) { FromWxPoint(wxp.x, wxp.y, x, y); }

	double scale, originx, originy;
};

class DrawCmd;

// The point class
// note: this actually refers to the x,y-coordinate in drawing commands, not the coordinate in the GUI
class Point
{
public:
	Point(int _x, int _y, PointSystem* ps, POINTTYPE t, DrawCmd* cmd, unsigned n = 0);

	// getters
	int x() { return x_; }
	int y() { return y_; }

	//set x and y
	void setXY(int _x, int _y)
	{
		x_ = _x;
		y_ = _y;
	}

	// simply returns true if px and py are the coordinate values
	bool IsAt(int px, int py) { return (x_ == px && y_ == py); }

	// convert this point to wxPoint using scale and originx, originy
	wxPoint ToWxPoint() { return ToWxPoint(true); }

	// convert this point to wxPoint using scale; also use originx and originy if useorigin = true
	wxPoint ToWxPoint(bool useorigin);

	// check if wxpoint is nearby this point
	bool CheckWxPoint(wxPoint wxpoint);

	POINTTYPE type;
	PointSystem *pointsys;

	// drawing commands that depend on this point
	DrawCmd* cmd_main;
	DrawCmd* cmd_next;
	bool isselected;
	unsigned num;

private:
	int x_, y_;
};

typedef std::list<Point*> PointList;
typedef std::set<Point*> PointSet;

// The base class for all draw commands
class DrawCmd
{
public:
	DrawCmd(int x, int y, PointSystem *ps, DrawCmd *pv);
	virtual ~DrawCmd();

	// Init the draw command (for example to generate the control points)
	virtual void Init() { initialized = true; }
	virtual wxString ToString() { return wxT(""); }

	CMDTYPE type;

	// main point (almost every command has one) for B and S it's the last (destination) point
	Point* m_point;

	// other points than the main point, subclasses must populate this list even if they define new variables for other points
	PointList controlpoints;

	// Linked list feature
	DrawCmd *prev;

	// Must set to true if the next command should NOT utilize this command for the drawing
	bool dobreak;

	// Set to true if invisible m_point (not drawn)
	bool invisible;

	// true if this DrawCmd has been initialized with Init(), false otherwise (initialized means that the control points have been generated)
	bool initialized;
};

typedef std::list<DrawCmd*> DrawCmdList;

// The M command
class DrawCmd_M: public DrawCmd
{
public:
	DrawCmd_M(int x, int y, PointSystem *ps, DrawCmd *prev) : DrawCmd(x, y, ps, prev) { type = M; }

	wxString ToString() { return wxString::Format(_T("m %d %d"), m_point->x(), m_point->y()); }
};

// The L command
class DrawCmd_L: public DrawCmd
{
public:
	DrawCmd_L(int x, int y, PointSystem *ps, DrawCmd *prev) : DrawCmd(x, y, ps, prev) { type = L; }

	wxString ToString() { return wxString::Format(_T("l %d %d"), m_point->x(), m_point->y()); }
};

// The B command
class DrawCmd_B: public DrawCmd
{
public:
	DrawCmd_B(int x, int y, int x1, int y1, int x2, int y2, PointSystem *ps, DrawCmd *prev);
	DrawCmd_B(int x, int y, PointSystem *ps, DrawCmd *prev);

	// Init this B command; generate controlpoints
	void Init();
	wxString ToString();

	bool C1Cont;
};

// The S command
class DrawCmd_S: public DrawCmd
{
public:
	DrawCmd_S(int x, int y, PointSystem *ps, DrawCmd *prev);
	DrawCmd_S(int x, int y, std::vector<int> vals, PointSystem *ps, DrawCmd *prev);

	// Init this S command; generate controlpoints
	void Init();

	wxString ToString();

	bool closed;
};

class ASSDrawEngine : public GUI::AGGWindow
{
public:
	ASSDrawEngine(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL);
	virtual ~ASSDrawEngine();

	virtual void SetDrawCmdSet(wxString set) { drawcmdset = set; }

	virtual void ResetEngine(bool addM = true);
	virtual void RefreshDisplay();

	void FitToViewPoint(int hmargin, int vmargin);
	void SetFitToViewPointOnNextPaint(int hmargin = -1, int vmargin = -1);

	PointSystem* _PointSystem() { return pointsys; }

	virtual int ParseASS(wxString str);
	virtual wxString GenerateASS();

	// drawing
	virtual void OnPaint(wxPaintEvent &event);

	// -------------------- adding new commands ----------------------------
	virtual DrawCmd* AppendCmd(CMDTYPE type, int x, int y) { return AppendCmd(NewCmd(type, x, y)); }
	virtual DrawCmd* AppendCmd(DrawCmd* cmd);

	// create draw command of type 'type' and m_point (x, y), insert to the list after the _cmd and return it
	virtual DrawCmd* InsertCmd(CMDTYPE type, int x, int y, DrawCmd* _cmd);
	// insert draw command cmd after _cmd
	virtual void InsertCmd(DrawCmd* cmd, DrawCmd* _cmd);

	DrawCmd* NewCmd(CMDTYPE type, int x, int y);

	// -------------------- read/modify commands ---------------------------
	virtual DrawCmdList::iterator Iterator() { return cmds.begin(); }
	virtual DrawCmdList::iterator IteratorEnd() { return cmds.end(); }

	virtual DrawCmd* LastCmd();

	// move all points by relative amount of x, y coordinates
	virtual void MovePoints(int x, int y);

	// transform all points using the calculation:
	//   | (m11)  (m12) | x | (x - mx) | + | nx |
	//   | (m21)  (m22) |   | (y - my) |   | ny |
	virtual void Transform(float m11, float m12, float m21, float m22, float mx, float my, float nx, float ny);

	// returns some DrawCmd if its m_point = (x, y)
	virtual DrawCmd* PointAt(int x, int y);
	// returns some DrawCmd if one of its control point = (x, y) also set &point to refer to that control point
	virtual DrawCmd* ControlAt(int x, int y, Point* &point);

	virtual bool DeleteCommand(DrawCmd* cmd);

	// Colours
	agg::rgba rgba_shape;
	PixelFormat::AGGType::color_type color_bg;

protected:
	typedef agg::renderer_base<PixelFormat::AGGType> RendererBase;
	typedef agg::renderer_primitives<RendererBase> RendererPrimitives;
	typedef agg::renderer_scanline_aa_solid<RendererBase> RendererSolid;

	typedef agg::conv_transform<agg::path_storage> ConvTrans;

	typedef agg::conv_transform<agg::path_storage, agg::trans_affine> ConvTransAffine;
	typedef agg::conv_curve<ConvTransAffine> ConvCurveTransAffine;
	typedef agg::conv_dash<ConvCurveTransAffine> ConvDashCurveTransAffine;

	typedef agg::conv_curve<agg::path_storage> ConvCurve;
	typedef agg::conv_dash<ConvCurve> ConvDashCurve;
	typedef agg::conv_stroke<ConvDashCurve> ConvStrokeDashCurve;

	enum DRAWCMDMODE {
		NORMAL,
		CTRL_LN,
		HILITE
	};

	DrawCmdList cmds;
	wxString drawcmdset;

	PointSystem* pointsys;

	// for FitToViewPoint feature
	bool setfitviewpoint;
	int fitviewpoint_vmargin, fitviewpoint_hmargin;

	// scanline stuff
	agg::rasterizer_scanline_aa<> rasterizer;
	agg::scanline_p8  scanline;
	void render_scanlines_aa_solid(RendererBase& rbase, agg::rgba rbga, bool affectboundaries = true);
	void render_scanlines(RendererSolid& rsolid, bool affectboundaries = true);
	int rendered_min_x, rendered_min_y, rendered_max_x, rendered_max_y;
	void UpdateRenderedBoundCoords(bool rendered_fresh = false);

	agg::path_storage m_path;
	agg::path_storage b_path;
	ConvTransAffine *rm_path;
	ConvTransAffine *rb_path;
	ConvCurveTransAffine *rm_curve;

	void draw();
	virtual void ConstructPathsAndCurves(agg::trans_affine& mtx, ConvTransAffine*& _rm_path, ConvTransAffine*& _rb_path, ConvCurveTransAffine*& _rm_curve);
	virtual void DoDraw(RendererBase& rbase, RendererPrimitives& rprim, RendererSolid& rsolid, agg::trans_affine& mtx);
	virtual void Draw_Clear(RendererBase& rbase);
	virtual void Draw_Draw(RendererBase& rbase, RendererPrimitives& rprim, RendererSolid& rsolid, agg::trans_affine& mtx, agg::rgba color);
	bool refresh_called;

	// set stuff to connect two drawing commands cmd1 and cmd2 such that cmd1 comes right before cmd2
	virtual void ConnectSubsequentCmds(DrawCmd* cmd1, DrawCmd* cmd2);

	virtual void AddDrawCmdToAGGPathStorage(DrawCmd* cmd, agg::path_storage& path, DRAWCMDMODE mode = NORMAL);

	virtual std::vector<bool> PrepareC1ContData();

	DECLARE_EVENT_TABLE()
};

namespace agg
{
	class simple_polygon_vertex_source
	{
	public:
		simple_polygon_vertex_source(const double* polygon, unsigned np, bool roundoff = false, bool close = true) : m_polygon(polygon), m_num_points(np), m_vertex(0), m_roundoff(roundoff), m_close(close) { }

		void close(bool f) { m_close = f; }
		bool close() const { return m_close; }

		void rewind(unsigned) { m_vertex = 0; }

		unsigned vertex(double* x, double* y)
		{
			if(m_vertex > m_num_points) return path_cmd_stop;
			if(m_vertex == m_num_points)
			{
				++m_vertex;
				return path_cmd_end_poly | (m_close ? path_flags_close : 0);
			}
			*x = m_polygon[m_vertex * 2];
			*y = m_polygon[m_vertex * 2 + 1];
			if(m_roundoff)
			{
				*x = floor(*x) + 0.5;
				*y = floor(*y) + 0.5;
			}
			++m_vertex;
			return (m_vertex == 1) ? path_cmd_move_to : path_cmd_line_to;
		}

	private:
		const double* m_polygon;
		unsigned m_num_points;
		unsigned m_vertex;
		bool     m_roundoff;
		bool     m_close;
	};
}

typedef agg::simple_polygon_vertex_source aggpolygon;
