/* See COPYING file for copyright and license details. */

#include "adwm.h"
#include "layout.h"

static void initlayout_TILE(Monitor *m, View *v, char code);
static void addclient_TILE(Client *c, Bool focusme, Bool raiseme);
static void delclient_TILE(Client *c);
static void raise_TILE(Client *c);
static void lower_TILE(Client *c);
static void raiselower_TILE(Client *c);
static Bool isfloating_TILE(Client *c, View *v);
static void getdecor_TILE(Client *c, View *v, ClientGeometry * g);
static void arrange_TILE(View *v);
static void setnmaster_TILE(View *v, SetValue how, int n);
static void rotate_TILE(Client *c, View *v, RotateDirection dir, RotateArea area);
static void zoom_TILE(Client *c);
static void zoomfloat_TILE(Client *c);
static void togglefloating_TILE(Client *c);
static void togglefill_TILE(Client *c);
static void togglefull_TILE(Client *c);
static void togglemax_TILE(Client *c);
static void togglemaxv_TILE(Client *c);
static void togglemaxh_TILE(Client *c);
static void togglelhalf_TILE(Client *c);
static void togglerhalf_TILE(Client *c);
static void toggleshade_TILE(Client *c);
static void toggledectiled_TILE(Client *c);

Arangement adwm_arrangement = {
	.name = "tile",
	.initlayout = &initlayout_TILE,
	.addclient = &addclient_TILE,
	.delclient = &delclient_TILE,
	.raise = &raise_TILE,
	.lower = &lower_TILE,
	.raiselower = &raiselower_TILE,
	.isfloating = &isfloating_TILE,
	.getdecor = &getdecor_TILE,
	.arrange = &arrange_TILE,
	.setnmaster = &setnmaster_TILE,
	.rotate = &rotate_TILE,
	.zoom = &zoom_TILE,
	.zoomfloat = &zoomfloat_TILE,
	.togglefloating = &togglefloating_TILE,
	.togglefill = &togglefill_TILE,
	.togglefull = &togglefull_TILE,
	.togglemax = &togglemax_TILE,
	.togglemaxv = &togglemaxv_TILE,
	.togglemaxh = &togglemaxh_TILE,
	.togglelhalf = &togglelhalf_TILE,
	.togglerhalf = &togglerhalf_TILE,
	.toggleshade = &toggleshade_TILE,
	.toggledectiled = &toggledectiled_TILE,
};


/*
 * This is a master/stacking area tile layout ala dwm, both in a tiled an
 * floating version.
 */

static void
initlayout_TILE(Monitor *m, View *v, char code)
{
}

static void
inittrees_TILE(void)
{
	if (adwm_arrangement.trees)
		return;
	adwm_arrangement.trees =
		calloc(nscr * MAXTAGS, sizeof(*adwm_arrangement.trees));
	return;
}

static Tree *
inittree_TILE(View *v)
{
	Tree *t, **slot;
	Monitor *m;
	Workarea wa;
	int b;

	inittrees_TILE();
	slot = adwm_arrangement.trees + scr->screen * MAXTAGS + v->index;
	if ((t = *slot))
		return t;

	/* initialize tree for view v on screen scr->screen */
	if (!(m = v->curmon))
		m = nearmonitor();
	t = *slot = calloc(1, sizeof(*t));
	t->type = TreeTypeTree;
	t->view = v->index;
	t->orient = FIXME;
	getworkarea(m, &wa);
	b = scr->style.border;
	t->c.b = t->r.b = t->s.b = b;
	t->c.x = t->r.x = t->s.x = wa.x;
	t->c.y = t->r.y = t->s.y = wa.y;
	t->c.w = t->r.w = t->s.w = wa.w - 2 * b;
	t->c.h = t->r.h = t->s.h = wa.h - 2 * b;



}

