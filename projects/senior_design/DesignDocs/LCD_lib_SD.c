#include "gpio.h"
#include "compiler.h"
#include "board.h"
#include "intc.h"
#include "pm.h"


//---------------------------------------------------------------
// CONSTANTS
//---------------------------------------------------------------

#define DISPOFF      0xAE
#define DISPON       0xAF
#define DISPSTART    0x40
#define PAGEADR      0xB0
#define COLADRL      0x00
#define COLADRH      0x10
#define ADCNORMAL    0xA0
#define ADCREVERSE   0xA1
#define COMNORMAL    0xC0
#define COMREVERSE   0xC8
#define DISPNORMAL   0xA6
#define DISPREVERSE  0xA7
#define LCDBIAS9     0xA2
#define LCDBIAS7     0xA3
#define RESET        0xE2
#define SETPOWERCTRL 0x2F
#define REGRESISTOR  0x27
#define SETCONTRAST  0x81
#define STATINDMODE  0xAC
#define BOOSTERRATIO 0xF8


//---------------------------------------------------------------
// PIN DEFINITION
//---------------------------------------------------------------

#define DOGCSPIN	AVR32_PIN_PA10     //stk600 pb2

#define DOGRESPIN	AVR32_PIN_PA19     //stk600 pc3

#define DOGA0PIN	AVR32_PIN_PA14     //stk600 pb6

#define DOGSCLPIN	AVR32_PIN_PA13     //stk600 pb5

#define DOGSIPIN	AVR32_PIN_PA12     //stk600 pb4


//---------------------------------------------------------------
// SPI SETUP DEFINITION
//---------------------------------------------------------------

#define SetBit(adr, bnr)	( (adr) |=  (1 << (bnr)) )
#define ClrBit(adr, bnr)	( (adr) &= ~(1 << (bnr)) )

#define	DOGENABLE  gpio_clr_gpio_pin(DOGCSPIN)
#define	DOGDISABLE gpio_set_gpio_pin(DOGCSPIN)

#define	DOGCOMMAND gpio_clr_gpio_pin(DOGA0PIN)
#define	DOGDATA    gpio_set_gpio_pin(DOGA0PIN)

#define	DOG_CLK_HIGH 	gpio_set_gpio_pin(DOGSCLPIN)
#define	DOG_CLK_LOW      gpio_clr_gpio_pin(DOGSCLPIN)

#define	DOG_SI_HIGH 	gpio_set_gpio_pin(DOGSIPIN)
#define	DOG_SI_LOW      gpio_clr_gpio_pin(DOGSIPIN)

#define DOG_RES			gpio_clr_gpio_pin(DOGRESPIN)
#define DOG_RES_DISB	gpio_set_gpio_pin(DOGRESPIN)



//---------------------------------------------------------------

void dogSPIout(unsigned char out)
{
  DOGENABLE;				//Chip Select on, select LCD
  unsigned char msk;

  msk = 0x80;
  while(msk>0){
	DOG_CLK_LOW;
     if(out & msk){
        DOG_SI_HIGH;
     }else{
        DOG_SI_LOW;
	}
     DOG_CLK_HIGH;
     msk >>= 1;
   }
  DOGDISABLE;				//Chip Select off, deselect LCD
}

//---------------------------------------------------------------

void initDOGM128(void)
{
  int contrast = 0x16;

  DOG_RES_DISB;
  DOGENABLE;
  DOGCOMMAND;

  dogSPIout(DISPSTART);
  dogSPIout(ADCREVERSE);
  dogSPIout(COMNORMAL);
  dogSPIout(DISPNORMAL);
  dogSPIout(LCDBIAS9);
  dogSPIout(SETPOWERCTRL);
  dogSPIout(BOOSTERRATIO);
  dogSPIout(0x00);
  dogSPIout(REGRESISTOR);
  dogSPIout(SETCONTRAST);
  dogSPIout(contrast);
  dogSPIout(STATINDMODE);
  dogSPIout(0x00);
  dogSPIout(DISPON);

  DOGDISABLE;
  DOGDATA;
}

//---------------------------------------------------------------

void ResetDOG(void)
{
  DOGENABLE;
  DOGCOMMAND;
  DOG_SI_HIGH;
  DOG_CLK_HIGH;

  DOG_RES_DISB;
  DOG_RES;
}

//---------------------------------------------------------------

void DogKontrast(contrast)
{
  DOGENABLE;
  DOGCOMMAND;

  dogSPIout(SETCONTRAST);
  dogSPIout(contrast);

  DOGDISABLE;
}

//---------------------------------------------------------------

void int2byte(int zeroTo255){ //Byte convert to bits


unsigned char value = 0x00;


    if(zeroTo255>=128){
        SetBit(value, 7);
        zeroTo255 = zeroTo255 - 128;
    }else if(zeroTo255>=64){
        SetBit(value, 6);
        zeroTo255 = zeroTo255 - 64;
    }else if(zeroTo255>=32){
        SetBit(value, 5);
        zeroTo255 = zeroTo255- 32;
    }else if(zeroTo255>=16){
        SetBit(value, 4);
        zeroTo255 = zeroTo255 - 16;
    }else if(zeroTo255>=8){
        SetBit(value, 3);
        zeroTo255 = zeroTo255 - 8;
    }else if(zeroTo255>=4){
        SetBit(value, 2);
        zeroTo255 = zeroTo255 - 4;
    }else if(zeroTo255>=2){
        SetBit(value, 1);
        zeroTo255 = zeroTo255 - 2;
    }else if(zeroTo255>0){
        SetBit(value, 0);
        zeroTo255 = zeroTo255 - 1;
    }

    dogSPIout(value);               //Send data to display

}

//---------------------------------------------------------------

void PageSelect(unsigned char page){          //0 to 7 spans page numbers

	DOGENABLE;
    DOGCOMMAND;                    //Command
    dogSPIout(176+ page);          //Select display page
    DOGDATA;                       //Data display select
    DOGDISABLE;

}

//---------------------------------------------------------------

void Reset(void){

    DOGCOMMAND;                //Command
    dogSPIout(226);             //Internal display reset
    DOGDATA;                   //Data display select

}

//---------------------------------------------------------------

void LineSelect(unsigned char line){          //Line addresses 0 to 63

    DOGCOMMAND;                    //Command
    dogSPIout(64+ line);            //Select display start line
    DOGDATA;                       //Data display select

}

//---------------------------------------------------------------

void ColumnSelect(unsigned char column){    //Column address 0 to 127

    DOGCOMMAND;                   //Command
    dogSPIout(16+(column >> 4));   //Select display column
                                  //highbyte
    dogSPIout((column & 15));      //Select display column lowbyte
    DOGDATA;                      //Data display select

}

//---------------------------------------------------------------


void ClearDisplay(void){
	DOGENABLE;
    unsigned char page=0;
	int i;
	// int counter=0;
    //for (page = 0; page <8; page++)
	while(page<8)
	{
        PageSelect(page++);
        ColumnSelect(0);

            for(i = 0; i < 128; i++){  //Write Column 0 to 127 with value 0

            	dogSPIout(0);
            }
	}
    DOGDISABLE;
}



//---------------------------------------------------------------

void Display_ON(void){

    DOGCOMMAND;                   //Command
    dogSPIout(175);                 //switch Display on
    DOGDATA;                      //Data display select

}

//---------------------------------------------------------------

void Display_OFF(void){

    DOGCOMMAND;                   //Command
    dogSPIout(174);                 //switch Display off
    DOGDATA;                      //Data display select

}

/*
void ClearDisplay(void) {
    unsigned char CLEAR_DIS[1024];             //The image size is 666*2 byte
    int pixel;
    int i=0;
    unsigned char page;
    int counter;

    // Display_OFF();            //Display off

    for(i=0;i<1024;i++){
    	CLEAR_DIS[ i ]=0;
    }
    counter=0;
    for(page = 0; page < 8; page++){
	   pixel = 0;
        PageSelect(page);
        ColumnSelect(0);
        while(pixel<128)
		{
            dogSPIout(CLEAR_DIS[counter]);
            pixel++;               //Increase pixel
            counter++;             //Increase counter
		}
    }

    // Display_ON();                  //Display on
}
*/

void ClearDisplay1(void){
	DOGENABLE;
    int page=1;
	int i;
    //for (page = 0; page <8; page++)
	while(page<8)
	{
        PageSelect(page);

            for(i = 0; i < 128; i++){  //Write Column 0 to 127 with value 0

            	dogSPIout(0x00);
            }
            page++;
	}
    DOGDISABLE;
}

//---------------------------------------------------------------

