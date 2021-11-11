//#include <stdint.h>
/*******************************************************
 * 5 Keys - Pulldown 1kOhm - connected to analogPins
 * 
 * ST 7725 Display
 * Breadboard
 * 
 */
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_GFX.h>    // Core graphics library

#define TFT_CS        9
#define TFT_RST       -1 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC        10

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

//Input Keys
#define MRPIN A0
#define MLPIN A1
#define RLPIN A3
#define RRPIN A4
#define FPIN  A2

//Tetris
#define FIELDH    20
#define FIELDW    10
#define WIDTH     128
#define HEIGHT    160
#define BLOCKSIZE 7
#define MLEFT     1
#define MRIGHT    2
#define RLEFT     4
#define RRIGHT    8
#define FALL      16

#define TRUE      1
#define FALSE     0

#define MOVEABLE  1
#define RESIDENT  65

static int tm [7][4][6]= {
  // T
  { {  1,10,11,12, 0,7 } , {  1,11,12,21,-1,7 } , { 10,11,12,21, 0,7 } , {  1,11,10,21, 0,8 } } ,
  // L gespiegelt
  { {  1, 2,11,21,-1,7 } , { 10,11,12,22, 0,7 } , {  1,11,20,21, 0,8 } , {  0,10,11,12, 0,7 } } ,
  // L
  { {  1,11,21,22,-1,7 } , { 10,11,12,20, 0,7 } , {  0, 1,11,21, 0,8 } , {  2,10,11,12, 0,7 } } ,
  // I
  { { 10,11,12,13, 0,6 } , {  2,12,22,32,-2,7 } , { 20,21,22,23, 0,6 } , {  1,11,21,31,-1,8 } } ,
  // Z
  { {  0, 1,11,12, 0,7 } , {  2,11,12,21,-1,7 } , { 10,11,21,22, 0,7 } , {  1,10,11,20, 0,8 } } ,
  // S
  { {  1, 2,10,11, 0,7 } , {  1,11,12,22,-1,7 } , { 11,12,20,21, 0,7 } , {  0,10,11,21, 0,8 } } ,
  // o
  { {  0, 1,10,11, 0,8 } , {  0, 1,10,11, 0,8 } , {  0, 1,10,11, 0,8 } , {  0, 1,10,11, 0,8 } }
} ;

unsigned char spielfeld[ FIELDW*FIELDH ];

void setup(void) {
 
  tft.initR(INITR_GREENTAB); // Put suitable init code for display
  tft.fillScreen(0x0000);

  randomSeed(analogRead(0));

  pinMode(MRPIN,INPUT);
  pinMode(MLPIN,INPUT);
  pinMode(RRPIN,INPUT);
  pinMode(RLPIN,INPUT);
  pinMode(FPIN,INPUT);

  //debug
  Serial.begin(9600);
}

void DrawPlayfield( void ) {

  int i,x,y,r,g,b ;
  uint16_t col;
  unsigned char s;
  
  for (i=0 ; i<(FIELDW*FIELDH) ; i++ ) {
    
    s=spielfeld[i] ;
    if (s>64) s=s-64 ;
    
    switch (s) {
      case 1  : r=255 ; g=0   ; b=0   ; break ;
      case 2  : r=0   ; g=255 ; b=0   ; break ;
      case 3  : r=0   ; g=0   ; b=255 ; break ;
      case 4  : r=255 ; g=255 ; b=0   ; break ;
      case 5  : r=0   ; g=255 ; b=255 ; break ;
      case 6  : r=255 ; g=0   ; b=255 ; break ;
      case 7  : r=128 ; g=128 ; b=128 ; break ;
      default: r=30 ; g=30  ; b=30  ; break ;
    }

    col=tft.color565(r, g, b);

    tft.fillRect( 
      (((i) % FIELDW) * BLOCKSIZE ) + 8 , 
      (((i) / FIELDW) * BLOCKSIZE ) + 16 , 
      BLOCKSIZE-1, 
      BLOCKSIZE-1, 
      col 
    );

  }
}

// Hardwareabhängig

int pressed ( void ) {
  int k=0 ; 
  int z;

  if ( digitalRead(MLPIN)==HIGH ) k=MLEFT ;
  if ( digitalRead(MRPIN)==HIGH ) k=MRIGHT ;
  if ( digitalRead(RLPIN)==HIGH ) k=RLEFT ;
  if ( digitalRead(RLPIN)==HIGH ) k=RRIGHT ;
  if ( digitalRead(FPIN )==HIGH ) k=FALL ; 
  return (k) ;
}
  
void loop(void) {
  
  unsigned long ticks=0 ;
  int quit=FALSE;
  int fixed=FALSE;
  int key;
  int mino,minonext,tetriscount;

  while(!quit){

    int gameover=0 ;
    int xmino, ymino, rotation ;
    int check;

    // Titel
    // To do

    minonext = random(8) ;

    clearplayfield(RESIDENT);

    while (!gameover){

      // Spiel
      // zeichne Spielfeld Dekor

      mino = minonext ;
      minonext = rand() % 7;

      ymino=0 ; xmino=3 ; rotation=0 ;
      fixed=0;

      if ( !checkmino (xmino,ymino,mino,rotation) ) gameover=1;

      while (!fixed) {  
        
        if ( (millis()-ticks)>1000 ) { 
          key=FALL;
          ticks=millis();
          //Serial.println(ticks);
        } 
        else {
           key=pressed();
        }

        switch (key) {
          case MLEFT:
              xmino-- ;
              //? Rand
              if ( xmino < tm[mino][rotation][4] ) xmino=tm[mino][rotation][4];

              //? Spielfeld
              if (checkmino(xmino,ymino,mino,rotation)) {
              putmino(xmino,ymino,mino,rotation,MOVEABLE);
            }
              else {
              xmino++;
              putmino(xmino,ymino,mino,rotation,MOVEABLE);
            }
          break ;

          case MRIGHT:
            xmino++;
            // ? Rand
            if ( xmino > tm[mino][rotation][5] ) xmino=tm[mino][rotation][5];

            //? Spielfeld
            if (checkmino(xmino,ymino,mino,rotation)) {
              putmino(xmino,ymino,mino,rotation,MOVEABLE);
            }
            else {
              xmino--;
              putmino(xmino,ymino,mino,rotation,MOVEABLE);
            }
          break ;

          case RLEFT:
            rotation-- ; rotation&=3 ;
            
            if ( xmino > tm[mino][rotation][5] ) xmino=tm[mino][rotation][5];
            if ( xmino < tm[mino][rotation][4] ) xmino=tm[mino][rotation][4];
            
            if (checkmino(xmino,ymino,mino,rotation)) {
              putmino(xmino,ymino,mino,rotation,MOVEABLE);
            }
                else {
              rotation++; rotation&3 ;
              putmino(xmino,ymino,mino,rotation,MOVEABLE);
            }
          break;

          case RRIGHT:
            rotation++ ; rotation&=3 ;
            
            if ( xmino > tm[mino][rotation][5] ) xmino=tm[mino][rotation][5];
            if ( xmino < tm[mino][rotation][4] ) xmino=tm[mino][rotation][4];
            
            if (checkmino(xmino,ymino,mino,rotation)) {
              putmino(xmino,ymino,mino,rotation,MOVEABLE);
            }
                else {
              rotation--; rotation&3 ;
              putmino(xmino,ymino,mino,rotation,MOVEABLE);
            }
          break;

          case FALL:
            ymino++;
            if ( checkmino(xmino,ymino,mino,rotation)==TRUE ) {
              putmino (xmino,ymino,mino,rotation,MOVEABLE);
            }
            else {
              fixed=TRUE;
              putmino (xmino,ymino-1,mino,rotation,RESIDENT);
              ymino=0 ; xmino=3 ;
            }
          break;

          default:
            putmino (xmino,ymino,mino,rotation,MOVEABLE);
          break;
        }

        DrawPlayfield();
        clearplayfield(MOVEABLE);
      }

      tetriscount=0 ;
      for (check=FIELDH-1 ; check>0 ; check--) {
        while ( checkline(check) ) {
          clearline(check);
          tetriscount++ ;
        }
      }
    }
  }
}

int checkmino ( int x,int y, int m, int r) {

  int i;
  int frei=TRUE;
  int unten=FALSE;
  int adr;
  
  //prüfe ob neue position frei ist
  for ( i=0 ; i<4; i++ ) {
    adr=x+y*FIELDW+tm[m][r][i];
    if (adr<(FIELDW*FIELDH)) {
      frei&=( spielfeld[ adr ] == 0 ) ;
    }
    else {
      unten=TRUE ;
    }
  }
  
  if (unten)
    return(FALSE);
  else
    return(frei);
}

int checkline(int y){
  int i,x,full;
  
  full=TRUE;
  
  i=y*FIELDW ;
  
  for (x=0 ; x<FIELDW ; x++) {
    full&=( spielfeld[i+x]>64);
  }
  return (full) ;
}

void clearline(int y){
  
  int yy,x ;
  
  for (yy=y ; yy>0 ; yy--) {
    for (x=0 ; x<FIELDW ; x++) {
      spielfeld[yy*FIELDW+x]=spielfeld[(yy-1)*FIELDW+x];
    }
    DrawPlayfield();
  }
}

void clearplayfield( int byte ) {
  int i;
  
  if (byte==RESIDENT) {
    for ( i=0 ; i<(FIELDW*FIELDH) ; i++ ) spielfeld[i]=0 ;
  }
  
  if (byte==MOVEABLE) {
    for ( i=0 ; i<(FIELDW*FIELDH) ; i++ ) {
      if ( spielfeld[i]<(unsigned char)65 ) {
        spielfeld[i]=(unsigned char)0;
      }
    }
  }
}

void putmino( int x, int y, int m, int r, int res ){
  
  int i;
  
  clearplayfield(MOVEABLE);
  
  for ( i=0 ; i<4; i++ ) {
      spielfeld[ x+y*FIELDW+tm[m][r][i] ] = (unsigned char)(m+res) ;
  }
}