static void
initdocks_TILE(void)
{
	unsigned i, nmons = 0;
	AScreen *s;

	if (adwm_arrangement.docks)
		return;
	for (s = screens, i = 0; i < nscr; i++, s++) {
		if (!s->managed)
			continue;
		nmons = max(nmons, s->nmons);
	}
	nmons = max(nmons, 8);
	adwm_arrangement.docks =
		calloc(nscr * nmons, sizeof(*adwm_arrangement.docks));
	adwm_arrangement.nmons = nmons;
}

static Tree *
initdock_TILE(View *v)
{
	Tree *t, **slot;
	Monitor *m;
	Workarea wa;
	int b;

	inittrees_TILE();
	if (!(m = v->curmon))
		return NULL;
	slot = adwm_arrangement.tree + scr->screen * adwm_arrangement.nmons + m->num;
	if ((t = slot))
		return t;
	t = *slot = calloc(1, sizeof(*t));
	t->type = TreeTypeTree;
	t->view = -1;
	t->orient = FIXME;
	getworksarea(m, &wa);
	b = scr->style.border;
	t->c.b = t->r.b = t->s.b = b;
	t->c.x = t->r.x = t->s.x = wa.x;
	t->c.y = t->r.y = t->s.y = wa.y;
	t->c.w = t->r.w = t->s.w = wa.w - 2 * b;
	t->c.h = t->r.h = t->s.h = wa.h - 2 * b;


	/* initialize tree for dock on screen scr->screen */
}

static void
initview_TILE(Client *c, View **view, Tree **tree)
{
	View *v;
	Tree *t;

	/* current visible view and monitor */
	if (!(v = clientview(c)))  {
		/* current view, no monitor */
		if (!(v = onview(c))) {
			/* monitor nearest pointer */
			v = nearview(c);
			c->tags = v->seltags;
		}
	}
	t = inittree_TILE(v);
	*view = v;
	*tree = t;
}

/*
 * We might need to know the client that was selected or focused at the time
 * that the client was originally launched (using startup notification), or at
 * least whether the client is allowed to steal the focus or not and the user
 * preference.  So, for example, for aside attachment, when the client is
 * allowed to steal the focus, we can attach where the focus is.  The 'front'
 * argument is true when startup notification permits the client to take the
 * focus, and false otherwise.  We can use aside or bside attachment settings
 * and focusNewWindows settings to determine where to place the client in the
 * tree.
 */
static void
addclient_TILE(Client *c, Bool focusme, Bool raiseme)
{
	if (!c->can.move) {
		int mx, my;
		View *cv;

		/* double check the monitor: wnck task bars figure the window is on the
		   monitor containing the center of the window. */
		mx = c->s.x + c->s.w / 2 + c->s.b;
		my = c->s.y + c->s.h / 2 + c->s.b;

		if (!(cv = getview(mx, my)))
			cv = closestview(mx, my);
		c->tags = cv->seltags;
	}
	attach(c, scr->options.attachaside);
	attachclist(c);
	attachflist(c, focusme);
	attachstack(c, raiseme);
	ewmh_update_net_client_list();
	if (c->is.managed)
		ewmh_update_net_window_desktop(c);
	initview_TILE(c);
}

static void
delclient_TILE(Client *c)
{
}

static void
raise_TILE(Client *c)
{
	detachstack(c);
	attachstack(c, True);
	restack();
}

static void
lower_TILE(Client *c)
{
	detachstack(c);
	attachstack(c, False);
	restack();
}

static void
raiselower_TILE(Client *c)
{
	if (c == scr->stack)
		lower_TILE(c);
	else
		raise_TILE(c);
}