void ISU_Logo(void) {
    unsigned char ISU_Pix[576];             //The image size is 576*2 byte
    int pixel;
    unsigned char page;
    int counter;

    Display_OFF();            //Display off

//page1
ISU_Pix[ 0]=0;            //Picture Data
ISU_Pix[ 1]=0;
ISU_Pix[ 2]=0;
ISU_Pix[ 3]=0;
ISU_Pix[ 4]=0;
ISU_Pix[ 5]=0;
ISU_Pix[ 6]=0;
ISU_Pix[ 7]=0;
ISU_Pix[ 8]=0;
ISU_Pix[ 9]=0;
ISU_Pix[ 10]=0;
ISU_Pix[ 11]=0;
ISU_Pix[ 12]=0;
ISU_Pix[ 13]=0;
ISU_Pix[ 14]=0;
ISU_Pix[ 15]=0;
ISU_Pix[ 16]=0;
ISU_Pix[ 17]=0;
ISU_Pix[ 18]=0;
ISU_Pix[ 19]=192;
ISU_Pix[ 20]=64;
ISU_Pix[ 21]=64;
ISU_Pix[ 22]=64;
ISU_Pix[ 23]=64;
ISU_Pix[ 24]=64;
ISU_Pix[ 25]=64;
ISU_Pix[ 26]=64;
ISU_Pix[ 27]=64;
ISU_Pix[ 28]=64;
ISU_Pix[ 29]=64;
ISU_Pix[ 30]=64;
ISU_Pix[ 31]=64;
ISU_Pix[ 32]=64;
ISU_Pix[ 33]=64;
ISU_Pix[ 34]=64;
ISU_Pix[ 35]=64;
ISU_Pix[ 36]=64;
ISU_Pix[ 37]=64;
ISU_Pix[ 38]=64;
ISU_Pix[ 39]=64;
ISU_Pix[ 40]=64;
ISU_Pix[ 41]=64;
ISU_Pix[ 42]=64;
ISU_Pix[ 43]=64;
ISU_Pix[ 44]=64;
ISU_Pix[ 45]=64;
ISU_Pix[ 46]=64;
ISU_Pix[ 47]=64;
ISU_Pix[ 48]=64;
ISU_Pix[ 49]=64;
ISU_Pix[ 50]=192;
ISU_Pix[ 51]=0;
ISU_Pix[ 52]=0;
ISU_Pix[ 53]=0;
ISU_Pix[ 54]=0;
ISU_Pix[ 55]=0;
ISU_Pix[ 56]=0;
ISU_Pix[ 57]=0;
ISU_Pix[ 58]=0;
ISU_Pix[ 59]=0;
ISU_Pix[ 60]=0;
ISU_Pix[ 61]=0;
ISU_Pix[ 62]=0;
ISU_Pix[ 63]=0;
ISU_Pix[ 64]=0;
ISU_Pix[ 65]=0;
ISU_Pix[ 66]=0;
ISU_Pix[ 67]=0;
ISU_Pix[ 68]=0;
ISU_Pix[ 69]=0;
ISU_Pix[ 70]=0;
ISU_Pix[ 71]=0;

//page2
ISU_Pix[ 72]=0;
ISU_Pix[ 73]=0;
ISU_Pix[ 74]=0;
ISU_Pix[ 75]=0;
ISU_Pix[ 76]=0;
ISU_Pix[ 77]=0;
ISU_Pix[ 78]=0;
ISU_Pix[ 79]=0;
ISU_Pix[ 80]=0;
ISU_Pix[ 81]=0;
ISU_Pix[ 82]=0;
ISU_Pix[ 83]=0;
ISU_Pix[ 84]=0;
ISU_Pix[ 85]=0;
ISU_Pix[ 86]=0;
ISU_Pix[ 87]=0;
ISU_Pix[ 88]=0;
ISU_Pix[ 89]=0;
ISU_Pix[ 90]=0;
ISU_Pix[ 91]=7;
ISU_Pix[ 92]=248;
ISU_Pix[ 93]=0;
ISU_Pix[ 94]=6;
ISU_Pix[ 95]=122;
ISU_Pix[ 96]=194;
ISU_Pix[ 97]=162;
ISU_Pix[ 98]=66;
ISU_Pix[ 99]=162;
ISU_Pix[ 100]=66;
ISU_Pix[ 101]=162;
ISU_Pix[ 102]=66;
ISU_Pix[ 103]=166;
ISU_Pix[ 104]=66;
ISU_Pix[ 105]=34;
ISU_Pix[ 106]=2;
ISU_Pix[ 107]=226;
ISU_Pix[ 108]=226;
ISU_Pix[ 109]=194;
ISU_Pix[ 110]=166;
ISU_Pix[ 111]=66;
ISU_Pix[ 112]=166;
ISU_Pix[ 113]=66;
ISU_Pix[ 114]=166;
ISU_Pix[ 115]=66;
ISU_Pix[ 116]=166;
ISU_Pix[ 117]=194;
ISU_Pix[ 118]=122;
ISU_Pix[ 119]=6;
ISU_Pix[ 120]=0;
ISU_Pix[ 121]=248;
ISU_Pix[ 122]=7;
ISU_Pix[ 123]=0;
ISU_Pix[ 124]=0;
ISU_Pix[ 125]=0;
ISU_Pix[ 126]=0;
ISU_Pix[ 127]=0;
ISU_Pix[ 128]=0;
ISU_Pix[ 129]=0;
ISU_Pix[ 130]=0;
ISU_Pix[ 131]=0;
ISU_Pix[ 132]=0;
ISU_Pix[ 133]=0;
ISU_Pix[ 134]=0;
ISU_Pix[ 135]=0;
ISU_Pix[ 136]=0;
ISU_Pix[ 137]=0;
ISU_Pix[ 138]=0;
ISU_Pix[ 139]=0;
ISU_Pix[ 140]=0;
ISU_Pix[ 141]=0;
ISU_Pix[ 142]=0;
ISU_Pix[ 143]=0;

//page 3
ISU_Pix[ 144]=0;
ISU_Pix[ 145]=0;
ISU_Pix[ 146]=0;
ISU_Pix[ 147]=0;
ISU_Pix[ 148]=0;
ISU_Pix[ 149]=0;
ISU_Pix[ 150]=0;
ISU_Pix[ 151]=0;
ISU_Pix[ 152]=0;
ISU_Pix[ 153]=0;
ISU_Pix[ 154]=0;
ISU_Pix[ 155]=0;
ISU_Pix[ 156]=0;
ISU_Pix[ 157]=0;
ISU_Pix[ 158]=0;
ISU_Pix[ 159]=0;
ISU_Pix[ 160]=0;
ISU_Pix[ 161]=0;
ISU_Pix[ 162]=0;
ISU_Pix[ 163]=0;
ISU_Pix[ 164]=0;
ISU_Pix[ 165]=7;
ISU_Pix[ 166]=8;
ISU_Pix[ 167]=144;
ISU_Pix[ 168]=145;
ISU_Pix[ 169]=146;
ISU_Pix[ 170]=243;
ISU_Pix[ 171]=130;
ISU_Pix[ 172]=131;
ISU_Pix[ 173]=254;
ISU_Pix[ 174]=129;
ISU_Pix[ 175]=0;
ISU_Pix[ 176]=0;
ISU_Pix[ 177]=128;
ISU_Pix[ 178]=64;
ISU_Pix[ 179]=127;
ISU_Pix[ 180]=127;
ISU_Pix[ 181]=127;
ISU_Pix[ 182]=255;
ISU_Pix[ 183]=255;
ISU_Pix[ 184]=254;
ISU_Pix[ 185]=131;
ISU_Pix[ 186]=130;
ISU_Pix[ 187]=131;
ISU_Pix[ 188]=242;
ISU_Pix[ 189]=145;
ISU_Pix[ 190]=144;
ISU_Pix[ 191]=136;
ISU_Pix[ 192]=135;
ISU_Pix[ 193]=128;
ISU_Pix[ 194]=0;
ISU_Pix[ 195]=0;
ISU_Pix[ 196]=0;
ISU_Pix[ 197]=0;
ISU_Pix[ 198]=0;
ISU_Pix[ 199]=0;
ISU_Pix[ 200]=0;
ISU_Pix[ 201]=0;
ISU_Pix[ 202]=0;
ISU_Pix[ 203]=0;
ISU_Pix[ 204]=0;
ISU_Pix[ 205]=0;
ISU_Pix[ 206]=0;
ISU_Pix[ 207]=0;
ISU_Pix[ 208]=0;
ISU_Pix[ 209]=0;
ISU_Pix[ 210]=0;
ISU_Pix[ 211]=0;
ISU_Pix[ 212]=0;
ISU_Pix[ 213]=0;
ISU_Pix[ 214]=0;
ISU_Pix[ 215]=0;

//page 4
ISU_Pix[ 216]=0;
ISU_Pix[ 217]=0;
ISU_Pix[ 218]=192;
ISU_Pix[ 219]=64;
ISU_Pix[ 220]=32;
ISU_Pix[ 221]=128;
ISU_Pix[ 222]=144;
ISU_Pix[ 223]=208;
ISU_Pix[ 224]=200;
ISU_Pix[ 225]=200;
ISU_Pix[ 226]=136;
ISU_Pix[ 227]=132;
ISU_Pix[ 228]=132;
ISU_Pix[ 229]=252;
ISU_Pix[ 230]=126;
ISU_Pix[ 231]=190;
ISU_Pix[ 232]=66;
ISU_Pix[ 233]=66;
ISU_Pix[ 234]=121;
ISU_Pix[ 235]=25;
ISU_Pix[ 236]=121;
ISU_Pix[ 237]=1;
ISU_Pix[ 238]=1;
ISU_Pix[ 239]=0;
ISU_Pix[ 240]=252;
ISU_Pix[ 241]=12;
ISU_Pix[ 242]=28;
ISU_Pix[ 243]=16;
ISU_Pix[ 244]=16;
ISU_Pix[ 245]=24;
ISU_Pix[ 246]=255;
ISU_Pix[ 247]=252;
ISU_Pix[ 248]=14;
ISU_Pix[ 249]=1;
ISU_Pix[ 250]=192;
ISU_Pix[ 251]=248;
ISU_Pix[ 252]=252;
ISU_Pix[ 253]=224;
ISU_Pix[ 254]=0;
ISU_Pix[ 255]=3;
ISU_Pix[ 256]=31;
ISU_Pix[ 257]=255;
ISU_Pix[ 258]=254;
ISU_Pix[ 259]=16;
ISU_Pix[ 260]=16;
ISU_Pix[ 261]=28;
ISU_Pix[ 262]=28;
ISU_Pix[ 263]=252;
ISU_Pix[ 264]=216;
ISU_Pix[ 265]=0;
ISU_Pix[ 266]=1;
ISU_Pix[ 267]=249;
ISU_Pix[ 268]=249;
ISU_Pix[ 269]=57;
ISU_Pix[ 270]=121;
ISU_Pix[ 271]=65;
ISU_Pix[ 272]=65;
ISU_Pix[ 273]=126;
ISU_Pix[ 274]=28;
ISU_Pix[ 275]=180;
ISU_Pix[ 276]=228;
ISU_Pix[ 277]=4;
ISU_Pix[ 278]=8;
ISU_Pix[ 279]=200;
ISU_Pix[ 280]=200;
ISU_Pix[ 281]=208;
ISU_Pix[ 282]=144;
ISU_Pix[ 283]=160;
ISU_Pix[ 284]=32;
ISU_Pix[ 285]=32;
ISU_Pix[ 286]=64;
ISU_Pix[ 287]=128;

//page 5
ISU_Pix[ 288]=0;
ISU_Pix[ 289]=255;
ISU_Pix[ 290]=128;
ISU_Pix[ 291]=128;
ISU_Pix[ 292]=198;
ISU_Pix[ 293]=205;
ISU_Pix[ 294]=72;
ISU_Pix[ 295]=100;
ISU_Pix[ 296]=36;
ISU_Pix[ 297]=227;
ISU_Pix[ 298]=3;
ISU_Pix[ 299]=3;
ISU_Pix[ 300]=2;
ISU_Pix[ 301]=254;
ISU_Pix[ 302]=248;
ISU_Pix[ 303]=0;
ISU_Pix[ 304]=0;
ISU_Pix[ 305]=0;
ISU_Pix[ 306]=128;
ISU_Pix[ 307]=64;
ISU_Pix[ 308]=63;
ISU_Pix[ 309]=0;
ISU_Pix[ 310]=0;
ISU_Pix[ 311]=0;
ISU_Pix[ 312]=191;
ISU_Pix[ 313]=176;
ISU_Pix[ 314]=224;
ISU_Pix[ 315]=240;
ISU_Pix[ 316]=144;
ISU_Pix[ 317]=158;
ISU_Pix[ 318]=135;
ISU_Pix[ 319]=128;
ISU_Pix[ 320]=240;
ISU_Pix[ 321]=30;
ISU_Pix[ 322]=5;
ISU_Pix[ 323]=253;
ISU_Pix[ 324]=253;
ISU_Pix[ 325]=253;
ISU_Pix[ 326]=252;
ISU_Pix[ 327]=248;
ISU_Pix[ 328]=192;
ISU_Pix[ 329]=128;
ISU_Pix[ 330]=129;
ISU_Pix[ 331]=159;
ISU_Pix[ 332]=145;
ISU_Pix[ 333]=240;
ISU_Pix[ 334]=224;
ISU_Pix[ 335]=191;
ISU_Pix[ 336]=159;
ISU_Pix[ 337]=0;
ISU_Pix[ 338]=0;
ISU_Pix[ 339]=63;
ISU_Pix[ 340]=47;
ISU_Pix[ 341]=192;
ISU_Pix[ 342]=0;
ISU_Pix[ 343]=0;
ISU_Pix[ 344]=0;
ISU_Pix[ 345]=0;
ISU_Pix[ 346]=128;
ISU_Pix[ 347]=143;
ISU_Pix[ 348]=249;
ISU_Pix[ 349]=0;
ISU_Pix[ 350]=0;
ISU_Pix[ 351]=227;
ISU_Pix[ 352]=243;
ISU_Pix[ 353]=36;
ISU_Pix[ 354]=100;
ISU_Pix[ 355]=29;
ISU_Pix[ 356]=15;
ISU_Pix[ 357]=8;
ISU_Pix[ 358]=24;
ISU_Pix[ 359]=15;

//page6
ISU_Pix[ 360]= 128;
ISU_Pix[ 361]=252;
ISU_Pix[ 362]=194;
ISU_Pix[ 363]=65;
ISU_Pix[ 364]=101;
ISU_Pix[ 365]=39;
ISU_Pix[ 366]=52;
ISU_Pix[ 367]=18;
ISU_Pix[ 368]=18;
ISU_Pix[ 369]=25;
ISU_Pix[ 370]=8;
ISU_Pix[ 371]=12;
ISU_Pix[ 372]=6;
ISU_Pix[ 373]=3;
ISU_Pix[ 374]=0;
ISU_Pix[ 375]=0;
ISU_Pix[ 376]=0;
ISU_Pix[ 377]=0;
ISU_Pix[ 378]=3;
ISU_Pix[ 379]=3;
ISU_Pix[ 380]=3;
ISU_Pix[ 381]=193;
ISU_Pix[ 382]=33;
ISU_Pix[ 383]=17;
ISU_Pix[ 384]=17;
ISU_Pix[ 385]=145;
ISU_Pix[ 386]=159;
ISU_Pix[ 387]=128;
ISU_Pix[ 388]=128;
ISU_Pix[ 389]=255;
ISU_Pix[ 390]=0;
ISU_Pix[ 391]=0;
ISU_Pix[ 392]=0;
ISU_Pix[ 393]=0;
ISU_Pix[ 394]=0;
ISU_Pix[ 395]=255;
ISU_Pix[ 396]=255;
ISU_Pix[ 397]=255;
ISU_Pix[ 398]=255;
ISU_Pix[ 399]=255;
ISU_Pix[ 400]=127;
ISU_Pix[ 401]=128;
ISU_Pix[ 402]=128;
ISU_Pix[ 403]=128;
ISU_Pix[ 404]=159;
ISU_Pix[ 405]=16;
ISU_Pix[ 406]=17;
ISU_Pix[ 407]=33;
ISU_Pix[ 408]=193;
ISU_Pix[ 409]=1;
ISU_Pix[ 410]=1;
ISU_Pix[ 411]=1;
ISU_Pix[ 412]=3;
ISU_Pix[ 413]=3;
ISU_Pix[ 414]=0;
ISU_Pix[ 415]=0;
ISU_Pix[ 416]=0;
ISU_Pix[ 417]=0;
ISU_Pix[ 418]=7;
ISU_Pix[ 419]=5;
ISU_Pix[ 420]=12;
ISU_Pix[ 421]=8;
ISU_Pix[ 422]=8;
ISU_Pix[ 423]=25;
ISU_Pix[ 424]=19;
ISU_Pix[ 425]=18;
ISU_Pix[ 426]=52;
ISU_Pix[ 427]=36;
ISU_Pix[ 428]=103;
ISU_Pix[ 429]=65;
ISU_Pix[ 430]=194;
ISU_Pix[ 431]=252;

//page7
ISU_Pix[ 432]=0;
ISU_Pix[ 433]=0;
ISU_Pix[ 434]=0;
ISU_Pix[ 435]=0;
ISU_Pix[ 436]=0;
ISU_Pix[ 437]=0;
ISU_Pix[ 438]=0;
ISU_Pix[ 439]=0;
ISU_Pix[ 440]=0;
ISU_Pix[ 441]=0;
ISU_Pix[ 442]=0;
ISU_Pix[ 443]=0;
ISU_Pix[ 444]=0;
ISU_Pix[ 445]=0;
ISU_Pix[ 446]=0;
ISU_Pix[ 447]=0;
ISU_Pix[ 448]=0;
ISU_Pix[ 449]=0;
ISU_Pix[ 450]=0;
ISU_Pix[ 451]=192;
ISU_Pix[ 452]=62;
ISU_Pix[ 453]=1;
ISU_Pix[ 454]=192;
ISU_Pix[ 455]=188;
ISU_Pix[ 456]=212;
ISU_Pix[ 457]=168;
ISU_Pix[ 458]=208;
ISU_Pix[ 459]=168;
ISU_Pix[ 460]=208;
ISU_Pix[ 461]=168;
ISU_Pix[ 462]=208;
ISU_Pix[ 463]=168;
ISU_Pix[ 464]=208;
ISU_Pix[ 465]=168;
ISU_Pix[ 466]=208;
ISU_Pix[ 467]=175;
ISU_Pix[ 468]=215;
ISU_Pix[ 469]=171;
ISU_Pix[ 470]=209;
ISU_Pix[ 471]=168;
ISU_Pix[ 472]=208;
ISU_Pix[ 473]=168;
ISU_Pix[ 474]=208;
ISU_Pix[ 475]=168;
ISU_Pix[ 476]=208;
ISU_Pix[ 477]=171;
ISU_Pix[ 478]=253;
ISU_Pix[ 479]=192;
ISU_Pix[ 480]=1;
ISU_Pix[ 481]=62;
ISU_Pix[ 482]=192;
ISU_Pix[ 483]=0;
ISU_Pix[ 484]=0;
ISU_Pix[ 485]=0;
ISU_Pix[ 486]=0;
ISU_Pix[ 487]=0;
ISU_Pix[ 488]=0;
ISU_Pix[ 489]=0;
ISU_Pix[ 490]=0;
ISU_Pix[ 491]=0;
ISU_Pix[ 492]=0;
ISU_Pix[ 493]=0;
ISU_Pix[ 494]=0;
ISU_Pix[ 495]=0;
ISU_Pix[ 496]=0;
ISU_Pix[ 497]=0;
ISU_Pix[ 498]=0;
ISU_Pix[ 499]=0;
ISU_Pix[ 500]=0;
ISU_Pix[ 501]=0;
ISU_Pix[ 502]=0;
ISU_Pix[ 503]=0;

//page8
ISU_Pix[ 504]=0;
ISU_Pix[ 505]=0;
ISU_Pix[ 506]=0;
ISU_Pix[ 507]=0;
ISU_Pix[ 508]=0;
ISU_Pix[ 509]=0;
ISU_Pix[ 510]=0;
ISU_Pix[ 511]=0;
ISU_Pix[ 512]=0;
ISU_Pix[ 513]=0;
ISU_Pix[ 514]=0;
ISU_Pix[ 515]=0;
ISU_Pix[ 516]=0;
ISU_Pix[ 517]=0;
ISU_Pix[ 518]=0;
ISU_Pix[ 519]=0;
ISU_Pix[ 520]=0;
ISU_Pix[ 521]=0;
ISU_Pix[ 522]=0;
ISU_Pix[ 523]=7;
ISU_Pix[ 524]=4;
ISU_Pix[ 525]=4;
ISU_Pix[ 526]=4;
ISU_Pix[ 527]=4;
ISU_Pix[ 528]=4;
ISU_Pix[ 529]=4;
ISU_Pix[ 530]=4;
ISU_Pix[ 531]=4;
ISU_Pix[ 532]=4;
ISU_Pix[ 533]=4;
ISU_Pix[ 534]=4;
ISU_Pix[ 535]=4;
ISU_Pix[ 536]=4;
ISU_Pix[ 537]=4;
ISU_Pix[ 538]=4;
ISU_Pix[ 539]=4;
ISU_Pix[ 540]=4;
ISU_Pix[ 541]=4;
ISU_Pix[ 542]=4;
ISU_Pix[ 543]=4;
ISU_Pix[ 544]=4;
ISU_Pix[ 545]=4;
ISU_Pix[ 546]=4;
ISU_Pix[ 547]=4;
ISU_Pix[ 548]=4;
ISU_Pix[ 549]=4;
ISU_Pix[ 550]=4;
ISU_Pix[ 551]=4;
ISU_Pix[ 552]=4;
ISU_Pix[ 553]=4;
ISU_Pix[ 554]=7;
ISU_Pix[ 555]=0;
ISU_Pix[ 556]=0;
ISU_Pix[ 557]=0;
ISU_Pix[ 558]=0;
ISU_Pix[ 559]=0;
ISU_Pix[ 560]=0;
ISU_Pix[ 561]=0;
ISU_Pix[ 562]=0;
ISU_Pix[ 563]=0;
ISU_Pix[ 564]=0;
ISU_Pix[ 565]=0;
ISU_Pix[ 566]=0;
ISU_Pix[ 567]=0;
ISU_Pix[ 568]=0;
ISU_Pix[ 569]=0;
ISU_Pix[ 570]=0;
ISU_Pix[ 571]=0;
ISU_Pix[ 572]=0;
ISU_Pix[ 573]=0;
ISU_Pix[ 574]=0;
ISU_Pix[ 575]=0;

    counter=0;

    for(page = 0; page < 8; page++){
	   pixel = 0;
        PageSelect(page);		//Select page 0-7
        ColumnSelect(29);         //Select column 29
        while(pixel<72)
		{
            dogSPIout(ISU_Pix[counter]);
            pixel++;               //Increase pixel
            counter++;             //Increase counter
		}
    }

    Display_ON();                  //Display on

}


