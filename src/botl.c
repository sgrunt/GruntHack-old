/*	SCCS Id: @(#)botl.c	3.4	1996/07/15	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#if defined(HPMON) && defined(TEXTCOLOR)
# ifndef WINTTY_H
#  include "wintty.h"
# endif
#endif

#ifdef OVL0
extern const char *hu_stat[];	/* defined in eat.c */

const char * const enc_stat[] = {
	"",
	"Burdened",
	"Stressed",
	"Strained",
	"Overtaxed",
	"Overloaded"
};

STATIC_DCL void NDECL(bot1);
STATIC_DCL void NDECL(bot2);
#endif /* OVL0 */

/* MAXCO must hold longest uncompressed status line, and must be larger
 * than COLNO
 *
 * longest practical second status line at the moment is
 *	Astral Plane $:12345 HP:700(700) Pw:111(111) AC:-127 Xp:30/123456789
 *	T:123456 Satiated Conf FoodPois Ill Blind Stun Hallu Overloaded
 * -- or somewhat over 130 characters
 */
#if COLNO <= 200
#define MAXCO 220
#else
#define MAXCO (COLNO+20)
#endif

#ifndef OVLB
STATIC_DCL int mrank_sz;
#else /* OVLB */
STATIC_OVL NEARDATA int mrank_sz = 0; /* loaded by max_rank_sz (from u_init) */
#endif /* OVLB */

STATIC_DCL const char *NDECL(rank);

#ifdef OVL1

/* convert experience level (1..30) to rank index (0..8) */
int
xlev_to_rank(xlev)
int xlev;
{
	return (xlev <= 2) ? 0 : (xlev <= 30) ? ((xlev + 2) / 4) : 8;
}

#if 0	/* not currently needed */
/* convert rank index (0..8) to experience level (1..30) */
int
rank_to_xlev(rank)
int rank;
{
	return (rank <= 0) ? 1 : (rank <= 8) ? ((rank * 4) - 2) : 30;
}
#endif

const char *
rank_of(lev, monnum, female)
	int lev;
	short monnum;
	boolean female;
{
	register struct Role *role;
	register int i;


	/* Find the role */
	for (role = (struct Role *) roles; role->name.m; role++)
	    if (monnum == role->malenum || monnum == role->femalenum)
	    	break;
	if (!role->name.m)
	    role = &urole;

	/* Find the rank */
	for (i = xlev_to_rank((int)lev); i >= 0; i--) {
	    if (female && role->rank[i].f) return (role->rank[i].f);
	    if (role->rank[i].m) return (role->rank[i].m);
	}

	/* Try the role name, instead */
	if (female && role->name.f) return (role->name.f);
	else if (role->name.m) return (role->name.m);
	return ("Player");
}


STATIC_OVL const char *
rank()
{
	return(rank_of(u.ulevel, Role_switch, flags.female));
}

int
title_to_mon(str, rank_indx, title_length)
const char *str;
int *rank_indx, *title_length;
{
	register int i, j;


	/* Loop through each of the roles */
	for (i = 0; roles[i].name.m; i++)
	    for (j = 0; j < 9; j++) {
	    	if (roles[i].rank[j].m && !strncmpi(str,
	    			roles[i].rank[j].m, strlen(roles[i].rank[j].m))) {
	    	    if (rank_indx) *rank_indx = j;
	    	    if (title_length) *title_length = strlen(roles[i].rank[j].m);
	    	    return roles[i].malenum;
	    	}
	    	if (roles[i].rank[j].f && !strncmpi(str,
	    			roles[i].rank[j].f, strlen(roles[i].rank[j].f))) {
	    	    if (rank_indx) *rank_indx = j;
	    	    if (title_length) *title_length = strlen(roles[i].rank[j].f);
	    	    return ((roles[i].femalenum != NON_PM) ?
	    	    		roles[i].femalenum : roles[i].malenum);
	    	}
	    }
	return NON_PM;
}

#endif /* OVL1 */
#ifdef OVLB

void
max_rank_sz()
{
	register int i, r, maxr = 0;
	for (i = 0; i < 9; i++) {
	    if (urole.rank[i].m && (r = strlen(urole.rank[i].m)) > maxr) maxr = r;
	    if (urole.rank[i].f && (r = strlen(urole.rank[i].f)) > maxr) maxr = r;
	}
	mrank_sz = maxr;
	return;
}


#endif /* OVLB */

#ifdef OVL0

static short oldrank = 0, olddata = 0, oldalign = 0, olduz = 1,
             oldstr = 1, olddex = 1, oldint = 1,
	     oldwis = 1, oldcon = 1, oldcha = 1,
	     oldac = 10;
static long  oldscore = 0, oldgold = 0, oldxp = 0, oldmoves = 0;
#ifdef REALTIME_ON_BOTL
static time_t oldtime = 0;
#endif

#ifdef SCORE_ON_BOTL
long
botl_score()
{
    int deepest = deepest_lev_reached(FALSE);
#ifndef GOLDOBJ
    long ugold = u.ugold + hidden_gold();

    if ((ugold -= u.ugold0) < 0L) ugold = 0L;
    return ugold + u.urexp + (long)(50 * (deepest - 1))
#else
    long umoney = money_cnt(invent) + hidden_gold();

    if ((umoney -= u.umoney0) < 0L) umoney = 0L;
    return umoney + u.urexp + (long)(50 * (deepest - 1))
#endif
			  + (long)(deepest > 30 ? 10000 :
				   deepest > 20 ? 1000*(deepest - 20) : 0);
}
#endif

#if defined(WIN32) && !defined(WIN32CON)
#define _term_start_color(x)
#define _term_end_color()
#else
#define _term_start_color(x) if (iflags.use_color) term_start_color(x)
#define _term_end_color()    if (iflags.use_color) term_end_color()
#endif

#ifdef DUMP_LOG
void bot1str(char *newbot1)
#else
STATIC_OVL void
bot1()
#endif
{
#ifndef DUMP_LOG
	char newbot1[MAXCO];
#endif
	register char *nb;
	register int i,j;

	boolean changed = FALSE;
		
	char mbot[BUFSZ];
	
	Strcpy(newbot1, plname);
	if('a' <= newbot1[0] && newbot1[0] <= 'z') newbot1[0] += 'A'-'a';
	newbot1[10] = 0;
	Sprintf(nb = eos(newbot1)," the ");
	        
	if (Upolyd) {
		int k = 0;

		Strcpy(mbot, mons[u.umonnum].mname);
		while(mbot[k] != 0) {
		    if ((k == 0 || (k > 0 && mbot[k-1] == ' ')) &&
					'a' <= mbot[k] && mbot[k] <= 'z')
			mbot[k] += 'A' - 'a';
		    k++;
		}

#ifdef TEXTCOLOR
                if ((changed = (olddata != u.umonnum)))
	            _term_start_color(CLR_BRIGHT_BLUE);
#endif
	    if (oldmoves != moves)
		olddata = u.umonnum;
	} else {
	    int newrank = xlev_to_rank(u.ulevel);
#ifdef TEXTCOLOR
            if (changed = (olddata != 0))
	        _term_start_color(CLR_BRIGHT_BLUE);
	    else if (changed = (oldrank < newrank))
	        _term_start_color(CLR_GREEN);
	    else if (changed = (oldrank > newrank))
	        _term_start_color(CLR_RED);
#endif /*TEXTCOLOR*/
	    Strcpy(mbot, rank());
	    if (oldmoves != moves)
	        oldrank = newrank,
	        olddata = 0;
	}
		
	Sprintf(nb = eos(nb),  mbot);
	Sprintf(nb = eos(nb), "  ");

	i = mrank_sz + 15;
	j = (nb + 2) - newbot1; /* aka strlen(newbot1) but less computation */
	if((i - j) > 0)
		Sprintf(nb = eos(nb),"%*s", i-j, " ");	/* pad with spaces */

        putstr(WIN_STATUS, 0, newbot1);

#ifdef TEXTCOLOR
        if (changed) {
	    _term_end_color();
	}
#endif /*TEXTCOLOR*/

#ifdef TEXTCOLOR
	curs(WIN_STATUS, 1, 0);
	/*putstr(WIN_STATUS, 0, newbot1);*/
        if ((changed = (oldstr < ACURR(A_STR))))
	    _term_start_color(CLR_GREEN);
        else if ((changed = (oldstr > ACURR(A_STR))))
	    _term_start_color(CLR_RED);
#endif /*TEXTCOLOR*/
	if (ACURR(A_STR) > 18) {
		if (ACURR(A_STR) > STR18(100))
		    Sprintf(nb = eos(nb),"St:%2d ",ACURR(A_STR)-100);
		else if (ACURR(A_STR) < STR18(100))
		    Sprintf(nb = eos(nb), "St:18/%02d ",ACURR(A_STR)-18);
		else
		    Sprintf(nb = eos(nb),"St:18/** ");
	} else
		Sprintf(nb = eos(nb), "St:%-1d ",ACURR(A_STR));
#ifdef TEXTCOLOR
	putstr(WIN_STATUS, 0, newbot1);
        if (changed) {
            _term_end_color();
	}
#endif /*TEXTCOLOR*/
	if (oldmoves != moves)
	    oldstr = ACURR(A_STR);

#ifdef TEXTCOLOR
	curs(WIN_STATUS, 1, 0);
	/*putstr(WIN_STATUS, 0, newbot1);*/
        if ((changed = (olddex < ACURR(A_DEX))))
	    _term_start_color(CLR_GREEN);
        else if ((changed = (olddex > ACURR(A_DEX))))
	    _term_start_color(CLR_RED);
#endif /*TEXTCOLOR*/
	Sprintf(nb = eos(nb),
		"Dx:%-1d ", 
		ACURR(A_DEX));
#ifdef TEXTCOLOR
	putstr(WIN_STATUS, 0, newbot1);
        if (changed) {
	       _term_end_color();
        }
#endif /*TEXTCOLOR*/
	if (oldmoves != moves)
	    olddex = ACURR(A_DEX);
        
#ifdef TEXTCOLOR
	curs(WIN_STATUS, 1, 0);
	/*putstr(WIN_STATUS, 0, newbot1);*/
	if ((changed = (oldcon < ACURR(A_CON))))
	    _term_start_color(CLR_GREEN);
        else if ((changed = (oldcon > ACURR(A_CON))))
	    _term_start_color(CLR_RED);
#endif /*TEXTCOLOR*/
	Sprintf(nb = eos(nb),
		"Co:%-1d ", 
		ACURR(A_CON));
#ifdef TEXTCOLOR
	putstr(WIN_STATUS, 0, newbot1);
        if (changed) {
	       _term_end_color();
        }
#endif /*TEXTCOLOR*/
	if (oldmoves != moves)
	    oldcon = ACURR(A_CON);
        
#ifdef TEXTCOLOR
	curs(WIN_STATUS, 1, 0);
	/*putstr(WIN_STATUS, 0, newbot1);*/
	if ((changed = (oldint < ACURR(A_INT))))
	    _term_start_color(CLR_GREEN);
        else if ((changed = (oldint > ACURR(A_INT))))
	    _term_start_color(CLR_RED);
#endif /*TEXTCOLOR*/
	Sprintf(nb = eos(nb),
		"In:%-1d ", 
		ACURR(A_INT));
#ifdef TEXTCOLOR
	putstr(WIN_STATUS, 0, newbot1);
        if (changed) {
	       _term_end_color();
        }
#endif /*TEXTCOLOR*/
	if (oldmoves != moves)
	    oldint = ACURR(A_INT);
        
#ifdef TEXTCOLOR
	curs(WIN_STATUS, 1, 0);
	/*putstr(WIN_STATUS, 0, newbot1);*/
	if ((changed = (oldwis < ACURR(A_WIS))))
	    _term_start_color(CLR_GREEN);
        else if ((changed = (oldwis > ACURR(A_WIS))))
	    _term_start_color(CLR_RED);
#endif /*TEXTCOLOR*/
	Sprintf(nb = eos(nb),
		"Wi:%-1d ", 
		ACURR(A_WIS));
#ifdef TEXTCOLOR
	putstr(WIN_STATUS, 0, newbot1);
        if (changed) {
	       _term_end_color();
        }
#endif /*TEXTCOLOR*/
	if (oldmoves != moves)
	   oldwis = ACURR(A_WIS);
        
#ifdef TEXTCOLOR
	curs(WIN_STATUS, 1, 0);
	/*putstr(WIN_STATUS, 0, newbot1);*/
	if ((changed = (oldcha < ACURR(A_CHA))))
	    _term_start_color(CLR_GREEN);
        else if ((changed = (oldcha > ACURR(A_CHA))))
	    _term_start_color(CLR_RED);
#endif
	Sprintf(nb = eos(nb),
		"Ch:%-1d ", 
		ACURR(A_CHA));
#ifdef TEXTCOLOR
	putstr(WIN_STATUS, 0, newbot1);
        if (changed) {
	       _term_end_color();
        }
#endif /*TEXTCOLOR*/
	if (oldmoves != moves)
	    oldcha = ACURR(A_CHA);

	Sprintf(nb = eos(nb), (u.ualign.type == A_CHAOTIC) ? "  Chaotic" :
			(u.ualign.type == A_NEUTRAL) ? "  Neutral" : "  Lawful");
#ifdef SCORE_ON_BOTL
#ifdef TEXTCOLOR
	curs(WIN_STATUS, 1, 0);
	/*putstr(WIN_STATUS, 0, newbot1);*/
        if ((changed = (oldscore != botl_score())))
	    _term_start_color(CLR_GREEN);
#endif /*TEXTCOLOR*/
	if (flags.showscore)
	    Sprintf(nb = eos(nb), " S:%ld", botl_score());
#ifdef TEXTCOLOR
	putstr(WIN_STATUS, 0, newbot1);
        if (changed) {
	       _term_end_color();
        }
#endif /*TEXTCOLOR*/
	if (oldmoves != moves)
            oldscore = botl_score();
#endif
#ifdef DUMP_LOG
}
STATIC_OVL void
bot1()
{
	char newbot1[MAXCO];

	bot1str(newbot1);
#endif
	curs(WIN_STATUS, 1, 0);
	putstr(WIN_STATUS, 0, newbot1);
}

/* provide the name of the current level for display by various ports */
int
describe_level(buf)
char *buf;
{
	int ret = 1;

	/* TODO:	Add in dungeon name */
	if (Is_knox(&u.uz))
		Sprintf(buf, "%s ", dungeons[u.uz.dnum].dname);
	else if (In_quest(&u.uz))
		Sprintf(buf, "Home %d ", dunlev(&u.uz));
	else if (In_endgame(&u.uz))
		Sprintf(buf,
			Is_astralevel(&u.uz) ? "Astral Plane " : "End Game ");
	else {
		/* ports with more room may expand this one */
		Sprintf(buf, "Dlvl:%-2d ", depth(&u.uz));
		ret = 0;
	}
	return ret;
}

#ifdef DUMP_LOG
void bot2str(newbot2)
char* newbot2;
#else
STATIC_OVL void
bot2()
#endif
{
#ifndef DUMP_LOG
	char  newbot2[MAXCO];
#endif
	register char *nb;
	int hp, hpmax;
#ifdef HPMON
	int hpcolor, hpattr;
	int pwcolor;
#endif
	int cap = near_capacity();

	boolean changed = FALSE;

#ifdef TEXTCOLOR
	/*curs(WIN_STATUS, 1, 1);*/
	/*putstr(WIN_STATUS, 0, newbot2);*/
#endif

	hp = Upolyd ? u.mh : u.uhp;
	hpmax = Upolyd ? u.mhmax : u.uhpmax;

        /* Egregious deaths FTW.*/
	/*if(hp < 0) hp = 0;*/
#ifdef TEXTCOLOR
	/*curs(WIN_STATUS, 1, 1);*/
	/*putstr(WIN_STATUS, 0, newbot2);*/
        if ((changed = (olduz != depth(&u.uz))))
	    _term_start_color(CLR_BRIGHT_BLUE);
#endif /*TEXTCOLOR*/
	(void) describe_level(newbot2);
#ifdef TEXTCOLOR
	putstr(WIN_STATUS, 0, newbot2);
        if (changed) {
	       _term_end_color();
        }
	if (oldmoves != moves)
	    olduz = depth(&u.uz);
#endif /*TEXTCOLOR*/
	curs(WIN_STATUS, 1, 1);
#ifdef TEXTCOLOR
        if ((changed = (oldgold < 
#ifndef GOLDOBJ
                                   u.ugold
#else
                                   money_cnt(invent)
#endif
	)))
	    _term_start_color(CLR_GREEN);
	else if ((changed = (oldgold > 
#ifndef GOLDOBJ
                                   u.ugold
#else
                                   money_cnt(invent)
#endif
	)))
	    _term_start_color(CLR_RED);
#endif /*TEXTCOLOR*/
	Sprintf(nb = eos(newbot2), "%c:%-2ld ", oc_syms[COIN_CLASS],
#ifndef GOLDOBJ
                u.ugold
#else
                money_cnt(invent)
#endif
                );
	
	putstr(WIN_STATUS, 0, newbot2);
#ifdef TEXTCOLOR
        if (changed) _term_end_color();
#endif
        if (oldmoves != moves) oldgold =
#ifndef GOLDOBJ
                u.ugold
#else
                money_cnt(invent)
#endif
        ;

	Sprintf(nb=eos(newbot2),
#ifdef HPMON
		"HP:");
#else /* HPMON */
		"HP:%d(%d)", hp, hpmax);
#endif /* HPMON */
#ifdef HPMON
	curs(WIN_STATUS, 1, 1);
	putstr(WIN_STATUS, 0, newbot2);

	Sprintf(nb = eos(newbot2), "%d(%d)", hp, hpmax);
#if defined(TEXTCOLOR) && (!defined(WIN32) || defined(WIN32CON))
	if (iflags.use_color) {
	  curs(WIN_STATUS, 1, 1);
	  hpattr = ATR_NONE;
	  if(hp == hpmax){
	    hpcolor = NO_COLOR;
	  } else if(hp > (hpmax*2/3)) {
	    hpcolor = CLR_GREEN;
	  } else if(hp <= (hpmax/3)) {
	    hpcolor = CLR_RED;
	    if(hp<=(hpmax/10)) 
	      hpattr = ATR_BLINK;
	  } else {
	    hpcolor = CLR_YELLOW;
	  }
	  if (hpcolor != NO_COLOR)
	    _term_start_color(hpcolor);
	  if(hpattr!=ATR_NONE)term_start_attr(hpattr);
	  putstr(WIN_STATUS, hpattr, newbot2);
	  if(hpattr!=ATR_NONE)term_end_attr(hpattr);
	  if (hpcolor != NO_COLOR)
	    _term_end_color();
	}
#endif /* TEXTCOLOR */
#endif /* HPMON */

	Sprintf(nb = eos(newbot2), " Pw:");
	
	curs(WIN_STATUS, 1, 1);
	putstr(WIN_STATUS, 0, newbot2);
	Sprintf(nb = eos(nb), "%d(%d)", 
		u.uen, u.uenmax);

#ifdef TEXTCOLOR
	if (iflags.use_color) {
	  curs(WIN_STATUS, 1, 1);
	  if(u.uen == u.uenmax){
	    pwcolor = NO_COLOR;
	  } else if(u.uen > (u.uenmax*2/3)) {
	    pwcolor = CLR_GREEN;
	  } else if(u.uen <= (u.uenmax/3)) {
	    pwcolor = CLR_RED;
	  } else {
	    pwcolor = CLR_YELLOW;
	  }
	  if (pwcolor != NO_COLOR)
	    _term_start_color(pwcolor);
	  putstr(WIN_STATUS, 0, newbot2);
	  if (pwcolor != NO_COLOR)
	    _term_end_color();
	}
#endif /* TEXTCOLOR */

#ifdef TEXTCOLOR
	curs(WIN_STATUS, 1, 1);
        if ((changed = (oldac > u.uac)))
	    _term_start_color(CLR_GREEN);
        else if ((changed = (oldac < u.uac)))
	    _term_start_color(CLR_RED);
#endif /*TEXTCOLOR*/
	Sprintf(nb = eos(nb), " AC:%-2d", u.uac);
#ifdef TEXTCOLOR
	putstr(WIN_STATUS, 0, newbot2);
	if (changed) _term_end_color();
#endif /*TEXTCOLOR*/
        if (oldmoves != moves) oldac = u.uac;

	curs(WIN_STATUS, 1, 1);
	if (Upolyd)
		Sprintf(nb = eos(nb), " HD:%d", mons[u.umonnum].mlevel),
		changed = FALSE;
#ifdef EXP_ON_BOTL
	else if(flags.showexp)
	{
#ifdef TEXTCOLOR
            if ((changed = (oldxp < u.uexp)))
	        _term_start_color(CLR_GREEN);
            else if ((changed = (oldxp > u.uexp)))
	        _term_start_color(CLR_RED);
#endif /*TEXTCOLOR*/
	    Sprintf(nb = eos(nb), " Xp:%u/%-1ld", u.ulevel,u.uexp);
	    if (oldmoves != moves) oldxp = u.uexp;
	}
#endif
	else
	{
#ifdef TEXTCOLOR
            if ((changed = (oldxp < u.ulevel)))
	        _term_start_color(CLR_GREEN);
            else if ((changed = (oldxp > u.ulevel)))
	        _term_start_color(CLR_RED);
#endif /*TEXTCOLOR*/
	    Sprintf(nb = eos(nb), " Exp:%u", u.ulevel);
	    if (oldmoves != moves) oldxp = u.ulevel;
	}

	putstr(WIN_STATUS, 0, newbot2);
	if (changed) _term_end_color();

	if(flags.time)
	{
	    curs(WIN_STATUS, 1, 1);
#ifdef TEXTCOLOR
            if ((changed = (oldmoves != moves)))
	        _term_start_color(CLR_BLUE);
#endif /*TEXTCOLOR*/
	    Sprintf(nb = eos(nb), " T:%ld", moves);
	    putstr(WIN_STATUS, 0, newbot2);
#ifdef TEXTCOLOR
	    if (changed) _term_end_color();
#endif /*TEXTCOLOR*/
	}

#ifdef REALTIME_ON_BOTL
  if(iflags.showrealtime) {
    time_t currenttime = get_realtime();
#ifdef TEXTCOLOR
    if ((changed = (oldtime != time)))
	term_start_color(CLR_BLUE);
#endif /*TEXTCOLOR*/
    Sprintf(nb = eos(nb), " %d:%2.2d", currenttime / 3600, 
                                       (currenttime % 3600) / 60);
#ifdef TEXTCOLOR
    if (changed) {
    	oldtime = time;
    	_term_end_color();
    }
#endif
  }
#endif

	if(strcmp(hu_stat[u.uhs], "        ")) {
	        curs(WIN_STATUS, 1, 1);
#ifdef TEXTCOLOR
                /* 3 == WEAK, 4 == FAINTING*/
                if (iflags.use_color)
	            _term_start_color(u.uhs >= 4  ?  CLR_ORANGE        :
				     u.uhs == 3  ?  CLR_RED           :
				                    CLR_YELLOW);
#endif /*TEXTCOLOR*/
		Sprintf(nb = eos(nb), " ");
		Strcat(newbot2, hu_stat[u.uhs]);
	        putstr(WIN_STATUS, 0, newbot2);
#ifdef TEXTCOLOR
                if (iflags.use_color) _term_end_color();
#endif /*TEXTCOLOR*/
	}
	if(Confusion) {
	    curs(WIN_STATUS, 1, 1);
#ifdef TEXTCOLOR
	    _term_start_color(CLR_BRIGHT_BLUE);
#endif /*TEXTCOLOR*/
	    Sprintf(nb = eos(nb), " Conf");

	    putstr(WIN_STATUS, 0, newbot2);
#ifdef TEXTCOLOR
	    _term_end_color();
#endif /*TEXTCOLOR*/
	}

	if(Sick && (u.usick_type & SICK_VOMITABLE)) {
	    curs(WIN_STATUS, 1, 1);
#ifdef TEXTCOLOR
	    _term_start_color(CLR_ORANGE);
#endif /*TEXTCOLOR*/
	    Sprintf(nb = eos(nb), " FoodPois");

	    putstr(WIN_STATUS, 0, newbot2);
#ifdef TEXTCOLOR
	    _term_end_color();
#endif /*TEXTCOLOR*/
	}
	if(Sick && (u.usick_type & (SICK_NONVOMITABLE|SICK_ZOMBIE))) {
	    curs(WIN_STATUS, 1, 1);
#ifdef TEXTCOLOR
	    _term_start_color(CLR_ORANGE);
#endif /*TEXTCOLOR*/
	    Sprintf(nb = eos(nb), " Ill");

	    putstr(WIN_STATUS, 0, newbot2);
#ifdef TEXTCOLOR
	    _term_end_color();
#endif /*TEXTCOLOR*/
	}
	if(Blind) {
	    curs(WIN_STATUS, 1, 1);
#ifdef TEXTCOLOR
	    _term_start_color(CLR_BRIGHT_BLUE);
#endif /*TEXTCOLOR*/
	    Sprintf(nb = eos(nb), " Blind");

	    putstr(WIN_STATUS, 0, newbot2);
#ifdef TEXTCOLOR
	    _term_end_color();
#endif /*TEXTCOLOR*/
	}
	if(Stunned) {
	    curs(WIN_STATUS, 1, 1);
#ifdef TEXTCOLOR
	    _term_start_color(CLR_RED);
#endif /*TEXTCOLOR*/
	    Sprintf(nb = eos(nb), " Stun");

	    putstr(WIN_STATUS, 0, newbot2);
#ifdef TEXTCOLOR
	    _term_end_color();
#endif /*TEXTCOLOR*/
	}
	if(HHallucination &&
	  !Halluc_resistance) {
	    curs(WIN_STATUS, 1, 1);
#ifdef TEXTCOLOR
	    _term_start_color(CLR_BRIGHT_MAGENTA);
#endif /*TEXTCOLOR*/
	    Sprintf(nb = eos(nb), " Hallu");

	    putstr(WIN_STATUS, 0, newbot2);
#ifdef TEXTCOLOR
	    _term_end_color();
#endif /*TEXTCOLOR*/
	}
	if(Slimed) {
	    curs(WIN_STATUS, 1, 1);
#ifdef TEXTCOLOR
	    _term_start_color(CLR_GREEN);
#endif /*TEXTCOLOR*/
	    Sprintf(nb = eos(nb), " Slime");

	    putstr(WIN_STATUS, 0, newbot2);
#ifdef TEXTCOLOR
	    _term_end_color();
#endif /*TEXTCOLOR*/
	}
	if(Stoned) {
	    curs(WIN_STATUS, 1, 1);
#ifdef TEXTCOLOR
	    _term_start_color(CLR_ORANGE);
#endif /*TEXTCOLOR*/
	    Sprintf(nb = eos(nb), " Stone");

	    putstr(WIN_STATUS, 0, newbot2);
#ifdef TEXTCOLOR
	    _term_end_color();
#endif /*TEXTCOLOR*/
	}
	if(cap > UNENCUMBERED)
	{
	        curs(WIN_STATUS, 1, 1);
#ifdef TEXTCOLOR
                if (iflags.use_color)
	            _term_start_color(cap == SLT_ENCUMBER ? CLR_BLUE :
		                     cap == MOD_ENCUMBER ? CLR_YELLOW :
				     cap == HVY_ENCUMBER ? CLR_RED :
				                           CLR_ORANGE);
#endif /*TEXTCOLOR*/
		Sprintf(nb = eos(nb), " %s", enc_stat[cap]);
	        putstr(WIN_STATUS, 0, newbot2);
#ifdef TEXTCOLOR
                if (iflags.use_color) _term_end_color();
#endif /*TEXTCOLOR*/
	}

	oldmoves = moves;
#ifdef DUMP_LOG
}
STATIC_OVL void
bot2()
{
	char newbot2[MAXCO];
	bot2str(newbot2);
#endif
	curs(WIN_STATUS, 1, 1);
	putstr(WIN_STATUS, 0, newbot2);
}

void
bot()
{
	bot1();
	bot2();
	flags.botl = flags.botlx = 0;
}

#endif /* OVL0 */

/*botl.c*/