/*
 * The following operations are per layout:
 *
 * arrangedock()
 * arrangefloats()
 * arrangemon()
 * attach() <= should call addclient()
 * isfloating() <= remove
 * configurerequest() <= rewrite to use other functions
 * detach() <= should call delclient()
 * enternotify() <= rewrite to use skip.sloppy only.
 * gavefocus()
 *
 *
 * -- the idea of raising or lowering a client is a layout concept --
 * raiseclient()
 * lowerclient()
 * raiselower()
 *
 * setfocus()
 * focus()
 * focusforw() <= for arrange list only
 * focusback() <= for arrange list only
 * focuslast() <= for arrange list only
 * focusnext() <= for arrange list only
 * focusprev() <= for arrange list only
 *
 * -- pretty much a characteristic of the client regardless of layout or view --
 *
 * iconify() ?
 * deiconify() ?
 * iconifyall() ?
 * deiconifyall() ?
 *
 * togglehidden() ?
 *  show()
 *  hide()
 *
 * toggleshowing() ?
 *  hideall()
 *  showall()
 *
 *
 * setnmaster()
 *
 * setborder() ? could act on view, currently on screen
 * incborder() ? could act on view, currently on screen
 * decborder() ? could act on view, currently on screen
 *
 * setmargin() ? could act on view, currently on screen
 * incmargin() ? could act on view, currently on screen
 * decmargin() ? could act on view, currently on screen
 *
 * movresizekb()
 *	-- only used for animated tile swapping during move --
 *	ontiled()
 *	ondockapp()
 *	onstacked()
 *	swapstacked()
 *
 * m_move()
 *  mousemove() ?
 *	move_begin()
 *	move_cancel()
 *	move_finish()
 *
 * m_resize() ?
 *  mouseresize() ?
 *	mouseresize_from() ?
 *	    resize_begin()
 *	    resize_cancel()
 *	    resize_finish()
 *
 * -- only used by layout algorithms --
 *
 * nexttiled()
 * prevtiled()
 * nextdockapp()
 * prevdockapp()
 *
 * -- only used by full floating placement algorithms --
 *
 * nextplaced()
 * qsort_t2b_l2r()
 * qsort_l2r_t2b()
 * qsort_cascade()
 * place_geom()
 * place_smart()
 * place_minoverlap()
 * place_undermouse()
 * place_random()
 * place()
 *
 * restack_client()
 * restack_belowif()
 * restack()
 * save()
 * restore()
 *
 * updatefloat()
 *	calc_full()
 *	calc_fill()
 *	calc_max()
 *	calc_maxv()
 *	calc_maxh()
 *	calc_lhalf()
 *	calc_rhalf()
 *	get_decor()
 *
 * setlayout() ?
 * setmwfact()
 * newview() ?
 * initlayouts() ?
 *
 * togglestruts() ? <= could toggle tree
 * toggledectiled() ? <= could toggle tree
 * togglefloating() ? <= could toggle leaf
 *
 * togglefill() ? <= could toggle leaf
 * togglefull() ? <= could toggle leaf
 * togglemax() ? <= could toggle leaf
 * togglemaxv() ? <= could toggle leaf
 * togglemaxh() ? <= could toggle leaf
 * togglelhalf() ? <= could toggle leaf
 * togglerhalf() ? <= could toggle leaf
 * toggleshade() ? <= could toggle leaf
 * togglesticky() ? <= could toggle leaf ??
 * togglemin() ? <= could toggle leaf ??
 * toggleabove() ? <= could toggle leaf ??
 * togglebelow() ? <= could toggle leaf ??
 * togglepager() ? XXX no
 * toggletaskbar() ? XXX no
 *
 * rotateview()
 * unrotateview()
 * rotatezone()
 * unrotatezone()
 * rotatewins()
 * unrotatewins()
 *
 * moveto()
 * moveby()
 * snapto()
 * edgeto()
 *
 * zoom()
 *
 *
 * -- only affecting the assembling of lists --
 *
 * k_floating()
 * k_tiled()
 *
 */

static void
arrange_dock_TILE(Monitor *m)
{
}