void BATT_Charging(void) {
    unsigned char BATT_CHG[666];             //The image size is 666*2 byte
    int pixel;
    unsigned char page;
    int counter;

    Display_OFF();            //Display off


//page1
BATT_CHG[ 0 ]=240;     //picture data
BATT_CHG[ 1 ]=248;
BATT_CHG[ 2 ]=12;
BATT_CHG[ 3]=4;
BATT_CHG[ 4]=4;
BATT_CHG[ 5]=4;
BATT_CHG[ 6]=12;
BATT_CHG[ 7]=8;
BATT_CHG[ 8]=0;
BATT_CHG[ 9]=4;
BATT_CHG[ 10]=252;
BATT_CHG[ 11]=252;
BATT_CHG[ 12]=64;
BATT_CHG[ 13]=32;
BATT_CHG[ 14]=224;
BATT_CHG[ 15]=192;
BATT_CHG[ 16]=0;
BATT_CHG[ 17]=0;
BATT_CHG[ 18]=64;
BATT_CHG[ 19]=32;
BATT_CHG[ 20]=160;
BATT_CHG[ 21]=224;
BATT_CHG[ 22]=192;
BATT_CHG[ 23]=0;
BATT_CHG[ 24]=32;
BATT_CHG[ 25]=224;
BATT_CHG[ 26]=224;
BATT_CHG[ 27]=64;
BATT_CHG[ 28]=96;
BATT_CHG[ 29]=0;
BATT_CHG[ 30]=192;
BATT_CHG[ 31]=224;
BATT_CHG[ 32]=32;
BATT_CHG[ 33]=32;
BATT_CHG[ 34]=224;
BATT_CHG[ 35]=224;
BATT_CHG[ 36]=32;
BATT_CHG[ 37]=32;
BATT_CHG[ 38]=224;
BATT_CHG[ 39]=224;
BATT_CHG[ 40]=64;
BATT_CHG[ 41]=32;
BATT_CHG[ 42]=224;
BATT_CHG[ 43]=192;
BATT_CHG[ 44]=0;
BATT_CHG[ 45]=32;
BATT_CHG[ 46]=236;
BATT_CHG[ 47]=236;
BATT_CHG[ 48]=0;
BATT_CHG[ 49]=32;
BATT_CHG[ 50]=224;
BATT_CHG[ 51]=224;
BATT_CHG[ 52]=64;
BATT_CHG[ 53]=32;
BATT_CHG[ 54]=224;
BATT_CHG[ 55]=192;
BATT_CHG[ 56]=0;
BATT_CHG[ 57]=0;
BATT_CHG[ 58]=192;
BATT_CHG[ 59]=224;
BATT_CHG[ 60]=32;
BATT_CHG[ 61]=32;
BATT_CHG[ 62]=192;
BATT_CHG[ 63]=192;
BATT_CHG[ 64]=32;
BATT_CHG[ 65]=0;
BATT_CHG[ 66]=0;
BATT_CHG[ 67]=0;
BATT_CHG[ 68]=4;
BATT_CHG[ 69]=252;
BATT_CHG[ 70]=252;
BATT_CHG[ 71]=68;
BATT_CHG[ 72]=68;
BATT_CHG[ 73]=68;
BATT_CHG[ 74]=252;
BATT_CHG[ 75]=88;
BATT_CHG[ 76]=0;
BATT_CHG[ 77]=0;
BATT_CHG[ 78]=64;
BATT_CHG[ 79]=32;
BATT_CHG[ 80]=160;
BATT_CHG[ 81]=224;
BATT_CHG[ 82]=192;
BATT_CHG[ 83]=0;
BATT_CHG[ 84]=32;
BATT_CHG[ 85]=248;
BATT_CHG[ 86]=248;
BATT_CHG[ 87]=32;
BATT_CHG[ 88]=32;
BATT_CHG[ 89]=248;
BATT_CHG[ 90]=248;
BATT_CHG[ 91]=32;
BATT_CHG[ 92]=0;
BATT_CHG[ 93]=192;
BATT_CHG[ 94]=224;
BATT_CHG[ 95]=160;
BATT_CHG[ 96]=224;
BATT_CHG[ 97]=192;
BATT_CHG[ 98]=0;
BATT_CHG[ 99]=32;
BATT_CHG[ 100]=224;
BATT_CHG[ 101]=224;
BATT_CHG[ 102]=64;
BATT_CHG[ 103]=96;
BATT_CHG[ 104]=32;
BATT_CHG[ 105]=224;
BATT_CHG[ 106]=224;
BATT_CHG[ 107]=0;
BATT_CHG[ 108]=32;
BATT_CHG[ 109]=224;
BATT_CHG[ 110]=32;

//page3
BATT_CHG[ 111]=1;
BATT_CHG[ 112]=3;
BATT_CHG[ 113]=6;
BATT_CHG[ 114]=4;
BATT_CHG[ 115]=4;
BATT_CHG[ 116]=4;
BATT_CHG[ 117]=4;
BATT_CHG[ 118]=2;
BATT_CHG[ 119]=0;
BATT_CHG[ 120]=4;
BATT_CHG[ 121]=7;
BATT_CHG[ 122]=7;
BATT_CHG[ 123]=4;
BATT_CHG[ 124]=0;
BATT_CHG[ 125]=7;
BATT_CHG[ 126]=7;
BATT_CHG[ 127]=4;
BATT_CHG[ 128]=0;
BATT_CHG[ 129]=2;
BATT_CHG[ 130]=7;
BATT_CHG[ 131]=4;
BATT_CHG[ 132]=7;
BATT_CHG[ 133]=7;
BATT_CHG[ 134]=4;
BATT_CHG[ 135]=4;
BATT_CHG[ 136]=7;
BATT_CHG[ 137]=7;
BATT_CHG[ 138]=4;
BATT_CHG[ 139]=0;
BATT_CHG[ 140]=0;
BATT_CHG[ 141]=54;
BATT_CHG[ 142]=77;
BATT_CHG[ 143]=77;
BATT_CHG[ 144]=77;
BATT_CHG[ 145]=77;
BATT_CHG[ 146]=60;
BATT_CHG[ 147]=24;
BATT_CHG[ 148]=4;
BATT_CHG[ 149]=7;
BATT_CHG[ 150]=7;
BATT_CHG[ 151]=4;
BATT_CHG[ 152]=0;
BATT_CHG[ 153]=7;
BATT_CHG[ 154]=7;
BATT_CHG[ 155]=4;
BATT_CHG[ 156]=4;
BATT_CHG[ 157]=7;
BATT_CHG[ 158]=7;
BATT_CHG[ 159]=4;
BATT_CHG[ 160]=4;
BATT_CHG[ 161]=7;
BATT_CHG[ 162]=7;
BATT_CHG[ 163]=4;
BATT_CHG[ 164]=0;
BATT_CHG[ 165]=7;
BATT_CHG[ 166]=7;
BATT_CHG[ 167]=4;
BATT_CHG[ 168]=0;
BATT_CHG[ 169]=54;
BATT_CHG[ 170]=77;
BATT_CHG[ 171]=77;
BATT_CHG[ 172]=77;
BATT_CHG[ 173]=77;
BATT_CHG[ 174]=60;
BATT_CHG[ 175]=24;
BATT_CHG[ 176]=0;
BATT_CHG[ 177]=0;
BATT_CHG[ 178]=0;
BATT_CHG[ 179]=4;
BATT_CHG[ 180]=7;
BATT_CHG[ 181]=7;
BATT_CHG[ 182]=4;
BATT_CHG[ 183]=4;
BATT_CHG[ 184]=4;
BATT_CHG[ 185]=7;
BATT_CHG[ 186]=3;
BATT_CHG[ 187]=0;
BATT_CHG[ 188]=0;
BATT_CHG[ 189]=2;
BATT_CHG[ 190]=7;
BATT_CHG[ 191]=4;
BATT_CHG[ 192]=7;
BATT_CHG[ 193]=7;
BATT_CHG[ 194]=4;
BATT_CHG[ 195]=0;
BATT_CHG[ 196]=3;
BATT_CHG[ 197]=7;
BATT_CHG[ 198]=4;
BATT_CHG[ 199]=0;
BATT_CHG[ 200]=3;
BATT_CHG[ 201]=7;
BATT_CHG[ 202]=4;
BATT_CHG[ 203]=0;
BATT_CHG[ 204]=3;
BATT_CHG[ 205]=7;
BATT_CHG[ 206]=4;
BATT_CHG[ 207]=4;
BATT_CHG[ 208]=4;
BATT_CHG[ 209]=0;
BATT_CHG[ 210]=4;
BATT_CHG[ 211]=7;
BATT_CHG[ 212]=7;
BATT_CHG[ 213]=4;
BATT_CHG[ 214]=0;
BATT_CHG[ 215]=64;
BATT_CHG[ 216]=64;
BATT_CHG[ 217]=99;
BATT_CHG[ 218]=15;
BATT_CHG[ 219]=3;
BATT_CHG[ 220]=0;
BATT_CHG[ 221]=0;

//page4
BATT_CHG[ 222]=0;
BATT_CHG[ 223]=0;
BATT_CHG[ 224]=0;
BATT_CHG[ 225]=0;
BATT_CHG[ 226]=0;
BATT_CHG[ 227]=0;
BATT_CHG[ 228]=0;
BATT_CHG[ 229]=0;
BATT_CHG[ 230]=0;
BATT_CHG[ 231]=0;
BATT_CHG[ 232]=0;
BATT_CHG[ 233]=0;
BATT_CHG[ 234]=0;
BATT_CHG[ 235]=0;
BATT_CHG[ 236]=0;
BATT_CHG[ 237]=0;
BATT_CHG[ 238]=0;
BATT_CHG[ 239]=0;
BATT_CHG[ 240]=224;
BATT_CHG[ 241]=48;
BATT_CHG[ 242]=16;
BATT_CHG[ 243]=16;
BATT_CHG[ 244]=16;
BATT_CHG[ 245]=16;
BATT_CHG[ 246]=16;
BATT_CHG[ 247]=16;
BATT_CHG[ 248]=16;
BATT_CHG[ 249]=16;
BATT_CHG[ 250]=16;
BATT_CHG[ 251]=16;
BATT_CHG[ 252]=16;
BATT_CHG[ 253]=16;
BATT_CHG[ 254]=16;
BATT_CHG[ 255]=16;
BATT_CHG[ 256]=16;
BATT_CHG[ 257]=16;
BATT_CHG[ 258]=16;
BATT_CHG[ 259]=16;
BATT_CHG[ 260]=16;
BATT_CHG[ 261]=16;
BATT_CHG[ 262]=16;
BATT_CHG[ 263]=16;
BATT_CHG[ 264]=16;
BATT_CHG[ 265]=16;
BATT_CHG[ 266]=16;
BATT_CHG[ 267]=16;
BATT_CHG[ 268]=16;
BATT_CHG[ 269]=16;
BATT_CHG[ 270]=16;
BATT_CHG[ 271]=16;
BATT_CHG[ 272]=16;
BATT_CHG[ 273]=16;
BATT_CHG[ 274]=16;
BATT_CHG[ 275]=16;
BATT_CHG[ 276]=16;
BATT_CHG[ 277]=16;
BATT_CHG[ 278]=16;
BATT_CHG[ 279]=16;
BATT_CHG[ 280]=16;
BATT_CHG[ 281]=16;
BATT_CHG[ 282]=16;
BATT_CHG[ 283]=16;
BATT_CHG[ 284]=16;
BATT_CHG[ 285]=16;
BATT_CHG[ 286]=16;
BATT_CHG[ 287]=16;
BATT_CHG[ 288]=16;
BATT_CHG[ 289]=16;
BATT_CHG[ 290]=16;
BATT_CHG[ 291]=16;
BATT_CHG[ 292]=16;
BATT_CHG[ 293]=16;
BATT_CHG[ 294]=16;
BATT_CHG[ 295]=16;
BATT_CHG[ 296]=16;
BATT_CHG[ 297]=16;
BATT_CHG[ 298]=16;
BATT_CHG[ 299]=16;
BATT_CHG[ 300]=16;
BATT_CHG[ 301]=16;
BATT_CHG[ 302]=16;
BATT_CHG[ 303]=16;
BATT_CHG[ 304]=16;
BATT_CHG[ 305]=16;
BATT_CHG[ 306]=16;
BATT_CHG[ 307]=16;
BATT_CHG[ 308]=16;
BATT_CHG[ 309]=16;
BATT_CHG[ 310]=16;
BATT_CHG[ 311]=16;
BATT_CHG[ 312]=16;
BATT_CHG[ 313]=48;
BATT_CHG[ 314]=224;
BATT_CHG[ 315]=0;
BATT_CHG[ 316]=0;
BATT_CHG[ 317]=0;
BATT_CHG[ 318]=0;
BATT_CHG[ 319]=0;
BATT_CHG[ 320]=0;
BATT_CHG[ 321]=0;
BATT_CHG[ 322]=0;
BATT_CHG[ 323]=0;
BATT_CHG[ 324]=0;
BATT_CHG[ 325]=0;
BATT_CHG[ 326]=0;
BATT_CHG[ 327]=0;
BATT_CHG[ 328]=0;
BATT_CHG[ 329]=0;
BATT_CHG[ 330]=0;
BATT_CHG[ 331]=0;
BATT_CHG[ 332]=0;

//page5
BATT_CHG[ 333]=0;
BATT_CHG[ 334]=0;
BATT_CHG[ 335]=0;
BATT_CHG[ 336]=0;
BATT_CHG[ 337]=0;
BATT_CHG[ 338]=0;
BATT_CHG[ 339]=0;
BATT_CHG[ 340]=0;
BATT_CHG[ 341]=0;
BATT_CHG[ 342]=0;
BATT_CHG[ 343]=0;
BATT_CHG[ 344]=0;
BATT_CHG[ 345]=0;
BATT_CHG[ 346]=0;
BATT_CHG[ 347]=0;
BATT_CHG[ 348]=0;
BATT_CHG[ 349]=0;
BATT_CHG[ 350]=0;
BATT_CHG[ 351]=255;
BATT_CHG[ 352]=0;
BATT_CHG[ 353]=0;
BATT_CHG[ 354]=0;
BATT_CHG[ 355]=255;
BATT_CHG[ 356]=1;
BATT_CHG[ 357]=1;
BATT_CHG[ 358]=253;
BATT_CHG[ 359]=253;
BATT_CHG[ 360]=253;
BATT_CHG[ 361]=253;
BATT_CHG[ 362]=249;
BATT_CHG[ 363]=241;
BATT_CHG[ 364]=225;
BATT_CHG[ 365]=193;
BATT_CHG[ 366]=133;
BATT_CHG[ 367]=13;
BATT_CHG[ 368]=29;
BATT_CHG[ 369]=61;
BATT_CHG[ 370]=125;
BATT_CHG[ 371]=253;
BATT_CHG[ 372]=253;
BATT_CHG[ 373]=253;
BATT_CHG[ 374]=253;
BATT_CHG[ 375]=253;
BATT_CHG[ 376]=253;
BATT_CHG[ 377]=253;
BATT_CHG[ 378]=253;
BATT_CHG[ 379]=253;
BATT_CHG[ 380]=249;
BATT_CHG[ 381]=241;
BATT_CHG[ 382]=225;
BATT_CHG[ 383]=193;
BATT_CHG[ 384]=133;
BATT_CHG[ 385]=13;
BATT_CHG[ 386]=29;
BATT_CHG[ 387]=61;
BATT_CHG[ 388]=125;
BATT_CHG[ 389]=253;
BATT_CHG[ 390]=253;
BATT_CHG[ 391]=253;
BATT_CHG[ 392]=253;
BATT_CHG[ 393]=253;
BATT_CHG[ 394]=253;
BATT_CHG[ 395]=253;
BATT_CHG[ 396]=253;
BATT_CHG[ 397]=253;
BATT_CHG[ 398]=249;
BATT_CHG[ 399]=241;
BATT_CHG[ 400]=225;
BATT_CHG[ 401]=193;
BATT_CHG[ 402]=133;
BATT_CHG[ 403]=13;
BATT_CHG[ 404]=29;
BATT_CHG[ 405]=61;
BATT_CHG[ 406]=125;
BATT_CHG[ 407]=253;
BATT_CHG[ 408]=253;
BATT_CHG[ 409]=253;
BATT_CHG[ 410]=253;
BATT_CHG[ 411]=253;
BATT_CHG[ 412]=253;
BATT_CHG[ 413]=253;
BATT_CHG[ 414]=253;
BATT_CHG[ 415]=253;
BATT_CHG[ 416]=253;
BATT_CHG[ 417]=253;
BATT_CHG[ 418]=253;
BATT_CHG[ 419]=1;
BATT_CHG[ 420]=1;
BATT_CHG[ 421]=255;
BATT_CHG[ 422]=0;
BATT_CHG[ 423]=0;
BATT_CHG[ 424]=0;
BATT_CHG[ 425]=255;
BATT_CHG[ 426]=4;
BATT_CHG[ 427]=12;
BATT_CHG[ 428]=248;
BATT_CHG[ 429]=0;
BATT_CHG[ 430]=0;
BATT_CHG[ 431]=0;
BATT_CHG[ 432]=0;
BATT_CHG[ 433]=0;
BATT_CHG[ 434]=0;
BATT_CHG[ 435]=0;
BATT_CHG[ 436]=0;
BATT_CHG[ 437]=0;
BATT_CHG[ 438]=0;
BATT_CHG[ 439]=0;
BATT_CHG[ 440]=0;
BATT_CHG[ 441]=0;
BATT_CHG[ 442]=0;
BATT_CHG[ 443]=0;
//page6
BATT_CHG[ 444]=0;
BATT_CHG[ 445]=0;
BATT_CHG[ 446]=0;
BATT_CHG[ 447]=0;
BATT_CHG[ 448]=0;
BATT_CHG[ 449]=0;
BATT_CHG[ 450]=0;
BATT_CHG[ 451]=0;
BATT_CHG[ 452]=0;
BATT_CHG[ 453]=0;
BATT_CHG[ 454]=0;
BATT_CHG[ 455]=0;
BATT_CHG[ 456]=0;
BATT_CHG[ 457]=0;
BATT_CHG[ 458]=0;
BATT_CHG[ 459]=0;
BATT_CHG[ 460]=0;
BATT_CHG[ 461]=0;
BATT_CHG[ 462]=255;
BATT_CHG[ 463]=0;
BATT_CHG[ 464]=0;
BATT_CHG[ 465]=0;
BATT_CHG[ 466]=255;
BATT_CHG[ 467]=0;
BATT_CHG[ 468]=0;
BATT_CHG[ 469]=255;
BATT_CHG[ 470]=255;
BATT_CHG[ 471]=255;
BATT_CHG[ 472]=255;
BATT_CHG[ 473]=255;
BATT_CHG[ 474]=255;
BATT_CHG[ 475]=255;
BATT_CHG[ 476]=255;
BATT_CHG[ 477]=255;
BATT_CHG[ 478]=255;
BATT_CHG[ 479]=254;
BATT_CHG[ 480]=252;
BATT_CHG[ 481]=248;
BATT_CHG[ 482]=240;
BATT_CHG[ 483]=225;
BATT_CHG[ 484]=195;
BATT_CHG[ 485]=135;
BATT_CHG[ 486]=15;
BATT_CHG[ 487]=31;
BATT_CHG[ 488]=63;
BATT_CHG[ 489]=127;
BATT_CHG[ 490]=255;
BATT_CHG[ 491]=255;
BATT_CHG[ 492]=255;
BATT_CHG[ 493]=255;
BATT_CHG[ 494]=255;
BATT_CHG[ 495]=255;
BATT_CHG[ 496]=255;
BATT_CHG[ 497]=254;
BATT_CHG[ 498]=252;
BATT_CHG[ 499]=248;
BATT_CHG[ 500]=240;
BATT_CHG[ 501]=225;
BATT_CHG[ 502]=195;
BATT_CHG[ 503]=135;
BATT_CHG[ 504]=15;
BATT_CHG[ 505]=31;
BATT_CHG[ 506]=63;
BATT_CHG[ 507]=127;
BATT_CHG[ 508]=255;
BATT_CHG[ 509]=255;
BATT_CHG[ 510]=255;
BATT_CHG[ 511]=255;
BATT_CHG[ 512]=255;
BATT_CHG[ 513]=255;
BATT_CHG[ 514]=255;
BATT_CHG[ 515]=254;
BATT_CHG[ 516]=252;
BATT_CHG[ 517]=248;
BATT_CHG[ 518]=240;
BATT_CHG[ 519]=225;
BATT_CHG[ 520]=195;
BATT_CHG[ 521]=135;
BATT_CHG[ 522]=15;
BATT_CHG[ 523]=31;
BATT_CHG[ 524]=63;
BATT_CHG[ 525]=127;
BATT_CHG[ 526]=255;
BATT_CHG[ 527]=255;
BATT_CHG[ 528]=255;
BATT_CHG[ 529]=255;
BATT_CHG[ 530]=0;
BATT_CHG[ 531]=0;
BATT_CHG[ 532]=255;
BATT_CHG[ 533]=0;
BATT_CHG[ 534]=0;
BATT_CHG[ 535]=0;
BATT_CHG[ 536]=255;
BATT_CHG[ 537]=128;
BATT_CHG[ 538]=192;
BATT_CHG[ 539]=127;
BATT_CHG[ 540]=0;
BATT_CHG[ 541]=0;
BATT_CHG[ 542]=0;
BATT_CHG[ 543]=0;
BATT_CHG[ 544]=0;
BATT_CHG[ 545]=0;
BATT_CHG[ 546]=0;
BATT_CHG[ 547]=0;
BATT_CHG[ 548]=0;
BATT_CHG[ 549]=0;
BATT_CHG[ 550]=0;
BATT_CHG[ 551]=0;
BATT_CHG[ 552]=0;
BATT_CHG[ 553]=0;
BATT_CHG[ 554]=0;

//page7
BATT_CHG[ 555]=0;
BATT_CHG[ 556]=0;
BATT_CHG[ 557]=0;
BATT_CHG[ 558]=0;
BATT_CHG[ 559]=0;
BATT_CHG[ 560]=0;
BATT_CHG[ 561]=0;
BATT_CHG[ 562]=0;
BATT_CHG[ 563]=0;
BATT_CHG[ 564]=0;
BATT_CHG[ 565]=0;
BATT_CHG[ 566]=0;
BATT_CHG[ 567]=0;
BATT_CHG[ 568]=0;
BATT_CHG[ 569]=0;
BATT_CHG[ 570]=0;
BATT_CHG[ 571]=0;
BATT_CHG[ 572]=0;
BATT_CHG[ 573]=31;
BATT_CHG[ 574]=48;
BATT_CHG[ 575]=32;
BATT_CHG[ 576]=32;
BATT_CHG[ 577]=35;
BATT_CHG[ 578]=34;
BATT_CHG[ 579]=34;
BATT_CHG[ 580]=34;
BATT_CHG[ 581]=34;
BATT_CHG[ 582]=34;
BATT_CHG[ 583]=34;
BATT_CHG[ 584]=34;
BATT_CHG[ 585]=34;
BATT_CHG[ 586]=34;
BATT_CHG[ 587]=34;
BATT_CHG[ 588]=34;
BATT_CHG[ 589]=34;
BATT_CHG[ 590]=34;
BATT_CHG[ 591]=34;
BATT_CHG[ 592]=34;
BATT_CHG[ 593]=34;
BATT_CHG[ 594]=34;
BATT_CHG[ 595]=34;
BATT_CHG[ 596]=34;
BATT_CHG[ 597]=34;
BATT_CHG[ 598]=34;
BATT_CHG[ 599]=34;
BATT_CHG[ 600]=34;
BATT_CHG[ 601]=34;
BATT_CHG[ 602]=34;
BATT_CHG[ 603]=34;
BATT_CHG[ 604]=34;
BATT_CHG[ 605]=34;
BATT_CHG[ 606]=34;
BATT_CHG[ 607]=34;
BATT_CHG[ 608]=34;
BATT_CHG[ 609]=34;
BATT_CHG[ 610]=34;
BATT_CHG[ 611]=34;
BATT_CHG[ 612]=34;
BATT_CHG[ 613]=34;
BATT_CHG[ 614]=34;
BATT_CHG[ 615]=34;
BATT_CHG[ 616]=34;
BATT_CHG[ 617]=34;
BATT_CHG[ 618]=34;
BATT_CHG[ 619]=34;
BATT_CHG[ 620]=34;
BATT_CHG[ 621]=34;
BATT_CHG[ 622]=34;
BATT_CHG[ 623]=34;
BATT_CHG[ 624]=34;
BATT_CHG[ 625]=34;
BATT_CHG[ 626]=34;
BATT_CHG[ 627]=34;
BATT_CHG[ 628]=34;
BATT_CHG[ 629]=34;
BATT_CHG[ 630]=34;
BATT_CHG[ 631]=34;
BATT_CHG[ 632]=34;
BATT_CHG[ 633]=34;
BATT_CHG[ 634]=34;
BATT_CHG[ 635]=34;
BATT_CHG[ 636]=34;
BATT_CHG[ 637]=34;
BATT_CHG[ 638]=34;
BATT_CHG[ 639]=34;
BATT_CHG[ 640]=34;
BATT_CHG[ 641]=34;
BATT_CHG[ 642]=34;
BATT_CHG[ 643]=35;
BATT_CHG[ 644]=32;
BATT_CHG[ 645]=32;
BATT_CHG[ 646]=48;
BATT_CHG[ 647]=31;
BATT_CHG[ 648]=0;
BATT_CHG[ 649]=0;
BATT_CHG[ 650]=0;
BATT_CHG[ 651]=0;
BATT_CHG[ 652]=0;
BATT_CHG[ 653]=0;
BATT_CHG[ 654]=0;
BATT_CHG[ 655]=0;
BATT_CHG[ 656]=0;
BATT_CHG[ 657]=0;
BATT_CHG[ 658]=0;
BATT_CHG[ 659]=0;
BATT_CHG[ 660]=0;
BATT_CHG[ 661]=0;
BATT_CHG[ 662]=0;
BATT_CHG[ 663]=0;
BATT_CHG[ 664]=0;
BATT_CHG[ 665]=0;
    counter=0;
    for(page = 1; page < 7; page++){
	   pixel = 0;
        PageSelect(page);		//Select page 1-6
        ColumnSelect(10);         //Select column 10
        while(pixel<111)
		{
            dogSPIout(BATT_CHG[counter]);
            pixel++;               //Increase pixel
            counter++;             //Increase counter
		}
    }

    Display_ON();                  //Display on
}

//--------------------------------------------------------------

void Template(void) {
    unsigned char Template[340];    //The image size is 268 byte
    int pixel;
    unsigned char page;
    int counter;

    Display_OFF();            //Display off

//page2
Template[ 0]=128;  //picture data
Template[ 1]=64;
Template[ 2]=128;
Template[ 3]=0;
Template[ 4]=0;
Template[ 5]=192;
Template[ 6]=64;
Template[ 7]=64;
Template[ 8]=0;
Template[ 9]=0;
Template[ 10]=0;
Template[ 11]=0;
Template[ 12]=0;
Template[ 13]=248;
Template[ 14]=0;
Template[ 15]=0;
Template[ 16]=0;
Template[ 17]=0;
Template[ 18]=0;
Template[ 19]=192;
Template[ 20]=64;
Template[ 21]=64;
Template[ 22]=64;
Template[ 23]=64;
Template[ 24]=0;
Template[ 25]=64;
Template[ 26]=129;
Template[ 27]=64;
Template[ 28]=64;
Template[ 29]=64;
Template[ 30]=64;
Template[ 31]=0;
Template[ 32]=192;
Template[ 33]=64;
Template[ 34]=192;
Template[ 35]=64;
Template[ 36]=192;
Template[ 37]=0;
Template[ 38]=128;
Template[ 39]=0;
Template[ 40]=0;
Template[ 41]=0;
Template[ 42]=0;
Template[ 43]=0;
Template[ 44]=0;
Template[ 45]=0;
Template[ 46]=0;
Template[ 47]=0;
Template[ 48]=0;
Template[ 49]=0;
Template[ 50]=0;
Template[ 51]=0;
Template[ 52]=0;
Template[ 53]=0;
Template[ 54]=0;
Template[ 55]=0;
Template[ 56]=0;
Template[ 57]=0;
Template[ 58]=0;
Template[ 59]=0;
Template[ 60]=128;
Template[ 61]=64;
Template[ 62]=64;
Template[ 63]=128;
Template[ 64]=0;
Template[ 65]=0;
Template[ 66]=0;
Template[ 67]=0;

//page3
Template[ 68]=0;
Template[ 69]=1;
Template[ 70]=0;
Template[ 71]=0;
Template[ 72]=0;
Template[ 73]=7;
Template[ 74]=1;
Template[ 75]=0;
Template[ 76]=0;
Template[ 77]=0;
Template[ 78]=0;
Template[ 79]=0;
Template[ 80]=0;
Template[ 81]=255;
Template[ 82]=0;
Template[ 83]=0;
Template[ 84]=0;
Template[ 85]=0;
Template[ 86]=0;
Template[ 87]=5;
Template[ 88]=5;
Template[ 89]=5;
Template[ 90]=5;
Template[ 91]=7;
Template[ 92]=0;
Template[ 93]=4;
Template[ 94]=7;
Template[ 95]=5;
Template[ 96]=5;
Template[ 97]=5;
Template[ 98]=4;
Template[ 99]=0;
Template[ 100]=0;
Template[ 101]=4;
Template[ 102]=7;
Template[ 103]=4;
Template[ 104]=0;
Template[ 105]=0;
Template[ 106]=2;
Template[ 107]=0;
Template[ 108]=0;
Template[ 109]=0;
Template[ 110]=32;
Template[ 111]=80;
Template[ 112]=32;
Template[ 113]=0;
Template[ 114]=0;
Template[ 115]=240;
Template[ 116]=48;
Template[ 117]=16;
Template[ 118]=0;
Template[ 119]=0;
Template[ 120]=0;
Template[ 121]=0;
Template[ 122]=0;
Template[ 123]=0;
Template[ 124]=0;
Template[ 125]=0;
Template[ 126]=0;
Template[ 127]=255;
Template[ 128]=0;
Template[ 129]=240;
Template[ 130]=240;
Template[ 131]=0;
Template[ 132]=255;
Template[ 133]=0;
Template[ 134]=0;
Template[ 135]=0;

//page4
Template[ 136]=0;
Template[ 137]=0;
Template[ 138]=0;
Template[ 139]=0;
Template[ 140]=0;
Template[ 141]=0;
Template[ 142]=0;
Template[ 143]=0;
Template[ 144]=0;
Template[ 145]=0;
Template[ 146]=0;
Template[ 147]=0;
Template[ 148]=0;
Template[ 149]=255;
Template[ 150]=0;
Template[ 151]=0;
Template[ 152]=0;
Template[ 153]=0;
Template[ 154]=0;
Template[ 155]=0;
Template[ 156]=0;
Template[ 157]=0;
Template[ 158]=0;
Template[ 159]=0;
Template[ 160]=0;
Template[ 161]=0;
Template[ 162]=0;
Template[ 163]=0;
Template[ 164]=0;
Template[ 165]=0;
Template[ 166]=0;
Template[ 167]=0;
Template[ 168]=0;
Template[ 169]=0;
Template[ 170]=0;
Template[ 171]=0;
Template[ 172]=0;
Template[ 173]=0;
Template[ 174]=0;
Template[ 175]=0;
Template[ 176]=0;
Template[ 177]=0;
Template[ 178]=0;
Template[ 179]=0;
Template[ 180]=0;
Template[ 181]=0;
Template[ 182]=0;
Template[ 183]=1;
Template[ 184]=0;
Template[ 185]=0;
Template[ 186]=0;
Template[ 187]=0;
Template[ 188]=0;
Template[ 189]=0;
Template[ 190]=0;
Template[ 191]=0;
Template[ 192]=0;
Template[ 193]=0;
Template[ 194]=0;
Template[ 195]=255;
Template[ 196]=0;
Template[ 197]=255;
Template[ 198]=255;
Template[ 199]=0;
Template[ 200]=255;
Template[ 201]=0;
Template[ 202]=0;
Template[ 203]=0;

//page5
Template[ 204]=0;
Template[ 205]=0;
Template[ 206]=0;
Template[ 207]=0;
Template[ 208]=0;
Template[ 209]=0;
Template[ 210]=0;
Template[ 211]=0;
Template[ 212]=0;
Template[ 213]=0;
Template[ 214]=0;
Template[ 215]=0;
Template[ 216]=0;
Template[ 217]=3;
Template[ 218]=0;
Template[ 219]=0;
Template[ 220]=0;
Template[ 221]=0;
Template[ 222]=0;
Template[ 223]=0;
Template[ 224]=0;
Template[ 225]=0;
Template[ 226]=0;
Template[ 227]=0;
Template[ 228]=0;
Template[ 229]=0;
Template[ 230]=0;
Template[ 231]=0;
Template[ 232]=0;
Template[ 233]=0;
Template[ 234]=0;
Template[ 235]=0;
Template[ 236]=0;
Template[ 237]=0;
Template[ 238]=0;
Template[ 239]=0;
Template[ 240]=0;
Template[ 241]=0;
Template[ 242]=0;
Template[ 243]=0;
Template[ 244]=0;
Template[ 245]=0;
Template[ 246]=0;
Template[ 247]=0;
Template[ 248]=0;
Template[ 249]=0;
Template[ 250]=0;
Template[ 251]=0;
Template[ 252]=0;
Template[ 253]=0;
Template[ 254]=0;
Template[ 255]=0;
Template[ 256]=0;
Template[ 257]=0;
Template[ 258]=0;
Template[ 259]=0;
Template[ 260]=0;
Template[ 261]=0;
Template[ 262]=128;
Template[ 263]=127;
Template[ 264]=0;
Template[ 265]=255;
Template[ 266]=255;
Template[ 267]=0;
Template[ 268]=127;
Template[ 269]=128;
Template[ 270]=0;
Template[ 271]=0;

//page6
Template[ 272]=0;
Template[ 273]=0;
Template[ 274]=0;
Template[ 275]=0;
Template[ 276]=0;
Template[ 277]=0;
Template[ 278]=0;
Template[ 279]=0;
Template[ 280]=0;
Template[ 281]=0;
Template[ 282]=0;
Template[ 283]=0;
Template[ 284]=0;
Template[ 285]=0;
Template[ 286]=0;
Template[ 287]=0;
Template[ 288]=0;
Template[ 289]=0;
Template[ 290]=0;
Template[ 291]=0;
Template[ 292]=0;
Template[ 293]=0;
Template[ 294]=0;
Template[ 295]=0;
Template[ 296]=0;
Template[ 297]=0;
Template[ 298]=0;
Template[ 299]=0;
Template[ 300]=0;
Template[ 301]=0;
Template[ 302]=0;
Template[ 303]=0;
Template[ 304]=0;
Template[ 305]=0;
Template[ 306]=0;
Template[ 307]=0;
Template[ 308]=0;
Template[ 309]=0;
Template[ 310]=0;
Template[ 311]=0;
Template[ 312]=0;
Template[ 313]=0;
Template[ 314]=0;
Template[ 315]=0;
Template[ 316]=0;
Template[ 317]=0;
Template[ 318]=0;
Template[ 319]=0;
Template[ 320]=0;
Template[ 321]=0;
Template[ 322]=0;
Template[ 323]=0;
Template[ 324]=0;
Template[ 325]=0;
Template[ 326]=0;
Template[ 327]=0;
Template[ 328]=30;
Template[ 329]=33;
Template[ 330]=76;
Template[ 331]=158;
Template[ 332]=191;
Template[ 333]=191;
Template[ 334]=191;
Template[ 335]=191;
Template[ 336]=158;
Template[ 337]=76;
Template[ 338]=33;
Template[ 339]=30;

    counter=0;
    for(page = 2; page < 7; page++){
	   pixel = 0;
        PageSelect(page);		//Select page 1-6
        ColumnSelect(55);         //Select column 10
        while(pixel<68)
		{
            dogSPIout(Template[counter]);
            pixel++;               //Increase pixel
            counter++;             //Increase counter
		}
    }

    Display_ON();                  //Display on
}

//--------------------------------------------------------------


void BattLogo(void) {
    unsigned char Batt_logo[13];    //The image size is 13 byte
    int pixel;
    unsigned char page;
    int counter;

    Display_OFF();            //Display off

//page6
Batt_logo[0]=27;
Batt_logo[1]=17;
Batt_logo[2]=21;
Batt_logo[3]=21;
Batt_logo[4]=4;
Batt_logo[5]=21;
Batt_logo[6]=21;
Batt_logo[7]=21;
Batt_logo[8]=4;
Batt_logo[9]=21;
Batt_logo[10]=17;
Batt_logo[11]=31;
Batt_logo[12]=14;

    page=1;
	counter = 0;
	pixel = 0;
	PageSelect(page);		//Select page 6
     ColumnSelect(107);         //Select column 107
        while(pixel<13)
		{
            dogSPIout(Batt_logo[counter]);
            pixel++;               //Increase pixel
            counter++;             //Increase counter
		}

    Display_ON();                  //Display on
}

//--------------------------------------------------------------

void BattLow(void) {
    unsigned char Batt_Low[31];    //The image size is 31 byte
    int pixel;
    unsigned char page;
    int counter;

    Display_OFF();            //Display off

//page6
Batt_Low[0]=31;
Batt_Low[1]=16;
Batt_Low[2]=16;
Batt_Low[3]=0;
Batt_Low[4]=31;
Batt_Low[5]=17;
Batt_Low[6]=31;
Batt_Low[7]=0;
Batt_Low[8]=31;
Batt_Low[9]=16;
Batt_Low[10]=31;
Batt_Low[11]=16;
Batt_Low[12]=31;
Batt_Low[13]=0;
Batt_Low[14]=0;
Batt_Low[15]=0;
Batt_Low[16]=31;
Batt_Low[17]=21;
Batt_Low[18]=31;
Batt_Low[19]=0;
Batt_Low[20]=31;
Batt_Low[21]=5;
Batt_Low[22]=31;
Batt_Low[23]=0;
Batt_Low[24]=1;
Batt_Low[25]=31;
Batt_Low[26]=1;
Batt_Low[27]=0;
Batt_Low[28]=1;
Batt_Low[29]=31;
Batt_Low[30] = 1;


     page=1;
	counter = 0;
	pixel = 0;
	PageSelect(page);		//Select page 6
     ColumnSelect(72);         //Select column 72
        while(pixel<31)
		{
            dogSPIout(Batt_Low[counter]);
            pixel++;               //Increase pixel
            counter++;             //Increase counter
		}
    Display_ON();                  //Display on
}

