/**
 * 
 * @brief Implement some properties Icom IC7000 for use with
 * 
 * @author DG3QQ
 *
 * The goal of this is to provide some properties of Icom IC7000 radio 
 * for use with CI-V protocol (from the radio CAT point of view).
 * 
 * This work was a need from my side for testing device drivers of CHIRP
 * project.
 *
 * This is a experimantal/preliminary !!!
 * 
 * There meight be issues and deficiencies; not bomb proof yet.  
 * 
 * This code has been built with the review of various sources:
 * - Icom IC7000 manual 
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * **************************************************************************/

#pragma once

#define ICOM_DEFAULT_CIV_ADDRESS_IC7000 0x70


/*

# http://www.vk4adc.com/
#     web/index.php/reference-information/49-general-ref-info/182-civ7400
MEM_IC7000_FORMAT = """
u8   bank;
bbcd number[2];
u8   spl:4,
     skip:4;
lbcd freq[5];
u8   mode;
u8   filter;
u8   duplex:4,
     tmode:4;
bbcd rtone[3];
bbcd ctone[3];
u8   dtcs_polarity;
bbcd dtcs[2];
lbcd freq_tx[5];
u8   mode_tx;
u8   filter_tx;
u8   duplex_tx:4,
     tmode_tx:4;
bbcd rtone_tx[3];
bbcd ctone_tx[3];
u8   dtcs_polarity_tx;
bbcd dtcs_tx[2];
char name[9];
"""

  FE FE E0 E0 FA FD                                 ......          
  FE FE E0 E0 FA FD                                 ......          

  FE FE 70 E0 1A 00 01 00  01 FD                    ..p.......      
  FE FE 70 E0 1A 00 01 00  01 FD                    ..p.......      

  FE FE start bytes 
  E0 dst - control (0xE0)
  70 src - radio (0x70)  
  1A cmd - resd/write memory
  00 sub - extended
  01 bnk - bank A
  00 01  - mn memmory 0001 
  01 sel
  00 00 60 44 01 frqu1 - 1446000000 
  05 mo1 - mode                                               .               
  01 fi1
  11 fg1 TxSub, DUP+,  
  00 08 85 s_tx1 
  00 08 85 S_rx1                                                .               
  00 00 23 dcs_1
  00 00 00 44 01 freq2 1440000000 
  05 mo2
  01 fi2
  00 fg2
  00 08 85 s_tx2
  00 08 85 s_rx2
  00 00 23 dcs_2
  46 54 20 20 20 20 20 20 20 name "FT_______" 
  FD end byte                                               .               


*/
// https://www.vk4adc.com/web/index.php/reference-information/49-general-ref-info/51-civ7400
// $1A $00 Read/Write Extended Memory Command
typedef struct /*__attribute__ ((packed)) */ {
    //byte bnk[1]; // Bnk Bank number , A = 1, B = 2, C = 3, D = 4, E=5
    //byte mn[2]; // mn1, mn2 Memory number in BCD. (2 bytes), 00-99 plus 0100 - 0102
    byte sel; //sel Selected for scans, typically $00
        // byte spl: 4;
        // byte skip: 4;
    byte frqu1[5]; // f15-1 Frequency 1, RX frequency when dup or split (5 bytes)
    byte mo1; // mo1 Mode for frequency 1 (1 byte)
    byte fi1; // fi1 Filter for frequency 1 (1 byte)
    byte fg1; // fg1 Flags for freq.1: $01=Tx Subtone on, $02=Rx Subtone on, $10 DUP-, $20 DUP+
        // byte duplex:4,
        // byte tmode:4;
    byte s_tx1[3]; // STx1 TX-Subtone for frequency 1 (3 bytes)
    byte s_rx1[3]; // SRx1 RX-Subtone for frequency 1 (3 bytes)
    byte dcs_1[3]; //  DCS1DTCS code #1 (3 bytes)
    byte frqu2[5]; // f25-1 Frequency 2, TX frequency when dup or split (5 bytes)
    byte mo2; // 2Mode for frequency 2 (1 byte)
    byte fi2; // fi2 Filter for frequency 2 (1 byte)
    byte fg2; // fg2 Flags for freq. 2: $01=Tx Subtone on, $02=Rx Subtone on, $10 DUP-, $20 DUP+
        // byte  duplex_tx:4,
        // byte tmode_tx:4;
    byte s_tx2[3]; // STx2 TX-Subtone for frequency 2 (3 bytes)
    byte s_rx2[3]; // SRx2 RX-Subtone for frequency 2 (3 bytes)
    byte dcs_2[3]; // DCS2DTCS code #2 (3 bytes)
    char name[9];
} mem_record_t;

#define MODE_LSB 0
#define MODE_USB 1
#define MODE_AM 2
#define MODE_CW 3
#define MODE_RTTY 4
#define MODE_FM 5
#define MODE_CW-R 7
#define MODE_RTTY-R 8 


// $1A $00 Read Extended Memory Command IC-7000
struct mem_cmd_t {
    byte bnk[1]; // Bnk Bank number , A = 1, B = 2, C = 3, D = 4, E=5
    byte mn[2]; // mn1, mn2  Memory number in BCD. (2 bytes)  00-99 for normal, 0100 – 0108 for scan edges and VHF & UHF call channels
};    
// 
    
    