static void
arrange_TILE(Monitor *cm)
{
	LayoutArgs wa, ma, sa;
	ClientGeometry n, m, s, g;
	Client *c, *mc;
	View *v;
	int i, overlap, th, gh, mg;
	Bool mtdec, stdec, mgdec, sgdec;

	if (!(c = nexttiled(scr->clients, cm)))
		return;

	th = scr->style.titleheight;
	gh = scr->style.gripsheight;
	mg = scr->style.margin;

	getworkarea(cm, (Workarea *) &wa);
	v = &scr->views[cm->curtag];
	for (wa.n = wa.s = ma.s = sa.s = 0, c = nexttiled(scr->clients, cm);
	     c; c = nexttiled(c->next, cm), wa.n++) {
		if (c->is.shaded && (c != sel || !options.autoroll)) {
			wa.s++;
			if (wa.n < v->nmaster)
				ma.s++;
			else
				sa.s++;
		}
	}

	/* window geoms */

	/* master & slave number */
	ma.n = (wa.n > v->nmaster) ? v->nmaster : wa.n;
	sa.n = (ma.n < wa.n) ? wa.n - ma.n : 0;
	XPRINTF("there are %d masters\n", ma.n);
	XPRINTF("there are %d slaves\n", sa.n);

	/* at least one unshaded */
	ma.s = (ma.s && ma.s == ma.n) ? ma.n - 1 : ma.s;
	sa.s = (sa.s && sa.s == sa.n) ? sa.n - 1 : sa.s;

	/* too confusing when unshaded windows are not decorated */
	mtdec = (v->dectiled || ma.s) ? True : False;
	stdec = (v->dectiled || sa.s) ? True : False;
	mgdec = v->dectiled ? True : False;
	sgdec = v->dectiled ? True : False;
#if 0
	if (!v->dectiled && (mdec || sdec))
		v->dectiled = True;
#endif

	/* master and slave work area dimensions */
	switch (v->major) {
	case OrientTop:
	case OrientBottom:
		ma.w = wa.w;
		ma.h = (sa.n > 0) ? wa.h * v->mwfact : wa.h;
		sa.w = wa.w;
		sa.h = wa.h - ma.h;
		break;
	case OrientLeft:
	case OrientRight:
	default:
		ma.w = (sa.n > 0) ? v->mwfact * wa.w : wa.w;
		ma.h = wa.h;
		sa.w = wa.w - ma.w;
		sa.h = wa.h;
		break;
	}
	ma.b = (ma.n > 1 || mtdec) ? scr->style.border : 0;
	ma.g = mg > ma.b ? mg - ma.b : 0;
	ma.th = th + 2 * (ma.b + ma.g);

	sa.b = scr->style.border;
	sa.g = mg > sa.b ? mg - sa.b : 0;
	sa.th = th + 2 * (sa.b + sa.g);

	overlap = (sa.n > 0) ? ma.b : 0;
	XPRINTF("overlap is %d\n", overlap);

	/* master and slave work area position */
	switch (v->major) {
	case OrientBottom:
		XPRINTF("major orientation is masters bottom(%d)\n", v->major);
		ma.x = wa.x;
		ma.y = wa.y + sa.h;
		sa.x = wa.x;
		sa.y = wa.y;
		ma.y -= overlap;
		ma.h += overlap;
		break;
	case OrientRight:
	default:
		XPRINTF("major orientation is masters right(%d)\n", v->major);
		ma.x = wa.x + sa.w;
		ma.y = wa.y;
		sa.x = wa.x;
		sa.y = wa.y;
		ma.x -= overlap;
		ma.w += overlap;
		break;
	case OrientLeft:
		XPRINTF("major orientation is masters left(%d)\n", v->major);
		ma.x = wa.x;
		ma.y = wa.y;
		sa.x = wa.x + ma.w;
		sa.y = wa.y;
		ma.w += overlap;
		break;
	case OrientTop:
		XPRINTF("major orientation is masters top(%d)\n", v->major);
		ma.x = wa.x;
		ma.y = wa.y;
		sa.x = wa.x;
		sa.y = wa.y + ma.h;
		ma.h += overlap;
		break;
	}
	XPRINTF("master work area %dx%d+%d+%d:%d\n", ma.w, ma.h, ma.x, ma.y, ma.b);
	XPRINTF("slave  work area %dx%d+%d+%d:%d\n", sa.w, sa.h, sa.x, sa.y, sa.b);

	/* master tile dimensions */
	switch (v->minor) {
	case OrientTop:
	case OrientBottom:
		m.w = ma.w;
		m.h = ((ma.n > 0) ? (ma.h - ma.s * ma.th) / (ma.n - ma.s) : ma.h) + ma.b;
		break;
	case OrientLeft:
	case OrientRight:
	default:
		m.w = ((ma.n > 0) ? ma.w / ma.n : ma.w) + ma.b;
		m.h = ma.h;
		break;
	}
	m.b = ma.b;
	m.t = mtdec ? th : 0;
	m.g = mgdec ? gh : 0;

	/* slave tile dimensions */
	switch (v->major) {
	case OrientTop:
	case OrientBottom:
		s.w = ((sa.n > 0) ? sa.w / sa.n : 0) + sa.b;
		s.h = sa.h;
		break;
	case OrientLeft:
	case OrientRight:
	default:
		s.w = sa.w;
		s.h = ((sa.n > 0) ? (sa.h - sa.s * sa.th) / (sa.n - sa.s) : 0) + sa.b;
		break;
	}
	s.b = sa.b;
	s.t = stdec ? th : 0;
	s.g = sgdec ? gh : 0;

	/* position of first master */
	switch (v->minor) {
	case OrientTop:
		XPRINTF("minor orientation is top to bottom(%d)\n", v->minor);
		m.x = ma.x;
		m.y = ma.y;
		break;
	case OrientBottom:
		XPRINTF("minor orientation is bottom to top(%d)\n", v->minor);
		m.x = ma.x;
		m.y = ma.y + ma.h - m.h;
		break;
	case OrientLeft:
		XPRINTF("minor orientation is left to right(%d)\n", v->minor);
		m.x = ma.x;
		m.y = ma.y;
		break;
	case OrientRight:
	default:
		XPRINTF("minor orientation is right to left(%d)\n", v->minor);
		m.x = ma.x + ma.w - m.w;
		m.y = ma.y;
		break;
	}
	XPRINTF("initial master %dx%d+%d+%d:%d\n", m.w, m.h, m.x, m.y, m.b);

	i = 0;
	c = mc = nexttiled(scr->clients, cm);

	/* lay out the master area */
	n = m;

	for (; c && i < ma.n; c = nexttiled(c->next, cm)) {
		if (c->is.max) {
			c->is.max = False;
			ewmh_update_net_window_state(c);
		}
		g = n;
		g.t = c->has.title ? g.t : 0;
		g.g = c->has.grips ? g.g : 0;
		if ((c->was.shaded = c->is.shaded) && (c != sel || !options.autoroll))
			if (!ma.s)
				c->is.shaded = False;
		g.x += ma.g;
		g.y += ma.g;
		g.w -= 2 * (ma.g + g.b);
		g.h -= 2 * (ma.g + g.b);
		if (!c->is.moveresize) {
			XPRINTF("CALLING reconfigure()\n");
			reconfigure(c, &g);
		} else {
			ClientGeometry C = g;

			/* center it where it was before */
			C.x = (c->c.x + c->c.w / 2) - C.w / 2;
			C.y = (c->c.y + c->c.h / 2) - C.h / 2;
			XPRINTF("CALLING reconfigure()\n");
			reconfigure(c, &C);
		}
		if (c->is.shaded && (c != sel || !options.autoroll))
			if (ma.s)
				n.h = ma.th;
		i++;
		switch (v->minor) {
		case OrientTop:
			n.y += n.h - n.b;
			if (i == ma.n - 1)
				n.h = ma.h - (n.y - ma.y);
			break;
		case OrientBottom:
			n.y -= n.h - n.b;
			if (i == ma.n - 1) {
				n.h += (n.y - ma.y);
				n.y = ma.y;
			}
			break;
		case OrientLeft:
			n.x += n.w - n.b;
			if (i == ma.n - 1)
				n.w = ma.w - (n.x - ma.x);
			break;
		case OrientRight:
		default:
			n.x -= n.w - n.b;
			if (i == ma.n - 1) {
				n.w += (n.x - ma.x);
				n.x = ma.x;
			}
			break;
		}
		if (ma.s && c->is.shaded && (c != sel || !options.autoroll)) {
			n.h = m.h;
			ma.s--;
		}
		c->is.shaded = c->was.shaded;
	}

	/* position of first slave */
	switch (v->major) {
	case OrientRight:
		XPRINTF("slave orientation is top to bottom(%d)\n", v->major);
		s.x = sa.x;
		s.y = sa.y;
		break;
	case OrientLeft:
		XPRINTF("slave orientation is bottom to top(%d)\n", v->major);
		s.x = sa.x;
		s.y = sa.y + sa.h - s.h;
		break;
	case OrientTop:
		XPRINTF("slave orientation is left to right(%d)\n", v->major);
		s.x = sa.x;
		s.y = sa.y;
		break;
	case OrientBottom:
	default:
		XPRINTF("slave orientation is right to left(%d)\n", v->major);
		s.x = sa.x + sa.w - s.w;
		s.y = sa.y;
		break;
	}
	XPRINTF("initial slave  %dx%d+%d+%d:%d\n", s.w, s.h, s.x, s.y, s.b);

	/* lay out the slave area - always top->bot, left->right */
	n = s;

	for (; c && i < wa.n; c = nexttiled(c->next, cm)) {
		if (c->is.max) {
			c->is.max = False;
			ewmh_update_net_window_state(c);
		}
		g = n;
		g.t = c->has.title ? g.t : 0;
		g.g = c->has.grips ? g.g : 0;
		if ((c->was.shaded = c->is.shaded) && (c != sel || !options.autoroll))
			if (!sa.s)
				c->is.shaded = False;
		g.x += sa.g;
		g.y += sa.g;
		g.w -= 2 * (sa.g + g.b);
		g.h -= 2 * (sa.g + g.b);
		if (!c->is.moveresize) {
			XPRINTF("CALLING reconfigure()\n");
			reconfigure(c, &g);
		} else {
			ClientGeometry C = g;

			/* center it where it was before */
			C.x = (c->c.x + c->c.w / 2) - C.w / 2;
			C.y = (c->c.y + c->c.h / 2) - C.h / 2;
			XPRINTF("CALLING reconfigure()\n");
			reconfigure(c, &C);
		}
		if (c->is.shaded && (c != sel || !options.autoroll))
			if (sa.s)
				n.h = sa.th;
		i++;
		switch (v->major) {
		case OrientTop:
			/* left to right */
			n.x += n.w - n.b;
			if (i == wa.n - 1)
				n.w = sa.w - (n.x - sa.x);
			break;
		case OrientBottom:
			/* right to left */
			n.x -= n.w - n.b;
			if (i == wa.n - 1) {
				n.w += (n.x - sa.x);
				n.x = sa.x;
			}
			break;
		case OrientLeft:
			/* bottom to top */
			n.y -= n.h - n.b;
			if (i == wa.n - 1) {
				n.h += (n.y - sa.y);
				n.y = sa.y;
			}
			break;
		case OrientRight:
		default:
			/* top to bottom */
			n.y += n.h - n.b;
			if (i == wa.n - 1)
				n.h = sa.h - (n.y - sa.y);
			break;
		}
		if (sa.s && c->is.shaded && (c != sel || !options.autoroll)) {
			n.h = s.h;
			sa.s--;
		}
		c->is.shaded = c->was.shaded;
	}
}