//--------------------------------------------------------------

void ClearBattLow(void) {
    int pixel;
    unsigned char page;

    Display_OFF();            //Display off

     page=1;
	pixel = 0;
	PageSelect(1);		//Select page 6
     ColumnSelect(72);         //Select column 107
        while(pixel<31)
		{
            dogSPIout(0);
            pixel++;               //Increase pixel
		}

    Display_ON();                  //Display on
}

//--------------------------------------------------------------


void MON(void) {
    unsigned char MON[23];    //The image size is 23 byte
    int pixel;
    unsigned char page;
    int counter;

    Display_OFF();            //Display off

//page6
MON[0]=17;
MON[1]=31;
MON[2]=17;
MON[3]=6;
MON[4]=8;
MON[5]=6;
MON[6]=17;
MON[7]=31;
MON[8]=17;
MON[9]=0;
MON[10]=31;
MON[11]=17;
MON[12]=17;
MON[13]=17;
MON[14]=31;
MON[15]=0;
MON[16]=16;
MON[17]=31;
MON[18]=2;
MON[19]=4;
MON[20]=8;
MON[21]=31;
MON[22]=1;

     page=1;
	counter = 0;
	pixel = 0;
	PageSelect(page);		//Select page 6
     ColumnSelect(10);         //Select column 10
        while(pixel<23)
		{
            dogSPIout(MON[counter]);
            pixel++;               //Increase pixel
            counter++;             //Increase counter
		}

    Display_ON();                  //Display on
}

//--------------------------------------------------------------

void TUE(void) {
    unsigned char TUE[20];    //The image size is 20 byte
    int pixel;
    unsigned char page;
    int counter;

    Display_OFF();            //Display off

//page6
TUE[0]=3;
TUE[1]=17;
TUE[2]=31;
TUE[3]=17;
TUE[4]=3;
TUE[5]=0;
TUE[6]=1;
TUE[7]=31;
TUE[8]=16;
TUE[9]=16;
TUE[10]=16;
TUE[11]=31;
TUE[12]=1;
TUE[13]=0;
TUE[14]=17;
TUE[15]=31;
TUE[16]=21;
TUE[17]=21;
TUE[18]=21;
TUE[19]=17;

     page=1;
	counter = 0;
	pixel = 0;
	PageSelect(page);		//Select page 6
     ColumnSelect(10);         //Select column 10
        while(pixel<20)
		{
            dogSPIout(TUE[counter]);
            pixel++;               //Increase pixel
            counter++;             //Increase counter
		}

    Display_ON();                  //Display on
}

//--------------------------------------------------------------


void WED(void) {
    unsigned char WED[25];    //The image size is 25 byte
    int pixel;
    unsigned char page;
    int counter;

    Display_OFF();            //Display off

//page6
WED[0]=1;
WED[1]=3;
WED[2]=12;
WED[3]=16;
WED[4]=13;
WED[5]=3;
WED[6]=13;
WED[7]=16;
WED[8]=12;
WED[9]=3;
WED[10]=1;
WED[11]=0;
WED[12]=17;
WED[13]=31;
WED[14]=21;
WED[15]=21;
WED[16]=21;
WED[17]=17;
WED[18]=0;
WED[19]=17;
WED[20]=31;
WED[21]=17;
WED[22]=17;
WED[23]=17;
WED[24]=14;

     page=1;
	counter = 0;
	pixel = 0;
	PageSelect(page);		//Select page 6
     ColumnSelect(10);         //Select column 10
        while(pixel<25)
		{
            dogSPIout(WED[counter]);
            pixel++;               //Increase pixel
            counter++;             //Increase counter
		}

    Display_ON();                  //Display on
}

//--------------------------------------------------------------


void THUR(void) {
    unsigned char THUR[28];    //The image size is 28 byte
    int pixel;
    unsigned char page;
    int counter;

    Display_OFF();            //Display off

//page6
THUR[0]=3;
THUR[1]=17;
THUR[2]=31;
THUR[3]=17;
THUR[4]=3;
THUR[5]=0;
THUR[6]=17;
THUR[7]=31;
THUR[8]=4;
THUR[9]=4;
THUR[10]=4;
THUR[11]=4;
THUR[12]=31;
THUR[13]=17;
THUR[14]=0;
THUR[15]=1;
THUR[16]=31;
THUR[17]=16;
THUR[18]=16;
THUR[19]=16;
THUR[20]=31;
THUR[21]=1;
THUR[22]=0;
THUR[23]=17;
THUR[24]=31;
THUR[25]=5;
THUR[26]=5;
THUR[27]=23;

     page=1;
	counter = 0;
	pixel = 0;
	PageSelect(page);		//Select page 6
     ColumnSelect(10);         //Select column 10
        while(pixel<28)
		{
            dogSPIout(THUR[counter]);
            pixel++;               //Increase pixel
            counter++;             //Increase counter
		}

    Display_ON();                  //Display on
}

//--------------------------------------------------------------

void FRI(void) {
    unsigned char FRI[17];    //The image size is 17 byte
    int pixel;
    unsigned char page;
    int counter;

    Display_OFF();            //Display off

//page6
FRI[0]=17;
FRI[1]=31;
FRI[2]=21;
FRI[3]=5;
FRI[4]=5;
FRI[5]=1;
FRI[6]=0;
FRI[7]=17;
FRI[8]=31;
FRI[9]=5;
FRI[10]=5;
FRI[11]=13;
FRI[12]=23;
FRI[13]=0;
FRI[14]=17;
FRI[15]=31;
FRI[16]=17;

     page=1;
	counter = 0;
	pixel = 0;
	PageSelect(page);		//Select page 6
     ColumnSelect(10);         //Select column 10
        while(pixel<17)
		{
            dogSPIout(FRI[counter]);
            pixel++;               //Increase pixel
            counter++;             //Increase counter
		}

    Display_ON();                  //Display on
}


//--------------------------------------------------------------

void SAT(void) {
    unsigned char SAT[17];    //The image size is 17 byte
    int pixel;
    unsigned char page;
    int counter;

    Display_OFF();            //Display off

//page6
SAT[0]=9;
SAT[1]=21;
SAT[2]=21;
SAT[3]=21;
SAT[4]=15;
SAT[5]=0;
SAT[6]=31;
SAT[7]=5;
SAT[8]=5;
SAT[9]=5;
SAT[10]=31;
SAT[11]=0;
SAT[12]=3;
SAT[13]=17;
SAT[14]=31;
SAT[15]=17;
SAT[16]=3;

     page=1;
	counter = 0;
	pixel = 0;
	PageSelect(page);		//Select page 6
     ColumnSelect(10);         //Select column 10
        while(pixel<17)
		{
            dogSPIout(SAT[counter]);
            pixel++;               //Increase pixel
            counter++;             //Increase counter
		}

    Display_ON();                  //Display on
}


//--------------------------------------------------------------

void SUN(void) {
    unsigned char SUN[21];    //The image size is 21 byte
    int pixel;
    unsigned char page;
    int counter;

    Display_OFF();            //Display off

//page6
SUN[0]=9;
SUN[1]=21;
SUN[2]=21;
SUN[3]=21;
SUN[4]=15;
SUN[5]=0;
SUN[6]=1;
SUN[7]=31;
SUN[8]=16;
SUN[9]=16;
SUN[10]=16;
SUN[11]=31;
SUN[12]=1;
SUN[13]=0;
SUN[14]=16;
SUN[15]=31;
SUN[16]=2;
SUN[17]=4;
SUN[18]=8;
SUN[19]=31;
SUN[20]=1;

     page=1;
	counter = 0;
	pixel = 0;
	PageSelect(page);		//Select page 6
     ColumnSelect(10);         //Select column 10
        while(pixel<21)
		{
            dogSPIout(SUN[counter]);
            pixel++;               //Increase pixel
            counter++;             //Increase counter
		}

    Display_ON();                  //Display on
}


//--------------------------------------------------------------

void ClearDay(void) {
    int pixel;
    unsigned char page;
    int counter;

    Display_OFF();            //Display off

     page=1;
	counter = 0;
	pixel = 0;
	PageSelect(page);		//Select page 6
     ColumnSelect(10);         //Select column 10
        while(pixel<28)
		{
            dogSPIout(0);
            pixel++;               //Increase pixel
            counter++;             //Increase counter
		}

    Display_ON();                  //Display on
}

//--------------------------------------------------------------

void TimeNums(char zero2nine, char zero2three) {

unsigned char Num[6];
unsigned char column;
unsigned char page;
unsigned char pixel;
unsigned char counter;


	if (zero2nine == 0) {

		Num[0] = 255; //0
		Num[1] = 129;
		Num[2] = 129;
		Num[3] = 129;
		Num[4] = 129;
		Num[5] = 255;
	}

	else if( zero2nine == 1 ){

		Num[0] = 0; //1
		Num[1] = 0;
		Num[2] = 0;
		Num[3] = 130;
		Num[4] = 225;
		Num[5] = 128;

	} else if (zero2nine == 2) {

		Num[0] = 251; //2
		Num[1] = 137;
		Num[2] = 137;
		Num[3] = 137;
		Num[4] = 137;
		Num[5] = 207;

	} else if (zero2nine == 3) {

		Num[0] = 195; //3
		Num[1] = 129;
		Num[2] = 129;
		Num[3] = 137;
		Num[4] = 137;
		Num[5] = 255;

	} else if (zero2nine == 4) {

		Num[0] = 15; //4
		Num[1] = 8;
		Num[2] = 8;
		Num[3] = 8;
		Num[4] = 252;
		Num[5] = 8;

	} else if (zero2nine == 5) {

		Num[0] = 207; //5
		Num[1] = 137;
		Num[2] = 137;
		Num[3] = 137;
		Num[4] = 137;
		Num[5] = 249;

	} else if (zero2nine == 6) {

		Num[0] = 255; //6
		Num[1] = 137;
		Num[2] = 137;
		Num[3] = 137;
		Num[4] = 137;
		Num[5] = 251;

	} else if (zero2nine == 7) {

		Num[0] = 3; //7
		Num[1] = 129;
		Num[2] = 193;
		Num[3] = 177;
		Num[4] = 13;
		Num[5] = 3;

	} else if (zero2nine == 8) {

		Num[0] = 255; //8
		Num[1] = 137;
		Num[2] = 137;
		Num[3] = 137;
		Num[4] = 137;
		Num[5] = 255;

	} else if (zero2nine == 9) {

		Num[0] = 207; //9
		Num[1] = 137;
		Num[2] = 137;
		Num[3] = 137;
		Num[4] = 137;
		Num[5] = 255;
	}

	if (zero2three == 0) {

		column = 35;

	} else if (zero2three == 1) {

		column = 27;

	} else if (zero2three == 2) {

		column = 15;

	}

	else {

		column = 7;
	}

     Display_OFF();            //Display off
     page=6;
	counter = 0;
	pixel = 0;
	PageSelect(page);		//Select page 1
     ColumnSelect(column);         //Select column
        while(pixel<6)
		{
            dogSPIout(Num[counter]);
            pixel++;               //Increase pixel
            counter++;             //Increase counter
		}

    Display_ON();                  //Display on
}


//--------------------------------------------------------------


