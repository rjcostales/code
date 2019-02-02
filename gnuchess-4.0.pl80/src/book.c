/*
 * book.c - C source for GNU CHESS
 *
 * Copyright (c) 1985-1996 Stuart Cracraft, John Stanback,
 *                         Daryl Baker, Conor McCarthy,
 *                         Mike McGann, Chua Kong Sian
 * Copyright (c) 1985-1996 Free Software Foundation
 *
 * This file is part of GNU CHESS.
 *
 * GNU Chess is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2, or (at your option) any later
 * version.
 *
 * GNU Chess is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * GNU Chess; see the file COPYING.  If not, write to the Free Software
 * Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include "gnuchess.h"
#include "ttable.h"		/* uses hashbd, hashkey */
#include "ataks.h"
#if !defined(AMIGADOS) && !defined(Think_C) && !defined(WIN32)
#include <unistd.h>
#endif
#ifdef Think_C
#include <unix.h>
#endif
#ifdef GENIT
FILE *GEN;
int GENline = 0;
int GENmove = 0;
CHAR gs0[12], gs1[12], gs2[12], gs3[12], GENITs[12];
#endif
#ifdef MSDOS
#include <io.h>
#endif
#if !defined MSDOS && !defined(Think_C)
#define O_BINARY 0
#endif
#include <fcntl.h>
unsigned long booksize = BOOKSIZE;
unsigned long int BKTBLSIZE;
unsigned long BOOKMASK;
unsigned long bookcount = 0;
unsigned long bookline = 0;
unsigned bookpocket = BOOKPOCKET;
UTSHORT bookmaxply = BOOKMAXPLY;

#ifdef ECO
#include <sys/types.h>
#include <sys/stat.h>
#endif

CHAR *bookfile = NULL;
CHAR *binbookfile = BINBOOK;

int GotBook = false;
unsigned long bhashbd, bhashkey;


#ifdef GENIT
GENITchk (gs)
     CHAR *gs;
{
    int gcnt, gpnt;
    struct leaf *node = NULL;
    gcnt = 0;
    gpnt = TrPnt[2];
    while (gpnt < TrPnt[3])
      {
	  node = &Tree[gpnt++];
	  algbr (node->f, node->t, (SHORT) node->flags);
	  if (strcmp (gs, mvstr[0]) == 0 || strcmp (gs, mvstr[1]) == 0 ||
	      strcmp (gs, mvstr[2]) == 0 || strcmp (gs, mvstr[3]) == 0
	      || strcmp (gs, mvstr[4]) == 0)
	    {
		gcnt++;
	    }
      }
    return gcnt;
}
#endif

#ifndef QUIETBOOKGEN
void
bkdisplay (s, cnt, moveno)
     CHAR *s;
     int cnt;
     int moveno;
{
    static SHORT pnt;
#ifndef SEMIQUIETBOOKGEN
    struct leaf *node;
    int r, c, l;
#endif

    pnt = TrPnt[2];
    printf ("matches = %d\n", cnt);
    printf ("inout move is :%s:move number %d side %s\n", s, moveno / 2 + 1, !(moveno & 1) ? "white" : "black");
#ifndef SEMIQUIETBOOKGEN
    printf ("legal moves are \n");
    while (pnt < TrPnt[3])
      {
	  node = &Tree[pnt++];
	  algbr (node->f, node->t, (SHORT) node->flags);
	  printf ("%s %s %s %s %s\n", mvstr[0], mvstr[1], mvstr[2], mvstr[3], mvstr[4]);
      }
    printf ("\n current board is\n");
    for (r = 7; r >= 0; r--)
      {
	  for (c = 0; c <= 7; c++)
	    {
		l = locn (r, c);
		if (color[l] == neutral)
		    printf (" -");
		else if (color[l] == white)
		    printf (" %c", qxx[board[l]]);
		else
		    printf (" %c", pxx[board[l]]);
	    }
	  printf ("\n");
      }
    printf ("\n\n");
#endif
}

#endif

int
BVerifyMove (CHAR * s, UTSHORT * mv, int moveno)

     /*
      * Compare the string 's' to the list of legal moves available for the
      * opponent. If a match is found, make the move on the board.
      */