/*
 * isfloating()
 *	configurerequest()
 *	enternotify()
 *	focus()
 *	mouseresizekb()
 *	move_begin() <= mousemove()
 *	move_cancel() <= mousemove()
 *	move_finish() <= mousemove()
 *	mousemove()
 *	resize_begin()
 *	resize_cancel()
 *	resize_finish()
 *	updatefloat()
 *	moveto()
 *	moveby()
 *	snapto()
 *	edgeto()
 *	m_zoom()
 *	clientmessage() (_XA_NET_MOVERESIZE_WINDOW)
 * updatefloat()
 *	arrangefloats()
 *	moveresizekb()
 *	move_begin() <= mousemove()
 *	move_cancel() <= mousemove()
 *	move_finish() <= mousemove()
 *	resize_begin() <= mouseresize_from()
 *	resize_cancel() <= mouseresize_from()
 *	resize_finish() <= mouseresize_from()
 *	togglefloating()
 *	togglefill()
 *	togglefull()
 *	togglemax()
 *	togglemaxv()
 *	togglemaxh()
 *	togglelhalf()
 *	togglerhalf()
 *	toggleshade()
 * k_floating()
 * k_tiled()
 */

Bool
move_begin_TILE(Client *c, Monitor *m, Bool toggle, int move)
{
	return True;		/* FIXME */
}