void AM_PM(char AM_Zero_Or_PM_One) { //AM=0 PM=1

	unsigned char AMPM[9];
	unsigned char page;
	unsigned char pixel;
	unsigned char counter;

	AMPM[0] = 240;
	if (AM_Zero_Or_PM_One == 0) {

		AMPM[2] = 240;

	} else {

		AMPM[2] = 112;

	}
	AMPM[1] = 80;
	AMPM[3] = 0;
	AMPM[4] = 240;
	AMPM[5] = 32;
	AMPM[6] = 64;
	AMPM[7] = 32;
	AMPM[8] = 240;

Display_OFF();            //Display off
     page=6;
	counter = 0;
	pixel = 0;
	PageSelect(page);		//Select page 1
     ColumnSelect(43);         //Select column
        while(pixel<9)
		{
            dogSPIout(AMPM[counter]);
            pixel++;               //Increase pixel
            counter++;             //Increase counter
		}

    Display_ON();                  //Display on
}


//--------------------------------------------------------------


void semicolon(void) { 
	PageSelect(6);		//Select page 1
     ColumnSelect(23);         //Select column 23
	dogSPIout(136);
}

//--------------------------------------------------------------

void ClrTimeNums(char zero2three) {

	unsigned char column;
	unsigned char page;
	unsigned char pixel;
	unsigned char counter;

	if (zero2three == 0) {

		column = 35;

	} else if (zero2three == 1) {

		column = 27;

	} else if (zero2three == 2) {

		column = 15;

	} else {

		column = 7;

	}

     Display_OFF();            //Display off
     page=6;
	counter = 0;
	pixel = 0;
	PageSelect(page);		//Select page 1
     ColumnSelect(column);         //Select column
        while(pixel<6)
		{
            dogSPIout(0);
            pixel++;               //Increase pixel
            counter++;             //Increase counter
		}

    Display_ON();                  //Display on
}

//--------------------------------------------------------------

void SetTempNums(char zero2nine, char zero2one) {

	unsigned char Num[5];
	unsigned char column;
	unsigned char page;
	unsigned char pixel;
	unsigned char counter;

	if (zero2nine == 0) {

		Num[0] = 255; //0
		Num[1] = 255;
		Num[2] = 129;
		Num[3] = 255;
		Num[4] = 255;
	}

	else if (zero2nine == 1) {

		Num[0] = 0; //1
		Num[1] = 0;
		Num[2] = 1;
		Num[3] = 255;
		Num[4] = 255;

	} else if (zero2nine == 2) {

		Num[0] = 251; //2
		Num[1] = 251;
		Num[2] = 137;
		Num[3] = 207;
		Num[4] = 207;

	} else if (zero2nine == 3) {

		Num[0] = 195; //3
		Num[1] = 195;
		Num[2] = 137;
		Num[3] = 255;
		Num[4] = 255;

	} else if (zero2nine == 4) {

		Num[0] = 63; //4
		Num[1] = 63;
		Num[2] = 32;
		Num[3] = 240;
		Num[4] = 240;

	} else if (zero2nine == 5) {

		Num[0] = 207; //5
		Num[1] = 143;
		Num[2] = 137;
		Num[3] = 249;
		Num[4] = 249;

	} else if (zero2nine == 6) {

		Num[0] = 255; //6
		Num[1] = 255;
		Num[2] = 137;
		Num[3] = 249;
		Num[4] = 251;

	} else if (zero2nine == 7) {

		Num[0] = 3; //7
		Num[1] = 1;
		Num[2] = 1;
		Num[3] = 255;
		Num[4] = 255;

	} else if (zero2nine == 8) {

		Num[0] = 255; //8
		Num[1] = 255;
		Num[2] = 137;
		Num[3] = 255;
		Num[4] = 255;

	} else if (zero2nine == 9) {

		Num[0] = 207; //9
		Num[1] = 143;
		Num[2] = 137;
		Num[3] = 255;
		Num[4] = 255;
	}


	if (zero2one == 0) {

		column = 90;

	} else if (zero2one == 1) {

		column = 84;

	}

	Display_OFF(); //Display off
	page = 4;
	counter = 0;
	pixel = 0;
	PageSelect(page); //Select page 3

	ColumnSelect(column);         //Select column
        while(pixel<5)
		{
            dogSPIout(Num[counter]);
            pixel++;               //Increase pixel
            counter++;             //Increase counter
		}

    Display_ON();                  //Display on
}


//--------------------------------------------------------------

void ClrSetTempNums(char zero2one) {

unsigned char column;
unsigned char page;
unsigned char pixel;
unsigned char counter;

	if (zero2one == 0) {

		column = 90;

	}else if(zero2one == 1) {

		column = 84;
	}

	Display_OFF(); 			//Display off
	page = 4;
	counter = 0;
	pixel = 0;
	PageSelect(page); 		//Select page 1
	ColumnSelect(column); 	//Select column

	while (pixel < 5) {
		dogSPIout(0);
		pixel++; 			//Increase pixel
		counter++; 			//Increase counter
	}

	Display_ON(); 			//Display on
}

//--------------------------------------------------------------

void CurTempNums(char zero2nine, char zero2one) {

unsigned char Num[16];
unsigned char column;
unsigned char page;
unsigned char pixel;
unsigned char counter;


	if (zero2nine == 0) {

		Num[0] = 255; //0
		Num[1] = 255;
		Num[2] = 255;
		Num[3] = 192;
		Num[4] = 192;
		Num[5] = 255;
		Num[6] = 255;
		Num[7] = 255;
		Num[8] = 254;
		Num[9] = 254;
		Num[10] = 254;
		Num[11] = 192;
		Num[12] = 192;
		Num[13] = 254;
		Num[14] = 254;
		Num[15] = 254;

	} else if (zero2nine == 1) {

		Num[0] = 0; //1
		Num[1] = 0;
		Num[2] = 0;
		Num[3] = 192;
		Num[4] = 255;
		Num[5] = 255;
		Num[6] = 255;
		Num[7] = 192;
		Num[8] = 0;
		Num[9] = 0;
		Num[10] = 0;
		Num[11] = 6;
		Num[12] = 254;
		Num[13] = 254;
		Num[14] = 254;
		Num[15] = 0;

	}else if(zero2nine == 2) {

		Num[0]=255; //2
		Num[1]=255;
		Num[2]=255;
		Num[3]=192;
		Num[4]=192;
		Num[5]=224;
		Num[6]=224;
		Num[7]=224;
		Num[8]=206;
		Num[9]=206;
		Num[10]=206;
		Num[11]=198;
		Num[12]=198;
		Num[13]=254;
		Num[14]=254;
		Num[15]=254;

	}else if(zero2nine == 3) {

		Num[0]=224; //3
		Num[1]=224;
		Num[2]=192;
		Num[3]=192;
		Num[4]=192;
		Num[5]=255;
		Num[6]=255;
		Num[7]=255;
		Num[8]=14;
		Num[9]=14;
		Num[10]=6;
		Num[11]=198;
		Num[12]=198;
		Num[13]=254;
		Num[14]=254;
		Num[15]=254;

	}else if(zero2nine == 4) {

		Num[0]=7; //4
		Num[1]=7;
		Num[2]=7;
		Num[3]=6;
		Num[4]=6;
		Num[5]=255;
		Num[6]=255;
		Num[7]=255;
		Num[8]=254;
		Num[9]=254;
		Num[10]=254;
		Num[11]=0;
		Num[12]=0;
		Num[13]=0;
		Num[14]=0;
		Num[15]=0;

	}else if(zero2nine == 5) {

		Num[0]=224; //5
		Num[1]=224;
		Num[2]=192;
		Num[3]=192;
		Num[4]=192;
		Num[5]=255;
		Num[6]=255;
		Num[7]=255;
		Num[8]=254;
		Num[9]=254;
		Num[10]=254;
		Num[11]=198;
		Num[12]=198;
		Num[13]=198;
		Num[14]=198;
		Num[15]=198;

	}else if(zero2nine == 6) {

		Num[0]=255; //6
		Num[1]=255;
		Num[2]=255;
		Num[3]=192;
		Num[4]=192;
		Num[5]=255;
		Num[6]=255;
		Num[7]=255;
		Num[8]=254;
		Num[9]=254;
		Num[10]=254;
		Num[11]=198;
		Num[12]=198;
		Num[13]=198;
		Num[14]=206;
		Num[15]=206;

	}else if(zero2nine == 7) {

		Num[0]=0; //7
		Num[1]=0;
		Num[2]=0;
		Num[3]=0;
		Num[4]=0;
		Num[5]=255;
		Num[6]=255;
		Num[7]=255;
		Num[8]=14;
		Num[9]=14;
		Num[10]=6;
		Num[11]=6;
		Num[12]=6;
		Num[13]=254;
		Num[14]=254;
		Num[15]=254;

	}else if(zero2nine == 8) {

		Num[0]=255; //8
		Num[1]=255;
		Num[2]=255;
		Num[3]=192;
		Num[4]=192;
		Num[5]=255;
		Num[6]=255;
		Num[7]=255;
		Num[8]=254;
		Num[9]=254;
		Num[10]=254;
		Num[11]=198;
		Num[12]=198;
		Num[13]=254;
		Num[14]=254;
		Num[15]=254;

	}else if(zero2nine == 9) {

		Num[0]=224; //9
		Num[1]=224;
		Num[2]=192;
		Num[3]=192;
		Num[4]=192;
		Num[5]=255;
		Num[6]=255;
		Num[7]=255;
		Num[8]=254;
		Num[9]=254;
		Num[10]=254;
		Num[11]=198;
		Num[12]=198;
		Num[13]=254;
		Num[14]=254;
		Num[15]=254;

	}


	if (zero2one == 0) {

		column = 44;

	}else if(zero2one == 1) {

		column = 35;
	}

     Display_OFF();            //Display off
	counter = 0;

     for(page=4; page > 2; page--){
	pixel = 0;
	PageSelect(page);		//Select page 2-3
     ColumnSelect(column);         //Select column
        while(pixel<8)
		{
            dogSPIout(Num[counter]);
            pixel++;               //Increase pixel
            counter++;             //Increase counter
		}
    }

    Display_ON();                  //Display on
}


//--------------------------------------------------------------

void ClrCurTempNums(char zero2one) {

	unsigned char column;
	unsigned char page;
	unsigned char pixel;
	unsigned char counter;

	if (zero2one == 0) {

		column = 44;

	}else if(zero2one == 1) {

		column = 35;

	}

     Display_OFF();            //Display off
	counter = 0;

	for(page=4; page > 2; page--){
	pixel = 0;
	PageSelect(page);		//Select page 3-4
     ColumnSelect(column);         //Select column
        while(pixel<8)
		{
            dogSPIout(0);
            pixel++;               //Increase pixel
            counter++;             //Increase counter
		}
	}
    Display_ON();                  //Display on
}

//--------------------------------------------------------------




int main(void){

    gpio_enable_gpio_pin(AVR32_PIN_PA10);         	//I/O MC direction:0=in,1=out
    gpio_enable_gpio_pin(AVR32_PIN_PA19);
    gpio_enable_gpio_pin(AVR32_PIN_PA14);
    gpio_enable_gpio_pin(AVR32_PIN_PA13);
    gpio_enable_gpio_pin(AVR32_PIN_PA12);

    ResetDOG();                 	//Hardwarereset

    initDOGM128();                     	//Display initialisation

    //Display_OFF();            //Display off

    //ClearDisplay();                     //Display clear
    //ClearDisplay1();

    //ISU_Logo();                    //Image output


    //while(1){
    //ISU_Logo();                    //Image output
    //BATT_Charging();
    //}

   // Template();
    //BattLogo();
    WED();
    //ClrTimeNums(0);
    TimeNums(9,2);
    TimeNums(6,1);
    TimeNums(3,0);
    AM_PM(0);
    //semicolon();
    SetTempNums(7,1);
    SetTempNums(5,0);
    CurTempNums(5,1);
    CurTempNums(9,0);
    //ClearDay();
    //ClearBattLow();
    return 0;
}