{
    static SHORT pnt, tempb, tempc, tempsf, tempst, cnt;
    static struct leaf xnode;
    struct leaf *node;

    *mv = 0;
    cnt = 0;
    VMoveList (opponent, 2);
    pnt = TrPnt[2];
    while (pnt < TrPnt[3])
      {
	  node = &Tree[pnt++];
	  algbr (node->f, node->t, (SHORT) node->flags);
	  if (strcmp (s, mvstr[0]) == 0 || strcmp (s, mvstr[1]) == 0 ||
	      strcmp (s, mvstr[2]) == 0 || strcmp (s, mvstr[3]) == 0 || strcmp (s, mvstr[4]) == 0)
	    {
		cnt++;
		xnode = *node;
	    }
      }
    if (cnt == 1)
      {
#ifdef GENIT

	  algbr (xnode.f, xnode.t, (SHORT) xnode.flags);
	  strcpy (gs0, mvstr[0]);
	  strcpy (gs1, mvstr[1]);
	  strcpy (gs2, mvstr[2]);
	  strcpy (gs3, mvstr[3]);
	  if (GENITchk (gs1) == 1)
	      strcpy (GENITs, gs1);
	  else if (GENITchk (gs2) == 1)
	      strcpy (GENITs, gs2);
	  else if (GENITchk (gs2) == 1)
	      strcpy (GENITs, gs1);
	  else
	      strcpy (GENITs, gs0);
	  if (strcmp ("e8g8", GENITs) == 0 || strcmp ("e1g1", GENITs) == 0)
	      strcpy (GENITs, "o-o");
	  else if (strcmp ("e8c8", GENITs) == 0 || strcmp ("e1c1", GENITs) == 0)
	      strcpy (GENITs, "o-o-o");
#endif
	  MakeMove (opponent, &xnode, &tempb, &tempc, &tempsf, &tempst);
	  if (SqAtakd (PieceList[opponent][0], computer))
	    {
		UnmakeMove (opponent, &xnode, &tempb, &tempc, &tempsf, &tempst);
		/* Illegal move in check */
#ifndef QUIETBOOKGEN
		printf (CP[77]);
		printf ("\n");
		bkdisplay (s, cnt, moveno);
#endif
		return (false);
	    }
	  else
	    {
		*mv = (xnode.f << 8) | xnode.t;
		algbr (xnode.f, xnode.t, false);
		if (board[xnode.t] == pawn)
		  {
		      if (xnode.t - xnode.f == 16)
			  epsquare = xnode.f + 8;
		      else if (xnode.f - xnode.t == 16)
			  epsquare = xnode.f - 8;
		  }
		else
		    epsquare = -1;

		return (true);
	    }
      }
    /* Illegal move */
#ifndef QUIETBOOKGEN
    printf (CP[75], s);
    bkdisplay (s, cnt, moveno);
#endif
    return (false);
}

void
RESET (void)

     /*
      * Reset the board and other variables to start a new game.
      */

{
    SHORT l;

    flag.illegal = flag.mate = flag.post = flag.quit = flag.reverse = flag.bothsides = flag.onemove = flag.force = false;
    flag.material = flag.coords = flag.hash = flag.easy = flag.rcptr = true;
    flag.stars = flag.shade = flag.back = flag.musttimeout = false;
#ifdef CHESSTOOL
    flag.beep = false;
#else
    flag.beep = true;
#endif
#ifdef CLIENT
    flag.gamein = true;
    flag.post = true;
#else
    flag.gamein = false;
#endif
    GenCnt = epsquare = 0;
    GameCnt = 0;
    Developed[white] = Developed[black] = false;
    castld[white] = castld[black] = false;
    PawnThreat[0] = CptrFlag[0] = false;
    opponent = white;
    computer = black;
    for (l = 0; l < 64; l++)
      {
	  board[l] = Stboard[l];
	  color[l] = Stcolor[l];
	  Mvboard[l] = 0;
      }
    InitializeStats ();
}

int
gnc (FILE * fd)
{
    int c;
    c = getc (fd);
    if (c == '(')
      {
#ifdef GENIT
	  fputc (GEN, c);
#endif
	  do
	    {
		c = getc (fd);
#ifdef GENIT
		fputc (GEN, c);
#endif
		if (c == ')')
		  {
		      c = getc (fd);
		      break;
		  }
		if (c == EOF)
		    break;
	    }
	  while (true);
      }
    else if (c == '{')
      {
#ifdef GENIT
	  fputc (GEN, c);
#endif
	  do
	    {
		c = getc (fd);
#ifdef GENIT
		fputc (GEN, c);
#endif
		if (c == '}')
		  {
		      c = getc (fd);
		      break;
		  }
		if (c == EOF)
		    break;
	    }
	  while (true);
      }
    return c;
}


int
Vparse (FILE * fd, UTSHORT * mv, SHORT side, CHAR * opening, int moveno)
{
    register int c, i;
    CHAR s[1024];
    CHAR *p;

    while (true)
      {

	  while ((c = gnc (fd)) == ' ' || c == '\n');
	  if (c == '\r')
	      continue;
	  i = 0;
	  if (c == '#' || c == '[' || c == '%')
	    {			/* comment */
#ifdef GENIT
		pcurr =
#endif
		    p = opening;
		do
		  {
		      *p++ = c;
		      c = gnc (fd);
		      if (c == '\r')
			  continue;
		      /* goes to end of line */
		      if (c == '\n')
			{
			    /* does the comment continue */
			    if (opening[0] == '[')
			      {
				  if ((c = getc (fd)) == '[')
				    {
#ifdef GENIT
					*p = '\0';
					fprintf (GEN, "%s\n", pcurr);
					pcurr = p;
#endif
					continue;
				    }
				  else
				      ungetc (c, fd);
			      }
			    *p = '\0';
#ifdef GENIT
			    GENline = 0;
			    fprintf (GEN, "%s\n", opening);
			    GENmove = 0;
#endif
			    return 0;
			}
		      if (c == EOF)
			  return -1;
		  }
		while (true);
	    }
	  /* is it a move number or analysis ( in [ ] or in { } ) */
	  /* number cannot start with a 0 because of 0-0 */
	  else if (!isalpha (c) && c != '0')
	    {
		int nlxx = false;
		int nonspace = false;

		while (true)
		  {
		      c = gnc (fd);
		      if (nlxx)
			  if (c == '#' || c == '[' || c == '%')
			    {
				ungetc (c, fd);
				return 0;
			    }
		      if (c == '\r')
			  continue;
		      if (c == '\n')
			{
			    nlxx = true;
			    continue;
			}
		      else
			  nlxx = false;
		      if (c == EOF)
			{
			    return -1;
			}
		      /* stop at first nonspace a ... is space */
		      /* must be nonspace because of 0-0 */
		      if (nonspace)
			{
			    if (c != '.' && c != ' ')
				break;
			}
		      if (c == '.')
			{
			    nonspace = true;
			}
		      /* stop if alpha must be move */
		      else if (isalpha (c))
			  break;
		  }
	    }
	  s[0] = (CHAR) c;

	  while ((c = gnc (fd)) != '\n' && c != ' ' && c != '\t' && c != EOF)
	    {
		if (isupper (c))
		  {
		      if (c != 'O')
			{
			    ungetc (c, fd);
			    c = ' ';
			    break;
			}
		  }
		if (c == '\r')
		    continue;
		if (c == '?')
		    break;
		if (c == '!')
		    continue;
		if (c == '+')
		    continue;
		if (c == '#')
		    continue;
		if (c == '%')
		    continue;
		if (c == '=')
		  {
		      c = gnc (fd);
		      c = tolower (c);
		  }
		if (c != 'x')
		    s[++i] = c;
	    }
	  s[++i] = '\0';

	  if (c == EOF)
	      return (-1);
	  if (s[0] == '!' || s[0] == ';')
	    {
		while (c != '\n' && c != EOF)
		    c = gnc (fd);
		if (c == EOF)
		    return -1;
		else
		    return (0);
	    }
	  if ((strcmp (s, "o-o-o") == 0) || (strcmp (s, "OOO") == 0) || (strcmp (s, "O-O-O") == 0) || (strcmp (s, "0-0-0") == 0))
	    {
		if (side == black)
		    strcpy (s, "e8c8");
		else
		    strcpy (s, "e1c1");
	    }
	  else if ((strcmp ("o-o", s) == 0) || (strcmp (s, "OO") == 0) || (strcmp (s, "O-O") == 0) || (strcmp (s, "0-0") == 0))
	    {
		if (side == black)
		    strcpy (s, "e8g8");
		else
		    strcpy (s, "e1g1");
	    }
	  else if (strcmp (s, "draw") == 0)
	      continue;
	  else if (strcmp (s, "Draw") == 0)
	      continue;
	  else if (strcmp (s, "1-0") == 0)
	      continue;
	  else if (strcmp (s, "0-1") == 0)
	      continue;
	  else if (strcmp (s, "2-1/2") == 0)
	      continue;
	  if (isupper (s[i - 1]))
	      s[i - 1] = tolower (s[i - 1]);

	  bhashkey = hashkey;
	  bhashbd = hashbd;

	  i = BVerifyMove (s, mv, moveno);
	  if (c == '?')
	    {			/* Bad move, not for the program to play */
		*mv |= BADMOVE;	/* Flag it ! */
		c = getc (fd);
	    }
	  else if (c == '+' || c == '\r')
	      c = gnc (fd);
	  if (!i)
	    {
		printf ("%s \n", opening);
		/* flush to start of next */
		while ((c = gnc (fd)) != '[' && c != EOF && c != '#');
		if (c == EOF)
		    return -1;
		else
		  {
		      ungetc (c, fd);
		      return i;
		  }
	    }
#ifdef GENIT
	  if (GENline++ > 15)
	    {
		GENline = 1;
		fprintf (GEN, "\n");
	    }
	  if ((GENmove / 2) * 2 == GENmove)
	      fprintf ("%d. ", (GENmove / 2) + 1);
	  if (c == '?')
	      fprintf (GEN, "%s? ", GENITs);
	  else
	      fprintf (GEN, "%s ", GENITs);
	  if (!(GENline & 1))
	      fprintf (GEN, " ");
	  GENmove++;
#endif
	  return (i);
      }
}


/*===================================== GDX =======================================*/

struct gdxadmin
{
    unsigned long bookcount;
    unsigned long booksize;
    unsigned long maxoffset;
}
ADMIN, B;

struct gdxdata
  {
      unsigned long hashbd;
      utshort hashkey;
      utshort bmove;
      utshort hint;
      utshort count;
  }
DATA;

#ifdef LONG64
#define lts(x) (utshort)(((x>>48)&0xfffe)|side)
#else
#define lts(x) (utshort)(((x>>16)&0xfffe)|side)
#endif
unsigned long currentoffset;
int gfd;

void
GetOpenings (void)

     /*
      * If a text file of opening chess plays (the Opening Book) is available: 
      * Read in the Opening Book file and parse the algebraic notation for a move
      * into an unsigned integer format indicating the from and to square. Create
      * or update a binary hash file with the recomended move/moves for each 
      * position.  
      * The binary hash file is opened with readonly access durring the game.
      */
{
    register SHORT i;
    CHAR opening[1024];
    CHAR msg[1024];
    int mustwrite = false;
    UTSHORT xside, doit, side;
    SHORT c;
    UTSHORT mv;
    UTSHORT ix;
    unsigned long x;
    unsigned long games = 0;

    FILE *fd;

    if ((fd = fopen (bookfile, "r")) == NULL)
	fd = fopen ("gnuchess.book", "r");
#ifdef GENIT
    if ((GEN = fopen ("GEN", "w")) == NULL)
      {
	  printf ("GEN FAIL\n");
	  exit (1);
      }
#endif
    if (fd != NULL)
      {
	  /* yes add to book */
	  /* open book as writer */
	  gfd = open (binbookfile, O_RDONLY | O_BINARY);
	  if (gfd >= 0)
	    {
		if (sizeof (struct gdxadmin) == read (gfd, &ADMIN, sizeof (struct gdxadmin)))
		  {
		      B.bookcount = ADMIN.bookcount;
		      B.booksize = ADMIN.booksize;
		      B.maxoffset = ADMIN.maxoffset;
		      if (B.booksize && !(B.maxoffset == ((unsigned long) (B.booksize - 1) * sizeof (struct gdxdata) + sizeof (struct gdxadmin))))
			{
			    printf ("bad format %s\n", binbookfile);
			    exit (1);
			}
		  }
		else
		  {
		      printf ("bad format %s\n", binbookfile);
		      exit (1);
		  }
		close (gfd);
		gfd = open (binbookfile, O_RDWR | O_BINARY);

	    }
	  else
	    {
#ifdef Think_C
		gfd = open (binbookfile, O_RDWR | O_CREAT | O_BINARY);
#else
		gfd = open (binbookfile, O_RDWR | O_CREAT | O_BINARY, 0644);
#endif
		ADMIN.bookcount = B.bookcount = 0;
		ADMIN.booksize = B.booksize = booksize;
		B.maxoffset = ADMIN.maxoffset = (unsigned long) (booksize - 1) * sizeof (struct gdxdata) + sizeof (struct gdxadmin);
		DATA.hashbd = 0;
		DATA.hashkey = 0;
		DATA.bmove = 0;
		DATA.hint = 0;
		DATA.count = 0;
		write (gfd, &ADMIN, sizeof (struct gdxadmin));
		printf ("creating bookfile %s  %ld %ld\n", binbookfile, B.maxoffset, B.booksize);
		for (x = 0; x < B.booksize; x++)
		  {
		      write (gfd, &DATA, sizeof (struct gdxdata));
		  }


	    }
	  if (gfd >= 0)
	    {


		/* setvbuf(fd,buffr,_IOFBF,2048); */
		side = white;
		xside = black;
		InitializeStats ();
		i = 0;

		while ((c = Vparse (fd, &mv, side, opening, i)) >= 0)
		  {
		      if (c == 1)
			{

			    /*
			     * if not first move of an opening and first
			     * time we have seen it save next move as
			     * hint
			     */
			    i++;
			    if (i < bookmaxply + 2)
			      {
				  if (i > 1)
				    {
					DATA.hint = mv & 0x3f3f;
				    }
				  if (i < bookmaxply + 1)
				    {
					doit = true;

					/*
					 * see if this position and
					 * move already exist from
					 * some other opening
					 */

					/*
					 * is this ethical, to offer
					 * the bad move as a
					 * hint?????
					 */
					ix = 0;
					if (mustwrite)
					  {
					      lseek (gfd, currentoffset, SEEK_SET);
					      write (gfd, &DATA, sizeof (struct gdxdata));
					      mustwrite = false;
					  }
					doit = true;
					currentoffset = (unsigned long) (bhashkey % B.booksize) * sizeof (struct gdxdata) + sizeof (struct gdxadmin);
					while (true)
					  {

					      lseek (gfd, currentoffset, SEEK_SET);
					      if ((read (gfd, &DATA, sizeof (struct gdxdata)) == 0))
						    break;

					      if (DATA.bmove == 0)
						  break;
					      if (DATA.hashkey == (utshort) (lts (bhashkey)) && DATA.hashbd == bhashbd)
						{

						    if ((DATA.bmove & (~(LASTMOVE | BADMOVE))) == (mv & ~BADMOVE))
						      {
							  DATA.count++;
							  if (mv & BADMOVE)
							      DATA.bmove |= BADMOVE;
							  /*
							   * yes so just bump count - count is
							   * used to choose opening move in
							   * proportion to its presence in the book
							   */
							  doit = false;
							  mustwrite = true;
							  break;
						      }
						    else if (DATA.bmove & LASTMOVE)
						      {
							  DATA.bmove &= (~LASTMOVE);
							  lseek (gfd, currentoffset, SEEK_SET);
							  write (gfd, &DATA, sizeof (struct gdxdata));
						      }
						}
					      currentoffset += sizeof (struct gdxdata);
					      if (currentoffset > B.maxoffset)
						  currentoffset = sizeof (struct gdxadmin);
					  }

					/*
					 * doesn`t exist so add it to
					 * the book
					 */
					if (!mustwrite)
					  {
					      B.bookcount++;
#if !defined CHESSTOOL && !defined XBOARD
					      if (B.bookcount % 1000 == 0)
						  printf ("%ld rec %ld openings processed\n", B.bookcount, games);
#endif
					      /* initialize a record */
					      DATA.hashbd = bhashbd;
					      DATA.hashkey = (utshort) (lts (bhashkey));
					      DATA.bmove = mv | LASTMOVE;
					      DATA.count = 1;
					      DATA.hint = 0;

					      mustwrite = true;
					  }
				    }
			      }
			    computer = opponent;
			    opponent = computer ^ 1;

			    xside = side;
			    side = side ^ 1;
			}
		      else if (i > 0)
			{
			    /* reset for next opening */
			    games++;
			    if (mustwrite)
			      {
				  lseek (gfd, currentoffset, SEEK_SET);
				  write (gfd, &DATA, sizeof (struct gdxdata));
				  mustwrite = false;
			      }
			    RESET ();
			    i = 0;
			    side = white;
			    xside = black;

			}
		  }
		if (mustwrite)
		  {
		      lseek (gfd, currentoffset, SEEK_SET);
		      write (gfd, &DATA, sizeof (struct gdxdata));
		      mustwrite = false;
		  }
		fclose (fd);
		/* write admin rec with counts */
		ADMIN.bookcount = B.bookcount;
		currentoffset = 0;
		lseek (gfd, currentoffset, SEEK_SET);
		write (gfd, &ADMIN, sizeof (struct gdxadmin));

		close (gfd);
	    }
      }
    if (binbookfile != NULL)
      {
	  /* open book as reader */
	  gfd = open (binbookfile, O_RDONLY | O_BINARY);
	  if (gfd >= 0)
	    {
		read (gfd, &ADMIN, sizeof (struct gdxadmin));
		B.bookcount = ADMIN.bookcount;
		B.booksize = ADMIN.booksize;
		B.maxoffset = ADMIN.maxoffset;
		if (B.booksize && !(B.maxoffset == ((unsigned long) (B.booksize - 1) * sizeof (struct gdxdata) + sizeof (struct gdxadmin))))
		  {
		      printf ("bad format %s\n", binbookfile);
		      exit (1);
		  }

	    }
	  else
	    {
		B.bookcount = 0;
		B.booksize = booksize;

	    }

#if !defined CHESSTOOL && !defined XBOARD
	  sprintf (msg, CP[213], B.bookcount, B.booksize);
	  ShowMessage (msg);
#endif
      }
    /* set every thing back to start game */
    Book = BOOKFAIL;
    RESET ();
    /* now get ready to play */
    if (!B.bookcount)
      {
#if !defined CHESSTOOL && !defined XBOARD
	  ShowMessage (CP[212]);
#endif
	  Book = 0;
      }
}


int
OpeningBook (SHORT * hint, SHORT side)

     /*
      * Go thru each of the opening lines of play and check for a match with the
      * current game listing. If a match occurs, generate a random number. If this
      * number is the largest generated so far then the next move in this line
      * becomes the current "candidate". After all lines are checked, the
      * candidate move is put at the top of the Tree[] array and will be played by
      * the program. Note that the program does not handle book transpositions.
      */

{
    UTSHORT r, m;
    int possibles = TrPnt[2] - TrPnt[1];
    register UTSHORT i, x;
    register UTSHORT rec = 0;
    register UTSHORT summ = 0;
    register UTSHORT h = 0, b = 0;
    struct gdxdata OBB[128];

    gsrand ((unsigned int) time ((time_t *) 0));
    m = 0;

    /*
     * find all the moves for this position  - count them and get their
     * total count
     */
    if (B.bookcount == 0)
      {
	Book--;
	return false;
      }
    currentoffset = (unsigned long) (hashkey % B.booksize) * sizeof (struct gdxdata) + sizeof (struct gdxadmin);
    x = 0;
    lseek (gfd, currentoffset, SEEK_SET);
    while (true)
      {
        if (read (gfd, &OBB[x], sizeof (struct gdxdata)) == 0)
	    break;
        if (OBB[x].bmove == 0)
	    break;

        if (OBB[x].hashkey == (utshort) (lts (hashkey)) && OBB[x].hashbd == hashbd)
	  {
	    x++;
	    if (OBB[x - 1].bmove & LASTMOVE)
		break;
	  }
	currentoffset += sizeof (struct gdxdata);
	if (currentoffset > B.maxoffset)
	  {
	    lseek (gfd, sizeof (struct gdxadmin), SEEK_SET);
	    currentoffset = sizeof (struct gdxadmin);
	  }

      }
    if (x == 0)
      {
        Book--;
        return false;
      }
#ifdef DEBUG33
      {
        int loop = true;
        while (loop)
	  {
	    loop = false;
	    for (i = 1; i < x; i++)
	      {
		struct gdxdata tmp;
		if (OBB[i].count > OBB[i - 1].count)
		  {
		    loop = true;
		    tmp = OBB[i - 1];
		    OBB[i - 1] = OBB[i];
		    OBB[i] = tmp;
		  }
	      }
	  }
      }
    for (i = 0; i < x; i++)
      {
        algbr ((OBB[i].bmove >> 8) & 0x3f, (OBB[i].bmove) & 0x3f, 0);
        printf (" %s ", mvstr[0], OBB[i].count);
        algbr ((OBB[i].hint >> 8) & 0x3f, (OBB[i].hint) & 0x3f, 0);
        printf ("%s %c %d\n", mvstr[0], (OBB[i].bmove & BADMOVE) ? '*' : ' ', OBB[i].count);
      }
#endif
    for (i = 0; i < x; i++)
      {
        if ((m = OBB[i].bmove) & BADMOVE)
	  {
	    m &= 0x3f3f;
	    /* is the move is in the MoveList */
	    for (b = TrPnt[1]; b < (unsigned) TrPnt[2]; b++)
	      {
	  	if (((Tree[b].f << 8) | Tree[b].t) == m)
		  {
		    if (--possibles)
		        Tree[b].score = DONTUSE;
		    pick (TrPnt[1], TrPnt[2] - 1);
		    break;
		  }
	      }
	  }
	else
	  summ += OBB[i].count;
      }
    if (summ == 0)
      {
        Book--;
        return false;
      }

    r = (urand () % summ);
#ifdef DEBUG33
    printf ("rand is %d, sum is %d\n", r, summ);
#endif
    for (i = 0; i < x; i++)
    if (!(OBB[i].bmove & BADMOVE))
      {
        if (r < OBB[i].count)
	  {
	    rec = i;
	    break;
	  }
	else
	    r -= OBB[i].count;
#ifdef DEBUG33
	printf ("rand is %d, sum is %d\n", r, summ);
#endif

      }

    h = ((OBB[rec].hint) & 0x3f3f);
    m = ((OBB[rec].bmove) & 0x3f3f);
    /* make sure the move is in the MoveList */
    for (b = TrPnt[1]; b < (unsigned) TrPnt[2]; b++)
      {
        if (((Tree[b].f << 8) | Tree[b].t) == m)
	  {
	    Tree[b].flags |= book;
	    Tree[b].score = 0;
	    break;
	  }
      }
    /* Make sure its the best */

    pick (TrPnt[1], TrPnt[2] - 1);
    if (Tree[TrPnt[1]].score)
      {
        /* no! */
        Book--;
        return false;
      }
    /* ok pick up the hint and go */
    *hint = h;
    return true;
}

void
LOpeningBook (SHORT side)

     /*
      * Go thru each of the opening lines of play and check for a match with the
      * current game listing. If a match occurs, generate a random number. If this
      * number is the largest generated so far then the next move in this line
      * becomes the current "candidate". After all lines are checked, the
      * candidate move is put at the top of the Tree[] array and will be played by
      * the program. Note that the program does not handle book transpositions.
      */

{
    char Lmove[12], Lhint[12];
    /*
     * find all the moves for this position  - count them and get their
     * total count
     */
    struct gdxdata OBB[128];
    SHORT x;
    if (B.bookcount == 0)
      {
	  return;
      }
    Ldisplay1 ();
    currentoffset = (unsigned long) (hashkey % B.booksize) * sizeof (struct gdxdata) + sizeof (struct gdxadmin);
    x = 0;
    lseek (gfd, currentoffset, SEEK_SET);
    while (true)
      {
	  if (read (gfd, &OBB[x], sizeof (struct gdxdata)) == 0)
	        return;
	  if (OBB[x].bmove == 0)
	      return;


	  if (OBB[x].hashkey == (utshort) (lts (hashkey)) && OBB[x].hashbd == hashbd)
	    {
		algbr ((OBB[x].bmove >> 8) & 0x3f, (OBB[x].bmove) & 0x3f, 0);
		strcpy (Lmove, mvstr[1]);
		if (OBB[x].bmove & BADMOVE)
		    strcat (Lmove, "?");
		algbr ((OBB[x].hint >> 8) & 0x3f, (OBB[x].hint) & 0x3f, 0);
		strcpy (Lhint, mvstr[1]);
		Ldisplay (Lmove, Lhint, OBB[x].count);
		if (OBB[x++].bmove & LASTMOVE)
		    break;
	    }
	  currentoffset += sizeof (struct gdxdata);
	  if (currentoffset > B.maxoffset)
	    {
		lseek (gfd, sizeof (struct gdxadmin), SEEK_SET);
		currentoffset = sizeof (struct gdxadmin);
	    }

      }
    Ldisplay2 ();
    return;
}

#ifdef IGNUAN
int
GOpeningBook (SHORT * hint, SHORT side, CHAR * mv)

     /*
      * Go thru each of the opening lines of play and check for a match with the
      * current game listing. If a match occurs, generate a random number. If this
      * number is the largest generated so far then the next move in this line
      * becomes the current "candidate". After all lines are checked, the
      * candidate move is put at the top of the Tree[] array and will be played by
      * the program. Note that the program does not handle book transpositions.
      */

{
    char Lmove[12], Lhint[12];
    /*
     * find all the moves for this position  - count them and get their
     * total count
     */
    struct gdxdata OBB[128];
    SHORT x;
    if (B.bookcount == 0)
      {
	  return false;
      }
    currentoffset = (unsigned long) (hashkey % B.booksize) * sizeof (struct gdxdata) + sizeof (struct gdxadmin);
    x = 0;
    lseek (gfd, currentoffset, SEEK_SET);
    while (true)
      {
	  if (read (gfd, &OBB[x], sizeof (struct gdxdata)) == 0)
	        return false;
	  if (OBB[x].bmove == 0)
	      return false;


	  if (OBB[x].hashkey == (utshort) (lts (hashkey)) && OBB[x].hashbd == hashbd)
	    {
		algbr ((OBB[x].bmove >> 8) & 0x3f, (OBB[x].bmove) & 0x3f, 0);
		if ((strcmp (mvstr[0], mv) == 0) ||
		    strcmp (mvstr[1], mv) == 0 ||
		    strcmp (mvstr[2], mv) == 0 ||
		    strcmp (mvstr[3], mv) == 0 ||
		    strcmp (mvstr[4], mv) == 0)
		    return true;
		if (OBB[x++].bmove & LASTMOVE)
		    return false;
	    }
	  currentoffset += sizeof (struct gdxdata);
	  if (currentoffset > B.maxoffset)
	    {
		lseek (gfd, sizeof (struct gdxadmin), SEEK_SET);
		currentoffset = sizeof (struct gdxadmin);
	    }

      }
    return false;
}
#endif

#ifdef ECO
#ifdef LONG64
#define rts(x) (unsigned long)(((x)&(~1))|(side))
#else
#define rts(x) (unsigned long)(((x)&(~1))|(side))
#endif
extern SHORT ecomove;
int efd = 0;
FILE *Efd;
unsigned int efdsize = 0;
void
EOpeningBook (SHORT side)
{
    char E[256];
    struct gdxecodata
      {
	  unsigned long hashbd;
	  unsigned long hashkey;
	  unsigned int ecoptr;
	  utshort cntr;
      };
    SHORT ECOmove = ecomove;

    /*
     * find all the moves for this position  - count them and get their
     * total count
     */
    struct gdxecodata OBB;
    SHORT x, k;
    int h = 0;
    int l = 0;
    unsigned int ecocur;
    k = 0;
    if (efd == 0)
      {
	  struct stat buf;
	  efd = open (BINECO, O_RDONLY | O_BINARY);
	  if (efd < 0)
	    {
		perror ("BINECO \n");
		return;
	    }
	  stat (BINECO, &buf);
	  efdsize = buf.st_size / sizeof (struct gdxecodata);
	  Efd = fopen (PGNECO, "r");
	  if (Efd == (FILE *) NULL)
	    {
		perror ("PGNECO \n");
		return;
	    }
      }
    Ldisplay3 ();
    while (ECOmove > 0)
      {
	  h = efdsize;
	  l = 0;
	  ecocur = (h + l) / 2;
	  x = 0;

	  while (true)
	    {
		SHORT ecofirst;
		currentoffset = ecocur * sizeof (struct gdxecodata);
		if (lseek (efd, currentoffset, SEEK_SET) < 0)
		  {
		      perror ("seek error\n");
		      exit (0);
		  }
		if (read (efd, &OBB, sizeof (struct gdxecodata)) == 0) break;
		if (OBB.hashbd == GameList[ECOmove].hashbd && OBB.hashkey == (unsigned long) (rts (GameList[ECOmove].hashkey))
		    && OBB.cntr == 0)
		  {
		      /* got it */
		      printf ("After %d plys:\n", ECOmove);
		      ecofirst = true;
		      while (true)
			{
			    if (!ecofirst) { if (read (efd, &OBB, sizeof (struct gdxecodata)) == 0) break; }
			    else ecofirst = false;
			    if (OBB.hashbd != GameList[ECOmove].hashbd) break;
			    if (OBB.hashkey != (unsigned long) (rts (GameList[ECOmove].hashkey))) continue;
			    if (fseek (Efd, (long) OBB.ecoptr, SEEK_SET) < 0)
			      { perror ("Eseek PGNECO"); exit (1); }
			    if (fgets (E, sizeof (E), Efd) <= (char *) NULL)
			      { perror ("fget "); exit (1); }
			    Ldisplay4 (E);
			    k++;
			    if (fgets (E, sizeof (E), Efd) <= (char *) NULL)
			      { perror ("fget "); exit (1); }
			    k += 2;
			    while (E[0] != '[')
			      {
				  Ldisplay4 (E);
				  k++;
				  if (fgets (E, sizeof (E), Efd) <= (char *) NULL)
				    { perror ("fget "); exit (1); }
			      }
			    Ldisplay4 ("\n");
			    k++;

			    if (k++ > 32){
				if (getchar () == 'q')
				  { k = 999; break; }
				else
				    k = 1;}
			}
		  }
		else if (OBB.hashbd > GameList[ECOmove].hashbd
			 || (OBB.hashbd == GameList[ECOmove].hashbd
			     && OBB.hashkey > (unsigned long) (rts (GameList[ECOmove].hashkey))
			     && OBB.cntr > 0)
			 || (OBB.hashbd == GameList[ECOmove].hashbd
			     && OBB.hashkey == (unsigned long) (rts (GameList[ECOmove].hashkey))
			     && OBB.cntr > 0))
		  {		/* high */
		      h = ecocur;
		  }
		else
		  {		/* low */
		      l = ecocur;
		  }
		if ((h == l) || (h - l) == 1)
		    break;
		ecocur = (h + l) / 2;

		if (k > 0)
		    break;
	    }
	  if (k > 0)
	      break;
	  ECOmove--;
      }
    Ldisplay2 ();
}
#endif