Bool
move_cancel_TILE(Client *c, Monitor *m, ClientGeometry * orig)
{
	return True;		/* FIXME */
}

Bool
move_finish_TILE(Client *c, Monitor *m)
{
	return True;		/* FIXME */
}

Bool
resize_begin_TILE(Client *c, Monitor *m, Bool toggle, int from)
{
	return True;		/* FIXME */
}

Bool
resize_cancel_TILE(Client *c, Monitor *m, ClientGeometry * orig)
{
	return True;		/* FIXME */
}

Bool
resize_finish_TILE(Client *c, Monitor *m)
{
	return True;		/* FIXME */
}

static void
initlayout_TILE(Monitor *m, View *v, char code)
{
}

static void
addclient_TILE(Client *c)
{
}

static void
delclient_TILE(Client *c)
{
}

static void
raise_TILE(Client *c)
{
}

static void
lower_TILE(Client *c)
{
}

static void
raiselower_TILE(Client *c)
{
}

static Bool
isfloating_TILE(Client *c, View *v)
{
	return False;
}

static void
getdecor_TILE(Client *c, View *v, ClientGeometry *g)
{
}

static void
arrange_TILE(View *v)
{
}


static void
setnmaster_TILE(View *v, SetValue how, int num)
{
}

static void
rotate(Client *c, View *v, RotateDirection dir, RotateArea area)
{
}

static void
zoom_TILE(Client *c)
{
}

static void
zoomfloat_TILE(Client *c)
{
}

static void
togglefloating_TILE(Client *c)
{
}

static void
togglefill_TILE(Client *c)
{
}

static void
togglefull_TILE(Client *c)
{
}

static void
togglemax_TILE(Client *c)
{
}

static void
togglemaxv_TILE(Client *c)
{
}

static void
togglemaxh_TILE(Client *c)
{
}

static void
togglelhalf(Client *c)
{
}

static void
togglerhalf(Client *c)
{
}

static void
toggleshade_TILE(Client *c)
{
}

static void
togglesdectiled_TILE(Client *c)
{
}

Layout adwm_layouts[] = {
	/* *INDENT-OFF* */
	/* function		symbol	features				major		minor		placement	 */
	{ &arrange_TILE,	't',	MWFACT | NMASTER | ZOOM | ROTL | MMOVE,	OrientLeft,	OrientBottom,	ColSmartPlacement },
	{ &arrange_TILE,	'b',	MWFACT | NMASTER | ZOOM | ROTL | MMOVE,	OrientBottom,	OrientLeft,	ColSmartPlacement },
	{ &arrange_TILE,	'u',	MWFACT | NMASTER | ZOOM | ROTL | MMOVE,	OrientTop,	OrientRight,	ColSmartPlacement },
	{ &arrange_TILE,	'l',	MWFACT | NMASTER | ZOOM | ROTL | MMOVE,	OrientRight,	OrientTop,	ColSmartPlacement },
	{ NULL,			'\0',	0,					0,		0,		0		  }
	/* *INDENT-ON* */
};

// vim: set sw=8 tw=80 com=srO\:/**,mb\:*,ex\:*/,srO\:/*,mb\:*,ex\:*/,b\:TRANS foldmarker=@{,@} foldmethod=marker:
